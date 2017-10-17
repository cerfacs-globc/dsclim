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

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2015)

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

/** Read X and Y dimensions in a NetCDF file. */
int
read_netcdf_xy(double **x, double **y, int *nx, int *ny, char *xname, char *yname, char *dimxname, char *dimyname, char *filename) {
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

  static short int error_report = 0;

  /* Read data in NetCDF file */

  /* Open NetCDF file for reading */
  printf("%s: Reading info from NetCDF input file %s\n", __FILE__, filename);
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
    if (error_report == 0) {
      (void) fprintf(stderr, "%s:: WARNING: We do not have 1D dimensions so it means that we have no coordinate system in our projection..\n", __FILE__);
      error_report = 1;
    }
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
    if (error_report == 0) {
      (void) fprintf(stderr, "%s:: WARNING: We do not have 1D dimensions so it means that we have no coordinate system in our projection..\n", __FILE__);
      error_report = 1;
    }
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
