/* ***************************************************** */
/* read_netcdf_dims_eof Read NetCDF dimensions for EOF.  */
/* read_netcdf_dims_eof.c                                */
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
/*! \file read_netcdf_dims_eof.c
    \brief Read NetCDF dimensions for EOF.
*/

#include <io.h>

short int read_netcdf_dims_eof(double **lon, double **lat, int *nlon, int *nlat, int *neof, char *coords,
                               char *lonname, char *latname, char *eofname, char *filename) {

  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat;
  int t;

  size_t dimval;

  int ncinid;
  int eofdiminid, londiminid, latdiminid;
  int latinid, loninid;
  nc_type vartype;
  int varndims;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  size_t start[3];
  size_t count[3];

  double *tmpd = NULL;

  int i;
  int j;
  int ndims;

  /* Read data in NetCDF file */
  printf("%s: Reading info from NetCDF input file %s.\n", __FILE__, filename);
  istat = nc_open(filename, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  if ( !strcmp(coords, "1D") ) {
    /** 1D dimensions lat & lon **/
    ndims = 1;

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
    /** 2D lat & lon variables **/
    ndims = 2;

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
  
  istat = nc_inq_dimid(ncinid, eofname, &eofdiminid);  /* get ID for eof dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncinid, eofdiminid, &dimval); /* get eof length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  *neof = (int) dimval;

  istat = nc_inq_varid(ncinid, latname, &latinid);  /* get ID for lat variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  /* Get lat dimensions and type */
  istat = nc_inq_var(ncinid, latinid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);  
  if (varndims != ndims) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions %d != %d.\n", varndims, ndims);
    return -1;
  }
  istat = nc_inq_varid(ncinid, lonname, &loninid);  /* get ID for lon variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  /* Get lat dimensions and type */
  istat = nc_inq_var(ncinid, loninid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);  
  if (varndims != ndims) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions %d != %d.\n", varndims, ndims);
    return -1;
  }

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

  return 0;
}
