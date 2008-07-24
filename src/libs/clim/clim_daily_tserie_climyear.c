/* ***************************************************** */
/* Compute daily climatology for climatological year     */
/* clim_daily_tserie_climyear.c                          */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file clim_daily_tserie_climyear.c
    \brief Compute daily climatology for climatological year.
*/

#include <clim.h>

void clim_daily_tserie_climyear(double *bufout, double *bufin, tstruct *buftime, double missing_val, int ntime) {

  short int *index = NULL;
  double mean = 0.0;
  double sum = 0.0;
  short int ndays = 0;
  short int month;

  int nt;
  int day;

  index = (int *) calloc(ntime, sizeof(short int));
  if (index == NULL) alloc_error(__FILE__, __LINE__);

  for (month=1; month<=12; month++) {

    for (day=1; day<=31; day++) {

      ndays = 0;

      for (nt=0; nt<ntime; nt++) {
        sum = 0.0;
        index[nt] = 0;
        if (buftime[nt].day == day && buftime[nt].month == month) {
          index[nt] = 1;
          if (bufin[nt] != missing_val) {
            sum += bufin[nt];
            ndays++;
          }
        }
      }
      if (ndays > 0) {
        mean = sum / (double) ndays;
        for (nt=0; nt<ntime; nt++)
          if (index[nt] == 1)
            bufout[nt] = mean;
      }
    }
  }

  (void) free(index);
}
