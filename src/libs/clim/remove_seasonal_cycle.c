/* ***************************************************** */
/* Remove seasonal cycle for a time serie                */
/* remove_seasonal_cycle.c                               */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file remove_seasonal_cycle.c
    \brief Remove seasonal cycle for a time serie.
*/

#include <clim.h>

/** Remove seasonal cycle using a time filter. */
void remove_seasonal_cycle(double *bufout, double *clim, double *bufin, tstruct *buftime, double missing_val,
                           int filter_width, char *type, short int clim_provided, int ni, int nj, int ntime) {
  /**
     @param[out]     bufout        Output data 3D matrix with seasonal cycle removed.
     @param[out,in]  clim          Climatology vector (on 366 days). Can pe already provided as input or not (clim_provided parameter).
     @param[in]      bufin         Input 3D matrix.
     @param[in]      buftime       Time vector for input vector data.
     @param[in]      type          Type of filter. Possible values: hanning.
     @param[in]      missing_val   Missing value.
     @param[in]      filter_width  Width of filter.
     @param[in]      clim_provided Set to 1 if clim is already calculated and provided as input.
     @param[in]      ni            Horizontal dimension of buffer input vector.
     @param[in]      nj            Horizontal dimension of buffer input vector.
     @param[in]      ntime         Dimension of buffer input vector.
  */
  
  double *tmpbuf = NULL; /* Temporary vector. */
  int i; /* Loop counter for ni. */
  int j; /* Loop counter for nj. */
  int t; /* Loop counter. */
  int ndays_m = 31; /* Maximum number of days in a month. */
  short int month; /* Climatological month. */
  short int day; /* Climatological day. */

  /* Allocate temporay buffer memory. */
  tmpbuf = (double *) calloc(ni*nj*ntime, sizeof(double));
  if (tmpbuf == NULL) alloc_error(__FILE__, __LINE__);

  (void) fprintf(stdout, "%s: Removing seasonal cycle for a time serie.\n", __FILE__);

  if (clim_provided != 1) {
    /* Climatology field was not provided */

    /* Compute daily climatologies for climatological year */
    (void) clim_daily_tserie_climyear(bufout, bufin, buftime, missing_val, ni, nj, ntime);
    (void) fprintf(stdout, "%s: Using a %s filter for climatology (wrap edges).\n", __FILE__, type);
    /* Filter climatologies using a filter (wrap edges) */
    (void) filter(tmpbuf, bufout, type, filter_width, ni, nj, ntime);
    
    /* Remove climatology from time serie */
    for (t=0; t<ntime; t++)
      for (j=0; j<nj; j++)
        for (i=0; i<ni; i++)
          bufout[i+j*ni+t*ni*nj] = bufin[i+j*ni+t*ni*nj] - tmpbuf[i+j*ni+t*ni*nj];
    
    /** Output filtered climatology value **/
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++)
        for (month=0; month<12; month++)
          for (day=0; day<ndays_m; day++)
            /* Loop over all the times */
            for (t=0; t<ntime; t++) {
              if (buftime[t].day == (day+1) && buftime[t].month == (month+1)) {
                clim[i+j*ni+(day+month*ndays_m)*ni*nj] = tmpbuf[i+j*ni+t*ni*nj];
                /* Exit loop: matched month and day */
                t = ntime;
              }
            }
  }
  else {
    /* Climatology field was provided */
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++)
        /* Loop over all the times */
        for (t=0; t<ntime; t++)
          bufout[i+j*ni+t*ni*nj] = bufin[i+j*ni+t*ni*nj] - clim[i+j*ni+(buftime[t].day+buftime[t].month*ndays_m)*ni*nj];
  }
  
  /* Free memory */
  (void) free(tmpbuf);
}
