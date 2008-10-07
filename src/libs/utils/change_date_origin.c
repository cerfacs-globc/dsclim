/* ***************************************************** */
/* Change date origin of time expressed in udunits.      */
/* change_date_origin.c                                  */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file change_date_origin.c
    \brief Change date origin of time expressed in udunits.
*/

#include <utils.h>

void change_date_origin(double *timeout, char *tunits_out, double *timein, char *tunits_in, int ntime) {

  int t;
  int istat;

  utUnit dataunit_in, dataunit_out;

  int *year = NULL;
  int *month = NULL;
  int *day = NULL;
  int *hour = NULL;
  int *minutes = NULL;
  float *seconds = NULL;

  if (utIsInit() != TRUE)
    istat = utInit("");
  
  istat = utScan(tunits_in,  &dataunit_in);
  istat = utScan(tunits_out, &dataunit_out);

  year = (int *) malloc(ntime * sizeof(int));
  if (year == NULL) alloc_error(__FILE__, __LINE__);
  month = (int *) malloc(ntime * sizeof(int));
  if (month == NULL) alloc_error(__FILE__, __LINE__);
  day = (int *) malloc(ntime * sizeof(int));
  if (day == NULL) alloc_error(__FILE__, __LINE__);
  hour = (int *) malloc(ntime * sizeof(int));
  if (hour == NULL) alloc_error(__FILE__, __LINE__);
  minutes = (int *) malloc(ntime * sizeof(int));
  if (minutes == NULL) alloc_error(__FILE__, __LINE__);
  seconds = (float *) malloc(ntime * sizeof(float));
  if (seconds == NULL) alloc_error(__FILE__, __LINE__);

  for (t=0; t<ntime; t++) {
    istat = utCalendar(timein[t], &dataunit_in, &(year[t]), &(month[t]), &(day[t]), &(hour[t]), &(minutes[t]), &(seconds[t]));
    istat = utInvCalendar(year[t], month[t], day[t], hour[t], minutes[t], seconds[t], &dataunit_out, &(timeout[t]));
  }

  (void) free(year);
  (void) free(month);
  (void) free(day);
  (void) free(hour);
  (void) free(minutes);
  (void) free(seconds);
  
  (void) utTerm();
}
