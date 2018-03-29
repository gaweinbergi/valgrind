/*
 * Invoke pthread_cond_destroy() on a condition variable that is being waited
 * upon.
 * NOTE: POSIX says behavior in this case is undefined. Linux returns an
 *       EBUSY error, but FreeBSD, for example, always destroys the cond and
 *       returns success. Our goal is to test Valgrind, not the OS, so we try
 *       to deal with either system behavior.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>     // printf()
#include <pthread.h>
#include "helgrind/helgrind.h"

static pthread_mutex_t s_mutex;
static pthread_cond_t  s_cond;
static int             s_i;

static const char* err_to_str(int errnum)
{
  switch (errnum) {
  case 0:      return "success";
  case EBUSY:  return "EBUSY";
  case EINVAL: return "EINVAL";
  default:     return "?";
  }
}

static void* thread_func(void* thread_arg)
{
  pthread_mutex_lock(&s_mutex);
  s_i = 1;
  pthread_cond_signal(&s_cond);
  while (s_i == 1)
    pthread_cond_wait(&s_cond, &s_mutex);
  pthread_mutex_unlock(&s_mutex);

  return 0;
}

int main(int argc, char** argv)
{
  pthread_t threadid;
  int ret;

#ifdef VGO_freebsd
  // On FreeBSD, locks are dynamically allocated. Races between the main
  // and worker threads in situations where POSIX specifies undefined
  // behavior are not what we are testing, so ignore them.
  VALGRIND_HG_DISABLE_CHECKING(&s_mutex, sizeof(pthread_mutex_t));
  VALGRIND_HG_DISABLE_CHECKING(&s_cond, sizeof(pthread_cond_t));
#endif
  pthread_mutex_init(&s_mutex, 0);
  pthread_cond_init(&s_cond, 0);

  pthread_create(&threadid, 0, thread_func, 0);

  pthread_mutex_lock(&s_mutex);
  while (s_i == 0)
    pthread_cond_wait(&s_cond, &s_mutex);
  pthread_mutex_unlock(&s_mutex);

  ret = pthread_cond_destroy(&s_cond);
  fprintf(stderr, "First pthread_cond_destroy() call returned %s.\n",
          err_to_str(ret));

  pthread_mutex_lock(&s_mutex);
  s_i = 2;
  pthread_cond_signal(&s_cond);
  pthread_mutex_unlock(&s_mutex);

  // If the pthread_cond_destroy() succeeded, the thread will never exit
  // pthread_cond_wait(), so don't try to join.
  if (ret != 0)
    pthread_join(threadid, 0);

  ret = pthread_cond_destroy(&s_cond);
  fprintf(stderr, "Second pthread_cond_destroy() call returned %s.\n",
          err_to_str(ret));
  pthread_mutex_destroy(&s_mutex);

  return 0;
}
