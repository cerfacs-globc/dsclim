/* ***************************************************** */
/* Compute day of year of 366-day climatological year    */
/* given a day and a month.                              */
/* dayofclimyear.c                                       */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file dayofclimyear.c
    \brief Compute day of year of 366-day climatological year given a day and a month.
*/

#include <clim.h>

/** Compute day of year of 366-day climatological year given a day and a month. */
short int dayofclimyear(short int day, short int month) {
  /**
     @param[in]      day           Day of the month.
     @param[in]      month         Month of the year.
  */
  short int daysinmonth[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  short int m;
  short int dayofclimy = 0;
  
  for (m=0; m<=(month-2); m++)
    dayofclimy += daysinmonth[m];
  dayofclimy += day;

  return dayofclimy;
}
