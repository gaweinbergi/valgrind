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
#if defined(VGO_solaris)
      // Solaris requires non-NULL argv parameter
      // XXX So does FreeBSD - so should this test succeed or fail?
      //     In the FreeBSD case we let it fail.
      char *const argv_exe[] = {"true", NULL};
      if (execve(TRUEPATH, argv_exe, NULL) < 0)
#else
      if (execve(TRUEPATH, NULL, NULL) < 0)
#endif
      {
         perror("execve");
         exit(1);
      }
   }
   
   exit(0);
}
