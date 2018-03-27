/* Expect 5 errors total (4 re cvs, 1 re exiting w/lock.).
   Tests passing bogus mutexes to pthread_cond_wait. */
#define _GNU_SOURCE 1 /* needed by glibc <= 2.3 for pthread_rwlock_* */
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pthread_mutex_t mx[4]; 
pthread_cond_t cv; pthread_rwlock_t rwl;
sem_t* quit_now;
static sem_t* my_sem_init(char*, int, unsigned);
static int my_sem_destroy(sem_t*);
static int my_sem_wait(sem_t*); static int my_sem_post(sem_t*);
void* rescue_me ( void* uu )
{
  /* wait for, and unblock, the first wait */
  sleep(1);
  assert(pthread_cond_signal(&cv) == 0);

  /* wait for, and unblock, the second wait */
  sleep(1);
  assert(pthread_cond_signal(&cv) == 0);

  /* wait for, and unblock, the third wait */
  sleep(1);
  assert(pthread_cond_signal(&cv) == 0);

  /* wait for, and unblock, the fourth wait */
  sleep(1);
  assert(pthread_cond_signal(&cv) == 0);

  my_sem_wait( quit_now );
  return NULL;
}

void* grab_the_lock ( void* uu )
{
   int r= pthread_mutex_lock( &mx[2] ); assert(!r);
   my_sem_wait( quit_now );
   r= pthread_mutex_unlock( &mx[2] ); assert(!r);
   return NULL;
}

int main ( void )
{
  int r;
  pthread_t my_rescuer, grabber;

  r= pthread_mutex_init(&mx[0], NULL); assert(!r);
  r= pthread_mutex_init(&mx[1], NULL); assert(!r);
  r= pthread_mutex_init(&mx[2], NULL); assert(!r);
  r= pthread_mutex_init(&mx[3], NULL); assert(!r);

  r= pthread_cond_init(&cv, NULL); assert(!r);
  r= pthread_rwlock_init(&rwl, NULL); assert(!r);

  quit_now = my_sem_init( "quit_now", 0, 0 ); assert(quit_now);

  r= pthread_create( &grabber, NULL, grab_the_lock, NULL ); assert(!r);
  sleep(1); /* let the grabber get there first */

  r= pthread_create( &my_rescuer, NULL, rescue_me, NULL );  assert(!r);

  /*
   * OSes are interesting things. On Linux, the pthread_cond_*() calls always
   * succeed, so strange things can happen with bogus parameters and we try to
   * catch them in DRD and recover here with rescue_me(). On FreeBSD, passing
   * a totally bogus mutex can cause the system to hang, but providing a
   * destroyed mutex will return EINVAL, and all the other conditions we test
   * for here will return EPERM, so for FreeBSD rescue_me() is essentially a
   * no-op. Other OSes behave somewhere in between.
   */

  /* mx is bogus */
  /* FreeBSD mutexes are dynamically allocated. Passing a bogus address */
  /* can result in hangs, so mark mutex as destroyed instead. */
#if defined(VGO_freebsd)
#define THR_MUTEX_DESTROYED ((pthread_mutex_t)2)
  pthread_mutex_t destroyed_mx = THR_MUTEX_DESTROYED;
  r= pthread_cond_wait(&cv, &destroyed_mx);
#else
  r= pthread_cond_wait(&cv, (pthread_mutex_t*)(4 + (char*)&mx[0]) );
#endif
  if (r) fprintf(stderr, "pthread_cond_wait(1): %s\n", strerror(r));

  /* mx is not locked */
  r= pthread_cond_wait(&cv, &mx[0]);
  if (r) fprintf(stderr, "pthread_cond_wait(2): %s\n", strerror(r));

  /* wrong flavour of lock */
  r= pthread_cond_wait(&cv, (pthread_mutex_t*)&rwl );
  if (r) fprintf(stderr, "pthread_cond_wait(3): %s\n", strerror(r));

  /* mx is held by someone else. */
  r= pthread_cond_wait(&cv, &mx[2] );
  if (r) fprintf(stderr, "pthread_cond_wait(4): %s\n", strerror(r));

  r= my_sem_post( quit_now ); assert(!r);
  r= my_sem_post( quit_now ); assert(!r);

  r= pthread_join( my_rescuer, NULL ); assert(!r);
  r= pthread_join( grabber, NULL ); assert(!r);

  r= my_sem_destroy( quit_now ); assert(!r);
  return 0;
}








static sem_t* my_sem_init (char* identity, int pshared, unsigned count)
{
   sem_t* s;

#if defined(VGO_linux) || defined(VGO_freebsd) || defined(VGO_solaris)
   s = malloc(sizeof(*s));
   if (s) {
      if (sem_init(s, pshared, count) < 0) {
	 perror("sem_init");
	 free(s);
	 s = NULL;
      }
   }
#elif defined(VGO_darwin)
   char name[100];
   sprintf(name, "anonsem_%s_pid%d", identity, (int)getpid());
   name[ sizeof(name)-1 ] = 0;
   if (0) printf("name = %s\n", name);
   s = sem_open(name, O_CREAT | O_EXCL, 0600, count);
   if (s == SEM_FAILED) {
      perror("sem_open");
      s = NULL;
   }
#else
#  error "Unsupported OS"
#endif

   return s;
}

static int my_sem_destroy ( sem_t* s )
{
   return sem_destroy(s);
}

static int my_sem_wait(sem_t* s)
{
  return sem_wait(s);
}

static int my_sem_post(sem_t* s)
{
  return sem_post(s);
}
