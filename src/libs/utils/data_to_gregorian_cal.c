/* ***************************************************** */
/* Convert 2D data stored in 360-days or no-leap         */
/* calendars to standard Gregorian calendar.             */
/* data_to_gregorian_cal.c                               */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file data_to_gregorian_cal.c
    \brief Convert 360-days or no-leap calendar to standard Gregorian calendar.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2009)

christian.page@cerfacs.fr

This software is a computer program whose purpose is to downscale climate
scenarios using a statistical methodology based on weather regimes.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software. You can use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty and the software's author, the holder of the
economic rights, and the successive licensors have only limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading, using, modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean that it is complicated to manipulate, and that also
therefore means that it is reserved for developers and experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and, more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.

LICENSE END */

#include <utils.h>

/** Convert 360-days or no-leap calendar to standard Gregorian calendar for double input/output buffer. */
int
data_to_gregorian_cal_d(double **bufout, double **outtimeval, int *ntimeout, double *bufin,
                        double *intimeval, char *tunits_in, char *tunits_out, char *cal_type, int ni, int nj, int ntimein) {
  /**
     @param[out] bufout        Output 3D buffer which have been adjusted to standard Gregorian Calendar
     @param[out] outtimeval    Output new time vector adjusted to standard Gregorian Calendar
     @param[out] ntimeout      Number of times in new output 3D buffer
     @param[in]  bufin         Input 3D buffer with non-standard calendar
     @param[in]  intimeval     Input time vector with non-standard calendar
     @param[in]  tunits_in     Input time units with non-standard calendar
     @param[out] tunits_out    Output time units
     @param[in]  cal_type      Input calendar type (non-standard calendar)
     @param[in]  ni            First dimension length
     @param[in]  nj            Second dimension length
     @param[in]  ntimein       Input time dimension length with non-standard calendar
   */

  utUnit timeslice; /* Time slicing used to compute the new standard calendar */
  double val; /* Temporary value */
  double curtime; /* Current time */
  double ccurtime; /* Current time in non-standard calendar */
  
  int ref_year; /* A given year */
  int ref_month; /* A given month */
  int ref_day; /* A given day */
  int ref_hour; /* A given hour */
  int ref_minutes; /* A given minute */
  float ref_seconds; /* A given second */

  int t; /* Time loop counter */
  int tt; /* Time loop counter */
  int i; /* Loop counter */
  int j; /* Loop counter */
  int istat; /* Diagnostic status */

  char *utstring = NULL; /* Time unit string */
  utUnit dataunit_in; /* Input data unit (udunits) */
  utUnit dataunit_out; /* Output data unit (udunits) */

  int *year = NULL; /* Year time vector */
  int *month = NULL; /* Month time vector */
  int *day = NULL; /* Day time vector */
  int *hour = NULL; /* Hour time vector */
  int *minutes = NULL; /* Minutes time vector */
  float *seconds = NULL; /* Seconds time vector */

  int cyear; /* A given year */
  int cmonth; /* A given month */
  int cday; /* A given day */
  int chour; /* A given hour */
  int cminutes; /* A given minute */
  float cseconds; /* A given second */

  /* Initializing */
  *bufout = NULL;
  *outtimeval = NULL;
  *ntimeout = 0;

  /** Just recopy the data when calendar type is standard or gregorian **/
  if ( !strcmp(cal_type, "standard") || !strcmp(cal_type, "gregorian") ) {
    *ntimeout = ntimein;
    /* Allocate memory */
    (*bufout) = (double *) malloc(ni*nj*(*ntimeout) * sizeof(double));
    if ( (*bufout) == NULL) alloc_error(__FILE__, __LINE__);
    (*outtimeval) = (double *) malloc((*ntimeout) * sizeof(double));
    if ( (*outtimeval) == NULL) alloc_error(__FILE__, __LINE__);

    /* Loop over all times and gridpoints */
    for (t=0; t<(*ntimeout); t++) {
      for (j=0; j<nj; j++)
        for (i=0; i<ni; i++)
          /* Get value */
          (*bufout)[i+j*ni+t*ni*nj] = (double) bufin[i+j*ni+t*ni*nj];
      /* Construct time vector */
      (*outtimeval)[t] = (double) intimeval[t];
    }
  }
  else {
    /** Non-standard calendar type **/

    /* Allocate memory */
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

    /* Initialize udunits */
    if (utIsInit() != TRUE)
      istat = utInit("");

    /* Generate time units strings */
    istat = utScan(tunits_in,  &dataunit_in);
    istat = utScan(tunits_out, &dataunit_out);

    /* Loop over all times */
    for (t=0; t<ntimein; t++) {
      /* Calculate date using non-standard calendar */
      istat = utCalendar_cal(intimeval[t], &dataunit_in, &(year[t]), &(month[t]), &(day[t]), &(hour[t]), &(minutes[t]), &(seconds[t]),
                             cal_type);
      if (istat < 0) {
        (void) free(year);
        (void) free(month);
        (void) free(day);
        (void) free(hour);
        (void) free(minutes);
        (void) free(seconds);
        (void) utTerm();
        return -1;
      }
#if DEBUG > 7
      istat = utInvCalendar_cal((year[t]), (month[t]), (day[t]), (hour[t]), (minutes[t]), (seconds[t]), &dataunit_in, &ccurtime, cal_type);
      printf("%s: %d %lf %lf %d %d %d %d %d %lf\n",__FILE__,t,intimeval[t],ccurtime,year[t],month[t],day[t],hour[t],minutes[t],seconds[t]);
      if (istat < 0) {
        (void) free(year);
        (void) free(month);
        (void) free(day);
        (void) free(hour);
        (void) free(minutes);
        (void) free(seconds);
        (void) utTerm();
        return -1;
      }
#endif
      /* Check that we really have daily data */
      if (t > 0) {
        if ( ((intimeval[t] - intimeval[t-1]) * dataunit_in.factor) != 86400.0 ) {
          (void) fprintf(stderr,
                         "%s: Fatal error: only daily data can be an input. Found %d seconds between timesteps %d and %d!\n",
                         __FILE__, (int) ((intimeval[t] - intimeval[t-1]) * dataunit_in.factor), t-1, t);          
          (void) free(year);
          (void) free(month);
          (void) free(day);
          (void) free(hour);
          (void) free(minutes);
          (void) free(seconds);
          (void) utTerm();
          return -10;
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

      /* Loop over all times */
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
        /* Find that time in the input time vector */
        for (tt=0; tt<ntimein; tt++)
          if ((int) ccurtime == (int) intimeval[tt]) {
            /* Found it */
            for (j=0; j<nj; j++)
              for (i=0; i<ni; i++)
                (*bufout)[i+j*ni+t*ni*nj] = (double) bufin[i+j*ni+tt*ni*nj];
            /* Construct new time vector */
            (*outtimeval)[t] = (double) curtime;
            /* Exit loop */
            tt = ntimein+10;
          }
        if (tt != (ntimein+10)) {
          /* We didn't found the time in the input time vector... */
          (void) fprintf(stderr, "%s: Cannot generate new time vector!! Algorithm internal error!\n", __FILE__);
          (void) free(year);
          (void) free(month);
          (void) free(day);
          (void) free(hour);
          (void) free(minutes);
          (void) free(seconds);
          (void) utTerm();
          return -11;
        }
#if DEBUG > 7
        printf("%s: Standard gregorian calendar type: %d %d %d %04d/%02d/%02d %02d:%02d\n", __FILE__,
               (int) dataunit_out.origin, (int) dataunit_out.factor, (int) curtime, cyear, cmonth, cday, chour, cminutes);
#endif

        curtime++;
      }
    }
    else {
      /* Non-supported calendar */
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

  /* Success status */
  return 0;
}

/** Convert 360-days or no-leap calendar to standard Gregorian calendar for float input/output buffer. */
int
data_to_gregorian_cal_f(float **bufout, double **outtimeval, int *ntimeout, float *bufin,
                        double *intimeval, char *tunits_in, char *tunits_out, char *cal_type, int ni, int nj, int ntimein) {
  /**
     @param[out] bufout        Output 3D buffer which have been adjusted to standard Gregorian Calendar
     @param[out] outtimeval    Output new time vector adjusted to standard Gregorian Calendar
     @param[out] ntimeout      Number of times in new output 3D buffer
     @param[in]  bufin         Input 3D buffer with non-standard calendar
     @param[in]  intimeval     Input time vector with non-standard calendar
     @param[in]  tunits_in     Input time units with non-standard calendar
     @param[out] tunits_out    Output time units
     @param[in]  cal_type      Input calendar type (non-standard calendar)
     @param[in]  ni            First dimension length
     @param[in]  nj            Second dimension length
     @param[in]  ntimein       Input time dimension length with non-standard calendar
   */

  utUnit timeslice; /* Time slicing used to compute the new standard calendar */
  double val; /* Temporary value */
  double curtime; /* Current time */
  double ccurtime; /* Current time in non-standard calendar */
  
  int ref_year; /* A given year */
  int ref_month; /* A given month */
  int ref_day; /* A given day */
  int ref_hour; /* A given hour */
  int ref_minutes; /* A given minute */
  float ref_seconds; /* A given second */

  int t; /* Time loop counter */
  int tt; /* Time loop counter */
  int i; /* Loop counter */
  int j; /* Loop counter */
  int istat; /* Diagnostic status */

  char *utstring = NULL; /* Time unit string */
  utUnit dataunit_in; /* Input data unit (udunits) */
  utUnit dataunit_out; /* Output data unit (udunits) */

  int *year = NULL; /* Year time vector */
  int *month = NULL; /* Month time vector */
  int *day = NULL; /* Day time vector */
  int *hour = NULL; /* Hour time vector */
  int *minutes = NULL; /* Minutes time vector */
  float *seconds = NULL; /* Seconds time vector */

  int cyear; /* A given year */
  int cmonth; /* A given month */
  int cday; /* A given day */
  int chour; /* A given hour */
  int cminutes; /* A given minute */
  float cseconds; /* A given second */

  /* Initializing */
  *bufout = NULL;
  *outtimeval = NULL;
  *ntimeout = 0;

  /** Just recopy the data when calendar type is standard or gregorian **/
  if ( !strcmp(cal_type, "standard") || !strcmp(cal_type, "gregorian") ) {
    *ntimeout = ntimein;
    /* Allocate memory */
    (*bufout) = (float *) malloc(ni*nj*(*ntimeout) * sizeof(float));
    if ( (*bufout) == NULL) alloc_error(__FILE__, __LINE__);
    (*outtimeval) = (double *) malloc((*ntimeout) * sizeof(double));
    if ( (*outtimeval) == NULL) alloc_error(__FILE__, __LINE__);

    /* Loop over all times and gridpoints */
    for (t=0; t<(*ntimeout); t++) {
      for (j=0; j<nj; j++)
        for (i=0; i<ni; i++)
          /* Get value */
          (*bufout)[i+j*ni+t*ni*nj] = (float) bufin[i+j*ni+t*ni*nj];
      /* Construct time vector */
      (*outtimeval)[t] = (double) intimeval[t];
    }
  }
  else {
    /** Non-standard calendar type **/

    /* Allocate memory */
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

    /* Initialize udunits */
    if (utIsInit() != TRUE)
      istat = utInit("");

    /* Generate time units strings */
    istat = utScan(tunits_in,  &dataunit_in);
    istat = utScan(tunits_out, &dataunit_out);

    /* Loop over all times */
    for (t=0; t<ntimein; t++) {
      /* Calculate date using non-standard calendar */
      istat = utCalendar_cal(intimeval[t], &dataunit_in, &(year[t]), &(month[t]), &(day[t]), &(hour[t]), &(minutes[t]), &(seconds[t]),
                             cal_type);
      if (istat < 0) {
        (void) free(year);
        (void) free(month);
        (void) free(day);
        (void) free(hour);
        (void) free(minutes);
        (void) free(seconds);
        (void) utTerm();
        return -1;
      }
#if DEBUG > 7
      istat = utInvCalendar_cal((year[t]), (month[t]), (day[t]), (hour[t]), (minutes[t]), (seconds[t]), &dataunit_in, &ccurtime, cal_type);
      printf("%s: %d %lf %lf %d %d %d %d %d %lf\n",__FILE__,t,intimeval[t],ccurtime,year[t],month[t],day[t],hour[t],minutes[t],seconds[t]);
      if (istat < 0) {
        (void) free(year);
        (void) free(month);
        (void) free(day);
        (void) free(hour);
        (void) free(minutes);
        (void) free(seconds);
        (void) utTerm();
        return -1;
      }
#endif
      /* Check that we really have daily data */
      if (t > 0) {
        if ( ((intimeval[t] - intimeval[t-1]) * dataunit_in.factor) != 86400.0 ) {
          (void) fprintf(stderr,
                         "%s: Fatal error: only daily data can be an input. Found %d seconds between timesteps %d and %d!\n",
                         __FILE__, (int) ((intimeval[t] - intimeval[t-1]) * dataunit_in.factor), t-1, t);          
          (void) free(year);
          (void) free(month);
          (void) free(day);
          (void) free(hour);
          (void) free(minutes);
          (void) free(seconds);
          (void) utTerm();
          return -10;
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

      /* Loop over all times */
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
            /* Found it */
            for (j=0; j<nj; j++)
              for (i=0; i<ni; i++)
                (*bufout)[i+j*ni+t*ni*nj] = (float) bufin[i+j*ni+tt*ni*nj];
            /* Construct new time vector */
            (*outtimeval)[t] = (double) curtime;
            /* Exit loop */
            tt = ntimein+10;
          }
        if (tt != (ntimein+10)) {
          /* We didn't found the time in the input time vector... */
          (void) fprintf(stderr, "%s: Cannot generate new time vector!! Algorithm internal error!\n", __FILE__);
          (void) free(year);
          (void) free(month);
          (void) free(day);
          (void) free(hour);
          (void) free(minutes);
          (void) free(seconds);
          (void) utTerm();
          return -11;
        }
#if DEBUG > 7
        printf("%s: Standard gregorian calendar type: %d %d %d %04d/%02d/%02d %02d:%02d\n", __FILE__,
               (int) dataunit_out.origin, (int) dataunit_out.factor, (int) curtime, cyear, cmonth, cday, chour, cminutes);
#endif

        curtime++;
      }
    }
    else {
      /* Non-supported calendar */
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

  /* Success status */
  return 0;
}
