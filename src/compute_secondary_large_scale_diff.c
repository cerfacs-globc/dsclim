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






#include <dsclim.h>

/** Compute secondary large-scale field difference between value of learning field at analog date vs model field at downscaled date. */
void
compute_secondary_large_scale_diff(double *delta, analog_day_struct analog_days, double *sup_field_index,
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
  //  double sup_diff; /**< Secondary large-scale field difference. */
  //  int count = 0;

  /* Process each downscaled day */
  for (t=0; t<ntimes; t++) {
    /** Compute normalized secondary large-scale field difference (delta) **/
    delta[t] = (sup_field_index[t] * sqrt(sup_field_var)) -
      (sup_field_index_learn[analog_days.tindex[t]] * sqrt(sup_field_var_learn));
    
    //    if (fabs(sup_diff) > 2.0) {
    //      delta[t] = sup_diff;
      //      printf("%d %lf\n",t,delta[t]);
      //      count++;
    //    }
    //    else
    //      delta[t] = 0.0;
    //    if (t == (ntimes-1))
    //      printf("%d %lf %lf %lf %d %d %d\n",count,delta[t],sup_field_index[t],sup_field_index_learn[analog_days.tindex[t]],analog_days.year[t],analog_days.month[t],analog_days.day[t]);

  }
}
