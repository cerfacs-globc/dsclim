/* ***************************************************** */
/* Get year,month,day,hour,min,sec given time in udunits */
/* get_calendar_ts.c                                     */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file get_calendar_ts.c
    \brief Get year,month,day,hour,min,sec (time structure) given time in udunits
*/

#include <utils.h>

int get_calendar_ts(tstruct *timeout, char *tunits, double *timein, int ntime) {

  int t;
  int istat;

  utUnit dataunit;

  if (utIsInit() != TRUE)
    istat = utInit("");
  
  istat = utScan(tunits,  &dataunit);
  for (t=0; t<ntime; t++)
    istat = utCalendar(timein[t], &dataunit, &(timeout[t].year), &(timeout[t].month), &(timeout[t].day), &(timeout[t].hour), &(timeout[t].min), &(timeout[t].sec));
  
  return 0;
}
