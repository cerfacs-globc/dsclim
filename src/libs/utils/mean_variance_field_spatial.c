/* ***************************************************** */
/* Compute mean and variance of a field averaged         */
/* spatially.                                            */
/* mean_variance_field_spatial.c                         */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file mean_variance_field_spatial.c
    \brief Compute mean and variance of a field averaged spatially.
*/

#include <utils.h>

void mean_variance_field_spatial(double *buf_mean, double *buf_var, double *buf, int ni, int nj, int ntime) {

  double sum;
  double *buf_smean = NULL;
  
  int t;
  int i;
  int j;

  buf_smean = (double *) malloc(ntime * sizeof(double));
  if (buf_smean == NULL) alloc_error(__FILE__, __LINE__);

  for (t=0; t<ntime; t++) {
    sum = 0.0;
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++)
        sum += buf[i+j*ni+t*ni*nj];
    buf_smean[t] = sum / (double) (ni*nj);
  }
  *buf_mean = gsl_stats_mean(buf_smean, 1, ntime);
  *buf_var = gsl_stats_variance(buf_smean, 1, ntime);

  (void) free(buf_smean);
}