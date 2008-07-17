#include <utils.h>

/** Subroutine to dump the core of the program (to debug) in the event of a memory allocation error. */
void alloc_error(void)
{
  fprintf(stderr,"alloc_error: Memory allocation error (malloc)\n\nExiting and dumping core...\n\n");
  (void) kill(getpid(), 5);
}
