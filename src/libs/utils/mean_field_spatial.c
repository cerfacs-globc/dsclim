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

void mean_field_spatial(double *buf_mean, double *buf, int ni, int nj, int ntime) {

  double sum;
  
  int t;
  int i;
  int j;

  for (t=0; t<ntime; t++) {
    sum = 0.0;
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++)
        sum += buf[i+j*ni+t*ni*nj];
    buf_mean[t] = sum / (double) (ni*nj);
  }
}
