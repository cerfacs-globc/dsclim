/* ***************************************************** */
/* write_netcdf_dims_3d Write NetCDF dimensions.         */
/* write_netcdf_dims_3d.c                                */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: oct 2010                            */
/* Last date of modification: oct 2010                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision: 1.1                                 */
/* ***************************************************** */
/* Revisions                                             */
/* 1.1 Added altitude optional variable                  */
/* ***************************************************** */
/*! \file write_netcdf_dims_3d.c
    \brief Write NetCDF dimensions and create output file.
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




#include <io.h>

/** Write NetCDF dimensions and create output file. */
int
write_netcdf_dims_3d(double *lon, double *lat, double *x, double *y, double *alt, double *timein, char *cal_type, char *time_units,
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
     @param[in]  alt                Altitude field
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
  int timeoutid, latoutid, lonoutid, xoutid, youtid, projoutid, altoutid;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  short int *alts = NULL;

  size_t start[3];
  size_t count[3];

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

  short int fillvalue = -99;

  /* Change directory to output directory for autofs notification */
  tmpstr = strdup(filename);
  istat = chdir(dirname(tmpstr));
  (void) free(tmpstr);

  tmpstr = (char *) malloc(MAXPATH * sizeof(char));
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

  if (alt != NULL) {
    /* Define Altitude variable */
    vardimids[0] = ydimoutid;
    vardimids[1] = xdimoutid;
    istat = nc_def_var(ncoutid, "Altitude", NC_SHORT, 2, vardimids, &altoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

  /* Set time attributes */
  istat = nc_put_att_text(ncoutid, timeoutid, "units", strlen(time_units), time_units);
  istat = nc_put_att_text(ncoutid, timeoutid, "calendar", strlen(cal_type), cal_type);
  (void) sprintf(tmpstr, "time in %s", time_units);
  istat = nc_put_att_text(ncoutid, timeoutid, "long_name", strlen(tmpstr), tmpstr);

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

  if (alt != NULL) {
    /* Set altitude attributes */
    (void) strcpy(tmpstr, "meters");
    istat = nc_put_att_text(ncoutid, altoutid, "units", strlen(tmpstr), tmpstr);
    (void) strcpy(tmpstr, "Altitude");
    istat = nc_put_att_text(ncoutid, altoutid, "long_name", strlen(tmpstr), tmpstr);
    (void) strcpy(tmpstr, "Altitude");
    istat = nc_put_att_text(ncoutid, altoutid, "standard_name", strlen(tmpstr), tmpstr);
    (void) strcpy(tmpstr, "lon lat");
    istat = nc_put_att_text(ncoutid, altoutid, "coordinates", strlen(tmpstr), tmpstr);
    fillvalue = -99;
    istat = nc_put_att_short(ncoutid, altoutid, "_FillValue", NC_SHORT, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_put_att_short(ncoutid, altoutid, "missing_value", NC_SHORT, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

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
    if (alt != NULL)
      istat = nc_put_att_text(ncoutid, altoutid, "grid_mapping_name", strlen(grid_mapping_name), grid_mapping_name);
    
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

  if (alt != NULL) {
    alts = (short int *) malloc(nlat*nlon * sizeof(short int));
    if (alts == NULL) alloc_error(__FILE__, __LINE__);
    for (i=0; i<(nlat*nlon); i++)
      alts[i] = (short int) alt[i];
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) nlat;
    count[1] = (size_t) nlon;
    count[2] = 0;
    istat = nc_put_vara_short(ncoutid, altoutid, start, count, alts);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    (void) free(alts);
  }

  /* Close the output netCDF file. */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  (void) free(tmpstr);

  return 0;
}
