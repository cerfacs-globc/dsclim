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

/** Read a 3D variable in a NetCDF file, and return information in info_field_struct structure and proj_struct. */
int read_netcdf_var_3d(double **buf, info_field_struct *info_field, proj_struct *proj, char *filename, char *varname,
                       char *lonname, char *latname, char *timename, int *nlon, int *nlat, int *ntime) {
  /**
     @param[out]  buf        3D variable
     @param[out]  info_field Information about the output variable
     @param[out]  proj       Information about the horizontal projection of the output variable
     @param[in]   filename   NetCDF input filename
     @param[in]   varname    NetCDF variable name
     @param[in]   lonname    Longitude dimension name
     @param[in]   latname    Latitude dimension name
     @param[in]   timename   Time dimension name
     @param[out]  nlon       Longitude dimension length
     @param[out]  nlat       Latitude dimension length
     @param[out]  ntime      Time dimension length
     
     \return           Status.
  */

  int istat; /* Diagnostic status */

  size_t dimval; /* Variable used to retrieve dimension length */

  int ncinid; /* NetCDF input file handle ID */
  int varinid; /* NetCDF variable ID */
  int projinid; /* Projection variable ID */
  nc_type vartype_main; /* Type of the variable (NC_FLOAT, NC_DOUBLE, etc.) */
  int varndims; /* Number of dimensions of variable */
  int vardimids[NC_MAX_VAR_DIMS]; /* Variable dimension ids */
  int timediminid; /* Time dimension ID */
  int londiminid; /* Longitude dimension ID */
  int latdiminid; /* Latitude dimension ID */

  size_t start[3]; /* Start position to read */
  size_t count[3]; /* Number of elements to read */

  float valf; /* Variable used to retrieve fillvalue */
  int vali; /* Variable used to retrieve integer values */
  char *tmpstr = NULL; /* Temporary string */
  size_t t_len; /* Length of string attribute */

  float *proj_latin = NULL; /* Parallel latitudes of projection */

  /* Allocate memory */
  tmpstr = (char *) malloc(5000 * sizeof(char));
  if (tmpstr == NULL) alloc_error(__FILE__, __LINE__);

  /* Read data in NetCDF file */

  /* Open NetCDF file for reading */
  printf("%s: Opening for reading NetCDF input file %s.\n", __FILE__, filename);
  istat = nc_open(filename, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  printf("%s: READ %s %s.\n", __FILE__, varname, filename);

  /* Get dimensions length */
  istat = nc_inq_dimid(ncinid, timename, &timediminid);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncinid, timediminid, &dimval); /* get time length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  *ntime = (int) dimval;

  istat = nc_inq_dimid(ncinid, latname, &latdiminid);  /* get ID for lat dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncinid, latdiminid, &dimval); /* get lat length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  *nlat = (int) dimval;

  istat = nc_inq_dimid(ncinid, lonname, &londiminid);  /* get ID for lon dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncinid, londiminid, &dimval); /* get lon length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  *nlon = (int) dimval;
  
  /* Get main variable ID */
  istat = nc_inq_varid(ncinid, varname, &varinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /** Read data variable **/
  
  /* Get variable information */
  istat = nc_inq_var(ncinid, varinid, (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Verify that variable is really 3D */
  if (varndims != 3) {
    (void) fprintf(stderr, "%s: Error NetCDF type and/or dimensions nlon %d nlat %d.\n", __FILE__, *nlon, *nlat);
    (void) free(tmpstr);
    istat = ncclose(ncinid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    return -1;
  }

  /* If info_field si not NULL, get some information about the read variable */
  if (info_field != NULL) {
    /* Get missing value */
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

    /* Get coordinates */
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

    /* Get grid projection */
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

    /* Get units */
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

    /* Get long name */
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
  }

  /* if proj is not NULL, retrieve informations about the horizontal projection parameters */
  if (proj != NULL) {
    /* Get projection variable ID */
    if ( !strcmp(info_field->grid_mapping, "Lambert_Conformal") ) {
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
      fprintf(stderr, "%s: WARNING: No projection parameter available for %s.\n", __FILE__, info_field->grid_mapping);
      proj->name = strdup("Latitude_Longitude");      
    }
  }

  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) *ntime;
  count[1] = (size_t) *nlat;
  count[2] = (size_t) *nlon;
  /* Allocate memory */
  (*buf) = (double *) malloc((*nlat)*(*nlon)*(*ntime) * sizeof(double));
  if ((*buf) == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid, varinid, start, count, *buf);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Close the input netCDF file. */
  istat = ncclose(ncinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Free memory */
  (void) free(tmpstr);

  /* Success status */
  return 0;
}
