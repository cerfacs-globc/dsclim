/* ***************************************************** */
/* read_analog_data Read analog data.                    */
/* read_analog_data.c                                    */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: feb 2009                            */
/* Last date of modification: feb 2009                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file read_analog_data.c
    \brief Read analog data from input NetCDF file.
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




#include <dsclim.h>

/** Read analog data from NetCDF input file. */
void
read_analog_data(analog_day_struct *analog_days, double **delta, double **time_ls, char *filename, char *timename) {
  /**
     @param[out]  analog_days           Analog days time indexes and dates with corresponding dates being downscaled.
     @param[out]  delta                 Temperature difference to apply to analog day data.
     @param[out]  time_ls               Time values in udunit.
     @param[in]   filename              Analog days output filename.
     @param[in]   timename              Time dimension name in NetCDF file.
  */

  int istat; /* Diagnostic status */

  int ncinid; /* NetCDF input file handle ID */
  int timediminid; /* NetCDF time dimension output ID */
  int timeinid; /* NetCDF time variable ID */
  int atimeinid; /* NetCDF analog dates variable ID */
  int ayearinid; /* NetCDF analog dates variable ID */
  int amonthinid; /* NetCDF analog dates variable ID */
  int adayinid; /* NetCDF analog dates variable ID */
  int dyearinid; /* NetCDF downscaled dates variable ID */
  int dmonthinid; /* NetCDF downscaled dates variable ID */
  int ddayinid; /* NetCDF downscaled dates variable ID */
  int deltatinid; /* NetCDF delta T variable ID */
    
  size_t start[1]; /* Start element when reading */
  size_t count[1]; /* Count of elements to read */
  size_t ntime; /* Time dimension length */

  istat = nc_open(filename, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = nc_inq_dimid(ncinid, timename, &timediminid);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncinid, timediminid, &ntime); /* get time dimension length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  analog_days->ntime = ntime;

  istat = nc_inq_varid(ncinid, timename, &timeinid); /* get time variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = nc_inq_varid(ncinid, "downscaled_date_year", &dyearinid); /* get variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_varid(ncinid, "downscaled_date_month", &dmonthinid); /* get variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_varid(ncinid, "downscaled_date_day", &ddayinid); /* get variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = nc_inq_varid(ncinid, "analog_date", &atimeinid); /* get variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_varid(ncinid, "analog_date_year", &ayearinid); /* get variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_varid(ncinid, "analog_date_month", &amonthinid); /* get variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_varid(ncinid, "analog_date_day", &adayinid); /* get variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = nc_inq_varid(ncinid, "analog_delta_t", &deltatinid); /* get variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Allocate memory and set start and count */
  start[0] = 0;
  count[0] = (size_t) analog_days->ntime;

  /* Read values from netCDF variables */

  analog_days->year_s = (int *) malloc(analog_days->ntime * sizeof(int));
  if (analog_days->year_s == NULL) alloc_error(__FILE__, __LINE__);
  istat = nc_get_vara_int(ncinid, dyearinid, start, count, analog_days->year_s);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  analog_days->month_s = (int *) malloc(analog_days->ntime * sizeof(int));
  if (analog_days->month_s == NULL) alloc_error(__FILE__, __LINE__);
  istat = nc_get_vara_int(ncinid, dmonthinid, start, count, analog_days->month_s);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  analog_days->day_s = (int *) malloc(analog_days->ntime * sizeof(int));
  if (analog_days->day_s == NULL) alloc_error(__FILE__, __LINE__);
  istat = nc_get_vara_int(ncinid, ddayinid, start, count, analog_days->day_s);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  analog_days->year = (int *) malloc(analog_days->ntime * sizeof(int));
  if (analog_days->year == NULL) alloc_error(__FILE__, __LINE__);
  istat = nc_get_vara_int(ncinid, ayearinid, start, count, analog_days->year);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  analog_days->month = (int *) malloc(analog_days->ntime * sizeof(int));
  if (analog_days->month == NULL) alloc_error(__FILE__, __LINE__);
  istat = nc_get_vara_int(ncinid, amonthinid, start, count, analog_days->month);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  analog_days->day = (int *) malloc(analog_days->ntime * sizeof(int));
  if (analog_days->day == NULL) alloc_error(__FILE__, __LINE__);
  istat = nc_get_vara_int(ncinid, adayinid, start, count, analog_days->day);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  (*delta) = (double *) malloc(analog_days->ntime * sizeof(double));
  if ((*delta) == NULL) alloc_error(__FILE__, __LINE__);
  istat = nc_get_vara_double(ncinid, deltatinid, start, count, *delta);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  (*time_ls) = (double *) malloc(analog_days->ntime * sizeof(double));
  if ((*time_ls) == NULL) alloc_error(__FILE__, __LINE__);
  istat = nc_get_vara_double(ncinid, timeinid, start, count, *time_ls);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Close the input netCDF file. */
  istat = ncclose(ncinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
}
