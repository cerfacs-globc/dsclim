/* ***************************************************** */
/* Compute the time mean and variance of a 2D field      */
/* time_mean_variance_field_2d.c                         */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file time_mean_variance_field_2d.c
    \brief Compute the time mean and variance of a 2D field.
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





#include <utils.h>

/** Compute the time mean and variance of a 2D field. */
void
time_mean_variance_field_2d(double *bufmean, double *bufvar, double *buf, int ni, int nj, int nt) {

  /** 
      @param[out]  bufmean       Time mean of 2D field
      @param[out]  bufvar        Time variance of 2D field
      @param[in]   buf           Input 3D buffer
      @param[in]   ni            First dimension
      @param[in]   nj            Second dimension
      @param[in]   nt            Time dimension
   */

  int i; /* Loop counter */
  int j; /* Loop counter */
  int t; /* Loop counter */

  double sum; /* Temporary sum for buf */
  double diff; /* Temporary difference for variance calculation */

  /* First calculate mean over time of a 2D field */
  for (j=0; j<nj; j++)
    for (i=0; i<ni; i++) {
      sum = 0.0;
      for (t=0; t<nt; t++)
        sum += buf[i+j*ni+t*ni*nj];
      bufmean[i+j*ni] = sum / (double) nt;
    }

  /* Then calculate variance over time of a 2D field */
  for (j=0; j<nj; j++)
    for (i=0; i<ni; i++) {
      sum = 0.0;
      for (t=0; t<nt; t++) {
        diff = buf[i+j*ni+t*ni*nj] - bufmean[i+j*ni];
        sum += (diff * diff);
      bufvar[i+j*ni] = sqrtf(sum / (double) (nt-1));
      }
    }
}

