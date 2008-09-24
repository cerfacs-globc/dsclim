/* ***************************************************** */
/* wt_downscaling Downscaling climate scenarios using    */
/* weather typing.                                       */
/* wt_downscaling.c                                      */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: sep 2008                            */
/* Last date of modification: sep 2008                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file wt_downscaling.c
    \brief Downscaling climate scenarios program using weather typing.
*/

#include <dsclim.h>

short int wt_downscaling(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  short int istat = 0;

  /** Step 1: Read large-scale fields **/
  istat = read_large_scale_fields(data);

  /** Step 2: Compute climatologies **/

  /* Remove seasonal cycle:
     - Fix calendar and generate a gregorian calendar
     - Compute climatology including Feb 29th
     - Filter climatology
     - Optionally save climatology in file */

  /*  (void) remove_seasonal_cycle(bufnoclim, clim, bufin, buftime, missing_value, data->conf->clim_filter_width,
                               data->conf->clim_filter_type, data->conf->clim_provided,
                               ni, nj, ntime);*/
  

  return 0;
}
