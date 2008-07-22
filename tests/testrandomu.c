/* ***************************************************** */
/* testrandomu Test random number GMP function.          */
/* testrandomu.c                                         */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file testrandomu.c
    \brief Test random number GMP function.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* C standard includes */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_LIBGEN_H
#  include <libgen.h>
#endif

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#define _GNU_SOURCE

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

  unsigned long int random_num, i;

  const gsl_rng_type *T;
  gsl_rng *rng;

  /* Print BEGIN banner */
  (void) banner(basename(argv[0]), "1.0", "BEGIN");

  /* Get command-line arguments and set appropriate variables */
  for (i=1; i<argc; i++) {
    if ( !strcmp(argv[i], "-h") ) {
      (void) show_usage(basename(argv[0]));
      (void) banner(basename(argv[0]), "OK", "END");
      return 0;
    }
    else {
      (void) fprintf(stderr, "%s:: Wrong arg %s.\n\n", basename(argv[0]), argv[i]);
      (void) show_usage(basename(argv[0]));
      (void) banner(basename(argv[0]), "ABORT", "END");
      return 1;
    }
  }

  /* Initialize random number generator */
  T = gsl_rng_default;
  rng = gsl_rng_alloc(T);
  (void) gsl_rng_set(rng, time());
  
  for (i=0; i<1000000; i++) {
    random_num = gsl_rng_uniform_int(rng, 100);  
    (void) fprintf(stdout,"%ld\n",random_num);
  }

  (void) gsl_rng_free(rng);

  /* Print END banner */
  (void) banner(basename(argv[0]), "OK", "END");

  return 0;
}


/** Local Subroutines **/

void show_usage(char *pgm) {

  (void) fprintf(stderr, "%s: usage:\n", pgm);
  (void) fprintf(stderr, "-h: help\n");

}
