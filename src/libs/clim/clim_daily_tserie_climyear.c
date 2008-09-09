/* ***************************************************** */
/* Compute daily climatology for climatological year     */
/* clim_daily_tserie_climyear.c                          */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file clim_daily_tserie_climyear.c
    \brief Compute daily climatology for climatological year.
*/

#include <clim.h>

/** Compute daily climatology for climatological months of a daily time serie. */
void clim_daily_tserie_climyear(double *bufout, double *bufin, tstruct *buftime, double missing_val, int ni, int nj, int nt) {
  /**
     @param[out]     bufout        Output 3D matrix of daily climatology for a year.
     @param[in]      bufin         Input 3D matrix.
     @param[in]      buftime       Time vector for input vector data.
     @param[in]      missing_val   Missing value.
     @param[in]      ni            Horizontal dimension of buffer input vector.
     @param[in]      nj            Horizontal dimension of buffer input vector.
     @param[in]      nt         Temporal dimension of buffer input vector.
  */

  short int *index = NULL; /* Index to flag matching a specific day and month in a time serie covering several years. */
  double mean = 0.0; /* Mean value over all matching days. */
  double sum = 0.0; /* Sum over all matching days. */
  short int ndays = 0; /* Number of days matching days. */
  short int month; /* Climatological month. */

  int t; /* Loop counter for time. */
  int i; /* Loop counter for ni. */
  int j; /* Loop counter for nj. */
  int day; /* Loop counter for days. */

  (void) fprintf(stdout, "%s: Computing climatological months of a daily time serie.\n", __FILE__);

  /* Allocate memory */
  index = (short int *) calloc(nt, sizeof(short int));
  if (index == NULL) alloc_error(__FILE__, __LINE__);

  for (j=0; j<nj; j++)
    for (i=0; i<ni; i++) {
      
      /* Loop over the 12 months of the year to generate daily climatological months */
      for (month=1; month<=12; month++) {
        
        /* Loop over each day of the month */
        for (day=1; day<=31; day++) {
          
          ndays = 0; /* Initialize the number of days */
          sum = 0.0;
          
          /* Loop over all the times */
          for (t=0; t<nt; t++) {
            /* Initialize */
            index[t] = 0;
            if (buftime[t].day == day && buftime[t].month == month) {
              /* The climatological day and month match */
              index[t] = 1; /* Flag it */
              if (bufin[i+j*ni+t*ni*nj] != missing_val) {
                /* Ignore missing values */
                sum += bufin[i+j*ni+t*ni*nj]; /* Sum all the values for this matching day/month */
                ndays++;
              }
            }
          }
          /* More than one day matching */
          if (ndays > 0) {
            /* Compute the mean over all the matching days and apply mean for all these flagged days */
            mean = sum / (double) ndays;
            for (t=0; t<nt; t++)
              /* Assign mean value for all flagged days used in computing this mean */
              if (index[t] == 1)
                bufout[i+j*ni+t*ni*nj] = mean;
          }
        }
      }
    }

  /* Free memory */
  (void) free(index);
}
