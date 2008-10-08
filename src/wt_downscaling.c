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

  int istat;

  /** Step 1: Read large-scale fields **/
  istat = read_large_scale_fields(data);
  if (istat != 0) return istat;

  /** Step 2: Compute climatologies **/
  istat = remove_clim(data);
  if (istat != 0) return istat;  

  /** Step 3: Project large scale fields on EOF **/
  
  /* Read EOFs and Singular Values */
  istat = read_large_scale_eof(data);
  if (istat != 0) return istat;
  
  /* Project field on EOF */
  //    istat = project_field_eof(psl_proj, psl_noclim, psl_eof_sub, psl_sing, fillvalue_eof, nlon_sub, nlat_sub, ntime, neof);
  
  return 0;
}
