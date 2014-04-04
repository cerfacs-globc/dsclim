/* ********************************************************* */
/* test_proj_eof Test EOF projection function.               */
/* test_proj_eof.c                                           */
/* ********************************************************* */
/* Author: Christian Page, CERFACS, Toulouse, France.        */
/* ********************************************************* */
/*! \file test_proj_eof.c
    \brief Test EOF projection function.
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






#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/** GNU extensions */
#define _GNU_SOURCE

/* C standard includes */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNSTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#include <zlib.h>
#include <hdf5.h>
#include <netcdf.h>

#include <utils.h>
#include <clim.h>
#include <filter.h>
#include <pceof.h>

/** C prototypes. */
void show_usage(char *pgm);
void handle_netcdf_error(int status, int lineno);

/** Main program. */
int main(int argc, char **argv)
{
  /**
     @param[in]  argc  Number of command-line arguments.
     @param[in]  argv  Vector of command-line argument strings.

     \return           Status.
   */

  int nlat;
  int nlon;
  int nlat_sub;
  int nlon_sub;
  int ntime;
  int nlat_eof;
  int nlon_eof;
  int neof;

  size_t dimval;

  char *filein = NULL;
  char *filein_eof = NULL;
  char *fileout = NULL;

  int istat, ncinid, ncinid_eof, ncoutid;
  int varinid, timeinid, timediminid, loninid, londiminid, latinid, latdiminid;
  int varinid_eof, varinid_sing, loninid_eof, londiminid_eof, latinid_eof, latdiminid_eof, eofdiminid_eof;
  int varoutid, timeoutid, timedimoutid, eofdimoutid;
  nc_type vartype_main;
  nc_type vartype_time;
  int varndims;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  size_t start[3];
  size_t count[3];

  size_t t_len;
  char *time_units = NULL;
  char *cal_type = NULL;

  char attname[1000];
  int natts;
  double fillvalue;
  double fillvalue_eof;
  double scale = 1.0;
  float valf;

  double *psl = NULL;
  double *psl_eof = NULL;
  double *psl_sub = NULL;
  double *psl_eof_sub = NULL;
  double *psl_sing = NULL;
  double *lon_sub = NULL;
  double *lat_sub = NULL;
  double *timein = NULL;
  tstruct *timein_ts;
  double *lat = NULL;
  double *lon = NULL;
  double *lat_eof = NULL;
  double *lon_eof = NULL;

  double *psl_proj = NULL;
  double *psl_clim = NULL;
  double *psl_noclim = NULL;

  int clim_filter_width;
  char clim_filter_type[500];
  int clim_provided;

  double minlon = -360.0;
  double maxlon = 360.0;
  double minlat = -90.0;
  double maxlat = 90.0;

  double curlon;
  double curlat;

  int i;
  int j;
  int t;
  int ii;
  int jj;

  /* Print BEGIN banner */
  (void) banner(basename(argv[0]), "1.0", "BEGIN");

  /* Get command-line arguments and set appropriate variables */
  for (i=1; i<argc; i++) {
    if ( !strcmp(argv[i], "-h") ) {
      (void) show_usage(basename(argv[0]));
      (void) banner(basename(argv[0]), "OK", "END");
      return 0;
    }
    else if ( !strcmp(argv[i], "-i") ) {
      filein = (char *) malloc((strlen(argv[++i])+1) * sizeof(char));
      if (filein == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(filein, argv[i]);
    }
    else if ( !strcmp(argv[i], "-i_eof") ) {
      filein_eof = (char *) malloc((strlen(argv[++i])+1) * sizeof(char));
      if (filein_eof == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(filein_eof, argv[i]);
    }
    else if ( !strcmp(argv[i], "-o") ) {
      fileout = (char *) malloc((strlen(argv[++i])+1) * sizeof(char));
      if (fileout == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(fileout, argv[i]);
    }
    else if ( !strcmp(argv[i], "-scale") )
      (void) sscanf(argv[++i], "%lf", &scale);
    else {
      (void) fprintf(stderr, "%s:: Wrong arg %s.\n\n", basename(argv[0]), argv[i]);
      (void) show_usage(basename(argv[0]));
      (void) banner(basename(argv[0]), "ABORT", "END");
      (void) abort();
    }
  }

  /* Read data in NetCDF file */
  printf("%s: Reading info from input file %s.\n", __FILE__, filein);
  istat = nc_open(filein, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_dimid(ncinid, "time", &timediminid);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, timediminid, &dimval); /* get time length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  ntime = (int) dimval;

  istat = nc_inq_dimid(ncinid, "lat", &latdiminid);  /* get ID for lat dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, latdiminid, &dimval); /* get lat length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  nlat = (int) dimval;

  istat = nc_inq_dimid(ncinid, "lon", &londiminid);  /* get ID for lon dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, londiminid, &dimval); /* get lon length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  nlon = (int) dimval;
  
  istat = nc_inq_varid(ncinid, "time", &timeinid);  /* get ID for time variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_varid(ncinid, "lat", &latinid);  /* get ID for lat variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_varid(ncinid, "lon", &loninid);  /* get ID for lon variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_varid(ncinid, "psl", &varinid); /* get psl variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /** Read dimensions variables **/
  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) nlat;
  count[1] = (size_t) nlon;
  count[2] = 0;
  lat = (double *) malloc(nlat * nlon * sizeof(double));
  if (lat == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid, latinid, start, count, lat);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) nlat;
  count[1] = (size_t) nlon;
  count[2] = 0;
  lon = (double *) malloc(nlat * nlon * sizeof(double));
  if (lon == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid, loninid, start, count, lon);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  
  /* Get time dimensions and type */
  istat = nc_inq_var(ncinid, timeinid, (char *) NULL, &vartype_time, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  
  if (varndims != 1) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions.\n");
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }

  /* Allocate memory and set start and count */
  start[0] = 0;
  count[0] = (size_t) ntime;
  timein = malloc(ntime * sizeof(double));
  if (timein == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid, timeinid, start, count, timein);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Check values of time variable because many times they are all zero. In that case assume a 1 increment and a start at zero. */
  for (t=0; t<ntime; t++)
    if (timein[t] != 0.0)
      break;
  if (t == ntime) {
    fprintf(stderr, "WARNING: Time variable values all zero!!! Fixing time variable to index value...\n");
    for (t=0; t<ntime; t++)
      timein[t] = (double) t;
  }

  /** Read data variable **/
  
  /* Get variable information */
  istat = nc_inq_var(ncinid, varinid, (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  if (varndims != 3) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions.\n");
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }

  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) ntime;
  count[1] = (size_t) nlat;
  count[2] = (size_t) nlon;
  /* Allocate memory */
  psl = (double *) calloc(nlat*nlon*ntime, sizeof(double));
  if (psl == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  printf("%s: Reading data from input file %s.\n", __FILE__, filein);
  istat = nc_get_vara_double(ncinid, varinid, start, count, psl);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Get time units attribute length */
  istat = nc_inq_attlen(ncinid, timeinid, "units", &t_len);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  /* Allocate required space before retrieving values */
  time_units = (char *) malloc(t_len + 1);
  if (time_units == NULL) alloc_error(__FILE__, __LINE__);
  /* Get time units attribute value */
  istat = nc_get_att_text(ncinid, timeinid, "units", time_units);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  time_units[t_len] = '\0'; /* null terminate */

  /* Get calendar type attribute length */
  istat = nc_inq_attlen(ncinid, timeinid, "calendar", &t_len);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  /* Allocate required space before retrieving values */
  cal_type = (char *) malloc(t_len + 1);
  if (cal_type == NULL) alloc_error(__FILE__, __LINE__);
  /* Get calendar type attribute value */
  istat = nc_get_att_text(ncinid, timeinid, "calendar", cal_type);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  cal_type[t_len] = '\0'; /* null terminate */




  /* Read data in NetCDF file */
  printf("%s: Reading info from input file %s.\n", __FILE__, filein_eof);
  istat = nc_open(filein_eof, NC_NOWRITE, &ncinid_eof);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_dimid(ncinid_eof, "lat", &latdiminid_eof);  /* get ID for lat dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid_eof, latdiminid_eof, &dimval); /* get lat length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  nlat_eof = (int) dimval;

  istat = nc_inq_dimid(ncinid_eof, "lon", &londiminid_eof);  /* get ID for lon dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid_eof, londiminid_eof, &dimval); /* get lon length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  nlon_eof = (int) dimval;
  
  istat = nc_inq_dimid(ncinid_eof, "eof", &eofdiminid_eof);  /* get ID for pc dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid_eof, eofdiminid_eof, &dimval); /* get pc length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  neof = (int) dimval;
  
  istat = nc_inq_varid(ncinid_eof, "lat", &latinid_eof);  /* get ID for lat variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_varid(ncinid_eof, "lon", &loninid_eof);  /* get ID for lon variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_varid(ncinid_eof, "psl_eof", &varinid_eof); /* get psl variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_varid(ncinid_eof, "psl_sing", &varinid_sing); /* get psl variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /** Read dimensions variables **/
  /* Allocate memory and set start and count */
  start[0] = 0;
  count[0] = (size_t) nlat_eof;
  count[1] = 0;
  count[2] = 0;
  lat_eof = (double *) malloc(nlat_eof * sizeof(double));
  if (lat_eof == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid_eof, latinid_eof, start, count, lat_eof);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Allocate memory and set start and count */
  start[0] = 0;
  count[0] = (size_t) nlon_eof;
  count[1] = 0;
  count[2] = 0;
  lon_eof = (double *) malloc(nlon_eof * sizeof(double));
  if (lon_eof == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid_eof, loninid_eof, start, count, lon_eof);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /** Read data variable **/
  
  /* Get variable information */
  istat = nc_inq_var(ncinid_eof, varinid_eof, (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  if (varndims != 3) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions.\n");
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }

  (void) strcpy(attname, "missing_value");
  if (vartype_main == NC_FLOAT) {
    istat = nc_get_att_float(ncinid_eof, varinid_eof, attname, &valf);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    fillvalue_eof = (double) valf;
  }
  else if (vartype_main == NC_DOUBLE) {
    istat = nc_get_att_double(ncinid_eof, varinid_eof, attname, &fillvalue_eof);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  }

  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) neof;
  count[1] = (size_t) nlat_eof;
  count[2] = (size_t) nlon_eof;
  /* Allocate memory */
  psl_eof = (double *) calloc(nlat_eof*nlon_eof*neof, sizeof(double));
  if (psl_eof == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  printf("%s: Reading data from input file %s.\n", __FILE__, filein_eof);
  istat = nc_get_vara_double(ncinid_eof, varinid_eof, start, count, psl_eof);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) neof;
  count[1] = 0;
  count[2] = 0;
  /* Allocate memory */
  psl_sing = (double *) calloc(neof, sizeof(double));
  if (psl_sing == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid_eof, varinid_sing, start, count, psl_sing);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);



  /** Open output file **/
  istat = nc_create(fileout, NC_CLOBBER, &ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Set dimensions */
  istat = nc_def_dim(ncoutid, "time", NC_UNLIMITED, &timedimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_def_dim(ncoutid, "eof", neof, &eofdimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Define dimensions variables */
  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "time", NC_DOUBLE, 1, vardimids, &timeoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Define main output variable */
  vardimids[0] = timedimoutid;
  vardimids[1] = eofdimoutid;
  istat = nc_def_var(ncoutid, "psl_proj", vartype_main, 2, vardimids, &varoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Copy time attributes */
  istat = nc_inq_varnatts(ncinid, timeinid, &natts);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  for (i=0; i<natts; i++) {
    istat = nc_inq_attname(ncinid, timeinid, i, attname);
    if (istat == NC_NOERR) {
      printf("Time attribute: %s\n", attname);
      istat = nc_copy_att(ncinid, timeinid, attname, ncoutid, timeoutid);
      if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    }
    else
      (void) handle_netcdf_error(istat, __LINE__);
  }

  /* Copy main variable attributes */
  fillvalue = 999999.9999;
  istat = nc_inq_varnatts(ncinid, varinid, &natts);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  for (i=0; i<natts; i++) {
    istat = nc_inq_attname(ncinid, varinid, i, attname);
    if (istat == NC_NOERR) {
      printf("Main variable attribute: %s\n", attname);
      if ( !strcmp(attname, "_FillValue") || !strcmp(attname, "missing_value") ) {
        if (vartype_main == NC_FLOAT) {
          istat = nc_get_att_float(ncinid, varinid, attname, &valf);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
          fillvalue = (double) valf;
          istat = nc_put_att_double(ncoutid, varoutid, attname, NC_DOUBLE, 1, &fillvalue);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
        }
        else if (vartype_main == NC_DOUBLE) {
          istat = nc_get_att_double(ncinid, varinid, attname, &fillvalue);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
          istat = nc_put_att_double(ncoutid, varoutid, attname, NC_DOUBLE, 1, &fillvalue);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
        }
        else {
          (void) fprintf(stderr, "Error NetCDF variable type for main variable.\n");
          (void) banner(basename(argv[0]), "ABORT", "END");
          (void) abort();
        }
      }
      else {
        istat = nc_copy_att(ncinid, varinid, attname, ncoutid, varoutid);
        if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
      }
    }
    else
      (void) handle_netcdf_error(istat, __LINE__);
  }
  if (fillvalue == 999999.9999) {
    (void) strcpy(attname, "missing_value");
    istat = nc_put_att_double(ncoutid, varoutid, attname, NC_DOUBLE, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  }

  /* End definition mode */
  istat = nc_enddef(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /** Project data onto EOF **/

  clim_filter_width = 60;
  (void) strcpy(clim_filter_type, "hanning");
  clim_provided = 0;

  psl_proj = (double *) calloc(ntime * neof, sizeof(double));
  if (psl_proj == NULL) alloc_error(__FILE__, __LINE__);
  timein_ts = (tstruct *) malloc(ntime * sizeof(tstruct));
  if (timein_ts == NULL) alloc_error(__FILE__, __LINE__);
  printf("NLON=%d %d\n",nlon,ntime);
  (void) get_calendar_ts(timein_ts, time_units, timein, ntime);
  printf("NLON=%d\n",nlon);

  /* Compute subdomain and apply to arrays */
  /*  blon=where(lone ge 345 or lone le 20)
      blat=where(late ge 35 and late le 60) */
  minlon = -10.0;
  maxlon = 17.5;
  minlat = 35.0;
  maxlat = 57.5;
  nlon_sub = nlat_sub = 0;
  for (i=0; i<nlat_eof; i++)
    if (lat_eof[i] >= minlat && lat_eof[i] <= maxlat)
      nlat_sub++;
  for (i=0; i<nlon_eof; i++) {
    if (lon_eof[i] > 180.0)
      curlon = lon_eof[i] - 360.0;
    else
      curlon = lon_eof[i];
    if (curlon >= minlon && curlon <= maxlon)
      nlon_sub++;
  }

  psl_sub = (double *) malloc(nlon_sub*nlat_sub*ntime * sizeof(double));
  if (psl_sub == NULL) alloc_error(__FILE__, __LINE__);
  psl_eof_sub = (double *) malloc(nlon_sub*nlat_sub*neof * sizeof(double));
  if (psl_eof_sub == NULL) alloc_error(__FILE__, __LINE__);
  psl_clim = (double *) calloc(nlat_sub*nlon_sub*31*12, sizeof(double));
  if (psl_clim == NULL) alloc_error(__FILE__, __LINE__);
  psl_noclim = (double *) calloc(nlat_sub*nlon_sub*ntime, sizeof(double));
  if (psl_noclim == NULL) alloc_error(__FILE__, __LINE__);

  lon_sub = (double *) malloc(nlon_sub*nlat_sub * sizeof(double));
  if (lon_sub == NULL) alloc_error(__FILE__, __LINE__);
  lat_sub = (double *) malloc(nlon_sub*nlat_sub * sizeof(double));
  if (lat_sub == NULL) alloc_error(__FILE__, __LINE__);

  ii = 0;
  jj = 0;
  for (j=0; j<nlat; j++) {
    if (ii > 0)
      jj++;
    ii = 0;
    printf("j=%d %d %d\n",j,nlon,nlat);
    for (i=0; i<nlon; i++) {
      printf("i=%d\n",i);
      if (lon_eof[i] > 180.0)
        curlon = lon_eof[i] - 360.0;
      else
        curlon = lon_eof[i];
      curlat = lat_eof[j];
      if (curlon >= minlon && curlon <= maxlon && curlat >= minlat && curlat <= maxlat) {
        lon_sub[ii+jj*nlon_sub] = lon[i+j*nlon];
        lat_sub[ii+jj*nlon_sub] = lat[i+j*nlon];
        for (t=0; t<ntime; t++)
          psl_sub[ii+jj*nlon_sub+t*nlon_sub*nlat_sub] = psl[i+j*nlon+t*nlon*nlat];
        for (t=0; t<neof; t++)
          psl_eof_sub[ii+jj*nlon_sub+t*nlon_sub*nlat_sub] = psl_eof[i+j*nlon+t*nlon*nlat];  
        ii++;
      }
    }
  }

  (void) remove_seasonal_cycle(psl_noclim, psl_clim, psl_sub, timein_ts, fillvalue, clim_filter_width, clim_filter_type,
                               clim_provided, nlon_sub, nlat_sub, ntime);
  (void) project_field_eof(psl_proj, psl_noclim, psl_eof_sub, psl_sing, fillvalue_eof, lon_sub, lat_sub,
                           scale, nlon_sub, nlat_sub, ntime, neof);

  (void) fprintf(stderr, "Input/output time units: %s\n", time_units);

  /* Write dimensions variables to NetCDF output file */
  start[0] = 0;
  count[0] = (size_t) ntime;
  count[1] = 0;
  count[2] = 0;
  istat = nc_put_vara_double(ncoutid, timeoutid, start, count, timein);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Write variable to NetCDF output file */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) ntime;
  count[1] = (size_t) neof;
  count[2] = 0;
  printf("%s: Writing data to output file %s.\n", __FILE__, fileout);
  istat = nc_put_vara_double(ncoutid, varoutid, start, count, psl_proj);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /** Close NetCDF files **/
  /* Close the intput netCDF file. */
  istat = ncclose(ncinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Close the output netCDF file. */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  (void) free(psl);
  (void) free(psl_proj);
  (void) free(psl_sing);
  (void) free(psl_eof);
  (void) free(psl_clim);
  (void) free(psl_noclim);
  (void) free(psl_sub);
  (void) free(psl_eof_sub);
  (void) free(lon_sub);
  (void) free(lat_sub);
  (void) free(lon);
  (void) free(lat);
  (void) free(lon_eof);
  (void) free(lat_eof);
  (void) free(timein);
  (void) free(cal_type);
  (void) free(time_units);
  (void) free(filein);
  (void) free(filein_eof);
  (void) free(fileout);
  (void) free(timein_ts);

  /* Print END banner */
  (void) banner(basename(argv[0]), "OK", "END");

  return 0;
}


/** Local Subroutines **/

/** Show usage for program command-line arguments. */
void show_usage(char *pgm) {
  /**
     @param[in]  pgm  Program name.
  */

  (void) fprintf(stderr, "%s: usage:\n", pgm);
  (void) fprintf(stderr, "-i: input NetCDF file\n");
  (void) fprintf(stderr, "-o: output NetCDF file\n");
  (void) fprintf(stderr, "-h: help\n");

}

/* Handle error */
void handle_netcdf_error(int status, int lineno)
{
  if (status != NC_NOERR) {
    fprintf(stderr, "Line: %d Error %d: %s\n", lineno, status, nc_strerror(status));
    exit(-1);
  }
}
