
#include <regress.h>

int regress(double *coef, double *x, double *y, double *cte, double *yreg, double *yerr, double *chisq, int nterm, int npts) {

  gsl_vector_view row;

  gsl_matrix *xx;
  gsl_matrix *cov;

  gsl_multifit_linear_workspace *work;

  gsl_vector *yy;
  gsl_vector *ccoef;

  int istat;
  int term;
  int pts;

  double val;

  xx = gsl_matrix_alloc(npts, nterm+1);
  yy = gsl_vector_alloc(npts);

  ccoef = gsl_vector_alloc(nterm+1);
  cov = gsl_matrix_alloc(nterm+1, nterm+1);

  for (term=0; term<nterm; term++)
    for (pts=0; pts<npts; pts++) {
      val = x[pts+term*npts];
      (void) gsl_matrix_set(xx, pts, term+1, val);
    }

  for (pts=0; pts<npts; pts++)
    (void) gsl_matrix_set(xx, pts, 0, 1.0);  
  
  for (pts=0; pts<npts; pts++) {
    val = y[pts];
    (void) gsl_vector_set(yy, pts, val);
  }

  work = gsl_multifit_linear_alloc(npts, nterm+1);
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
  (void) gsl_multifit_linear_free(work);

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

  for (term=0; term<nterm; term++)
    coef[term] = gsl_vector_get(ccoef, term+1);
  *cte = gsl_vector_get(ccoef, 0);

  (void) fprintf(stdout, "%s: Vector reconstruction\n", __FILE__);
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

  (void) gsl_matrix_free(xx);
  (void) gsl_matrix_free(cov);
  (void) gsl_vector_free(yy);
  (void) gsl_vector_free(ccoef);

  return 0;
}
