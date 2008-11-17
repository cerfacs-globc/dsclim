/* ***************************************************** */
/* get_time_attributes Get time NetCDF attributes.       */
/* get_time_attributes.c                                 */
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
/*! \file get_time_attributes.c
    \brief Get time NetCDF attributes.
*/

#include <io.h>

int get_time_attributes(char **time_units, char **cal_type, char *filename, char *varname) {

  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat;

  int ncinid;
  int timeinid;

  size_t t_len;

  /* Read data in NetCDF file */
  printf("%s: Opening for reading NetCDF input file %s.\n", __FILE__, filename);
  istat = nc_open(filename, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = nc_inq_varid(ncinid, varname, &timeinid);  /* get ID for time variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Get time units attribute length */
  istat = nc_inq_attlen(ncinid, timeinid, "units", &t_len);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  /* Allocate required space before retrieving values */
  (*time_units) = (char *) malloc((t_len+1) * sizeof(char));
  if ((*time_units) == NULL) alloc_error(__FILE__, __LINE__);
  /* Get time units attribute value */
  istat = nc_get_att_text(ncinid, timeinid, "units", *time_units);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  if ((*time_units)[t_len-2] == 'Z')
    (*time_units)[t_len-2] = '\0'; /* null terminate */
  else if ((*time_units)[t_len-1] == 'Z')
    (*time_units)[t_len-1] = '\0'; /* null terminate */
  else
    (*time_units)[t_len] = '\0';

  /* Get calendar type attribute length */
  istat = nc_inq_attlen(ncinid, timeinid, "calendar", &t_len);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  /* Allocate required space before retrieving values */
  (*cal_type) = (char *) malloc(t_len + 1);
  if ((*cal_type) == NULL) alloc_error(__FILE__, __LINE__);
  /* Get calendar type attribute value */
  istat = nc_get_att_text(ncinid, timeinid, "calendar", *cal_type);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (*cal_type)[t_len] = '\0'; /* null terminate */

  /** Close NetCDF files **/
  /* Close the intput netCDF file. */
  istat = ncclose(ncinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  return 0;
}
