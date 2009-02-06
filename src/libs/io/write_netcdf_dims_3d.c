/* ***************************************************** */
/* write_netcdf_dims_3d Write NetCDF dimensions.         */
/* write_netcdf_dims_3d.c                                */
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
/*! \file write_netcdf_dims_3d.c
    \brief Write NetCDF dimensions and create output file.
*/

#include <io.h>

/** Write NetCDF dimensions and create output file. */
int write_netcdf_dims_3d(double *lon, double *lat, double *x, double *y, double *timein, char *cal_type, char *time_units,
                         int nlon, int nlat, int ntime, char *timestep, char *gridname, char *coords,
                         char *grid_mapping_name, double latin1, double latin2,
                         double lonc, double lat0, double false_easting, double false_northing,
                         char *lonname, char *latname, char *timename,
                         char *filename, int outinfo) {
  /**
     @param[in]  lon                Longitude field
     @param[in]  lat                Latitude field
     @param[in]  x                  X field
     @param[in]  y                  Y field
     @param[in]  timein             Time field
     @param[in]  cal_type           Calendar-type (udunits)
     @param[in]  time_units         Time units (udunits)
     @param[in]  nlon               Longitude dimension
     @param[in]  nlat               Latitude dimension
     @param[in]  ntime              Time dimension
     @param[in]  timestep           Timestep string (NetCDF attribute)
     @param[in]  gridname           Grid type name in the NetCDF file (NetCDF attribute)
     @param[in]  coords             Coordinates type (NetCDF attribute)
     @param[in]  grid_mapping_name  Grid mapping name in the NetCDF file (CF-1.0 compliant)
     @param[in]  latin1             Latin1 projection parameter
     @param[in]  latin2             Latin2 projection parameter
     @param[in]  lonc               Lonc center longitude projection parameter
     @param[in]  lat0               Lat0 first latitude projection parameter
     @param[in]  false_easting      False_easting projection parameter
     @param[in]  false_northing     False_northing projection parameter
     @param[in]  filename           Output NetCDF filename
     @param[in]  lonname            Longitude name dimension in the NetCDF file
     @param[in]  latname            Latitude name dimension in the NetCDF file
     @param[in]  timename           Time name dimension in the NetCDF file
     @param[in]  outinfo            TRUE if we want information output, FALSE if not
     
     \return                        Status.
  */

  int istat;

  int ncoutid;
  int timedimoutid, xdimoutid, ydimoutid;
  int timeoutid, latoutid, lonoutid, xoutid, youtid, projoutid;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  size_t start[3];
  size_t count[3];

  utUnit dataunit;

  int year;
  int month;
  int day;
  int hour;
  int minutes;
  float seconds;

  int vali;
  int i;

  float *proj_latin = NULL;

  char *tmpstr = NULL;
  double *tmpd = NULL;
  int *tmpi = NULL;

  double minlat;
  double maxlat;
  double minlon;
  double maxlon;

  tmpstr = (char *) malloc(5000 * sizeof(char));
  if (tmpstr == NULL) alloc_error(__FILE__, __LINE__);

  /* Open NetCDF file */
  if (outinfo == TRUE)
    printf("%s: Writing info from NetCDF output file %s\n", __FILE__, filename);
  istat = nc_open(filename, NC_WRITE, &ncoutid);  /* open NetCDF file */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = nc_redef(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Set dimensions */
  istat = nc_def_dim(ncoutid, timename, NC_UNLIMITED, &timedimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  if ( !strcmp(gridname, "Latitude_Longitude")) {
    istat = nc_def_dim(ncoutid, lonname, nlon, &xdimoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_def_dim(ncoutid, latname, nlat, &ydimoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }
  else if ( !strcmp(gridname, "list")) {
    istat = nc_def_dim(ncoutid, lonname, nlon, &xdimoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_def_dim(ncoutid, latname, nlon, &ydimoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }
  else {
    istat = nc_def_dim(ncoutid, "x", nlon, &xdimoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_def_dim(ncoutid, "y", nlat, &ydimoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

  /* Define dimensions variables */
  vardimids[0] = timedimoutid;
  vardimids[1] = 0;
  istat = nc_def_var(ncoutid, timename, NC_INT, 1, vardimids, &timeoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  if ( !strcmp(coords, "1D") ) {
    vardimids[0] = xdimoutid;
    vardimids[1] = 0;
    istat = nc_def_var(ncoutid, lonname, NC_DOUBLE, 1, vardimids, &lonoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    vardimids[0] = ydimoutid;
    vardimids[1] = 0;
    istat = nc_def_var(ncoutid, latname, NC_DOUBLE, 1, vardimids, &latoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }
  else if ( !strcmp(gridname, "Latitude_Longitude") && !strcmp(coords, "2D") ) {
    vardimids[0] = ydimoutid;
    vardimids[1] = xdimoutid;
    istat = nc_def_var(ncoutid, lonname, NC_DOUBLE, 2, vardimids, &lonoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    vardimids[0] = ydimoutid;
    vardimids[1] = xdimoutid;
    istat = nc_def_var(ncoutid, latname, NC_DOUBLE, 2, vardimids, &latoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }
  else {
    vardimids[0] = ydimoutid;
    vardimids[1] = xdimoutid;
    istat = nc_def_var(ncoutid, lonname, NC_DOUBLE, 2, vardimids, &lonoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    vardimids[0] = xdimoutid;
    vardimids[1] = 0;
    istat = nc_def_var(ncoutid, "x", NC_INT, 1, vardimids, &xoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    vardimids[0] = ydimoutid;
    vardimids[1] = xdimoutid;
    istat = nc_def_var(ncoutid, latname, NC_DOUBLE, 2, vardimids, &latoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    vardimids[0] = ydimoutid;
    vardimids[1] = 0;
    istat = nc_def_var(ncoutid, "y", NC_INT, 1, vardimids, &youtid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

  /* Define projection variable */
  if ( !strcmp(gridname, "Lambert_Conformal")) {
    istat = nc_def_var(ncoutid, gridname, NC_INT, 0, 0, &projoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

  /* Set time attributes */
  istat = nc_put_att_text(ncoutid, timeoutid, "units", strlen(time_units), time_units);
  istat = nc_put_att_text(ncoutid, timeoutid, "calendar", strlen(cal_type), cal_type);
  (void) sprintf(tmpstr, "time in %s", time_units);
  istat = nc_put_att_text(ncoutid, timeoutid, "long_name", strlen(tmpstr), tmpstr);

  /* Set time global attributes */
  if (utIsInit() != TRUE)
    istat = utInit("");
  
  istat = utScan(time_units,  &dataunit);

  istat = utCalendar(timein[0], &dataunit, &year, &month, &day, &hour, &minutes, &seconds);
  (void) sprintf(tmpstr, "%04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minutes, (int) seconds);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "time_coverage_start", strlen(tmpstr), tmpstr);

  if (ntime > 0) {
    istat = utCalendar(timein[ntime-1], &dataunit, &year, &month, &day, &hour, &minutes, &seconds);
    (void) sprintf(tmpstr, "%04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minutes, (int) seconds);
    istat = nc_put_att_text(ncoutid, NC_GLOBAL, "time_coverage_end", strlen(tmpstr), tmpstr);
  }
  else {
    istat = utCalendar(timein[0], &dataunit, &year, &month, &day, &hour, &minutes, &seconds);
    (void) sprintf(tmpstr, "%04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minutes, (int) seconds);
    istat = nc_put_att_text(ncoutid, NC_GLOBAL, "time_coverage_end", strlen(tmpstr), tmpstr);
  }

  (void) utTerm();

  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "timestep", strlen(timestep), timestep);

  /* Set longitude attributes */
  (void) strcpy(tmpstr, "degrees_east");
  istat = nc_put_att_text(ncoutid, lonoutid, "units", strlen(tmpstr), tmpstr);
  (void) strcpy(tmpstr, "longitude coordinate");
  istat = nc_put_att_text(ncoutid, lonoutid, "long_name", strlen(tmpstr), tmpstr);
  (void) strcpy(tmpstr, "longitude");
  istat = nc_put_att_text(ncoutid, lonoutid, "standard_name", strlen(tmpstr), tmpstr);

  /* Set latitude attributes */
  (void) strcpy(tmpstr, "degrees_north");
  istat = nc_put_att_text(ncoutid, latoutid, "units", strlen(tmpstr), tmpstr);
  (void) strcpy(tmpstr, "latitude coordinate");
  istat = nc_put_att_text(ncoutid, latoutid, "long_name", strlen(tmpstr), tmpstr);
  (void) strcpy(tmpstr, "latitude");
  istat = nc_put_att_text(ncoutid, latoutid, "standard_name", strlen(tmpstr), tmpstr);

  if ( strcmp(gridname, "Latitude_Longitude") && strcmp(gridname, "list")) {
    /* Set x attributes */
    /*          int x(x) ;
                x:units = "m" ;
                x:long_name = "x coordinate of projection" ;
                x:standard_name = "projection_x_coordinate" ;*/
    (void) strcpy(tmpstr, "m");
    istat = nc_put_att_text(ncoutid, xoutid, "units", strlen(tmpstr), tmpstr);
    (void) strcpy(tmpstr, "x coordinate of projection");
    istat = nc_put_att_text(ncoutid, xoutid, "long_name", strlen(tmpstr), tmpstr);
    (void) strcpy(tmpstr, "projection_x_coordinate");
    istat = nc_put_att_text(ncoutid, xoutid, "standard_name", strlen(tmpstr), tmpstr);  
    
    /* Set y attributes */
    /*        int y(y) ;
              y:units = "m" ;
              y:long_name = "y coordinate of projection" ;
              y:standard_name = "projection_y_coordinate" ;*/
    (void) strcpy(tmpstr, "m");
    istat = nc_put_att_text(ncoutid, youtid, "units", strlen(tmpstr), tmpstr);
    (void) strcpy(tmpstr, "y coordinate of projection");
    istat = nc_put_att_text(ncoutid, youtid, "long_name", strlen(tmpstr), tmpstr);
    (void) strcpy(tmpstr, "projection_y_coordinate");
    istat = nc_put_att_text(ncoutid, youtid, "standard_name", strlen(tmpstr), tmpstr);  

    /* Set projection attributes */
    /*          int Lambert_Conformal ;
                Lambert_Conformal:grid_mapping_name = "lambert_conformal_conic" ;
                Lambert_Conformal:standard_parallel = 45.89892f, 47.69601f ;
                Lambert_Conformal:longitude_of_central_meridian = 2.337229f ;
                Lambert_Conformal:latitude_of_projection_origin = 46.8f ;
                Lambert_Conformal:false_easting = 600000.f ;
                Lambert_Conformal:false_northing = 2200000.f ;
    */
  }

  if ( !strcmp(gridname, "Lambert_Conformal")) {
    
    istat = nc_put_att_text(ncoutid, projoutid, "grid_mapping_name", strlen(grid_mapping_name), grid_mapping_name);
    
    proj_latin = (float *) malloc(2 * sizeof(float));
    if (proj_latin == NULL) alloc_error(__FILE__, __LINE__);
    proj_latin[0] = (float) latin1;
    proj_latin[1] = (float) latin2;
    istat = nc_put_att_float(ncoutid, projoutid, "standard_parallel", NC_FLOAT, 2, proj_latin);
    (void) free(proj_latin);
    
    istat = nc_put_att_double(ncoutid, projoutid, "longitude_of_central_meridian", NC_FLOAT, 1, &lonc);
    istat = nc_put_att_double(ncoutid, projoutid, "latitude_of_projection_origin", NC_FLOAT, 1, &lat0);
    istat = nc_put_att_double(ncoutid, projoutid, "false_easting", NC_FLOAT, 1, &false_easting);
    istat = nc_put_att_double(ncoutid, projoutid, "false_northing", NC_FLOAT, 1, &false_northing);
  }

  (void) strcpy(tmpstr, "Grid");
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "cdm_datatype", strlen(tmpstr), tmpstr);

  /* Geographic global attributes */
  maxlat = -9999.9;
  minlat = 9999.9;
  maxlon = -9999.9;
  minlon = 9999.9;
  if ( !strcmp(gridname, "list") ) {
    for (i=0; i<(nlon); i++) {
      if (lat[i] > maxlat) maxlat = lat[i];
      if (lat[i] < minlat) minlat = lat[i];
    }
    for (i=0; i<(nlon); i++) {
      if (lon[i] > maxlon) maxlon = lon[i];
      if (lon[i] < minlon) minlon = lon[i];
    }
  }
  else {
    for (i=0; i<(nlat*nlon); i++) {
      if (lat[i] > maxlat) maxlat = lat[i];
      if (lat[i] < minlat) minlat = lat[i];
    }
    for (i=0; i<(nlat*nlon); i++) {
      if (lon[i] > maxlon) maxlon = lon[i];
      if (lon[i] < minlon) minlon = lon[i];
    }
  }
  istat = nc_put_att_double(ncoutid, NC_GLOBAL, "geospatial_lat_max", NC_DOUBLE, 1, &maxlat);
  istat = nc_put_att_double(ncoutid, NC_GLOBAL, "geospatial_lat_min", NC_DOUBLE, 1, &minlat);
  istat = nc_put_att_double(ncoutid, NC_GLOBAL, "geospatial_lon_max", NC_DOUBLE, 1, &maxlon);
  istat = nc_put_att_double(ncoutid, NC_GLOBAL, "geospatial_lon_min", NC_DOUBLE, 1, &minlon);

  /* End definition mode */
  istat = nc_enddef(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  if ( !strcmp(gridname, "Lambert_Conformal")) {
    /* Write projection variable */
    vali = 1;
    istat = nc_put_var1_int(ncoutid, projoutid, 0, &vali);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);  
  }

  /* Write dimensions variables to NetCDF output file */
  if (ntime > 0) {
    start[0] = 0;
    count[0] = (size_t) ntime;
    count[1] = 0;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, timeoutid, start, count, timein);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

  if ( !strcmp(gridname, "list") ) {
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) nlon;
    count[1] = 0;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, latoutid, start, count, lat);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) nlon;
    count[1] = 0;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, lonoutid, start, count, lon);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }
  else if ( !strcmp(coords, "1D") ) {
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) nlat;
    count[1] = 0;
    count[2] = 0;
    tmpd = (double *) malloc(nlat * sizeof(double));
    if (tmpd == NULL) alloc_error(__FILE__, __LINE__);
    for (i=0; i<nlat; i++)
      tmpd[i] = lat[i*nlon];
    istat = nc_put_vara_double(ncoutid, latoutid, start, count, tmpd);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) nlon;
    count[1] = 0;
    count[2] = 0;
    tmpd = (double *) realloc(tmpd, nlon * sizeof(double));
    if (tmpd == NULL) alloc_error(__FILE__, __LINE__);
    for (i=0; i<nlon; i++)
      tmpd[i] = lon[i];
    istat = nc_put_vara_double(ncoutid, lonoutid, start, count, tmpd);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    (void) free(tmpd);
  }
  else {
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) nlat;
    count[1] = (size_t) nlon;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, latoutid, start, count, lat);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) nlat;
    count[1] = (size_t) nlon;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, lonoutid, start, count, lon);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

  if ( !strcmp(gridname, "Lambert_Conformal") && x != NULL && y != NULL) {
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) nlon;
    count[1] = 0;
    count[2] = 0;
    tmpi = (int *) realloc(tmpi, nlon * sizeof(int));
    if (tmpi == NULL) alloc_error(__FILE__, __LINE__);
    for (i=0; i<nlon; i++)
      tmpi[i] = x[i];

    istat = nc_put_vara_int(ncoutid, xoutid, start, count, tmpi);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) nlat;
    count[1] = 0;
    count[2] = 0;
    tmpi = (int *) realloc(tmpi, nlat * sizeof(int));
    if (tmpi == NULL) alloc_error(__FILE__, __LINE__);
    for (i=0; i<nlat; i++)
      tmpi[i] = y[i];
    istat = nc_put_vara_int(ncoutid, youtid, start, count, tmpi);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    (void) free(tmpi);
  }

  /* Close the output netCDF file. */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  (void) free(tmpstr);

  return 0;
}
