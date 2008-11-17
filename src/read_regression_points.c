/* ***************************************************** */
/* read_regression_points Read regression                */
/* point positions.                                      */
/* read_regression_points.c                              */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: oct 2008                            */
/* Last date of modification: oct 2008                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file read_regression_points.c
    \brief Read regression point positions. 
*/

#include <dsclim.h>

int read_regression_points(reg_struct *reg) {
  /**
     @param[in]  data  MASTER regression structure.
     
     \return           Status.
  */

  int npts;
  int istat;

  istat = read_netcdf_var_generic_1d(&(reg->lat), (info_field_struct *) NULL, reg->filename, reg->latname, reg->ptsname, &npts);
  if (istat != 0) {
    (void) free(reg->lat);
    return istat;
  }

  istat = read_netcdf_var_generic_1d(&(reg->lon), (info_field_struct *) NULL, reg->filename, reg->lonname, reg->ptsname, &(reg->npts));
  if (istat != 0 || npts != reg->npts) {
    (void) free(reg->lat);
    (void) free(reg->lon);
    return istat;
  }

  (void) fprintf(stdout, "%s: %d regression point positions read successfully.\n", __FILE__, reg->npts);

  return 0;
}
