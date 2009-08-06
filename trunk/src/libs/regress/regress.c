/* ***************************************************** */
/* Compute regression coefficients with a regression     */
/* constant given two vectors, and do it for npts        */
/* regression points.                                    */
/* regress.c                                             */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file regress.c
    \brief Compute regression coefficients with a regression constant given two vectors, and do it for npts regression points.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2009)

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

#include <regress.h>

/** Compute regression coefficients with a regression constant given two vectors, and do it for npts regression points. */
int
regress(double *coef, double *x, double *y, double *cte, double *yreg, double *yerr, double *chisq, int nterm, int npts) {
  /**
     @param[out]  coef      Regression coefficients
     @param[in]   x         X vectors (npts regression points)
     @param[in]   y         Y vectors (npts regression points)
     @param[out]  cte       Regression constant
     @param[out]  yreg      Y vector reconstructed with regression
     @param[out]  yerr      Y error vector when reconstructing Y vector with regression
     @param[out]  chisq     Chi-square diagnostic
     @param[in]   nterm     Vector dimension
     @param[in]   npts      Number of regression points

     \return      Status
  */
  
  gsl_vector_view row; /* To retrieve vector rows */

  gsl_matrix *xx; /* X matrix */
  gsl_matrix *cov; /* Covariance matrix */

  gsl_multifit_linear_workspace *work; /* Workspace */

  gsl_vector *yy; /* Y vector */
  gsl_vector *ccoef; /* Coefficients vector */

  int istat; /* Diagnostic status */
  int term; /* Loop counter for vector dimension */
  int pts; /* Loop counter for regression points */

  double val; /* Value retrieved */

  /* Allocate memory and create matrices and vectors */
  xx = gsl_matrix_alloc(npts, nterm+1);
  yy = gsl_vector_alloc(npts);

  ccoef = gsl_vector_alloc(nterm+1);
  cov = gsl_matrix_alloc(nterm+1, nterm+1);

  /* Create X matrix */
  for (term=0; term<nterm; term++)
    for (pts=0; pts<npts; pts++) {
      val = x[pts+term*npts];
      (void) gsl_matrix_set(xx, pts, term+1, val);
    }

  /* Create first column of matrix for regression constant */
  for (pts=0; pts<npts; pts++)
    (void) gsl_matrix_set(xx, pts, 0, 1.0);  
  
  /* Create Y vector for all regression points */
  for (pts=0; pts<npts; pts++) {
    val = y[pts];
    (void) gsl_vector_set(yy, pts, val);
  }

  /* Allocate workspace */
  work = gsl_multifit_linear_alloc(npts, nterm+1);

  /* Perform linear regression */
  istat = gsl_multifit_linear(xx, yy, ccoef, cov, chisq, work);
  if (istat != GSL_SUCCESS) {
    (void) fprintf(stderr, "%s: Line %d: Error %d in multifitting algorithm!\n", __FILE__, __LINE__, istat);
    (void) gsl_multifit_linear_free(work);
    (void) gsl_matrix_free(xx);
    (void) gsl_matrix_free(cov);
    (void) gsl_vector_free(yy);
    (void) gsl_vector_free(ccoef);
    return istat;
  }
  /* Free workspace */
  (void) gsl_multifit_linear_free(work);

  /* Debug output */
#if DEBUG >= 7
#define C(i) (gsl_vector_get(ccoef,(i)))
#define COV(i,j) (gsl_matrix_get(cov,(i),(j)))
#define X(i,j) (gsl_matrix_get(xx,(i),(j)))
     
  {
    printf ("# best fit: Y = %g + %g X + %g X^2\n", 
            C(0), C(1), C(2));

    printf ("# x matrix:\n");
    
    for (term=0; term<nterm+1; term++)
      for (pts=0; pts<npts; pts++)
        printf("term=%d pts=%d x=%lf\n", term, pts, X(pts,term));
    
    printf ("# covariance matrix:\n");
    printf ("[ %+.5e, %+.5e, %+.5e  \n",
            COV(0,0), COV(0,1), COV(0,2));
    printf ("  %+.5e, %+.5e, %+.5e  \n", 
            COV(1,0), COV(1,1), COV(1,2));
    printf ("  %+.5e, %+.5e, %+.5e ]\n", 
            COV(2,0), COV(2,1), COV(2,2));
    printf ("# chisq = %g\n", *chisq);
  }
#endif

  /* Retrieve regression coefficients */
  for (term=0; term<nterm; term++)
    coef[term] = gsl_vector_get(ccoef, term+1);
  /* Retrieve regression constant */
  *cte = gsl_vector_get(ccoef, 0);

  /* Reconstruct vector using regression coefficients, and calculate error */
#if DEBUG >= 7
  (void) fprintf(stdout, "%s: Vector reconstruction\n", __FILE__);
#endif
  for (pts=0; pts<npts; pts++) {
    row = gsl_matrix_row(xx, pts);
    istat = gsl_multifit_linear_est(&row.vector, ccoef, cov, &(yreg[pts]), &(yerr[pts]));
    if (istat != GSL_SUCCESS) {
      (void) fprintf(stderr, "%s: Line %d: Error %d in multifitting linear values estimation!\n", __FILE__, __LINE__, istat);
      (void) gsl_matrix_free(xx);
      (void) gsl_matrix_free(cov);
      (void) gsl_vector_free(yy);
      (void) gsl_vector_free(ccoef);
      return istat;
    }
  }
  
  /* Dealloc matrices and vectors memory */
  (void) gsl_matrix_free(xx);
  (void) gsl_matrix_free(cov);
  (void) gsl_vector_free(yy);
  (void) gsl_vector_free(ccoef);

  /* Success status */
  return 0;
}
