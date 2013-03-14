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

/** Read dimensions in a NetCDF file for EOF data. */
int
read_netcdf_dims_eof(double **lon, double **lat, int *nlon, int *nlat, int *neof, char *coords,
                     char *lonname, char *latname, char *dimxname, char *dimyname, char *eofname, char *filename) {
  /**
     @param[out]  lon      Longitude field
     @param[out]  lat      Latitude field
     @param[out]  nlon     Longitude dimension
     @param[out]  nlat     Latitude dimension
     @param[out]  neof     EOF dimension
     @param[in]   coords   Coordinates arrangement of latitude and longitude data: either 1D or 2D
     @param[in]   lonname  Longitude variable name
     @param[in]   latname  Latitude variable name
     @param[in]   dimxname Longitude dimension name
     @param[in]   dimyname Latitude dimension name
     @param[in]   eofname  EOF dimension name
     @param[in]   filename Input NetCDF filename
     
     \return           Status.
  */

  int istat; /* Diagnostic status */

  size_t dimval; /* Variable used to retrieve dimension length */

  int ncinid; /* NetCDF input file handle ID */
  int latinid; /* Latitude variable ID */
  int loninid; /* Longitude variable ID */
  nc_type vartype; /* Type of the variable (NC_FLOAT, NC_DOUBLE, etc.) */
  int varndims; /* Number of dimensions of variable */
  int vardimids[NC_MAX_VAR_DIMS]; /* Variable dimension ids */
  int eofdiminid; /* EOF dimension ID */
  int londiminid; /* Longitude dimension ID */
  int latdiminid; /* Latitude dimension ID */

  size_t start[3]; /* Start position to read */
  size_t count[3]; /* Number of elements to read */

  double *tmpd = NULL; /* Temporary buffer to read variable from NetCDF file */

  int i; /* Loop counter */
  int j; /* Loop counter */
  int ndims; /* Number of dimensions of latitude and longitude variables, 1 or 2 for 1D and 2D respectively */

  /* Read data in NetCDF file */

  /* Open NetCDF file for reading */
  printf("%s: Reading info from NetCDF input file %s\n", __FILE__, filename);
  istat = nc_open(filename, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

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
  else {
    /** 2D lat & lon variables **/
    ndims = 2;

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
  
  /* Get dimensions length */
  istat = nc_inq_dimid(ncinid, eofname, &eofdiminid);  /* get ID for eof dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncinid, eofdiminid, &dimval); /* get eof length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  *neof = (int) dimval;

  /* Get variables IDs */
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

  istat = ncclose(ncinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Success status */
  return 0;
}
