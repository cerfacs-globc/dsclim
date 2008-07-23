/* ***************************************************** */
/* testclassif Test classification algorithm.            */
/* testclassif.c                                         */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file testclassif.c
    \brief Test classification algorithm.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

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

#include <classif.h>

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
  int j;
  int neof;
  int ndays;
  int nclusters;
  int nclassif;

  double *pc_eof_days = NULL;
  double *clusters = NULL;

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

  /* Use simulated 10 EOFs */
  neof = 10;
  /* Use simulated 15000 days */
  ndays = 15000;
  /* Try 1000 classifications */
  nclassif = 1000;
  /* Use 8 clusters */
  nclusters = 8;

  /* Initialize random number generator */
  T = gsl_rng_default;
  rng = gsl_rng_alloc(T);
  (void) gsl_rng_set(rng, time());

  /* Allocate memory */
  pc_eof_days = (double *) calloc(neof*ndays, sizeof(double));
  if (pc_eof_days == NULL) alloc_error();
  clusters = (double *) calloc(neof*nclusters, sizeof(double));
  if (clusters == NULL) alloc_error();
  
  /* Generate a double between 0.0 and 1.0 */
  for (i=0; i<neof; i++) {
    for (j=0; j<ndays/2; j++) {
      pc_eof_days[i+j*neof] = fabs(gsl_ran_gaussian(rng, 1.0) / 2.0);
#ifdef DEBUG
      /*      (void) fprintf(stderr, "eof %d day %d pc_eof_days %lf\n", i, j, pc_eof_days[i+j*neof]);*/
#endif
    }
    for (j=ndays/2; j<ndays; j++) {
      pc_eof_days[i+j*neof] = fabs((gsl_ran_gaussian(rng, 1.0) / 2.0))+ 0.5;
#ifdef DEBUG
      /*      (void) fprintf(stderr, "eof %d day %d pc_eof_days %lf\n", i, j, pc_eof_days[i+j*neof]);*/
#endif
    }
  }

  (void) gsl_rng_free(rng);

  /* Find clusters: test classification algorithm */
  (void) generate_clusters(clusters, pc_eof_days, "euclidian", nclassif, neof, nclusters, ndays);

  /* Output data */
  for (i=0; i<neof; i++)
    for (j=0; j<nclusters; j++)
      (void) fprintf(stdout, "%d %d %lf\n", i, j, clusters[i+j*neof]);

  /* Free memory */
  (void) free(pc_eof_days);
  (void) free(clusters);

  /* Print END banner */
  (void) banner(basename(argv[0]), "OK", "END");

  return 0;
}


/** Local Subroutines **/

void show_usage(char *pgm) {

  (void) fprintf(stderr, "%s: usage:\n", pgm);
  (void) fprintf(stderr, "-h: help\n");

}
