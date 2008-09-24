/* ***************************************************** */
/* handle_netcdf_error Handle NetCDF errors.             */
/* handle_netcdf_error.c                                 */
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
/*! \file handle_netcdf_error.c
    \brief Handle NetCDF errors.
*/

#include <io.h>

/* Handle NetCDF error */
void handle_netcdf_error(int status, int lineno)
{
  /**
     @param[in]  status  Return status error code.
     @param[in]  lineno  Line number of the error.
  */

  if (status != NC_NOERR) {
    fprintf(stderr, "Line: %d Error %d: %s\n", lineno, status, nc_strerror(status));
    exit(-1);
  }
}
