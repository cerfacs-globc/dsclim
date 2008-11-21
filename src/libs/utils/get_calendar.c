/* ***************************************************** */
/* Get year,month,day,hour,min,sec given time in udunits */
/* get_calendar.c                                        */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file get_calendar.c
    \brief Get year,month,day,hour,min,sec given time in udunits.
*/

#include <utils.h>

/** Get year,month,day,hour,min,sec given time in udunits. */
int get_calendar(int *year, int *month, int *day, int *hour, int *minutes, float *seconds, char *tunits, double *timein, int ntime) {

  /** 
      @param[out]  year       Year vector
      @param[out]  month      Month vector
      @param[out]  day        Day vector
      @param[out]  hour       Hour vector
      @param[out]  minutes    Minutes vector
      @param[out]  seconds    Seconds vector
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
    istat = utCalendar(timein[t], &dataunit, &(year[t]), &(month[t]), &(day[t]), &(hour[t]), &(minutes[t]), &(seconds[t]));
  
  /* Terminate udunits */
  (void) utTerm();

  /* Success status */
  return 0;
}
