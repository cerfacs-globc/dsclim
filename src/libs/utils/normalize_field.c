/* ***************************************************** */
/* Normalize a 3D variable by the mean and variance.     */
/* normalize_field.c                                     */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file normalize_field.c
    \brief Normalize a 3D variable by a mean and variance.
*/

#include <utils.h>

/** Normalize a 3D variable by a mean and variance. */
void normalize_field(double *nbuf, double *buf, double mean, double var, int ndima, int ndimb, int ntime) {

  /**
     @param[out]  nbuf    Normalized 3D buffer
     @param[in]   buf     Input 3D buffer
     @param[in]   mean    Mean
     @param[in]   var     Variance
     @param[in]   ndima   First dimension
     @param[in]   ndimb   Second dimension
     @param[in]   ntime   Time dimension
   */

  int nt; /* Time loop counter */
  int dima; /* First dimension counter */
  int dimb; /* Second dimension counter */

  /* Loop over all elements and normalize */
  for (nt=0; nt<ntime; nt++)
    for (dimb=0; dimb<ndimb; dimb++)
      for (dima=0; dima<ndima; dima++)
        nbuf[dima+dimb*ndima+nt*ndima*ndimb] = (buf[dima+dimb*ndima+nt*ndima*ndimb] - mean) / sqrt(var);
}
