/* ********************************************************* */
/* testregress Test multifit linear regression GSL function. */
/* testregress.c                                             */
/* ********************************************************* */
/* Author: Christian Page, CERFACS, Toulouse, France.        */
/* ********************************************************* */
/*! \file testregress.c
    \brief Test multifit linear regression GSL function.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2013)

christian.page@cerfacs.fr

This software is a computer program whose purpose is to downscale climate
scenarios using a statistical methodology based on weather regimes.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software. You can use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty and the software's author, the holder of the
economic rights, and the successive licensors have only limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading, using, modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean that it is complicated to manipulate, and that also
therefore means that it is reserved for developers and experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and, more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.

LICENSE END */





#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/** GNU extensions */
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
#ifdef HAVE_UNSTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
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
#include <libgen.h>
#endif

#include <utils.h>
#include <regress.h>

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

  int npts;
  int nterm;
  int pts;
#if DEBUG >= 6
  int term;
#endif

  int i;
  int istat;

  double *x = NULL;
  double *y = NULL;
  double *yreg = NULL;
  double *coef = NULL;
  double *yerr = NULL;
  double *vif = NULL;
  double cte;
  double chisq;
  double rsq;
  double autocor;

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
      (void) abort();
    }
  }

  npts = 6;
  nterm = 2;

  x = (double *) calloc(npts*nterm, sizeof(double));
  if (x == NULL) alloc_error(__FILE__, __LINE__);
  
  x[0+0*npts] = 1.0;
  x[1+0*npts] = 2.0;
  x[2+0*npts] = 4.0;
  x[3+0*npts] = 8.0;
  x[4+0*npts] = 16.0;
  x[5+0*npts] = 32.0;

  x[0+1*npts] = 0.0;
  x[1+1*npts] = 1.0;
  x[2+1*npts] = 2.0;
  x[3+1*npts] = 3.0;
  x[4+1*npts] = 4.0;
  x[5+1*npts] = 5.0;

  y = (double *) calloc(npts, sizeof(double));
  if (y == NULL) alloc_error(__FILE__, __LINE__);
  yreg = (double *) calloc(npts, sizeof(double));
  if (yreg == NULL) alloc_error(__FILE__, __LINE__);
  yerr = (double *) calloc(npts, sizeof(double));
  if (yerr == NULL) alloc_error(__FILE__, __LINE__);
  coef = (double *) calloc(nterm, sizeof(double));
  if (coef == NULL) alloc_error(__FILE__, __LINE__);
  vif = (double *) calloc(nterm, sizeof(double));
  if (vif == NULL) alloc_error(__FILE__, __LINE__);

  for (pts=0; pts<npts; pts++)
    y[pts] = 5.0 + 3.0 * x[pts+0*npts] - 4.0 * x[pts+1*npts];

  istat = regress(coef, x, y, &cte, yreg, yerr, &chisq, &rsq, vif, &autocor, nterm, npts);

#if DEBUG >= 6
  for (term=0; term<nterm; term++)
    for (pts=0; pts<npts; pts++)
      (void) fprintf(stdout, "%s: term=%d pts=%d x=%lf\n", __FILE__, term, pts, x[pts+term*npts]);

  for (term=0; term<nterm; term++)
    (void) fprintf(stdout, "%s: term=%d coef=%lf\n", __FILE__, term, coef[term]);

  (void) fprintf(stdout, "%s: cte=%lf chisq=%lf\n", __FILE__, cte, chisq);

  for (pts=0; pts<npts; pts++)
    (void) fprintf(stdout, "%s: pts=%d y=%lf yreg=%lf yerr=%lf\n", __FILE__, pts, y[pts], yreg[pts], yerr[pts]);
#endif

  (void) free(coef);
  (void) free(yreg);
  (void) free(yerr);
  (void) free(vif);
  (void) free(y);
  (void) free(x);

  /* Print END banner */
  (void) banner(basename(argv[0]), "OK", "END");

  return 0;
}


/** Local Subroutines **/

/** Show usage for program command-line arguments. */
void show_usage(char *pgm) {
  /**
     @param[in]  pgm  Program name.
  */

  (void) fprintf(stderr, "%s: usage:\n", pgm);
  (void) fprintf(stderr, "-h: help\n");

}
