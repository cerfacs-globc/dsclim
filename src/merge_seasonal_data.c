/* ***************************************************** */
/* Merge seasonal 3D double field data usign analog day  */
/* structure.                                            */
/* merge_seasonal_data.c                                 */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file merge_seasonal_data.c
    \brief Merge seasonal 3D double field data using analog day structure.
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




#include <dsclim.h>

/** Merge seasonal 3D double field data using analog day structure. */
int
merge_seasonal_data(double *buf_merged, double *buf, analog_day_struct analog_days, int *merged_itimes, int dimx, int dimy,
                    int ntimes_merged, int ntimes) {
  /**
     @param[out]  buf_merged            3D field dimx X dimy X ntimes_merged
     @param[in]   buf                   3D field dimx X dimy X ntimes
     @param[in]   analog_days           Analog days time indexes and dates with corresponding dates being downscaled (non-merged)
     @param[in]   merged_itimes         Time index of total time vector against current merged time vector that could span less than the whole year
     @param[in]   dimx                  X dimension
     @param[in]   dimy                  Y dimension
     @param[in]   ntimes_merged         Number of times of days to downscale for this period, all seasons merged
     @param[in]   ntimes                Number of times of days to downscale for this period
  */
  
  int t; /* Time loop counter */
  int i; /* Loop counter */
  int j; /* Loop counter */
  int curindex; /* Current index in the merged times vector */
  int index_all; /* Current index in the whole time vector */

  /* Process each downscaled day for a specific season subperiod */
  for (t=0; t<ntimes; t++) {
    /* Index of season-specific time into ntime_ls whole time vector */
    index_all = analog_days.tindex_s_all[t];
    /* Retrieve index in merge time vector from index of whole time vector ntime_ls */
    curindex = merged_itimes[index_all];
    /* Retrieve values */
    for (i=0; i<dimx; i++)
      for (j=0; j<dimy; j++)
        buf_merged[i+j*dimx+curindex*dimx*dimy] = buf[i+j*dimx+t*dimx*dimy];
  }
  
  /* Success status */
  return 0;
}
