/* ***************************************************** */
/* Compute the spatial covariance of two fields.         */
/* covariance_fields_spatial.c                           */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file covariance_fields_spatial.c
    \brief Compute the spatial covariance of two fields.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2012)

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

/** Compute the spatial covariance of two fields. */
void
covariance_fields_spatial(double *cov, double *buf1, double *buf2, short int *mask, int t1, int t2, int ni, int nj) {

  /** 
      @param[out]  cov           Spatial covariance
      @param[in]   buf1          Input 3D buffer 1
      @param[in]   buf2          Input 3D buffer 2
      @param[in]   mask          Input 2D mask
      @param[in]   t1            Time index of buf1 to process
      @param[in]   t2            Time index of buf2 to process
      @param[in]   ni            First dimension
      @param[in]   nj            Second dimension
   */

  int i; /* Loop counter */
  int j; /* Loop counter */
  int pts = 0; /* Points counter */
  int num = 0; /* Elements counter */

  double sum1; /* Temporary sum for buf1 */
  double sum2; /* Temporary sum for buf2 */
  double mean1; /* Mean for buf1 */
  double mean2; /* Mean for buf2 */

  /* Compute spatial covariance, optionally using a mask */
  if (mask == NULL) {

    /* Calculate mean of each field */
    sum1 = 0.0;
    sum2 = 0.0;
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++) {
        sum1 += buf1[i+j*ni+t1*ni*nj];
        sum2 += buf2[i+j*ni+t2*ni*nj];
      }
    mean1 = sum1 / (double) (ni*nj);
    mean2 = sum2 / (double) (ni*nj);

    /* Calculate spatial covariance */
    *cov = 0.0;
    num = 0;
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++) {
        /* Sum of the squares (remove mean) */
        (*cov) += ( ( (buf1[i+j*ni+t1*ni*nj] - mean1) * (buf2[i+j*ni+t2*ni*nj] - mean2) ) - (*cov)) / (double) (num + 1);
        num++;
      }
  }
  else {
    /* Calculate mean of each field */
    sum1 = 0.0;
    sum2 = 0.0;
    pts = 0;
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++)
        if (mask[i+j*ni] == 1) {
          sum1 += buf1[i+j*ni+t1*ni*nj];
          sum2 += buf2[i+j*ni+t2*ni*nj];
          pts++;
        }
    mean1 = sum1 / (double) (pts);
    mean2 = sum2 / (double) (pts);

    /* Calculate spatial covariance */
    *cov = 0.0;
    num = 0;
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++) {
        if (mask[i+j*ni] == 1) {
          /* Sum of the squares (remove mean) */
          (*cov) += ( ( (buf1[i+j*ni+t1*ni*nj] - mean1) * (buf2[i+j*ni+t2*ni*nj] - mean2) ) - (*cov)) / (double) (num + 1);
          num++;
        }
      }
  }
}
