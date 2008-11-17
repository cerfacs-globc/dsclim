/* ***************************************************** */
/* write_netcdf_var_3d Write a 3D NetCDF variable.       */
/* write_netcdf_var_3d.c                                 */
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
/*! \file write_netcdf_var_3d.c
    \brief Write a NetCDF variable.
*/

#include <io.h>

int write_netcdf_var_3d(double *buf, double fillvalue, char *filename,
                        char *varname, char *gridname, char *lonname, char *latname, char *timename,
                        int nlon, int nlat, int ntime) {

  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat;

  size_t dimval;

  int ncoutid;
  int varoutid, timedimoutid, londimoutid, latdimoutid;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  int ntime_file;
  int nlat_file;
  int nlon_file;

  size_t start[3];
  size_t count[3];

  char *attname = NULL;
  char *tmpstr = NULL;

  attname = (char *) malloc(5000 * sizeof(char));
  if (attname == NULL) alloc_error(__FILE__, __LINE__);

  /** Open already existing output file **/
  istat = nc_open(filename, NC_WRITE, &ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  
  istat = nc_inq_dimid(ncoutid, timename, &timedimoutid);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncoutid, timedimoutid, &dimval); /* get time length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  ntime_file = (int) dimval;

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
  
  if ( (ntime_file != ntime) || (nlat_file != nlat) || (nlon_file != nlon) ) {
    (void) fprintf(stderr, "%s: Error NetCDF type and/or dimensions.\n", __FILE__);
    return -1;
  }

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
  istat = sprintf(tmpstr, "Climatology of %s", varname);
  istat = nc_put_att_text(ncoutid, varoutid, "long_name", strlen(tmpstr), tmpstr);
  istat = nc_put_att_text(ncoutid, varoutid, "grid_mapping", strlen(gridname), gridname);
  istat = sprintf(tmpstr, "lon lat");
  istat = nc_put_att_text(ncoutid, varoutid, "coordinates", strlen(tmpstr), tmpstr);
  (void) free(tmpstr);

  /* End definition mode */
  istat = nc_enddef(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Write variable to NetCDF output file */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) ntime;
  count[1] = (size_t) nlat;
  count[2] = (size_t) nlon;
  printf("%s: WRITE %s %s.\n", __FILE__, varname, filename);
  istat = nc_put_vara_double(ncoutid, varoutid, start, count, buf);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Close the output netCDF file. */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  (void) free(attname);

  return 0;
}
