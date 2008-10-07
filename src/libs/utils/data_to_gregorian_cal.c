/* ***************************************************** */
/* Convert 2D data stored in 360-days or no-leap         */
/* calendars to standard Gregorian calendar.             */
/* data_to_gregorian_cal.c                               */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file data_to_gregorian_cal.c
    \brief Convert 360-days or no-leap calendar to standard Gregorian calendar
*/

#include <utils.h>

int data_to_gregorian_cal_d(double **bufout, double **outtimeval, int *ntimeout, double *bufin,
                            double *intimeval, char *tunits_in, char *tunits_out, char *cal_type, int ni, int nj, int ntimein) {
  utUnit timeslice;
  double val;
  double curtime;
  double ccurtime;
  
  int ref_year;
  int ref_month;
  int ref_day;
  int ref_hour;
  int ref_minutes;
  float ref_seconds;

  int t;
  int tt;
  int i;
  int j;
  int istat;

  char *utstring = NULL;
  utUnit dataunit_in;
  utUnit dataunit_out;

  int *year = NULL;
  int *month = NULL;
  int *day = NULL;
  int *hour = NULL;
  int *minutes = NULL;
  float *seconds = NULL;

  int cyear;
  int cmonth;
  int cday;
  int chour;
  int cminutes;
  float cseconds;

  *bufout = NULL;
  *outtimeval = NULL;
  *ntimeout = 0;

  if ( !strcmp(cal_type, "standard") || !strcmp(cal_type, "gregorian") ) {
    /** Just recopy the data when calendar type is standard or gregorian **/
    *ntimeout = ntimein;
    /* Allocate memory */
    (*bufout) = (double *) malloc(ni*nj*(*ntimeout) * sizeof(double));
    if ( (*bufout) == NULL) alloc_error(__FILE__, __LINE__);
    (*outtimeval) = (double *) malloc((*ntimeout) * sizeof(double));
    if ( (*outtimeval) == NULL) alloc_error(__FILE__, __LINE__);
    for (t=0; t<(*ntimeout); t++) {
      for (j=0; j<nj; j++)
        for (i=0; i<ni; i++)
          (*bufout)[i+j*ni+t*ni*nj] = (double) bufin[i+j*ni+t*ni*nj];
      (*outtimeval)[t] = (double) intimeval[t];
    }
  }
  else {
    /** Non-standard calendar type **/

    year = (int *) malloc(ntimein * sizeof(int));
    if (year == NULL) alloc_error(__FILE__, __LINE__);
    month = (int *) malloc(ntimein * sizeof(int));
    if (month == NULL) alloc_error(__FILE__, __LINE__);
    day = (int *) malloc(ntimein * sizeof(int));
    if (day == NULL) alloc_error(__FILE__, __LINE__);
    hour = (int *) malloc(ntimein * sizeof(int));
    if (hour == NULL) alloc_error(__FILE__, __LINE__);
    minutes = (int *) malloc(ntimein * sizeof(int));
    if (minutes == NULL) alloc_error(__FILE__, __LINE__);
    seconds = (float *) malloc(ntimein * sizeof(float));
    if (seconds == NULL) alloc_error(__FILE__, __LINE__);

    if (utIsInit() != TRUE)
      istat = utInit("");

    istat = utScan(tunits_in,  &dataunit_in);
    istat = utScan(tunits_out, &dataunit_out);
    for (t=0; t<ntimein; t++) {
      istat = utCalendar_cal(intimeval[t], &dataunit_in, &(year[t]), &(month[t]), &(day[t]), &(hour[t]), &(minutes[t]), &(seconds[t]),
                             cal_type);
#if DEBUG > 7
      istat = utInvCalendar_cal((year[t]), (month[t]), (day[t]), (hour[t]), (minutes[t]), (seconds[t]), &dataunit_in, &ccurtime, cal_type);
      printf("%s: %d %lf %lf %d %d %d %d %d %lf\n",__FILE__,t,intimeval[t],ccurtime,year[t],month[t],day[t],hour[t],minutes[t],seconds[t]);
#endif
      if (t > 0) {
        if ( ((intimeval[t] - intimeval[t-1]) * dataunit_in.factor) != 86400.0 ) {
          (void) fprintf(stderr,
                         "%s: Fatal error: only daily data can be an input. Found %d seconds between timesteps %d and %d! Aborting...\n",
                         __FILE__, (int) ((intimeval[t] - intimeval[t-1]) * dataunit_in.factor), t-1, t);          
          (void) abort();
        }
      }
    }

    /* For noleap/365-day or 360-day calendar types */
    if ( !strcmp(cal_type, "noleap") || !strcmp(cal_type, "365_day") || !strcmp(cal_type, "360_day") ) {

      /* Compute the new output total timesteps (days) in a standard year */
      /*
       * NB: The following specification gives both
       * the start time and the sampling interval (1 day). 
       */

      /* Set 1 day as a timestep to compute number of days in standard calendar */
      utstring = (char *) malloc(1000 * sizeof(char));
      if (utstring == NULL) alloc_error(__FILE__, __LINE__);
      (void) sprintf(utstring, "1 day since %d-%d-%d", year[0], month[0], day[0]);
      istat = utScan(utstring, &timeslice);
      (void) free(utstring);

      /* Set end period date */
      ref_year = year[ntimein-1];
      ref_month = month[ntimein-1];
      ref_day = day[ntimein-1];
      ref_hour = hour[ntimein-1];
      ref_minutes = 0;
      ref_seconds = 0.0;
    
      /* Get number of timesteps (days) */
      istat = utInvCalendar(ref_year, ref_month, ref_day, ref_hour, ref_minutes, ref_seconds, &timeslice, &val);
      *ntimeout = (int) val;

      /* Allocate memory */
      (*bufout) = (double *) malloc(ni*nj*(*ntimeout) * sizeof(double));
      if ( (*bufout) == NULL) alloc_error(__FILE__, __LINE__);
      (*outtimeval) = (double *) malloc((*ntimeout) * sizeof(double));
      if ( (*outtimeval) == NULL) alloc_error(__FILE__, __LINE__);

      /* Set start period date */
      ref_year = year[0];
      ref_month = month[0];
      ref_day = day[0];
      ref_hour = hour[0];
      ref_minutes = 0;
      ref_seconds = 0.0;
      /* Get start time units from date */
      istat = utInvCalendar(ref_year, ref_month, ref_day, ref_hour, ref_minutes, ref_seconds, &dataunit_out, &curtime);
      for (t=0; t<(*ntimeout); t++) {
        /* Get standard calendar date from output time units */
        istat = utCalendar(curtime, &dataunit_out, &cyear, &cmonth, &cday, &chour, &cminutes, &cseconds);
        /* Adjust hour, minutes and seconds to 00:00:00 */
        istat = utInvCalendar(cyear, cmonth, cday, 0, 0, 0.0, &dataunit_out, &curtime);
        /* Get corresponding time units in special calendar type */
        istat = utInvCalendar_cal(cyear, cmonth, cday, chour, cminutes, cseconds, &dataunit_in, &ccurtime, cal_type);
#if DEBUG > 7
        printf("%s: Original %s calendar type: %d %d %d %04d/%02d/%02d %02d:%02d\n", __FILE__, cal_type,
               (int) dataunit_in.origin, (int) dataunit_in.factor, (int) ccurtime, cyear, cmonth, cday, chour, cminutes);
#endif
        /* Find that time in the time vector */
        for (tt=0; tt<ntimein; tt++)
          if ((int) ccurtime == (int) intimeval[tt]) {
            for (j=0; j<nj; j++)
              for (i=0; i<ni; i++)
                (*bufout)[i+j*ni+t*ni*nj] = (double) bufin[i+j*ni+tt*ni*nj];
            (*outtimeval)[t] = (double) curtime;
            tt = ntimein+10;
          }
        if (tt == (ntimein+10)) {
          (void) fprintf(stderr, "%s: Cannot generate new time vector!! Algorithm internal error. Aborting.\n", __FILE__);
          (void) abort();
        }
#if DEBUG > 7
        printf("%s: Standard gregorian calendar type: %d %d %d %04d/%02d/%02d %02d:%02d\n", __FILE__,
               (int) dataunit_out.origin, (int) dataunit_out.factor, (int) curtime, cyear, cmonth, cday, chour, cminutes);
#endif

        curtime++;
      }
    }
    else {
      (void) fprintf(stderr, "%s: not-supported calendar. Sorry!\n", __FILE__);
      (void) free(year);
      (void) free(month);
      (void) free(day);
      (void) free(hour);
      (void) free(minutes);
      (void) free(seconds);
      (void) utTerm();
      return -1;
    }

    (void) free(year);
    (void) free(month);
    (void) free(day);
    (void) free(hour);
    (void) free(minutes);
    (void) free(seconds);
    (void) utTerm();  
  }

  return 0;
}

