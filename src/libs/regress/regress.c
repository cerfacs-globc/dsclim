/* ***************************************************** */
/* Compute regression coefficients with a regression     */
/* constant given two vectors, having nterm variables    */
/* and npts dimension (usually time)                     */
/* regress.c                                             */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file regress.c
    \brief Compute regression coefficients with a regression constant given two vectors, having nterm variables and npts dimension (usually time).
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2015)

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
#include <misc.h>

/** Compute regression coefficients with a regression constant given two vectors,
    having nterm variables (usually parameters) and npts dimension (usually time). */
int
regress(double *coef, double *x, double *y, double *cte, double *yreg, double *yerr,
        double *chisq, double *rsq, double *vif, double *autocor, int nterm, int npts) {
  /**
     @param[out]  coef      Regression coefficients
     @param[in]   x         X vectors (nterm X npts) npts is usually time, nterm the number of parameters
     @param[in]   y         Y vectors (npts) npts is usually time
     @param[out]  cte       Regression constant
     @param[out]  yreg      Y vector reconstructed with regression
     @param[out]  yerr      Y error vector when reconstructing Y vector with regression
     @param[out]  chisq     Chi-square diagnostic
     @param[out]  rsq       Coefficient of determination diagnostic R^2 = 1 - \chi^2 / TSS
     @param[out]  vif       Variance Inflation Factor for each parameter
     @param[out]  autocor   Autocorrelation of residuals (Durbin-Watson test)
     @param[in]   nterm     Variables dimension
     @param[in]   npts      Vector dimension

     \return      Status
  */
  
  gsl_vector_view row; /* To retrieve vector rows */

  gsl_matrix *xx; /* X matrix */
  gsl_matrix *cov; /* Covariance matrix */

  gsl_multifit_linear_workspace *work; /* Workspace */

  gsl_vector *yy; /* Y vector */
  gsl_vector *ccoef; /* Coefficients vector */
  gsl_vector *res; /* Residuals vector */

  int istat; /* Diagnostic status */
  int term; /* Loop counter for variables dimension */
  int vterm;
  int cterm;
  int pts; /* Loop counter for vector dimension */

  size_t stride = 1; /* Stride for GSL functions */

  double vchisq; /* Temporary value for chi^2 for VIF */
  double *ytmp; /* Temporary vector for y for VIF */
  double ystd; /* Standard deviation of fitted function */

  /* Allocate memory and create matrices and vectors */
  xx = gsl_matrix_alloc(npts, nterm+1);
  yy = gsl_vector_alloc(npts);

  ccoef = gsl_vector_alloc(nterm+1);
  cov = gsl_matrix_alloc(nterm+1, nterm+1);

  /* Create X matrix */
  for (term=0; term<nterm; term++)
    for (pts=0; pts<npts; pts++)
      (void) gsl_matrix_set(xx, pts, term+1, x[pts+term*npts]);

  /* Create first column of matrix for regression constant */
  for (pts=0; pts<npts; pts++)
    (void) gsl_matrix_set(xx, pts, 0, 1.0);  
  
  /* Create Y vector for all vector dimension */
  for (pts=0; pts<npts; pts++)
    (void) gsl_vector_set(yy, pts, y[pts]);

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

  /* Compute R^2 = 1 - \chi^2 / TSS */
  *rsq = 1.0 - ( (*chisq) / gsl_stats_tss(y, stride, (size_t) pts) );

  /* Reconstruct vector using regression coefficients, and calculate its standard deviation */
#if DEBUG >= 7
  (void) fprintf(stdout, "%s: Vector reconstruction\n", __FILE__);
#endif
  for (pts=0; pts<npts; pts++) {
    row = gsl_matrix_row(xx, pts);
    istat = gsl_multifit_linear_est(&row.vector, ccoef, cov, &(yreg[pts]), &ystd);
    if (istat != GSL_SUCCESS) {
      (void) fprintf(stderr, "%s: Line %d: Error %d in multifitting linear values estimation!\n", __FILE__, __LINE__, istat);
      (void) gsl_matrix_free(xx);
      (void) gsl_matrix_free(cov);
      (void) gsl_vector_free(yy);
      (void) gsl_vector_free(ccoef);
      return istat;
    }
  }
  /* Compute also residuals */
#if DEBUG >= 7
  (void) fprintf(stdout, "%s: Residuals calculation\n", __FILE__);
#endif
  res = gsl_vector_alloc(npts);
  istat = gsl_multifit_linear_residuals(xx, yy, ccoef, res);
  if (istat != GSL_SUCCESS) {
    (void) fprintf(stderr, "%s: Line %d: Error %d in multifitting linear values residuals estimation!\n", __FILE__, __LINE__, istat);
    (void) gsl_matrix_free(xx);
    (void) gsl_matrix_free(cov);
    (void) gsl_vector_free(yy);
    (void) gsl_vector_free(res);
    (void) gsl_vector_free(ccoef);
    return istat;
  }
  /* Store residuals */
  for (pts=0; pts<npts; pts++)
    yerr[pts] = gsl_vector_get(res, pts);

  /* Compute autocorrelation of residuals (Durbin-Watson) */
  *autocor = gsl_stats_lag1_autocorrelation(yerr, stride, npts);

  /* Dealloc matrices and vectors memory */
  (void) gsl_matrix_free(xx);
  (void) gsl_matrix_free(cov);
  (void) gsl_vector_free(yy);
  (void) gsl_vector_free(res);
  (void) gsl_vector_free(ccoef);


  /** Regression diagnostics **/
  
  /* VIF */
#if DEBUG >= 7
  (void) fprintf(stdout, "%s: VIF calculation\n", __FILE__);
#endif
  xx = gsl_matrix_alloc(npts, nterm);
  yy = gsl_vector_alloc(npts);

  ccoef = gsl_vector_alloc(nterm);
  cov = gsl_matrix_alloc(nterm, nterm);

  ytmp = (double *) malloc(npts * sizeof(double));
  if (ytmp == NULL) alloc_error(__FILE__, __LINE__);
  
  /* Loop over parameters */
  for (vterm=0; vterm<nterm; vterm++) {

    (void) gsl_matrix_set_zero(xx);
    (void) gsl_matrix_set_zero(cov);

    (void) gsl_vector_set_zero(yy);
    (void) gsl_vector_set_zero(ccoef);

    /* Create X matrix */
    cterm = 0;
    for (term=0; term<nterm; term++) {
      if (term != vterm) {
        for (pts=0; pts<npts; pts++)
          (void) gsl_matrix_set(xx, pts, cterm+1, x[pts+term*npts]);
        cterm++;
      }
    }
    
    /* Create first column of matrix for regression constant */
    for (pts=0; pts<npts; pts++)
      (void) gsl_matrix_set(xx, pts, 0, 1.0);  
    
    /* Create Y vector for all vector dimension, using the vterm X values */
    for (pts=0; pts<npts; pts++) {
      ytmp[pts] = x[pts+vterm*npts];
      (void) gsl_vector_set(yy, pts, ytmp[pts]);
    }
    
    /* Allocate workspace */
    work = gsl_multifit_linear_alloc(npts, nterm);

    /* Perform linear regression just to get chi^2 */
    istat = gsl_multifit_linear(xx, yy, ccoef, cov, &vchisq, work);
    if (istat != GSL_SUCCESS) {
      (void) fprintf(stderr, "%s: Line %d: Error %d in multifitting algorithm!\n", __FILE__, __LINE__, istat);
      (void) gsl_multifit_linear_free(work);
      (void) gsl_matrix_free(xx);
      (void) gsl_matrix_free(cov);
      (void) gsl_vector_free(yy);
      (void) gsl_vector_free(ccoef);
      (void) free(ytmp);
      return istat;
    }
    
    /* Free workspace */
    (void) gsl_multifit_linear_free(work);

    /* Compute R^2 = 1 - \chi^2 / TSS */
    /* and finally VIF = 1.0 / (1.0 - R^2) */
    vif[vterm] = 1.0 / (1.0 - (1.0 - ( vchisq / gsl_stats_tss(ytmp, stride, (size_t) pts) ) ));
  }
    
  /* Dealloc matrices and vectors memory */
  (void) gsl_matrix_free(xx);
  (void) gsl_matrix_free(cov);
  (void) gsl_vector_free(yy);
  (void) gsl_vector_free(ccoef);
  
  (void) free(ytmp);

  /* Success status */
  return 0;
}
