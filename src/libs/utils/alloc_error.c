
#include <utils.h>

/** Subroutine to dump the core of the program (to debug) in the event of a memory allocation error. */
void alloc_error(char *filename, int line)
{
  /**
     @param[in]      filename      Filename of source code file.
     @param[in]      line          Line number.
  */

  /** Print error on standard error along with source code filename and line number. */
  (void) fprintf(stderr,"alloc_error: Memory allocation error (malloc)\n\nExiting and dumping core... File=%s Line=%d\n\n",
                 filename, line);

  /** Kill process and dump core for debugging purposes. */
  (void) kill(getpid(), 5);
}
