/* ***************************************************** */
/* Merge seasonal data.                                  */
/* merge_seasons.c                                       */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file merge_seasons.c
    \brief Merge seasonal data.
*/

#include <dsclim.h>

/** Merge seasonal data */
int merge_seasons(analog_day_struct analog_days_merged, analog_day_struct analog_days, int ntimes_merged, int ntimes) {
  /**
     @param[out]  analog_days_merged    Analog days time indexes and dates with corresponding dates being downscaled, all seasons merged
     @param[in]   analog_days           Analog days time indexes and dates with corresponding dates being downscaled
     @param[in]   ntimes_merged         Number of times of days to downscale for this period, all seasons merged
     @param[in]   ntimes                Number of times of days to downscale for this period
  */
  
  int t; /* Time loop counter */
  int curindex_merged; /* Current index in the merged season vector */

  /* Process each downscaled day for a specific season subperiod */
  for (t=0; t<ntimes; t++) {
    /* Retrieve current index */
    curindex_merged = analog_days.tindex_s_all[t];
    /* Check for bounds */
    if (curindex_merged < 0 || curindex_merged >= ntimes_merged) {
      (void) fprintf(stderr, "%s: Fatal error: index in merged season vector outside bounds! curindex_merged=%d max=%d\n",
                     __FILE__, curindex_merged, ntimes_merged-1);
      return -1;
    }
    /* Retrieve values */
    analog_days_merged.tindex_s_all[curindex_merged] = analog_days.tindex_all[t];
    analog_days_merged.year_s[curindex_merged] = analog_days.year[t];
    analog_days_merged.month_s[curindex_merged] = analog_days.month[t];
    analog_days_merged.day_s[curindex_merged] = analog_days.day[t];
  }
  
  /* Success status */
  return 0;
}
