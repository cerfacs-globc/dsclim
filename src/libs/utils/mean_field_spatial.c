/* ***************************************************** */
/* Compute the spatial mean of a field.                  */
/* mean_field_spatial.c                                  */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file mean_field_spatial.c
    \brief Compute the spatial mean of a field.
*/

#include <utils.h>

/** Compute the spatial mean of a field. */
void mean_field_spatial(double *buf_mean, double *buf, int ni, int nj, int ntime) {

  /** 
      @param[out]  buf_mean      Vector (over time) of spatially averaged data
      @param[in]   buf           Input 3D buffer
      @param[in]   ni            First dimension
      @param[in]   nj            Second dimension
      @param[in]   ntime         Time dimension
   */

  double sum; /* Sum used to calculate the mean */
  
  int t; /* Time loop counter */
  int i; /* Loop counter */
  int j; /* Loop counter */

  /* Loop over all time and average spatially */
  for (t=0; t<ntime; t++) {
    sum = 0.0;
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++)
        sum += buf[i+j*ni+t*ni*nj];
    buf_mean[t] = sum / (double) (ni*nj);
  }
}
