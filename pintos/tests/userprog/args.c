/* Prints the command-line arguments.
   This program is used for all of the args-* tests.  Grading is
   done differently for each of the args-* tests based on the
   output. */

#include "tests/lib.h"

int
main (int argc, char *argv[]) 
{
  int i;

  test_name = "args";

  if (((unsigned long long) argv & 7) != 0)
    msg ("argv and stack must be word-aligned, actually %p", argv);

//todo write(fd=1, buffer, size) 먼저 붙이기


  msg ("begin");
  msg ("argc = %d", argc);
  for (i = 0; i <= argc; i++)
    if (argv[i] != NULL)
      msg ("argv[%d] = '%s'", i, argv[i]);

    else
      msg ("argv[%d] = null", i);

  msg ("end");

  return 0;
}
  