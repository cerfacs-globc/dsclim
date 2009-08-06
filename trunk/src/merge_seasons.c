/* ***************************************************** */
/* Merge seasonal analog date data.                      */
/* merge_seasons.c                                       */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file merge_seasons.c
    \brief Merge seasonal analog date data.
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

#include <dsclim.h>

/** Merge seasonal analog date data */
int
merge_seasons(analog_day_struct analog_days_merged, analog_day_struct analog_days, int ntimes_merged, int ntimes) {
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
    analog_days_merged.tindex_all[curindex_merged] = analog_days.tindex_all[t];
    analog_days_merged.year[curindex_merged] = analog_days.year[t];
    analog_days_merged.month[curindex_merged] = analog_days.month[t];
    analog_days_merged.day[curindex_merged] = analog_days.day[t];

    analog_days_merged.tindex_s_all[curindex_merged] = analog_days.tindex_s_all[t];
    analog_days_merged.year_s[curindex_merged] = analog_days.year_s[t];
    analog_days_merged.month_s[curindex_merged] = analog_days.month_s[t];
    analog_days_merged.day_s[curindex_merged] = analog_days.day_s[t];

  }

  /* Success status */
  return 0;
}
