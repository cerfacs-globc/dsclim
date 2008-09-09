/* ***************************************************** */
/* Get year,month,day,hour,min,sec given time in udunits */
/* get_calendar.c                                        */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file get_calendar.c
    \brief Get year,month,day,hour,min,sec given time in udunits
*/

#include <utils.h>

int get_calendar(int *year, int *month, int *day, int *hour, int *minutes, float *seconds, char *tunits, double *timein, int ntime) {

  int t;
  int istat;

  utUnit dataunit;

  if (utIsInit() != TRUE)
    istat = utInit("");
  
  istat = utScan(tunits,  &dataunit);
  for (t=0; t<ntime; t++)
    istat = utCalendar(timein[t], &dataunit, &(year[t]), &(month[t]), &(day[t]), &(hour[t]), &(minutes[t]), &(seconds[t]));
  
  return 0;
}
