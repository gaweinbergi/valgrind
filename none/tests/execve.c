#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(VGO_freebsd) || defined(VGO_darwin)
#define TRUEPATH "/usr/bin/true"
#else
#define TRUEPATH "/bin/true"
#endif

int main(int argc, char **argv)
{
   if (argc == 1)
   {
      // This tests the case where argv and envp are NULL, which is easy to
      // get wrong because it's an unusual case.
      if (execve(TRUEPATH, NULL, NULL) < 0)
      {
         perror("execve");
         exit(1);
      }
   }
   
   exit(0);
}
