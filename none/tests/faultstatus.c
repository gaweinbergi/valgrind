/* 
   Check that a fault signal handler gets the expected info
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <setjmp.h>
#include "tests/sys_mman.h"
#include <unistd.h>

/* Division by zero triggers a SIGFPE on x86 and x86_64,
   but not on the PowerPC architecture.

   On ARM-Linux, we do get a SIGFPE, but not from the faulting of a
   division instruction (there isn't any such thing) but rather
   because the process exits via tgkill, sending itself a SIGFPE.
   Hence we get a SIGFPE but the SI_CODE is different from that on
   x86/amd64-linux.
 */
#if defined(__powerpc__) || defined(__aarch64__)
#  define DIVISION_BY_ZERO_TRIGGERS_FPE 0
#  define DIVISION_BY_ZERO_SI_CODE      SI_TKILL
#elif defined(__arm__)
#  define DIVISION_BY_ZERO_TRIGGERS_FPE 1
#  define DIVISION_BY_ZERO_SI_CODE      SI_TKILL
#else
#  define DIVISION_BY_ZERO_TRIGGERS_FPE 1
#  define DIVISION_BY_ZERO_SI_CODE      FPE_INTDIV
#endif

/* Accessing non-mapped virtual address results in SIGBUS
 * with si_code equal to BUS_ADRERR on Linux, whereas in SIGBUS
 * with si_code equal to BUS_OBJERR on Solaris. On Solaris,
 * BUS_ADRERR is used for bus time out while BUS_OBJERR is translated
 * from underlying codes FC_OBJERR (x86) or ASYNC_BERR (sparc).
 */
#if defined(VGO_solaris)
#  define BUS_ERROR_SI_CODE  BUS_OBJERR
#else
#  define BUS_ERROR_SI_CODE  BUS_ADRERR
#endif

struct test {
	void (*test)(void);
	int sig_act;
	int sig_want;
	int code_act;
	int code_want;
	void *addr_act;
	void volatile *addr_want;
};

static struct test *cur_test;

static int zero();

static sigjmp_buf escape;

#define BADADDR	((int *)0x1234)

#define FILESIZE	(4*__pagesize)
#define MAPSIZE		(2*FILESIZE)
static unsigned int __pagesize;
static char volatile *volatile mapping;

static void handler(int sig, siginfo_t *si, void *uc)
{
	cur_test->sig_act = sig;
	cur_test->code_act = si->si_code;
	cur_test->addr_act = si->si_addr;

	siglongjmp(escape, 1);
}


static void test1(void)
{
	*BADADDR = 'x';
}

static void test2()
{
	mapping[0] = 'x';
}

static void test3()
{
	mapping[FILESIZE+10];
}

static void test4()
{
	volatile int v = 44/zero();

	(void)v;
#if DIVISION_BY_ZERO_TRIGGERS_FPE == 0
	raise(SIGFPE);
#endif
}

int main()
{
	int fd, i, ok;
	static const int sigs[] = { SIGSEGV, SIGILL, SIGBUS, SIGFPE, SIGTRAP };
	struct sigaction sa;
	__pagesize = (unsigned int)sysconf(_SC_PAGE_SIZE);
	sa.sa_sigaction = handler;
	sa.sa_flags = SA_SIGINFO;
	sigfillset(&sa.sa_mask);
	
	for(i = 0; i < sizeof(sigs)/sizeof(*sigs); i++)
		sigaction(sigs[i], &sa, NULL);

	/* we need O_RDWR for the truncate below */
	fd = open("faultstatus.tmp", O_CREAT|O_TRUNC|O_EXCL|O_RDWR, 0600);
	if (fd == -1) {
		perror("tmpfile");
		exit(1);
	}
	unlink("faultstatus.tmp");
	ftruncate(fd, FILESIZE);

	mapping = mmap(0, MAPSIZE, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);

	{
		struct test tests[] = {
#define T(n, sig, code, addr) { test##n, 0, sig, 0, code, NULL, addr }
			T(1, SIGSEGV, SEGV_MAPERR,	BADADDR),
#if defined(VGO_freebsd)
// FIXME: Workaround until we can get Valgrind to properly load
//        the PT_NOTE section of the client executable.
			T(2, SIGBUS, BUS_PAGE_FAULT,	mapping),
			T(3, SIGBUS, BUS_PAGE_FAULT,	&mapping[FILESIZE+10]),
#else
			T(2, SIGSEGV, SEGV_ACCERR,	mapping),
			T(3, SIGBUS,  BUS_ERROR_SI_CODE, &mapping[FILESIZE+10]),
#endif
			T(4, SIGFPE,  DIVISION_BY_ZERO_SI_CODE, 0),
#undef T
		};

		for(i = 0; i < sizeof(tests)/sizeof(*tests); i++) {
			cur_test = &tests[i];
		
			if ((ok = sigsetjmp(escape, 1)) == 0) {
				fprintf(stderr, "Test %d: ", i+1);
				tests[i].test();
				fprintf(stderr, "  FAIL: no fault, or handler returned\n");
			}
			if (cur_test->sig_act != cur_test->sig_want)
				fprintf(stderr,
					"  FAIL: expected signal %d, not %d\n",
					cur_test->sig_want, cur_test->sig_act);
			else if (cur_test->code_act != cur_test->code_want)
				fprintf(stderr,
					"  FAIL: expected si_code==%d, not %d\n",
					cur_test->code_want, cur_test->code_act);
			else if (cur_test->addr_want &&
				/* Some architectures (e.g. s390) just provide
				 * enough information to resolve the page fault,
				 * but do not provide the offset within a page
				 */
#if defined(__s390__)
			       (cur_test->addr_act !=
				(void *)((unsigned long)cur_test->addr_want & ~0xffful)))
#else
			       (cur_test->addr_act != cur_test->addr_want))
#endif
				fprintf(stderr,
					"  FAIL: expected si_addr==%p, not %p\n",
					cur_test->addr_want, cur_test->addr_act);
			else
				fprintf(stderr, "  PASS\n");
		}
	}

	return 0;
}

static int zero()
{
	return 0;
}
