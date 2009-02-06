/* ***************************************************** */
/* Merge seasonal 3D field data usign analog day         */
/* structure.                                            */
/* merge_seasonal_data.c                                 */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file merge_seasonal_data.c
    \brief Merge seasonal 3D field data using analog day structure.
*/

#include <dsclim.h>

/** Merge seasonal 3D field data using analog day structure. */
int merge_seasonal_data(double *buf_merged, double *buf, analog_day_struct analog_days, int dimx, int dimy,
                        int ntimes_merged, int ntimes) {
  /**
     @param[out]  buf_merged            3D field dimx X dimy X ntimes_merged
     @param[in]   buf                   3D field dimx X dimy X ntimes
     @param[in]   analog_days           Analog days time indexes and dates with corresponding dates being downscaled (non-merged)
     @param[in]   dimx                  X dimension
     @param[in]   dimy                  Y dimension
     @param[in]   ntimes_merged         Number of times of days to downscale for this period, all seasons merged
     @param[in]   ntimes                Number of times of days to downscale for this period
  */
  
  int t; /* Time loop counter */
  int i; /* Loop counter */
  int j; /* Loop counter */
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
    for (i=0; i<dimx; i++)
      for (j=0; j<dimy; j++)
        buf_merged[i+j*dimx+curindex_merged*dimx*dimy] = buf[i+j*dimx+t*dimx*dimy];
  }
  
  /* Success status */
  return 0;
}
