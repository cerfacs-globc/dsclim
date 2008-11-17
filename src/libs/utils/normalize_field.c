/* ***************************************************** */
/* Normalize a 3D variable by the mean and variance.     */
/* normalize_field.c                                     */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file normalize_field.c
    \brief Normalize a 3D variable by the mean and variance.
*/

#include <utils.h>

void normalize_field(double *nbuf, double *buf, double mean, double var, int ndima, int ndimb, int ntime) {

  int nt;
  int dima;
  int dimb;

  for (nt=0; nt<ntime; nt++)
    for (dimb=0; dimb<ndimb; dimb++)
      for (dima=0; dima<ndima; dima++)
        nbuf[dima+dimb*ndima+nt*ndima*ndimb] = (buf[dima+dimb*ndima+nt*ndima*ndimb] - mean) / sqrt(var);
}
