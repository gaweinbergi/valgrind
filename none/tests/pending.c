/* 
   Test pending signals

   1. Signals should remain pending while blocked, and not delivered early
   
   2. When unblocking the signal, the signal should have been delivered
      by the time sigprocmask syscall is complete.
 */
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "config.h"

#define STATE_INIT 0
#define STATE_EARLY 1
#define STATE_GOTSIG 4

static volatile int gotsig = STATE_INIT;
static volatile int early = 1;
static volatile int savedsig;

static void handler(int sig)
{
	savedsig = sig;
	gotsig = early ? STATE_EARLY : STATE_GOTSIG;
}

int main()
{
	sigset_t all;
	sigset_t sigusr1;
	
	sigfillset(&all);
	sigemptyset(&sigusr1);
	sigaddset(&sigusr1, SIGUSR1);

	sigprocmask(SIG_BLOCK, &all, NULL);

	signal(SIGUSR1, handler);
	signal(SIGHUP,  handler);

	printf("1: sending signal\n");
	kill(getpid(), SIGUSR1);
	kill(getpid(), SIGHUP);

	printf("2: sleeping\n");
	sleep(1);

	if (gotsig) {
		fprintf(stderr, "FAILED: signal delivered too early\n");
		return 1;
	}

	printf("3: unblocking\n");
	early = 0;	
	sigprocmask(SIG_UNBLOCK, &sigusr1, NULL);

	if (gotsig == STATE_EARLY) {
		fprintf(stderr, "FAILED: signal delivered early (in handler)\n");
		return 1;
	};
	if (gotsig != STATE_GOTSIG) {
		fprintf(stderr, "FAILED: Invalid state %d\n", gotsig);
		return 1;
	}
	printf("4: got signal %s\n",
		( savedsig == SIGUSR1 ? "SIGUSR1" : "unexpected signal" ));
	if (savedsig != SIGUSR1) {
		fprintf(stderr, "FAILED: got signal %d instead\n", savedsig);
		return 1;
	}

	printf("5: unblocked...\n");
	if (!gotsig) {
		fprintf(stderr, "FAILED: signal not delivered\n");
		return 1;
	}

	printf("6: checking SIGHUP still pending...\n");
#	if HAVE_SIGWAITINFO
	{
		siginfo_t info;
		if (sigwaitinfo(&all, &info) == -1) {
			perror("FAILED: sigwaitinfo failed");
			return 1;
		}
		if (info.si_signo != SIGHUP) {
			fprintf(stderr, "FAILED: SIGHUP not still pending; got signal %d\n", 
				info.si_signo);
			return 1;
		}
	}
#	endif

	printf("OK\n");
	return 0;
}
