/* ***************************************************** */
/* compute_time_info Compute time info from NetCDF time. */
/* compute_time_info.c                                   */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: oct 2008                            */
/* Last date of modification: oct 2008                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file compute_time_info.c
    \brief Compute time info from NetCDF time.
*/

#include <io.h>

/** Compute time info from NetCDF time. */
int compute_time_info(time_struct *time_s, double *timeval, char *time_units, char *cal_type, int ntime) {

  /**
     @param[out]  time_s      Time field in time structure
     @param[in]   timeval     Time field
     @param[in]   time_units  Time units (udunits)
     @param[in]   cal_type    Calendar type (udunits)
     @param[in]   ntime       Time dimension
     
     \return           Status.
  */

  int istat; /* Diagnostic status */
  utUnit dataunits; /* Time units (udunits) */
  int t; /* Time loop counter */

  /* Check values of time variable because many times they are all zero. In that case assume a 1 increment and a start at zero. */
  for (t=0; t<ntime; t++)
    if (timeval[t] != 0.0)
      break;
  if (t == ntime) {
    (void) fprintf(stderr, "WARNING: Time variable values all zero!!! Fixing time variable to index value...\n");
    for (t=0; t<ntime; t++)
      timeval[t] = (double) t;
  }

  /* Compute time info */
  time_s->year = (int *) malloc(ntime * sizeof(int));
  if (time_s->year == NULL) alloc_error(__FILE__, __LINE__);
  time_s->month = (int *) malloc(ntime * sizeof(int));
  if (time_s->month == NULL) alloc_error(__FILE__, __LINE__);
  time_s->day = (int *) malloc(ntime * sizeof(int));
  if (time_s->day == NULL) alloc_error(__FILE__, __LINE__);
  time_s->hour = (int *) malloc(ntime * sizeof(int));
  if (time_s->hour == NULL) alloc_error(__FILE__, __LINE__);
  time_s->minutes = (int *) malloc(ntime * sizeof(int));
  if (time_s->minutes == NULL) alloc_error(__FILE__, __LINE__);
  time_s->seconds = (float *) malloc(ntime * sizeof(float));
  if (time_s->seconds == NULL) alloc_error(__FILE__, __LINE__);

  istat = utInit("");

  istat = utScan(time_units, &dataunits);
  for (t=0; t<ntime; t++)
    istat = utCalendar_cal(timeval[t], &dataunits, &(time_s->year[t]), &(time_s->month[t]), &(time_s->day[t]),
                           &(time_s->hour[t]), &(time_s->minutes[t]), &(time_s->seconds[t]), cal_type);

  (void) utTerm();

  /* Success status */
  return 0;
}
