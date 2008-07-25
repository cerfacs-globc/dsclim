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
void remove_seasonal_cycle(double *bufout, double *bufin, tstruct *buftime, double missing_val, int filter_width, char *type, int ntime) {
  /**
     @param[out]     bufout        Output data vector with seasonal cycle removed.
     @param[in]      bufin         Input vector data.
     @param[in]      buftime       Time vector for input vector data.
     @param[in]      type          Type of filter. Possible values: hanning.
     @param[in]      missing_val   Missing value.
     @param[in]      filter_width  Width of filter.
     @param[in]      ntime         Dimension of buffer input vector.
  */
  
  double *tmpbuf = NULL; /* Temporary vector. */
  int nt; /* Loop counter. */

  /* Allocate temporay buffer memory. */
  tmpbuf = (double *) calloc(ntime, sizeof(double));
  if (tmpbuf == NULL) alloc_error(__FILE__, __LINE__);

  (void) fprintf(stdout, "%s: Removing seasonal cycle for a time serie.\n", __FILE__);

  /* Compute daily climatologies for climatological year */
  (void) clim_daily_tserie_climyear(bufout, bufin, buftime, missing_val, ntime);
  (void) fprintf(stdout, "%s: Using a %s filter for climatology (wrap edges).\n", __FILE__, type);
  /* Filter climatologies using a filter (wrap edges) */
  (void) filter(tmpbuf, bufout, type, filter_width, ntime);

  /* Remove climatology from time serie */
  for (nt=0; nt<ntime; nt++)
    bufout[nt] = bufin[nt] - tmpbuf[nt];

  /* Free memory */
  (void) free(tmpbuf);
}
