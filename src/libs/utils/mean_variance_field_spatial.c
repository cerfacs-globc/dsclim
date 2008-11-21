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

/** Compute mean and variance of a field averaged spatially. */
void mean_variance_field_spatial(double *buf_mean, double *buf_var, double *buf, int ni, int nj, int ntime) {

  /**
     @param[out]  buf_mean       Mean of spatially averaged field
     @param[out]  buf_var        Variance of spatially averaged field
     @param[in]   buf            Input 3D buffer
     @param[in]   ni             First dimension
     @param[in]   nj             Second dimension
     @param[in]   ntime          Time dimension
   */

  double sum; /* Sum to compute mean */
  double *buf_smean = NULL; /* Vector (over time) of spatially averaged data */
  
  int t; /* Time loop counter */
  int i; /* Loop counter */
  int j; /* Loop counter */

  /* Allocate memory */
  buf_smean = (double *) malloc(ntime * sizeof(double));
  if (buf_smean == NULL) alloc_error(__FILE__, __LINE__);

  /* Loop over all times and calculate spatial average */
  for (t=0; t<ntime; t++) {
    sum = 0.0;
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++)
        sum += buf[i+j*ni+t*ni*nj];
    buf_smean[t] = sum / (double) (ni*nj);
  }
  
  /* Compute mean and variance over time */
  *buf_mean = gsl_stats_mean(buf_smean, 1, ntime);
  *buf_var = gsl_stats_variance(buf_smean, 1, ntime);

  /* Free memory */
  (void) free(buf_smean);
}
