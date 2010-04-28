/* ***************************************************** */
/* Remove seasonal cycle for a time serie                */
/* remove_seasonal_cycle.c                               */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file remove_seasonal_cycle.c
    \brief Remove seasonal cycle for a time serie.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2010)

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

/** Remove seasonal cycle using a time filter. */
void
remove_seasonal_cycle(double *bufout, double *clim, double *bufin, tstruct *buftime, double missing_val,
                      int filter_width, char *type, int clim_provided, int ni, int nj, int ntime) {
  /**
     @param[out]     bufout        Output data 3D matrix with seasonal cycle removed.
     @param[out,in]  clim          Climatology vector (on 366 days). Can be already provided as input or not (clim_provided parameter).
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
  int month; /* Climatological month. */
  int day; /* Climatological day. */
  int dayofclimy; /* Day of year in a 366-day climatological year */

  /* Allocate temporay buffer memory. */
  tmpbuf = (double *) calloc(ni*nj*ntime, sizeof(double));
  if (tmpbuf == NULL) alloc_error(__FILE__, __LINE__);

  (void) fprintf(stdout, "%s: Removing seasonal cycle for a time serie.\n", __FILE__);

  if (clim_provided != TRUE) {
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
    for (month=0; month<12; month++)
      for (day=0; day<ndays_m; day++)
        /* Loop over all the times */
        for (t=0; t<ntime; t++) {
          if (buftime[t].day == (day+1) && buftime[t].month == (month+1)) {
            dayofclimy = dayofclimyear(day+1, month+1);
            for (j=0; j<nj; j++)
              for (i=0; i<ni; i++)
                clim[i+j*ni+(dayofclimy-1)*ni*nj] = tmpbuf[i+j*ni+t*ni*nj];
            /* Exit loop: matched month and day */
            t = ntime;
          }
        }
  }
  else {
    /* Climatology field was provided */
    /* Loop over all the times */
    for (t=0; t<ntime; t++) {
      dayofclimy = dayofclimyear(buftime[t].day, buftime[t].month);
      for (j=0; j<nj; j++)
        for (i=0; i<ni; i++)
          bufout[i+j*ni+t*ni*nj] = bufin[i+j*ni+t*ni*nj] - clim[i+j*ni+(dayofclimy-1)*ni*nj];
    }
  }
  
  /* Free memory */
  (void) free(tmpbuf);
}
