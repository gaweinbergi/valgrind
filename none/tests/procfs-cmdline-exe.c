/*
 * Read /proc/self/cmdline and /proc/self/exe such that it can be tested
 * whether Valgrind intercepts the system calls that access these pseudo-files
 * properly on Linux and whether Valgrind does not modify the behavior of
 * accessing these files on other operating systems.
 */

#define _ATFILE_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "../../config.h"

#define CMDLINE_PID "/proc/%ld/cmdline"
#ifdef VGO_freebsd
#define CMDLINE_SELF "/proc/curproc/cmdline"
#define EXEFILE_LABEL "/proc/<pid>/file"
#define EXEFILE_PID "/proc/%ld/file"
#define EXEFILE_SELF "/proc/curproc/file"
#else
#define CMDLINE_SELF "/proc/self/cmdline"
#define EXEFILE_LABEL "/proc/<pid>/exe"
#define EXEFILE_PID "/proc/%d/exe"
#define EXEFILE_SELF "/proc/self/exe"
#endif

static void test_cmdline(const char* const cwd, const char* const label,
                         const char* const path)
{
  int fd, n;
  char ch;
      
  fprintf(stderr, "%s:\n", label);
  fd = open(path, 0); 
  if (fd >= 0)
  {
    while ((n = read(fd, &ch, 1)) > 0)
    {
      if (ch == '\\')
        fprintf(stderr, "\\\\");
      else if (ch == 0)
        fprintf(stderr, "\\0");
      else if (isprint((unsigned)ch))
        fprintf(stderr, "%c", ch);
      else
        fprintf(stderr, "\\0%o", ch);
    }
    fprintf(stderr, "\n");
    close(fd);
  }
  else
    perror("open()");
}

static void test_readlink(const char* const cwd, const char* const label,
                          const char* const path)
{
  char buf[512];
  const char* p;
  int n;

  if ((n = readlink(path, buf, sizeof(buf) - 1)) >= 0)
  {
    buf[n] = 0;
    p = buf;
    if (strncmp(buf, cwd, strlen(cwd)) == 0)
      p += strlen(cwd);
    fprintf(stderr, "Result of readlink(\"%s\"): %s\n", label, p);
  }
  else
    perror("readlink");
}

static void test_readlinkat(const char* const cwd, const char* const label,
                            const char* const path)
{
#if HAVE_READLINKAT
  char buf[512];
  const char* p;
  int n;

  if ((n = readlinkat(AT_FDCWD, path, buf, sizeof(buf) - 1)) >= 0)
  {
    buf[n] = 0;
    p = buf;
    if (strncmp(buf, cwd, strlen(cwd)) == 0)
      p += strlen(cwd);
    fprintf(stderr, "Result of readlinkat(\"%s\"): %s\n", label, p);
  }
  else
    perror("readlinkat");
#else
  errno = ENOSYS;
  perror("readlinkat");
#endif
}

int main(int argc, char** argv)
{
  char cwd[512];
  char path[512];

  cwd[0] = 0;
  if (! getcwd(cwd, sizeof(cwd)))
    perror("getcwd");
  strcat(cwd, "/");

  snprintf(path, sizeof(path), CMDLINE_PID, (long)getpid());

  test_cmdline(cwd, CMDLINE_SELF, CMDLINE_SELF);
  test_cmdline(cwd, "/proc/<pid>/cmdline", path);

  snprintf(path, sizeof(path), EXEFILE_PID, (long)getpid());

  test_readlink(cwd, EXEFILE_SELF, EXEFILE_SELF);
  test_readlink(cwd, EXEFILE_LABEL, path);

  test_readlinkat(cwd, EXEFILE_SELF, EXEFILE_SELF);
  test_readlinkat(cwd, EXEFILE_LABEL, path);

  return 0;
}
