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

/** Change date origin of time expressed in udunits. */
void change_date_origin(double *timeout, char *tunits_out, double *timein, char *tunits_in, int ntime) {

  /**
     @param[out] timeout    Output time vector
     @param[out] tunits_out Output time units (udunits)
     @param[in]  timein     Input time vector
     @param[in]  tunits_in  Input time units (udunits)
     @param[in]  ntime      Number of times
   */

  int t; /* Time loop counter */
  int istat; /* Diagnostic status */

  utUnit dataunit_in; /* Input data time units (udunits) */
  utUnit dataunit_out; /* Output data time units (udunits) */

  int *year = NULL; /* Year vector */
  int *month = NULL; /* Month vector */
  int *day = NULL; /* Day vector */
  int *hour = NULL; /* Hour vector */
  int *minutes = NULL; /* Minutes vector */
  float *seconds = NULL; /* Seconds vector */

  /* Initialize udunits */
  if (utIsInit() != TRUE)
    istat = utInit("");

  /* Generate time units (udunits) */
  istat = utScan(tunits_in,  &dataunit_in);
  istat = utScan(tunits_out, &dataunit_out);

  /* Allocate memory */
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

  /* Parse all times and convert time info */
  for (t=0; t<ntime; t++) {
    istat = utCalendar(timein[t], &dataunit_in, &(year[t]), &(month[t]), &(day[t]), &(hour[t]), &(minutes[t]), &(seconds[t]));
    istat = utInvCalendar(year[t], month[t], day[t], hour[t], minutes[t], seconds[t], &dataunit_out, &(timeout[t]));
  }

  /* Free memory */
  (void) free(year);
  (void) free(month);
  (void) free(day);
  (void) free(hour);
  (void) free(minutes);
  (void) free(seconds);

  /* Terminate udunits */
  (void) utTerm();
}