int data_to_gregorian_cal_f(float **bufout, double **outtimeval, int *ntimeout, float *bufin,
                            double *intimeval, char *tunits_in, char *tunits_out, char *cal_type, int ni, int nj, int ntimein) {
  utUnit timeslice;
  double val;
  double curtime;
  double ccurtime;
  
  int ref_year;
  int ref_month;
  int ref_day;
  int ref_hour;
  int ref_minutes;
  float ref_seconds;

  int t;
  int tt;
  int i;
  int j;
  int istat;

  char *utstring = NULL;
  utUnit dataunit_in;
  utUnit dataunit_out;

  int *year = NULL;
  int *month = NULL;
  int *day = NULL;
  int *hour = NULL;
  int *minutes = NULL;
  float *seconds = NULL;

  int cyear;
  int cmonth;
  int cday;
  int chour;
  int cminutes;
  float cseconds;

  *bufout = NULL;
  *outtimeval = NULL;
  *ntimeout = 0;

  if ( !strcmp(cal_type, "standard") || !strcmp(cal_type, "gregorian") ) {
    /** Just recopy the data when calendar type is standard or gregorian **/
    *ntimeout = ntimein;
    /* Allocate memory */
    (*bufout) = (float *) malloc(ni*nj*(*ntimeout) * sizeof(float));
    if ( (*bufout) == NULL) alloc_error(__FILE__, __LINE__);
    (*outtimeval) = (double *) malloc((*ntimeout) * sizeof(double));
    if ( (*outtimeval) == NULL) alloc_error(__FILE__, __LINE__);
    for (t=0; t<(*ntimeout); t++) {
      for (j=0; j<nj; j++)
        for (i=0; i<ni; i++)
          (*bufout)[i+j*ni+t*ni*nj] = (float) bufin[i+j*ni+t*ni*nj];
      (*outtimeval)[t] = (double) intimeval[t];
    }
  }
  else {
    /** Non-standard calendar type **/

    year = (int *) malloc(ntimein * sizeof(int));
    if (year == NULL) alloc_error(__FILE__, __LINE__);
    month = (int *) malloc(ntimein * sizeof(int));
    if (month == NULL) alloc_error(__FILE__, __LINE__);
    day = (int *) malloc(ntimein * sizeof(int));
    if (day == NULL) alloc_error(__FILE__, __LINE__);
    hour = (int *) malloc(ntimein * sizeof(int));
    if (hour == NULL) alloc_error(__FILE__, __LINE__);
    minutes = (int *) malloc(ntimein * sizeof(int));
    if (minutes == NULL) alloc_error(__FILE__, __LINE__);
    seconds = (float *) malloc(ntimein * sizeof(float));
    if (seconds == NULL) alloc_error(__FILE__, __LINE__);

    if (utIsInit() != TRUE)
      istat = utInit("");

    istat = utScan(tunits_in,  &dataunit_in);
    istat = utScan(tunits_out, &dataunit_out);
    for (t=0; t<ntimein; t++) {
      istat = utCalendar_cal(intimeval[t], &dataunit_in, &(year[t]), &(month[t]), &(day[t]), &(hour[t]), &(minutes[t]), &(seconds[t]),
                             cal_type);
#if DEBUG > 7
      istat = utInvCalendar_cal((year[t]), (month[t]), (day[t]), (hour[t]), (minutes[t]), (seconds[t]), &dataunit_in, &ccurtime, cal_type);
      printf("%s: %d %lf %lf %d %d %d %d %d %lf\n",__FILE__,t,intimeval[t],ccurtime,year[t],month[t],day[t],hour[t],minutes[t],seconds[t]);
#endif
      if (t > 0) {
        if ( ((intimeval[t] - intimeval[t-1]) * dataunit_in.factor) != 86400.0 ) {
          (void) fprintf(stderr,
                         "%s: Fatal error: only daily data can be an input. Found %d seconds between timesteps %d and %d! Aborting...\n",
                         __FILE__, (int) ((intimeval[t] - intimeval[t-1]) * dataunit_in.factor), t-1, t);          
          (void) abort();
        }
      }
    }

    /* For noleap/365-day or 360-day calendar types */
    if ( !strcmp(cal_type, "noleap") || !strcmp(cal_type, "365_day") || !strcmp(cal_type, "360_day") ) {

      /* Compute the new output total timesteps (days) in a standard year */
      /*
       * NB: The following specification gives both
       * the start time and the sampling interval (1 day). 
       */

      /* Set 1 day as a timestep to compute number of days in standard calendar */
      utstring = (char *) malloc(1000 * sizeof(char));
      if (utstring == NULL) alloc_error(__FILE__, __LINE__);
      (void) sprintf(utstring, "1 day since %d-%d-%d", year[0], month[0], day[0]);
      istat = utScan(utstring, &timeslice);
      (void) free(utstring);

      /* Set end period date */
      ref_year = year[ntimein-1];
      ref_month = month[ntimein-1];
      ref_day = day[ntimein-1];
      ref_hour = hour[ntimein-1];
      ref_minutes = 0;
      ref_seconds = 0.0;
    
      /* Get number of timesteps (days) */
      istat = utInvCalendar(ref_year, ref_month, ref_day, ref_hour, ref_minutes, ref_seconds, &timeslice, &val);
      *ntimeout = (int) val;

      /* Allocate memory */
      (*bufout) = (float *) malloc(ni*nj*(*ntimeout) * sizeof(float));
      if ( (*bufout) == NULL) alloc_error(__FILE__, __LINE__);
      (*outtimeval) = (double *) malloc((*ntimeout) * sizeof(double));
      if ( (*outtimeval) == NULL) alloc_error(__FILE__, __LINE__);

      /* Set start period date */
      ref_year = year[0];
      ref_month = month[0];
      ref_day = day[0];
      ref_hour = hour[0];
      ref_minutes = 0;
      ref_seconds = 0.0;
      /* Get start time units from date */
      istat = utInvCalendar(ref_year, ref_month, ref_day, ref_hour, ref_minutes, ref_seconds, &dataunit_out, &curtime);
      for (t=0; t<(*ntimeout); t++) {
        /* Get standard calendar date from output time units */
        istat = utCalendar(curtime, &dataunit_out, &cyear, &cmonth, &cday, &chour, &cminutes, &cseconds);
        /* Adjust hour, minutes and seconds to 00:00:00 */
        istat = utInvCalendar(cyear, cmonth, cday, 0, 0, 0.0, &dataunit_out, &curtime);
        /* Get corresponding time units in special calendar type */
        istat = utInvCalendar_cal(cyear, cmonth, cday, chour, cminutes, cseconds, &dataunit_in, &ccurtime, cal_type);
#if DEBUG > 7
        printf("%s: Original %s calendar type: %d %d %d %04d/%02d/%02d %02d:%02d\n", __FILE__, cal_type,
               (int) dataunit_in.origin, (int) dataunit_in.factor, (int) ccurtime, cyear, cmonth, cday, chour, cminutes);
#endif
        /* Find that time in the time vector */
        for (tt=0; tt<ntimein; tt++)
          if ((int) ccurtime == (int) intimeval[tt]) {
            for (j=0; j<nj; j++)
              for (i=0; i<ni; i++)
                (*bufout)[i+j*ni+t*ni*nj] = (float) bufin[i+j*ni+tt*ni*nj];
            (*outtimeval)[t] = (double) curtime;
            tt = ntimein+10;
          }
        if (tt == (ntimein+10)) {
          (void) fprintf(stderr, "%s: Cannot generate new time vector!! Algorithm internal error. Aborting.\n", __FILE__);
          (void) abort();
        }
#if DEBUG > 7
        printf("%s: Standard gregorian calendar type: %d %d %d %04d/%02d/%02d %02d:%02d\n", __FILE__,
               (int) dataunit_out.origin, (int) dataunit_out.factor, (int) curtime, cyear, cmonth, cday, chour, cminutes);
#endif

        curtime++;
      }
    }
    else {
      (void) fprintf(stderr, "%s: not-supported calendar. Sorry!\n", __FILE__);
      (void) free(year);
      (void) free(month);
      (void) free(day);
      (void) free(hour);
      (void) free(minutes);
      (void) free(seconds);
      (void) utTerm();
      return -1;
    }

    (void) free(year);
    (void) free(month);
    (void) free(day);
    (void) free(hour);
    (void) free(minutes);
    (void) free(seconds);
    (void) utTerm();
  }

  return 0;
}
