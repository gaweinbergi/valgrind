#include <pthread.h>
#include <assert.h>

int main(int argc, char *argv[])
{
   pthread_cond_t c;
   int r;
   r = pthread_cond_init(& c, NULL); assert(r == 0);
   r = pthread_cond_destroy(& c); assert(r == 0);
   return 0;
} 
