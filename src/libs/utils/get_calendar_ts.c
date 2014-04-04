/* ***************************************************** */
/* Get year,month,day,hour,min,sec given time in udunits */
/* get_calendar_ts.c                                     */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file get_calendar_ts.c
    \brief Get year,month,day,hour,min,sec (time structure) given time in udunits.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2014)

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

/** Get year,month,day,hour,min,sec (time structure) given time in udunits. */
int
get_calendar_ts(tstruct *timeout, char *tunits, double *timein, int ntime) {

  /** 
      @param[out]  timeout    Time structure vector
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
  for (t=0; t<ntime; t++) {
    istat = utCalendar(timein[t], &dataunit, &(timeout[t].year), &(timeout[t].month), &(timeout[t].day), &(timeout[t].hour), &(timeout[t].min), &(timeout[t].sec));
    if (istat < 0) {
      (void) utTerm();
      return -1;
    }
  }

  /* Terminate udunits */
  (void) utTerm();

  /* Success status */
  return 0;
}
