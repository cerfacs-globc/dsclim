/* ***************************************************** */
/* Compute daily climatology for climatological year     */
/* clim_daily_tserie_climyear.c                          */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file clim_daily_tserie_climyear.c
    \brief Compute daily climatology for climatological year.
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




#include <clim.h>

/** Compute daily climatology for climatological months of a daily time serie. */
void
clim_daily_tserie_climyear(double *bufout, double *bufin, tstruct *buftime, double missing_val, int ni, int nj, int nt) {
  /**
     @param[out]     bufout        Output 3D matrix of daily climatology for a year.
     @param[in]      bufin         Input 3D matrix.
     @param[in]      buftime       Time vector for input vector data.
     @param[in]      missing_val   Missing value.
     @param[in]      ni            Horizontal dimension of buffer input vector.
     @param[in]      nj            Horizontal dimension of buffer input vector.
     @param[in]      nt         Temporal dimension of buffer input vector.
  */

  int *index = NULL; /* Index to flag matching a specific day and month in a time serie covering several years. */
  double *sum; /* Sum over all matching days. */
  int *ndays = NULL; /* Number of days matching days. */
  int month; /* Climatological month. */

  int t; /* Loop counter for time. */
  int i; /* Loop counter for ni. */
  int j; /* Loop counter for nj. */
  int day; /* Loop counter for days. */

  (void) fprintf(stdout, "%s: Computing climatological months of a daily time serie.\n", __FILE__);

  /* Allocate memory */
  index = (int *) calloc(nt, sizeof(int));
  if (index == NULL) alloc_error(__FILE__, __LINE__);
  sum = (double *) malloc(ni*nj * sizeof(double));
  if (sum == NULL) alloc_error(__FILE__, __LINE__);
  ndays = (int *) malloc(ni*nj * sizeof(int));
  if (ndays == NULL) alloc_error(__FILE__, __LINE__);

  /* Loop over the 12 months of the year to generate daily climatological months */
  for (month=1; month<=12; month++) {
    
    /* Loop over each day of the month */
    for (day=1; day<=31; day++) {
      
      for (j=0; j<nj; j++)
        for (i=0; i<ni; i++) {
          sum[i+j*ni] = 0.0;
          ndays[i+j*ni] = 0; /* Initialize the number of days */
        }
      
      /* Loop over all the times */
      for (t=0; t<nt; t++) {
        /* Initialize */
        index[t] = 0;
        if (buftime[t].day == day && buftime[t].month == month) {
          /* The climatological day and month match */
          index[t] = 1; /* Flag it */
          for (j=0; j<nj; j++)
            for (i=0; i<ni; i++)
              if (bufin[i+j*ni+t*ni*nj] != missing_val) {
                /* Ignore missing values */
                sum[i+j*ni] += bufin[i+j*ni+t*ni*nj]; /* Sum all the values for this matching day/month */
                ndays[i+j*ni]++;
              }
        }
      }
      for (t=0; t<nt; t++)
        /* Compute the mean over all the matching days and apply mean for all these flagged days */
        /* Assign mean value for all flagged days used in computing this mean */
        if (index[t] == 1) {
          for (j=0; j<nj; j++)
            for (i=0; i<ni; i++)
              if (ndays[i+j*ni] > 0) {
                bufout[i+j*ni+t*ni*nj] = sum[i+j*ni] / (double) ndays[i+j*ni];
              }
              else {
                bufout[i+j*ni+t*ni*nj] = missing_val;
              }
        }
    }
  }

  /* Free memory */
  (void) free(index);
  (void) free(sum);
  (void) free(ndays);
}
