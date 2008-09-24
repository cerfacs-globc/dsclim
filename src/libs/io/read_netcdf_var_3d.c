/* ***************************************************** */
/* read_netcdf_var_3d Read a 3D NetCDF variable.         */
/* read_netcdf_var_3d.c                                  */
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
/*! \file read_netcdf_var_3d.c
    \brief Read a NetCDF variable.
*/

#include <io.h>

short int read_netcdf_var_3d(double **buf, double *fillvalue, char *filename,
                             char *varname, char *lonname, char *latname, char *timename,
                             int nlon, int nlat, int ntime) {

  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat;

  size_t dimval;

  int ncinid;
  int varinid, timediminid, londiminid, latdiminid;
  nc_type vartype_main;
  int varndims;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  int ntime_file;
  int nlat_file;
  int nlon_file;

  size_t start[3];
  size_t count[3];

  float valf;

  /* Read data in NetCDF file */
  printf("%s: Reading info from NetCDF input file %s.\n", __FILE__, filename);
  istat = nc_open(filename, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_dimid(ncinid, timename, &timediminid);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, timediminid, &dimval); /* get time length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  ntime_file = (int) dimval;

  istat = nc_inq_dimid(ncinid, latname, &latdiminid);  /* get ID for lat dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, latdiminid, &dimval); /* get lat length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  nlat_file = (int) dimval;

  istat = nc_inq_dimid(ncinid, lonname, &londiminid);  /* get ID for lon dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, londiminid, &dimval); /* get lon length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  nlon_file = (int) dimval;
  
  istat = nc_inq_varid(ncinid, varname, &varinid); /* get main variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /** Read data variable **/
  
  /* Get variable information */
  istat = nc_inq_var(ncinid, varinid, (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  if (varndims != 3 || ( (ntime_file*nlat_file*nlon_file) != (ntime*nlat*nlon) ) || (ntime_file != ntime) ||
      (nlat_file != nlat) || (nlon_file != nlon) ) {
    (void) fprintf(stderr, "%s: Error NetCDF type and/or dimensions.\n", __FILE__);
    return -1;
  }

  /* Get fillvalue */
  if (vartype_main == NC_FLOAT) {
    istat = nc_get_att_float(ncinid, varinid, "missing_value", &valf);
    if (istat != NC_NOERR)
      *fillvalue = -9999.0;
    else
      *fillvalue = (double) valf;
  }
  else if (vartype_main == NC_DOUBLE) {
    istat = nc_get_att_double(ncinid, varinid, "missing_value", fillvalue);
    if (istat != NC_NOERR)
      *fillvalue = -9999.0;
  }

  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) ntime_file;
  count[1] = (size_t) nlat_file;
  count[2] = (size_t) nlon_file;
  /* Allocate memory */
  (*buf) = (double *) malloc(nlat_file*nlon_file*ntime_file * sizeof(double));
  if ((*buf) == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  printf("%s: Reading data from input file %s.\n", __FILE__, filename);
  istat = nc_get_vara_double(ncinid, varinid, start, count, *buf);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  return 0;
}
