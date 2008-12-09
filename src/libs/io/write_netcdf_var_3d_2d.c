/* ***************************************************** */
/* write_netcdf_var_3d_2d Write a 2D field in a          */
/* 3D NetCDF variable.                                   */
/* write_netcdf_var_3d_2d.c                              */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: nov 2008                            */
/* Last date of modification: nov 2008                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file write_netcdf_var_3d_2d.c
    \brief Write a 2D field in a 3D NetCDF variable.
*/

#include <io.h>

/** Write a 2D field in a 3D NetCDF variable. */
int write_netcdf_var_3d_2d(double *buf, double *timein, double fillvalue, char *filename,
                           char *varname, char *longname, char *units, char *height,
                           char *gridname, char *lonname, char *latname, char *timename,
                           int t, int newfile, int nlon, int nlat, int ntime, int outinfo) {
  /**
     @param[in]  buf         3D Field to write
     @param[in]  timein      Time dimension value
     @param[in]  fillvalue   Missing value
     @param[in]  filename    Output NetCDF filename
     @param[in]  varname     Variable name in the NetCDF file
     @param[in]  longname    Variable long name in the NetCDF file
     @param[in]  units       Variable units in the NetCDF file
     @param[in]  height      Variable height in the NetCDF file
     @param[in]  gridname    Grid type name in the NetCDF file
     @param[in]  lonname     Longitude name dimension in the NetCDF file
     @param[in]  latname     Latitude name dimension in the NetCDF file
     @param[in]  timename    Time name dimension in the NetCDF file
     @param[in]  t           Time index of value to write
     @param[in]  newfile     TRUE is new NetCDF file, FALSE if not
     @param[in]  outinfo     TRUE if we want information output, FALSE if not
     @param[in]  nlon        Longitude dimension
     @param[in]  nlat        Latitude dimension
     @param[in]  ntime       Time dimension
     
     \return                 Status.
  */

  int istat; /* Diagnostic status */

  size_t dimval; /* Temporary variable used to get values from dimension lengths */

  int ncoutid; /* NetCDF output file handle ID */
  int varoutid; /* NetCDF variable output ID */
  int timedimoutid; /* NetCDF time dimension output ID */
  int timeid; /* NetCDF time variable ID */
  int londimoutid; /* NetCDF longitude dimension output ID */
  int latdimoutid; /* NetCDF latitude dimension output ID */
  int vardimids[NC_MAX_VAR_DIMS]; /* NetCDF dimension IDs */

  int ntime_file; /* Time dimension in NetCDF output file */
  int nlat_file; /* Latitude dimension in NetCDF output file */
  int nlon_file; /* Longitude dimension in NetCDF output file */

  size_t start[3]; /* Start element when writing */
  size_t count[3]; /* Count of elements to write */

  char *attname = NULL; /* Attribute name */
  char *tmpstr = NULL; /* Temporary string */

  /* Allocate memory */
  attname = (char *) malloc(5000 * sizeof(char));
  if (attname == NULL) alloc_error(__FILE__, __LINE__);

  /** Open already existing output file **/
  istat = nc_open(filename, NC_WRITE, &ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  
  /* Get dimension lengths */
  istat = nc_inq_dimid(ncoutid, timename, &timedimoutid);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncoutid, timedimoutid, &dimval); /* get time length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  ntime_file = (int) dimval;

  istat = nc_inq_varid(ncoutid, timename, &timeid);  /* get ID for time variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = nc_inq_dimid(ncoutid, latname, &latdimoutid);  /* get ID for lat dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncoutid, latdimoutid, &dimval); /* get lat length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  nlat_file = (int) dimval;

  istat = nc_inq_dimid(ncoutid, lonname, &londimoutid);  /* get ID for lon dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncoutid, londimoutid, &dimval); /* get lon length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  nlon_file = (int) dimval;

  /* Verify that they match the provided ones in parameters */
  if ( ((nlat_file != nlat) || (nlon_file != nlon) )) {
    (void) fprintf(stderr, "%s: Error NetCDF type and/or dimensions.\n", __FILE__);
    return -1;
  }

  /* Go into NetCDF define mode only if first element */
  if (newfile == TRUE) {
    istat = nc_redef(ncoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    
    /* Define main output variable */
    vardimids[0] = timedimoutid;
    vardimids[1] = latdimoutid;
    vardimids[2] = londimoutid;
    istat = nc_def_var(ncoutid, varname, NC_DOUBLE, 3, vardimids, &varoutid);  
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    
    /* Set main variable attributes */
    (void) strcpy(attname, "_Fillvalue");
    istat = nc_put_att_double(ncoutid, varoutid, attname, NC_DOUBLE, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    
    (void) strcpy(attname, "missing_value");
    istat = nc_put_att_double(ncoutid, varoutid, attname, NC_DOUBLE, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    
    tmpstr = (char *) malloc(100 * sizeof(char));
    if (tmpstr == NULL) alloc_error(__FILE__, __LINE__);
    istat = nc_put_att_text(ncoutid, varoutid, "long_name", strlen(longname), longname);
    istat = nc_put_att_text(ncoutid, varoutid, "grid_mapping", strlen(gridname), gridname);
    istat = nc_put_att_text(ncoutid, varoutid, "units", strlen(units), units);
    istat = nc_put_att_text(ncoutid, varoutid, "height", strlen(height), height);
    istat = sprintf(tmpstr, "lon lat");
    istat = nc_put_att_text(ncoutid, varoutid, "coordinates", strlen(tmpstr), tmpstr);
    (void) free(tmpstr);
    
    /* End definition mode */
    istat = nc_enddef(ncoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }
  else {
    /* Get variable ID */
    istat = nc_inq_varid(ncoutid, varname, &varoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

  /* Write time dimension variable to NetCDF output file */
  start[0] = ntime_file;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) 1;
  count[1] = 0;
  count[2] = 0;
  istat = nc_put_vara_double(ncoutid, timeid, start, count, &(timein[t]));
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Write variable to NetCDF output file */
  start[0] = ntime_file;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) 1;
  count[1] = (size_t) nlat;
  count[2] = (size_t) nlon;
  if (outinfo == TRUE)
    printf("%s: WRITE %s %s.\n", __FILE__, varname, filename);
  istat = nc_put_vara_double(ncoutid, varoutid, start, count, buf);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Close the output netCDF file. */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Free memory */
  (void) free(attname);

  /* Diagnostic status */
  return 0;
}