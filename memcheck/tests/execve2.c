#include <assert.h>
#include <unistd.h>

int main ( int argc, char** argv, char** envp )
{
   char* null_filename = NULL;
   char* null_argv[] = {NULL};
   char* null_envp[] = {NULL};

   execve(null_filename,      null_argv, null_envp);
   execve("../../tests/true", null_argv, envp);
   assert(0);  // shouldn't get here
}
