/* ********************************************************* */
/* testcalendar Test calendar conversion functions.          */
/* testcalendar.c                                            */
/* ********************************************************* */
/* Author: Christian Page, CERFACS, Toulouse, France.        */
/* ********************************************************* */
/*! \file testcalendar.c
    \brief Test calendar conversion functions.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2010)

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
#include <szlib.h>
#include <hdf5.h>
#include <netcdf.h>

#include <utils.h>

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
  int ntime;

  size_t dimval;
  int i;

  char *filein = NULL;
  char *fileout = NULL;

  int istat, ncinid, ncoutid;
  int varinid, timeinid, timediminid, loninid, londiminid, latinid, latdiminid;
  int varoutid, timeoutid, timedimoutid, lonoutid, londimoutid, latoutid, latdimoutid;
  nc_type vartype_main;
  nc_type vartype_time;
  int varndims;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  size_t start[3];
  size_t count[3];

  size_t t_len;
  char *time_units = NULL;
  char *cal_type = NULL;
  char cal_type_out[500];

  char attname[1000];
  int natts;
  float fillvaluef;
  double fillvalued;

  void *psl = NULL;
  double *timein = NULL;
  double *lat = NULL;
  double *lon = NULL;

  double *psloutd = NULL;
  float *psloutf = NULL;
  double *outtimeval = NULL;
  int ntimeout;

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
    else if ( !strcmp(argv[i], "-o") ) {
      fileout = (char *) malloc((strlen(argv[++i])+1) * sizeof(char));
      if (fileout == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(fileout, argv[i]);
    }
    else {
      (void) fprintf(stderr, "%s:: Wrong arg %s.\n\n", basename(argv[0]), argv[i]);
      (void) show_usage(basename(argv[0]));
      (void) banner(basename(argv[0]), "ABORT", "END");
      (void) abort();
    }
  }

  /* Read data in NetCDF file */
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
  count[0] = (size_t) nlat;
  lat = (double *) malloc(nlat * sizeof(double));
  if (lat == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid, latinid, start, count, lat);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Allocate memory and set start and count */
  start[0] = 0;
  count[0] = (size_t) nlon;
  lon = (double *) malloc(nlon * sizeof(double));
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
  if (vartype_main == NC_DOUBLE)
    psl = calloc(nlat*nlon*ntime, sizeof(double));
  else if (vartype_main == NC_FLOAT)
    psl = calloc(nlat*nlon*ntime, sizeof(float));
  else {
    (void) fprintf(stderr, "Error NetCDF variable type for main variable.\n");
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }    
  if (psl == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  printf("%s: Reading data from input file %s.\n", __FILE__, filein);
  istat = nc_get_vara(ncinid, varinid, start, count, psl);
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

  /** Adjust calendar **/
  outtimeval = NULL;
  if (vartype_main == NC_FLOAT) {
    psloutf = NULL;
    (void) data_to_gregorian_cal_f(&psloutf, &outtimeval, &ntimeout, psl, timein, time_units, "days since 1900-01-01 00:00:00",
                                   "noleap", nlon, nlat, ntime);
  }
  else {
    psloutd = NULL;
    (void) data_to_gregorian_cal_d(&psloutd, &outtimeval, &ntimeout, psl, timein, time_units, "days since 1900-01-01 00:00:00",
                                   "noleap", nlon, nlat, ntime);
  }

  (void) fprintf(stderr, "Input time units: %s\n", time_units);
  (void) fprintf(stderr, "Output time units: days since 1900-01-01 00:00:00\n");

  /** Write data to NetCDF file **/
  /* Create and open output file */
  istat = nc_create(fileout, NC_CLOBBER, &ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Set dimensions */
  istat = nc_def_dim(ncoutid, "time", NC_UNLIMITED, &timedimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_def_dim(ncoutid, "lat", nlat, &latdimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_def_dim(ncoutid, "lon", nlon, &londimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Define dimensions variables */
  vardimids[0] = latdimoutid;
  istat = nc_def_var(ncoutid, "lat", NC_DOUBLE, 1, vardimids, &latoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  vardimids[0] = londimoutid;
  istat = nc_def_var(ncoutid, "lon", NC_DOUBLE, 1, vardimids, &lonoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "time", NC_DOUBLE, 1, vardimids, &timeoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Define main output variable */
  vardimids[0] = timedimoutid;
  vardimids[1] = latdimoutid;
  vardimids[2] = londimoutid;
  istat = nc_def_var(ncoutid, "psl", vartype_main, 3, vardimids, &varoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Copy lat attributes */
  istat = nc_inq_varnatts(ncinid, latinid, &natts);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  for (i=0; i<natts; i++) {
    istat = nc_inq_attname(ncinid, latinid, i, attname);
    if (istat == NC_NOERR) {
      istat = nc_copy_att(ncinid, latinid, attname, ncoutid, latoutid);
      if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    }
    else
      (void) handle_netcdf_error(istat, __LINE__);
  }

  /* Copy lon attributes */
  istat = nc_inq_varnatts(ncinid, loninid, &natts);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  for (i=0; i<natts; i++) {
    istat = nc_inq_attname(ncinid, loninid, i, attname);
    if (istat == NC_NOERR) {
      istat = nc_copy_att(ncinid, loninid, attname, ncoutid, lonoutid);
      if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    }
    else
      (void) handle_netcdf_error(istat, __LINE__);
  }

  /* Copy time attributes */
  istat = nc_inq_varnatts(ncinid, timeinid, &natts);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  for (i=0; i<natts; i++) {
    istat = nc_inq_attname(ncinid, timeinid, i, attname);
    if (istat == NC_NOERR) {
      printf("Time attribute: %s\n", attname);
      if ( !strcmp(attname, "calendar") ) {
        /* Change calendar type to standard */
        (void) strcat(cal_type_out, "standard");
        istat = nc_put_att_text(ncoutid, timeoutid, "calendar", strlen(cal_type_out), cal_type_out);
      }
      else
        /* Other time attributes */
        istat = nc_copy_att(ncinid, timeinid, attname, ncoutid, timeoutid);
      if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    }
    else
      (void) handle_netcdf_error(istat, __LINE__);
  }

  /* Copy main variable attributes */
  istat = nc_inq_varnatts(ncinid, varinid, &natts);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  for (i=0; i<natts; i++) {
    istat = nc_inq_attname(ncinid, varinid, i, attname);
    if (istat == NC_NOERR) {
      printf("Main variable attribute: %s\n", attname);
      if ( !strcmp(attname, "_FillValue") || !strcmp(attname, "missing_value") ) {
        if (vartype_main == NC_FLOAT) {
          istat = nc_get_att_float(ncinid, varoutid, attname, &fillvaluef);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
          istat = nc_put_att_float(ncoutid, varoutid, attname, NC_FLOAT, 1, &fillvaluef);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
        }
        else if (vartype_main == NC_DOUBLE) {
          istat = nc_get_att_double(ncinid, varoutid, attname, &fillvalued);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
          istat = nc_put_att_double(ncoutid, varoutid, attname, NC_DOUBLE, 1, &fillvalued);
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

  /* End definition mode */
  istat = nc_enddef(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Write dimensions variables to NetCDF output file */
  start[0] = 0;
  count[0] = (size_t) nlat;
  count[1] = 0;
  count[2] = 0;
  istat = nc_put_vara_double(ncoutid, latoutid, start, count, lat);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  count[0] = (size_t) nlon;
  istat = nc_put_vara_double(ncoutid, lonoutid, start, count, lon);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  count[0] = (size_t) ntimeout;
  istat = nc_put_vara_double(ncoutid, timeoutid, start, count, outtimeval);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  for (i=0; i<ntimeout; i++)
    printf("Output Time Dimension values. index=%d time=%lf\n",i,outtimeval[i]);

  /* Write variable to NetCDF output file */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) ntimeout;
  count[1] = (size_t) nlat;
  count[2] = (size_t) nlon;
  printf("%s: Writing data to output file %s.\n", __FILE__, fileout);
  if (vartype_main == NC_FLOAT)
    istat = nc_put_vara_float(ncoutid, varoutid, start, count, psloutf);
  else
    istat = nc_put_vara_double(ncoutid, varoutid, start, count, psloutd);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /** Close NetCDF files **/
  /* Close the intput netCDF file. */
  istat = ncclose(ncinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Close the output netCDF file. */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  (void) free(psl);
  if (vartype_main == NC_FLOAT)
    (void) free(psloutf);
  else
    (void) free(psloutd);
  (void) free(outtimeval);
  (void) free(lon);
  (void) free(lat);
  (void) free(timein);
  (void) free(cal_type);
  (void) free(time_units);
  (void) free(filein);
  (void) free(fileout);

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
