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
                              int *nlon, int *nlat, int *ntime, char *lonname, char *latname, char *timename,
                              char *filename) {

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
  size_t t_len;
  int varndims;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  size_t start[3];
  size_t count[3];

  /* Read data in NetCDF file */
  printf("%s: Reading info from NetCDF input file %s.\n", __FILE__, filename);
  istat = nc_open(filename, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_dimid(ncinid, timename, &timediminid);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, timediminid, &dimval); /* get time length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  *ntime = (int) dimval;

  istat = nc_inq_dimid(ncinid, latname, &latdiminid);  /* get ID for lat dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, latdiminid, &dimval); /* get lat length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  *nlat = (int) dimval;

  istat = nc_inq_dimid(ncinid, lonname, &londiminid);  /* get ID for lon dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, londiminid, &dimval); /* get lon length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  *nlon = (int) dimval;
  
  istat = nc_inq_varid(ncinid, "time", &timeinid);  /* get ID for time variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  /* Get time dimensions and type */
  istat = nc_inq_var(ncinid, timeinid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  if (varndims != 1) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions.\n");
    return -1;
  }
  istat = nc_inq_varid(ncinid, "lat", &latinid);  /* get ID for lat variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  /* Get lat dimensions and type */
  istat = nc_inq_var(ncinid, latinid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);  
  if (varndims != 2) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions.\n");
    return -1;
  }
  istat = nc_inq_varid(ncinid, "lon", &loninid);  /* get ID for lon variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  /* Get lat dimensions and type */
  istat = nc_inq_var(ncinid, loninid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);  
  if (varndims != 2) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions.\n");
    return -1;
  }

  /* Get time units attribute length */
  istat = nc_inq_attlen(ncinid, timeinid, "units", &t_len);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  /* Allocate required space before retrieving values */
  (*time_units) = (char *) malloc(t_len + 1);
  if ((*time_units) == NULL) alloc_error(__FILE__, __LINE__);
  /* Get time units attribute value */
  istat = nc_get_att_text(ncinid, timeinid, "units", (*time_units));
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  (*time_units)[t_len] = '\0'; /* null terminate */

  /* Get calendar type attribute length */
  istat = nc_inq_attlen(ncinid, timeinid, "calendar", &t_len);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  /* Allocate required space before retrieving values */
  (*cal_type) = (char *) malloc(t_len + 1);
  if ((*cal_type) == NULL) alloc_error(__FILE__, __LINE__);
  /* Get calendar type attribute value */
  istat = nc_get_att_text(ncinid, timeinid, "calendar", (*cal_type));
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  (*cal_type)[t_len] = '\0'; /* null terminate */

  /** Read dimensions variables **/
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
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

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
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  
  /* Allocate memory and set start and count */
  start[0] = 0;
  count[0] = (size_t) (*ntime);
  (*timeval) = malloc((*ntime) * sizeof(double));
  if ((*timeval) == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid, timeinid, start, count, (*timeval));
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

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
