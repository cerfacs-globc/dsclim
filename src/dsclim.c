/* ***************************************************** */
/* dsclim Downscaling climate scenarios                  */
/* dsclim.c                                              */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file dsclim.c
    \brief Downscaling climate scenarios program.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

/* C standard includes */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

/* Local C includes */
#include <utils.h>
#include <filter.h>

/** C prototypes. */
void show_usage(char *pgm);

/** Main program. */
int main(int argc, char **argv)
{
  /**
     @param[in]  argc  Number of command-line arguments.
     @param[in]  argv  Vector of command-line argument strings.

     \return           Status.
   */

  int i;

  /* Command-line arguments variables */
  char filein[500]; /* Input filename */

  /* Print BEGIN banner */
  (void) banner(basename(argv[0]), "0.1", "BEGIN");

  /* Get command-line arguments and set appropriate variables */
  if (argc <= 1) {
    (void) show_usage(basename(argv[0]));
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }
  else
    for (i=1; i<argc; i++) {
      if ( !strcmp(argv[i], "-i") )
        (void) strcpy(filein, argv[++i]);
      else {
        (void) fprintf(stderr, "%s:: Wrong arg %s.\n\n", basename(argv[0]), argv[i]);
        (void) show_usage(basename(argv[0]));
        (void) banner(basename(argv[0]), "ABORT", "END");
        (void) abort();
      }
    }
  
  /* Print END banner */
  (void) banner(basename(argv[0]), "OK", "END");

  return 0;
}


/** Subroutines **/

void show_usage(char *pgm) {

  (void) fprintf(stderr, "%s:: usage:\n", pgm);
  (void) fprintf(stderr, "-i: input file\n");

}
