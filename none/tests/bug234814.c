/* Refer https://bugs.kde.org/show_bug.cgi?id=234814
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

const char kSigbus[] = "I caught the SIGBUS signal!\n";

int GLOB = 3;

void mysigbus() {
   GLOB--;
   return;
}

int main() {
   struct sigaction sa;
   sa.sa_handler = mysigbus;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   if (sigaction(SIGBUS, &sa, NULL) == -1) {
      perror("ERROR:");
   }
   while(GLOB) {
      int oldglob = GLOB;
      kill(getpid(), SIGBUS);
      if (GLOB == oldglob - 1)
         write(1, kSigbus, sizeof(kSigbus)-1);
   };
   return 0;
}

