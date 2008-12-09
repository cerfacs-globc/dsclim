/* ***************************************************** */
/* read_netcdf_xy Read NetCDF X and Y dimensions.        */
/* read_netcdf_xy.c                                      */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: dec 2008                            */
/* Last date of modification: dec 2008                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file read_netcdf_xy.c
    \brief Read NetCDF X and Y dimensions.
*/

#include <io.h>

/** Read X and Y dimensions in a NetCDF file. */
int read_netcdf_xy(double **x, double **y, int *nx, int *ny, char *xname, char *yname, char *dimxname, char *dimyname, char *filename) {
  /**
     @param[out]  x          X field
     @param[out]  y          Y field
     @param[out]  nx         X dimension
     @param[out]  ny         Y dimension
     @param[in]   xname      X variable name
     @param[in]   yname      Y variable name
     @param[in]   dimxname   X dimension name
     @param[in]   dimyname   Y dimension name
     @param[in]   filename   Input NetCDF filename
     
     \return           Status.
  */

  int istat; /* Diagnostic status */

  size_t dimval; /* Variable used to retrieve dimension length */

  int ncinid; /* NetCDF input file handle ID */
  int yinid; /* Y variable ID */
  int xinid; /* X variable ID */
  nc_type vartype; /* Type of the variable (NC_FLOAT, NC_DOUBLE, etc.) */
  int varndims; /* Number of dimensions of variable */
  int vardimids[NC_MAX_VAR_DIMS]; /* Variable dimension ids */
  int xdiminid; /* X dimension ID */
  int ydiminid; /* Y dimension ID */

  size_t start[3]; /* Start position to read */
  size_t count[3]; /* Number of elements to read */

  int ndims; /* Number of dimensions of y and x variables */

  /* Read data in NetCDF file */

  /* Open NetCDF file for reading */
  printf("%s: Reading info from NetCDF input file %s.\n", __FILE__, filename);
  istat = nc_open(filename, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /** 1D dimensions y and x dimensions **/
  ndims = 1;
  
  /* Get dimensions length */
  istat = nc_inq_dimid(ncinid, dimyname, &ydiminid);  /* get ID for Y dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncinid, ydiminid, &dimval); /* get y length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  *ny = (int) dimval;
  
  istat = nc_inq_dimid(ncinid, dimxname, &xdiminid);  /* get ID for X dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncinid, xdiminid, &dimval); /* get x length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  *nx = (int) dimval;
  
  /* Get dimension ID */
  istat = nc_inq_varid(ncinid, yname, &yinid);  /* get ID for y variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  /* Get y dimensions and type */
  istat = nc_inq_var(ncinid, yinid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);  
  if (varndims != ndims) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions %d != %d.\n", varndims, ndims);
    istat = ncclose(ncinid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    return -1;
  }

  /* Get dimension ID */
  istat = nc_inq_varid(ncinid, xname, &xinid);  /* get ID for x variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  /* Get x dimensions and type */
  istat = nc_inq_var(ncinid, xinid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);  
  if (varndims != ndims) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions %d != %d.\n", varndims, ndims);
    istat = ncclose(ncinid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    return -1;
  }

  /** Read dimensions variables **/
  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) (*ny);
  count[1] = 0;
  count[2] = 0;
  (*y) = (double *) malloc((*ny) * sizeof(double));
  if ((*y) == NULL) alloc_error(__FILE__, __LINE__);
  
  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid, yinid, start, count, *y);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  
  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) (*nx);
  count[1] = 0;
  count[2] = 0;
  (*x) = (double *) malloc((*nx) * sizeof(double));
  if ((*x) == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid, xinid, start, count, *x);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = ncclose(ncinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Success status */
  return 0;
}
