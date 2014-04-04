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

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2014)

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

/** Read dimensions in a NetCDF file for 3D variables. */
int
read_netcdf_dims_3d(double **lon, double **lat, double **timeval, char **cal_type, char **time_units,
                    int *nlon, int *nlat, int *ntime, info_struct *info, char *coords, char *gridname,
                    char *lonname, char *latname, char *dimxname, char *dimyname, char *timename, char *filename) {
  /**
     @param[out]  lon        Longitude field
     @param[out]  lat        Latitude field
     @param[out]  timeval    Time field
     @param[out]  cal_type   Calendar type (udunits)
     @param[out]  time_units Time units (udunits)
     @param[out]  nlon       Longitude dimension
     @param[out]  nlat       Latitude dimension
     @param[out]  ntime      Time dimension
     @param[out]  info       Information structure for global NetCDF attributes
     @param[in]   coords     Coordinates arrangement of latitude and longitude data: either 1D or 2D
     @param[in]   gridname   Projection grid name
     @param[in]   lonname    Longitude dimension name
     @param[in]   latname    Latitude dimension name
     @param[in]   dimxname   X Dimension name
     @param[in]   dimyname   Y Dimension name
     @param[in]   timename   Time dimension name
     @param[in]   filename   Input NetCDF filename
     
     \return           Status.
  */

  int istat; /* Diagnostic status */
  int fixtime = FALSE; /* If we fixed time or not */

  size_t dimval; /* Variable used to retrieve dimension length */

  int ncinid; /* NetCDF input file handle ID */
  int latinid; /* Latitude variable ID */
  int loninid; /* Longitude variable ID */
  int timeinid; /* Time variable ID */
  nc_type vartype; /* Type of the variable (NC_FLOAT, NC_DOUBLE, etc.) */
  int varndims; /* Number of dimensions of variable */
  int vardimids[NC_MAX_VAR_DIMS]; /* Variable dimension ids */
  int timediminid; /* Time dimension ID */
  int londiminid; /* Longitude dimension ID */
  int latdiminid; /* Latitude dimension ID */

  size_t start[3]; /* Start position to read */
  size_t count[3]; /* Number of elements to read */

  double *tmpd = NULL; /* Temporary buffer to read variable from NetCDF file */

  int i; /* Loop counter */
  int j; /* Loop counter */
  int t; /* Time loop counter */
  int ndims; /* Number of dimensions of latitude and longitude variables, 1 or 2 for 1D and 2D respectively */

  /* Read data in NetCDF file */

  /* Open NetCDF file for reading */
  printf("%s: Reading info from NetCDF input file %s\n", __FILE__, filename);
  istat = nc_open(filename, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Get dimensions ID */
  istat = nc_inq_dimid(ncinid, timename, &timediminid);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncinid, timediminid, &dimval); /* get time length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  *ntime = (int) dimval;

  if ( !strcmp(coords, "1D") ) {
    /** 1D dimensions lat & lon **/
    ndims = 1;

    /* Get dimensions length */
    istat = nc_inq_dimid(ncinid, dimyname, &latdiminid);  /* get ID for lat dimension */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_inq_dimlen(ncinid, latdiminid, &dimval); /* get lat length */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    *nlat = (int) dimval;
    
    istat = nc_inq_dimid(ncinid, dimxname, &londiminid);  /* get ID for lon dimension */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_inq_dimlen(ncinid, londiminid, &dimval); /* get lon length */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    *nlon = (int) dimval;
  }
  else if ( !strcmp(coords, "2D") && !strcmp(gridname, "Latitude_Longitude") ) {
    /** 1D dimensions x and y with 2D lat & lon related variables **/
    ndims = 2;

    /* Get dimensions length */
    istat = nc_inq_dimid(ncinid, dimyname, &latdiminid);  /* get ID for lat dimension */
    if (istat != NC_NOERR) {
      (void) fprintf(stderr, "%s: Dimension name %s.\n", __FILE__, dimyname);
      handle_netcdf_error(istat, __FILE__, __LINE__);
    }
    istat = nc_inq_dimlen(ncinid, latdiminid, &dimval); /* get lat length */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    *nlat = (int) dimval;
    
    istat = nc_inq_dimid(ncinid, dimxname, &londiminid);  /* get ID for lon dimension */
    if (istat != NC_NOERR) {
      (void) fprintf(stderr, "%s: Dimension name %s.\n", __FILE__, dimxname);
      handle_netcdf_error(istat, __FILE__, __LINE__);
    }
    istat = nc_inq_dimlen(ncinid, londiminid, &dimval); /* get lon length */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    *nlon = (int) dimval;
  }
  else {
    /** 1D dimensions x and y with 2D lat & lon related variables **/
    ndims = 2;

    /* Get dimensions length */
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
  
  /* Get time dimension ID */
  istat = nc_inq_varid(ncinid, timename, &timeinid);  /* get ID for time variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  /* Get time dimensions and type */
  istat = nc_inq_var(ncinid, timeinid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  if (varndims != 1) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions %d != 1.\n", varndims);
    istat = ncclose(ncinid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    return -1;
  }

  /* Get dimension ID */
  istat = nc_inq_varid(ncinid, latname, &latinid);  /* get ID for lat variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  /* Get lat dimensions and type */
  istat = nc_inq_var(ncinid, latinid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);  
  if (varndims != ndims) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions %d != %d.\n", varndims, ndims);
    istat = ncclose(ncinid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    return -1;
  }

  /* Get dimension ID */
  istat = nc_inq_varid(ncinid, lonname, &loninid);  /* get ID for lon variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  /* Get lat dimensions and type */
  istat = nc_inq_var(ncinid, loninid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);  
  if (varndims != ndims) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions %d != %d.\n", varndims, ndims);
    istat = ncclose(ncinid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
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
  if (info->title == NULL) {
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
    (void) get_attribute_str(&(info->contact_email), ncinid, NC_GLOBAL, "contact_email");
    (void) get_attribute_str(&(info->contact_name), ncinid, NC_GLOBAL, "contact_name");
    (void) get_attribute_str(&(info->other_contact_email), ncinid, NC_GLOBAL, "other_contact_email");
    (void) get_attribute_str(&(info->other_contact_name), ncinid, NC_GLOBAL, "other_contact_name");
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
    fprintf(stderr, "\n%s: IMPORTANT WARNING: Time variable values all zero!!! Fixing time variable to index value, STARTING at 0...\n\n", __FILE__);
    fixtime = TRUE;
    for (t=0; t<(*ntime); t++)
      (*timeval)[t] = (double) t;
  }

  /* Close NetCDF file */
  istat = ncclose(ncinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  if (fixtime == FALSE)
    /* Success status */
    return 0;
  else
    return 1;
}
