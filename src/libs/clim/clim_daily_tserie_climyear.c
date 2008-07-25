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
void clim_daily_tserie_climyear(double *bufout, double *bufin, tstruct *buftime, double missing_val, int ntime) {
  /**
     @param[out]     bufout        Output vector of daily climatology for a year.
     @param[in]      bufin         Input vector data.
     @param[in]      buftime       Time vector for input vector data.
     @param[in]      missing_val   Missing value.
     @param[in]      ntime         Dimension of buffer input vector.
  */

  short int *index = NULL; /* Index to flag matching a specific day and month in a time serie covering several years. */
  double mean = 0.0; /* Mean value over all matching days. */
  double sum = 0.0; /* Sum over all matching days. */
  short int ndays = 0; /* Number of days matching days. */
  short int month; /* Climatological month. */

  int nt; /* Loop counter for time. */
  int day; /* Loop counter for days. */

  (void) fprintf(stdout, "%s: Computing climatological months of a daily time serie.\n", __FILE__);

  /* Allocate memory */
  index = (short int *) calloc(ntime, sizeof(short int));
  if (index == NULL) alloc_error(__FILE__, __LINE__);
  
  /* Loop over the 12 months of the year to generate daily climatological months */
  for (month=1; month<=12; month++) {

    /* Loop over each day of the month */
    for (day=1; day<=31; day++) {

      ndays = 0; /* Initialize the number of days */

      /* Loop over all the times */
      for (nt=0; nt<ntime; nt++) {
        /* Initialize */
        sum = 0.0;
        index[nt] = 0;
        if (buftime[nt].day == day && buftime[nt].month == month) {
          /* The climatological day and month match */
          index[nt] = 1; /* Flag it */
          if (bufin[nt] != missing_val) {
            /* Ignore missing values */
            sum += bufin[nt]; /* Sum all the values for this matching day/month */
            ndays++;
          }
        }
      }
      /* More than one day matching */
      if (ndays > 0) {
        /* Compute the mean over all the matching days and apply mean for all these days */
        mean = sum / (double) ndays;
        for (nt=0; nt<ntime; nt++)
          if (index[nt] == 1)
            bufout[nt] = mean;
      }
    }
  }

  /* Free memory */
  (void) free(index);
}
