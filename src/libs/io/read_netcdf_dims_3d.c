/* ***************************************************** */
/* read_netcdf_dims_3d Read NetCDF dimensions.           */
/* read_netcdf_dims_3d.c                                 */
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
/*! \file read_netcdf_dims_3d.c
    \brief Read NetCDF dimensions.
*/

#include <io.h>

short int read_netcdf_dims_3d(double **lon, double **lat, double **timeval, char **cal_type, char **time_units,
                              int *nlon, int *nlat, int *ntime, info_struct *info, char *coords, char *gridname,
                              char *lonname, char *latname, char *timename, char *filename) {

  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat;
  int t;

  size_t dimval;

  int ncinid;
  int timediminid, londiminid, latdiminid;
  int timeinid, latinid, loninid;
  nc_type vartype;
  int varndims;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  size_t start[3];
  size_t count[3];

  double *tmpd = NULL;

  int i;
  int j;

  /* Read data in NetCDF file */
  printf("%s: Reading info from NetCDF input file %s.\n", __FILE__, filename);
  istat = nc_open(filename, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = nc_inq_dimid(ncinid, timename, &timediminid);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncinid, timediminid, &dimval); /* get time length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  *ntime = (int) dimval;

  if ( !strcmp(coords, "1D") ) {
    /** 1D dimensions lat & lon **/
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
  }
  else if ( !strcmp(coords, "2D") && !strcmp(gridname, "Latitude_Longitude") ) {
    /** 1D dimensions x and y with 2D lat & lon related variables **/
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
  }
  else {
    /** 1D dimensions x and y with 2D lat & lon related variables **/
    istat = nc_inq_dimid(ncinid, "y", &latdiminid);  /* get ID for lat dimension */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_inq_dimlen(ncinid, latdiminid, &dimval); /* get lat length */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    *nlat = (int) dimval;
    
    istat = nc_inq_dimid(ncinid, "x", &londiminid);  /* get ID for lon dimension */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_inq_dimlen(ncinid, londiminid, &dimval); /* get lon length */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    *nlon = (int) dimval;
  }
  
  istat = nc_inq_varid(ncinid, timename, &timeinid);  /* get ID for time variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  /* Get time dimensions and type */
  istat = nc_inq_var(ncinid, timeinid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  if (varndims != 1) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions.\n");
    return -1;
  }
  istat = nc_inq_varid(ncinid, latname, &latinid);  /* get ID for lat variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  /* Get lat dimensions and type */
  istat = nc_inq_var(ncinid, latinid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);  
  if (varndims != 2) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions.\n");
    return -1;
  }
  istat = nc_inq_varid(ncinid, lonname, &loninid);  /* get ID for lon variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  /* Get lat dimensions and type */
  istat = nc_inq_var(ncinid, loninid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);  
  if (varndims != 2) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions.\n");
    return -1;
  }

  /* Get time units attributes */
  (void) get_attribute_str(time_units, ncinid, timeinid, "units");
  (void) get_attribute_str(cal_type, ncinid, timeinid, "calendar");
  if ( (*cal_type)[0] == '\0' ) {
    (void) free(*cal_type);
    (*cal_type) = strdup("standard");
  }

  /* Get global attributes */
  (void) get_attribute_str(&(info->title), ncinid, NC_GLOBAL, "title");
  (void) get_attribute_str(&(info->title_french), ncinid, NC_GLOBAL, "title_french");
  (void) get_attribute_str(&(info->summary), ncinid, NC_GLOBAL, "summary");
  (void) get_attribute_str(&(info->summary_french), ncinid, NC_GLOBAL, "summary_french");
  (void) get_attribute_str(&(info->keywords), ncinid, NC_GLOBAL, "keywords");
  (void) get_attribute_str(&(info->processor), ncinid, NC_GLOBAL, "processor");
  (void) get_attribute_str(&(info->description), ncinid, NC_GLOBAL, "description");
  (void) get_attribute_str(&(info->institution), ncinid, NC_GLOBAL, "institution");
  (void) get_attribute_str(&(info->creator_email), ncinid, NC_GLOBAL, "creator_email");
  (void) get_attribute_str(&(info->creator_url), ncinid, NC_GLOBAL, "creator_url");
  (void) get_attribute_str(&(info->creator_name), ncinid, NC_GLOBAL, "creator_name");
  (void) get_attribute_str(&(info->version), ncinid, NC_GLOBAL, "version");
  (void) get_attribute_str(&(info->scenario), ncinid, NC_GLOBAL, "scenario");
  (void) get_attribute_str(&(info->scenario_co2), ncinid, NC_GLOBAL, "scenario_co2");
  (void) get_attribute_str(&(info->model), ncinid, NC_GLOBAL, "model");
  (void) get_attribute_str(&(info->institution_model), ncinid, NC_GLOBAL, "institution_model");
  (void) get_attribute_str(&(info->country), ncinid, NC_GLOBAL, "country");
  (void) get_attribute_str(&(info->member), ncinid, NC_GLOBAL, "member");
  (void) get_attribute_str(&(info->downscaling_forcing), ncinid, NC_GLOBAL, "downscaling_forcing");
  (void) get_attribute_str(&(info->timestep), ncinid, NC_GLOBAL, "timestep");
  (void) get_attribute_str(&(info->member), ncinid, NC_GLOBAL, "member");
  (void) get_attribute_str(&(info->contact_email), ncinid, NC_GLOBAL, "contact_email");
  (void) get_attribute_str(&(info->contact_name), ncinid, NC_GLOBAL, "contact_name");
  (void) get_attribute_str(&(info->other_contact_email), ncinid, NC_GLOBAL, "other_contact_email");
  (void) get_attribute_str(&(info->other_contact_name), ncinid, NC_GLOBAL, "other_contact_name");

  /** Read dimensions variables **/
  if ( !strcmp(coords, "1D") ) {
    /* Allocate memory and set start and count */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) (*nlat);
    count[1] = 0;
    count[2] = 0;
    (*lat) = (double *) malloc((*nlat) * (*nlon) * sizeof(double));
    if ((*lat) == NULL) alloc_error(__FILE__, __LINE__);
    tmpd = (double *) malloc((*nlat) * sizeof(double));
    if (tmpd == NULL) alloc_error(__FILE__, __LINE__);
    
    /* Read values from netCDF variable */
    istat = nc_get_vara_double(ncinid, latinid, start, count, tmpd);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    for (j=0; j<(*nlat); j++)
      for (i=0; i<(*nlon); i++)
        (*lat)[i+j*(*nlon)] = tmpd[j];
    
    /* Allocate memory and set start and count */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) (*nlon);
    count[1] = 0;
    count[2] = 0;
    (*lon) = (double *) malloc((*nlat) * (*nlon) * sizeof(double));
    if ((*lon) == NULL) alloc_error(__FILE__, __LINE__);
    tmpd = (double *) realloc(tmpd, (*nlon) * sizeof(double));
    if (tmpd == NULL) alloc_error(__FILE__, __LINE__);
    
    /* Read values from netCDF variable */
    istat = nc_get_vara_double(ncinid, loninid, start, count, tmpd);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    for (j=0; j<(*nlat); j++)
      for (i=0; i<(*nlon); i++)
        (*lon)[i+j*(*nlon)] = tmpd[i];

    (void) free(tmpd);
    
  }
  else {  
    /* Allocate memory and set start and count */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) (*nlat);
    count[1] = (size_t) (*nlon);
    count[2] = 0;
    (*lat) = (double *) malloc((*nlat) * (*nlon) * sizeof(double));
    if ((*lat) == NULL) alloc_error(__FILE__, __LINE__);
    
    /* Read values from netCDF variable */
    istat = nc_get_vara_double(ncinid, latinid, start, count, (*lat));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    
    /* Allocate memory and set start and count */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) (*nlat);
    count[1] = (size_t) (*nlon);
    count[2] = 0;
    (*lon) = (double *) malloc((*nlat) * (*nlon) * sizeof(double));
    if ((*lon) == NULL) alloc_error(__FILE__, __LINE__);
    
    /* Read values from netCDF variable */
    istat = nc_get_vara_double(ncinid, loninid, start, count, (*lon));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

  /* Allocate memory and set start and count */
  start[0] = 0;
  count[0] = (size_t) (*ntime);
  (*timeval) = malloc((*ntime) * sizeof(double));
  if ((*timeval) == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid, timeinid, start, count, (*timeval));
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Check values of time variable because many times they are all zero. In that case assume a 1 increment and a start at zero. */
  for (t=0; t<(*ntime); t++)
    if ((*timeval)[t] != 0.0)
      break;
  if (t == (*ntime)) {
    fprintf(stderr, "%s: WARNING: Time variable values all zero!!! Fixing time variable to index value...\n", __FILE__);
    for (t=0; t<(*ntime); t++)
      (*timeval)[t] = (double) t;
  }

  return 0;
}
