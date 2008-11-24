/* ***************************************************** */
/* Compute secondary large-scale field difference        */
/* between value of learning field at analog date vs     */
/* model field at downscaled date.                       */
/* compute_secondary_large_scale_diff.c                  */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file compute_secondary_large_scale_diff.c
    \brief Compute secondary large-scale field difference between value of learning field at analog date vs model field at downscaled date.
*/

#include <dsclim.h>

/** Compute secondary large-scale field difference between value of learning field at analog date vs model field at downscaled date. */
void compute_secondary_large_scale_diff(double *delta, analog_day_struct analog_days, double *sup_field_index,
                                        double *sup_field_index_learn, double sup_field_var, double sup_field_var_learn, int ntimes) {
  /**
     @param[out]  delta                 Difference between value of secondary large-scale learning field at analog date vs model field at downscaled date
     @param[in]   analog_days           Analog days time indexes and dates with corresponding dates being downscaled
     @param[in]   sup_field_index       Secondary large-scale field index of days to downscale
     @param[in]   sup_field_index_learn Secondary large-scale field index of learning period
     @param[in]   sup_field_var         Secondary large-scale field index variance of days to downscale
     @param[in]   sup_field_var_learn   Secondary large-scale field index variance of learning period
     @param[in]   ntimes                Number of times of days to downscale for this period
  */
  
  int t; /* Time loop counter */
  double sup_diff; /**< Secondary large-scale field difference. */

  /* Process each downscaled day */
  for (t=0; t<ntimes; t++) {
    /** Compute normalized secondary large-scale field difference (delta) **/
    sup_diff = (sup_field_index[t] * sqrt(sup_field_var)) -
                (sup_field_index_learn[analog_days.tindex[t]] * sqrt(sup_field_var_learn));
    if (fabs(sup_diff) > 2.0)
      delta[t] = sup_diff;
    else
      delta[t] = 0.0;
  }
}
