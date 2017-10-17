/* ***************************************************** */
/* Extract a sub period of a vector of selected months.  */
/* extract_subperiod.c                                   */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file extract_subperiod_months.c
    \brief Extract a sub period of a vector of selected months.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2015)

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

/** Extract a sub period of a vector of selected months. */
void
extract_subperiod_months(double **buf_sub, int *ntime_sub, double *bufin, int *year, int *month, int *day,
                         char *time_units, char *cal_type, period_struct *period,
                         int *smonths, int timedim, double *time_ls, int ndima, int ndimb, int ntime, int nmonths) {
  /**
     @param[out] buf_sub       3D buffer spanning only time subperiod
     @param[out] ntime_sub     Number of times in subperiod
     @param[in]  bufin         3D input buffer
     @param[in]  year          Year vector
     @param[in]  month         Month vector
     @param[in]  day           Day vector
     @param[in]  smonths       Selected months vector (values 1-12)
     @param[in]  time_units    Output base time units
     @param[in]  cal_type      Output calendar-type
     @param[in]  period        Period structure for downscaling output
     @param[in]  timedim       Time dimension position (1 or 3)
     @param[in]  time_ls       Time values
     @param[in]  ndima         First dimension length
     @param[in]  ndimb         Second dimension length
     @param[in]  ntime         Time dimension length
     @param[in]  nmonths       Number of months in smonths vector
   */
  
  int *buf_sub_i = NULL; /* Temporary buffer */

  int i; /* Loop counter */
  int j; /* Loop counter */
  int t; /* Time loop counter */
  int tt; /* Time subperiod loop counter */

  int istat; /* Diagnostic status */

  ut_system *unitSystem = NULL; /* Unit System (udunits) */
  ut_unit *dataunits = NULL; /* udunits variable */

  double period_begin;
  double period_end;

  int year;
  int month;
  int day;
  int hour;
  int minutes;
  double seconds;

  /* Initializing */
  *ntime_sub = 0;
  
  /* Initialize udunits */
  ut_set_error_message_handler(ut_ignore);
  unitSystem = ut_read_xml(NULL);
  ut_set_error_message_handler(ut_write_to_stderr);
  dataunits = ut_parse(unitSystem, time_units, UT_ASCII);

  /* Compute time limits for writing */
  if (period->year_begin != -1) {
    (void) printf("%s: Analog output from %02d/%02d/%04d to %02d/%02d/%04d inclusively.\n", __FILE__,
                  period->month_begin, period->day_begin, period->year_begin,
                  period->month_end, period->day_end, period->year_end);
    istat = utInvCalendar2(period->year_begin, period->month_begin, period->day_begin, 0, 0, 0.0, dataunits, &period_begin);
    istat = utInvCalendar2(period->year_end, period->month_end, period->day_end, 23, 59, 0.0, dataunits, &period_end);
  }
  else {
    istat = utCalendar2(time_ls[0], dataunits, &year, &month, &day, &hour, &minutes, &seconds);
    (void) printf("%s: Analog for the whole period: %02d/%02d/%04d", __FILE__, month, day, year);
    istat = utCalendar2(time_ls[ntime-1], dataunits, &year, &month, &day, &hour, &minutes, &seconds);
    (void) printf(" to %02d/%02d/%04d inclusively.\n", month, day, year);
    period_begin = time_ls[0];
    period_end = time_ls[ntime-1];
  }

  /* Retrieve time index spanning selected months */
  for (t=0; t<ntime; t++)
    if (time_ls[t] >= period_begin && time_ls[t] <= period_end)
      for (tt=0; tt<nmonths; tt++)
        if (month[t] == smonths[tt]) {
          buf_sub_i = (int *) realloc(buf_sub_i, ((*ntime_sub)+1) * sizeof(int));
          if (buf_sub_i == NULL) alloc_error(__FILE__, __LINE__);
          buf_sub_i[(*ntime_sub)++] = t;
        }
  
  /* Allocate memory */
  (*buf_sub) = (double *) malloc((*ntime_sub)*ndima*ndimb * sizeof(double));
  if ((*buf_sub) == NULL) alloc_error(__FILE__, __LINE__);

  /* Construct new 3D buffer */
  if (timedim == 3)
    /* Time dimension is the last one */
    for (t=0; t<(*ntime_sub); t++)
      for (j=0; j<ndimb; j++)
        for (i=0; i<ndima; i++)
          (*buf_sub)[i+j*ndima+t*ndima*ndimb] = bufin[i+j*ndima+buf_sub_i[t]*ndima*ndimb];
  else
    /* Time dimension is the first one */
    for (t=0; t<(*ntime_sub); t++)
      for (j=0; j<ndimb; j++)
        for (i=0; i<ndima; i++)
          (*buf_sub)[t+i*(*ntime_sub)+j*(*ntime_sub)*ndima] = bufin[buf_sub_i[t]+i*ntime+j*ntime*ndima];
  
  /* Free memory */
  (void) free(buf_sub_i);
}
