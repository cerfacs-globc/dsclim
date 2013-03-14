/* ***************************************************** */
/* compute_time_info Compute time info from NetCDF time. */
/* compute_time_info.c                                   */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: oct 2008                            */
/* Last date of modification: jul 2011                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision: 1.1                                 */
/* ***************************************************** */
/* Revisions                                             */
/* 1.1: Updated for utCalendar2_cal (udunits2)           */
/* ***************************************************** */
/*! \file compute_time_info.c
    \brief Compute time info from NetCDF time.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2012)

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




#include <io.h>

/** Compute time info from NetCDF time. */
int
compute_time_info(time_vect_struct *time_s, double *timeval, char *time_units, char *cal_type, int ntime) {

  /**
     @param[out]  time_s      Time field in time structure
     @param[in]   timeval     Time field
     @param[in]   time_units  Time units (udunits)
     @param[in]   cal_type    Calendar type (udunits)
     @param[in]   ntime       Time dimension
     
     \return           Status.
  */

  int istat; /* Diagnostic status */
  ut_system *unitSystem = NULL; /* Unit System (udunits) */
  ut_unit *dataunits = NULL; /* Data units (udunits) */
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
  time_s->seconds = (double *) malloc(ntime * sizeof(double));
  if (time_s->seconds == NULL) alloc_error(__FILE__, __LINE__);

  /* Initialize udunits */
  ut_set_error_message_handler(ut_ignore);
  unitSystem = ut_read_xml(NULL);
  ut_set_error_message_handler(ut_write_to_stderr);

  dataunits = ut_parse(unitSystem, time_units, UT_ASCII);
  for (t=0; t<ntime; t++) {
    istat = utCalendar2_cal(timeval[t], dataunits, &(time_s->year[t]), &(time_s->month[t]), &(time_s->day[t]),
                            &(time_s->hour[t]), &(time_s->minutes[t]), &(time_s->seconds[t]), cal_type);
    if (istat < 0) {
      (void) ut_free(dataunits);
      (void) ut_free_system(unitSystem);  
      return -1;
    }
  }

  (void) ut_free(dataunits);
  (void) ut_free_system(unitSystem);  

  /* Success status */
  return 0;
}
