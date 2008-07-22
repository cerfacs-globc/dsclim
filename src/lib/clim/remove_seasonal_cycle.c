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

void remove_seasonal_cycle(double *bufout, double *bufin, tstruct *buftime, double missing_val, int filter_width, char *type, int ntime) {
  
  double *tmpbuf = NULL;
  int nt;

  tmpbuf = (double *) calloc(ntime, sizeof(double));
  if (tmpbuf == NULL) alloc_error();

  (void) fprintf(stdout, "remove_seasonal_cycle: Removing seasonal cycle for a time serie.\n");

  /* Compute daily climatologies for climatological year */
  (void) clim_daily_tserie_climyear(bufout, bufin, buftime, missing_val, ntime);
  if ( !strcmp(type, "hanning") ) {
    (void) fprintf(stdout, "remove_seasonal_cycle: Using a hanning filter for climatology (wrap edges).\n");
    /* Filter climatologies using a Hanning filter (wrap edges) */
    (void) filter(tmpbuf, bufout, filter_width, ntime);
  }
  else {
    (void) fprintf(stderr, "remove_seasonal_cycle: ABORT: Unknown filtering type for climatology: %s\n", type);
    exit(1);
  }

  /* Remove climatology from time serie */
  for (nt=0; nt<ntime; nt++)
    bufout[nt] = bufin[nt] - tmpbuf[nt];

  (void) free(tmpbuf);
}
