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

short int read_netcdf_var_3d(double **buf, info_field_struct *info_field, proj_struct *proj, char *filename, char *varname,
                             char *lonname, char *latname, char *timename, int nlon, int nlat, int ntime) {

  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat;

  size_t dimval;

  int ncinid;
  int varinid, timediminid, londiminid, latdiminid, projinid;
  nc_type vartype_main;
  int varndims;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  int ntime_file;
  int nlat_file;
  int nlon_file;

  size_t start[3];
  size_t count[3];

  float valf;
  int vali;
  char *tmpstr = NULL;
  size_t t_len;

  float *proj_latin = NULL;

  tmpstr = (char *) malloc(5000 * sizeof(char));
  if (tmpstr == NULL) alloc_error(__FILE__, __LINE__);

  /* Read data in NetCDF file */
  printf("%s: Opening for reading NetCDF input file %s.\n", __FILE__, filename);
  istat = nc_open(filename, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = nc_inq_dimid(ncinid, timename, &timediminid);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncinid, timediminid, &dimval); /* get time length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  ntime_file = (int) dimval;

  istat = nc_inq_dimid(ncinid, latname, &latdiminid);  /* get ID for lat dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncinid, latdiminid, &dimval); /* get lat length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  nlat_file = (int) dimval;

  istat = nc_inq_dimid(ncinid, lonname, &londiminid);  /* get ID for lon dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncinid, londiminid, &dimval); /* get lon length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  nlon_file = (int) dimval;
  
  istat = nc_inq_varid(ncinid, varname, &varinid); /* get main variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /** Read data variable **/
  
  /* Get variable information */
  istat = nc_inq_var(ncinid, varinid, (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  if (varndims != 3 || ( (ntime_file*nlat_file*nlon_file) != (ntime*nlat*nlon) ) || (ntime_file != ntime) ||
      (nlat_file != nlat) || (nlon_file != nlon) ) {
    (void) fprintf(stderr, "%s: Error NetCDF type and/or dimensions.\n", __FILE__);
    (void) free(tmpstr);
    istat = ncclose(ncinid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    return -1;
  }

  /* Get fillvalue */
  if (vartype_main == NC_FLOAT) {
    istat = nc_get_att_float(ncinid, varinid, "missing_value", &valf);
    if (istat != NC_NOERR)
      info_field->fillvalue = -9999.0;
    else
      info_field->fillvalue = (double) valf;
  }
  else if (vartype_main == NC_DOUBLE) {
    istat = nc_get_att_double(ncinid, varinid, "missing_value", &(info_field->fillvalue));
    if (istat != NC_NOERR)
      info_field->fillvalue = -9999.0;
  }

  istat = nc_inq_attlen(ncinid, varinid, "coordinates", &t_len);
  if (istat == NC_NOERR) {
    istat = nc_get_att_text(ncinid, varinid, "coordinates", tmpstr);
    if (istat == NC_NOERR) {
      if (tmpstr[t_len-1] != '\0')
        tmpstr[t_len] = '\0';
      info_field->coordinates = strdup(tmpstr);
    }
    else
      info_field->coordinates = strdup("lon lat");
  }
  else
    info_field->coordinates = strdup("lon lat");

  istat = nc_inq_attlen(ncinid, varinid, "grid_mapping", &t_len);
  if (istat == NC_NOERR) {
    handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_get_att_text(ncinid, varinid, "grid_mapping", tmpstr);
    if (istat == NC_NOERR) {
      if (tmpstr[t_len-1] != '\0')
        tmpstr[t_len] = '\0';
      info_field->grid_mapping = strdup(tmpstr);
    }
    else
      info_field->grid_mapping = strdup("unknown");
  }
  else
    info_field->grid_mapping = strdup("unknown");

  istat = nc_inq_attlen(ncinid, varinid, "units", &t_len);
  if (istat == NC_NOERR) {
    handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_get_att_text(ncinid, varinid, "units", tmpstr);
    if (istat == NC_NOERR) {
      if (tmpstr[t_len-1] != '\0')
        tmpstr[t_len] = '\0';
      info_field->units = strdup(tmpstr);
    }
    else
      info_field->units = strdup("unknown");
  }
  else
    info_field->units = strdup("unknown");

  istat = nc_inq_attlen(ncinid, varinid, "long_name", &t_len);
  if (istat == NC_NOERR) {
    handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_get_att_text(ncinid, varinid, "long_name", tmpstr);
    if (istat == NC_NOERR) {
      if (tmpstr[t_len-1] != '\0')
        tmpstr[t_len] = '\0';
      info_field->long_name = strdup(tmpstr);
    }
    else
      info_field->long_name = strdup(varname);
  }
  else
    info_field->long_name = strdup(varname);

  /* Get projection variable ID */
  if ( !strcmp(info_field->grid_mapping, "Lambert_Conformal") || !strcmp(info_field->grid_mapping, "Latitude_Longitude") ) {
    istat = nc_inq_varid(ncinid, info_field->grid_mapping, &projinid); /* get projection variable ID */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }
  if ( (strcmp(info_field->grid_mapping, "Lambert_Conformal") && strcmp(info_field->grid_mapping, "Latitude_Longitude")) &&
       ( !strcmp(proj->name, "Latitude_Longitude") || !strcmp(proj->name, "Lambert_Conformal") ) ) {
    (void) free(info_field->grid_mapping);
    info_field->grid_mapping = strdup(proj->name);
  }
  if ( !strcmp(info_field->grid_mapping, "Lambert_Conformal") ) {
    /*              int Lambert_Conformal ;
                    Lambert_Conformal:grid_mapping_name = "lambert_conformal_conic" ;
                    Lambert_Conformal:standard_parallel = 45.89892f, 47.69601f ;
                    Lambert_Conformal:longitude_of_central_meridian = 2.337229f ;
                    Lambert_Conformal:latitude_of_projection_origin = 46.8f ;
                    Lambert_Conformal:false_easting = 600000.f ;
                      Lambert_Conformal:false_northing = 2200000.f ;
    */
    istat = nc_get_var1_int(ncinid, projinid, 0, &vali);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);  
    proj->name = strdup(info_field->grid_mapping);
    istat = get_attribute_str(&(proj->grid_mapping_name), ncinid, projinid, "grid_mapping_name");
    
    proj_latin = (float *) malloc(2 * sizeof(float));
    if (proj_latin == NULL) alloc_error(__FILE__, __LINE__);
    istat = nc_get_att_float(ncinid, projinid, "standard_parallel", proj_latin);
    proj->latin1 = (double) proj_latin[0];
    proj->latin2 = (double) proj_latin[1];
    (void) free(proj_latin);
    
    istat = nc_get_att_double(ncinid, projinid, "longitude_of_central_meridian", &(proj->lonc));
    istat = nc_get_att_double(ncinid, projinid, "latitude_of_projection_origin", &(proj->lat0));
    istat = nc_get_att_double(ncinid, projinid, "false_easting", &(proj->false_easting));
    istat = nc_get_att_double(ncinid, projinid, "false_northing", &(proj->false_northing));
    
  }
  else if ( !strcmp(info_field->grid_mapping, "Latitude_Longitude") )
    proj->name = strdup(info_field->grid_mapping);      
  else {
    fprintf(stderr, "%s: Projection %s not supported! Cannot read projection parameters.\n", __FILE__, info_field->grid_mapping);
    (void) free(tmpstr);
    istat = ncclose(ncinid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    return -1;
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
  printf("%s: READ %s %s.\n", __FILE__, varname, filename);
  istat = nc_get_vara_double(ncinid, varinid, start, count, *buf);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Close the input netCDF file. */
  istat = ncclose(ncinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  (void) free(tmpstr);

  return 0;
}
