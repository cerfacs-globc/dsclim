/* ***************************************************** */
/* Get year,month,day,hour,min,sec given time in udunits */
/* get_calendar_ts.c                                     */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file get_calendar_ts.c
    \brief Get year,month,day,hour,min,sec (time structure) given time in udunits.
*/

#include <utils.h>

/** Get year,month,day,hour,min,sec (time structure) given time in udunits. */
int get_calendar_ts(tstruct *timeout, char *tunits, double *timein, int ntime) {

  /** 
      @param[out]  timeout    Time structure vector
      @param[out]  tunits     Time units (udunits)
      @param[in]   timein     Input time vector values
      @param[in]   ntime      Number of times
   */

  int t; /* Time loop counter */
  int istat; /* Diagnostic status */

  utUnit dataunit; /* Data time units */

  /* Initialize udunits */
  if (utIsInit() != TRUE)
    istat = utInit("");

  /* Get time units */
  istat = utScan(tunits,  &dataunit);
  
  /* Loop over times and retrieve day, month, year */
  for (t=0; t<ntime; t++)
    istat = utCalendar(timein[t], &dataunit, &(timeout[t].year), &(timeout[t].month), &(timeout[t].day), &(timeout[t].hour), &(timeout[t].min), &(timeout[t].sec));

  /* Terminate udunits */
  (void) utTerm();

  /* Success status */
  return 0;
}
