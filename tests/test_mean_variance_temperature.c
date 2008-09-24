/* ********************************************************* */
/* test_mean_variance_temperature                            */
/* Test mean_variance_field_spatial function.                */
/* test_mean_variance_temperature.c                          */
/* ********************************************************* */
/* Author: Christian Page, CERFACS, Toulouse, France.        */
/* ********************************************************* */
/*! \file test_mean_variance_temperature.c
    \brief Test mean_variance_field_spatial function.
*/

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

#include <udunits.h>

#include <utils.h>
#include <filter.h>
#include <clim.h>

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
  int ntime_sub;
  int nseason_learn;
  int *ntime_learn = NULL;

  int *year = NULL;
  int *month = NULL;
  int *day = NULL;
  int *hour = NULL;
  int *min = NULL;
  float *sec = NULL;

  double mean;
  double var;

  char varname[500];

  size_t dimval;

  char *filein = NULL;
  char *filein_learn = NULL;

  int istat, ncinid, ncinid_learn;
  int varinid, timeinid, timediminid, loninid, londiminid, latinid, latdiminid;
  int seasondiminid_learn;
  int *time_learn_dimid = NULL;
  int *time_learn_varid = NULL;
  nc_type vartype_main;
  nc_type vartype_time;
  int varndims;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  size_t start[3];
  size_t count[3];

  size_t t_len;
  size_t t_len_learn;
  char *time_units = NULL;
  char *time_units_learn = NULL;
  char *cal_type = NULL;
  char *cal_type_learn = NULL;
  utUnit dataunits;
  utUnit dataunits_learn;

  double *tas = NULL;
  double *tas_sub = NULL;
  double *tas_subt = NULL;
  double *tas_smean = NULL;
  double *timein = NULL;
  double *lat = NULL;
  double *lon = NULL;
  double **time_learn = NULL;
  double *buftmp = NULL;
  int **year_learn = NULL;
  int **month_learn = NULL;
  int **day_learn = NULL;
  int **hour_learn = NULL;
  int **min_learn = NULL;
  float **sec_learn = NULL;

  double minlon = -15.0;
  double maxlon = 20.0;
  double minlat = 38.0;
  double maxlat = 60.0;

  int i;
  int t;
  int season;

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
    else if ( !strcmp(argv[i], "-i_learn") ) {
      filein_learn = (char *) malloc((strlen(argv[++i])+1) * sizeof(char));
      if (filein_learn == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(filein_learn, argv[i]);
    }
    else {
      (void) fprintf(stderr, "%s:: Wrong arg %s.\n\n", basename(argv[0]), argv[i]);
      (void) show_usage(basename(argv[0]));
      (void) banner(basename(argv[0]), "ABORT", "END");
      (void) abort();
    }
  }

  istat = utInit("");

  /* Read data in NetCDF file */
  printf("%s: Reading info from input file %s.\n", __FILE__, filein);
  istat = nc_open(filein, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_dimid(ncinid, "time", &timediminid);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, timediminid, &dimval); /* get time length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  ntime = (int) dimval;

  istat = nc_inq_dimid(ncinid, "lat1", &latdiminid);  /* get ID for lat dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, latdiminid, &dimval); /* get lat length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  nlat = (int) dimval;

  istat = nc_inq_dimid(ncinid, "lon1", &londiminid);  /* get ID for lon dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, londiminid, &dimval); /* get lon length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  nlon = (int) dimval;
  
  istat = nc_inq_varid(ncinid, "time", &timeinid);  /* get ID for time variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_varid(ncinid, "er40.lat", &latinid);  /* get ID for lat variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_varid(ncinid, "er40.lon", &loninid);  /* get ID for lon variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_varid(ncinid, "tas", &varinid); /* get tas variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /** Read dimensions variables **/
  buftmp = (double *) malloc(nlat * nlon * sizeof(double));
  if (buftmp == NULL) alloc_error(__FILE__, __LINE__);
  lat = (double *) malloc(nlat * sizeof(double));
  if (lat == NULL) alloc_error(__FILE__, __LINE__);
  lon = (double *) malloc(nlon * sizeof(double));
  if (lon == NULL) alloc_error(__FILE__, __LINE__);
  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) nlat;
  count[1] = (size_t) nlon;
  count[2] = 0;

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid, latinid, start, count, buftmp);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  for (i=0; i<nlat; i++)
    lat[i] = buftmp[i*nlon];

  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) nlat;
  count[1] = (size_t) nlon;
  count[2] = 0;

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid, loninid, start, count, buftmp);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  for (i=0; i<nlon; i++)
    lon[i] = buftmp[i];
  (void) free(buftmp);
  
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
  tas = (double *) calloc(nlat*nlon*ntime, sizeof(double));
  if (tas == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  printf("%s: Reading datafrom input file %s.\n", __FILE__, filein);
  istat = nc_get_vara_double(ncinid, varinid, start, count, tas);
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
  printf("%s: Reading info from input file %s.\n", __FILE__, filein_learn);
  istat = nc_open(filein_learn, NC_NOWRITE, &ncinid_learn);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_dimid(ncinid_learn, "season", &seasondiminid_learn);  /* get ID for season dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid_learn, seasondiminid_learn, &dimval); /* get season length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  nseason_learn = (int) dimval;

  /** Dimensions function of season **/
  time_learn_dimid = malloc(nseason_learn * sizeof(int));
  if (time_learn_dimid == NULL) alloc_error(__FILE__, __LINE__);
  time_learn_varid = malloc(nseason_learn * sizeof(int));
  if (time_learn_varid == NULL) alloc_error(__FILE__, __LINE__);

  ntime_learn = malloc(nseason_learn * sizeof(int));
  if (ntime_learn == NULL) alloc_error(__FILE__, __LINE__);

  /** Variables function of season **/
  time_learn = (double **) malloc(nseason_learn * sizeof(double *));
  if (time_learn == NULL) alloc_error(__FILE__, __LINE__);

  year_learn = (int **) malloc(nseason_learn * sizeof(int *));
  if (year_learn == NULL) alloc_error(__FILE__, __LINE__);
  month_learn = (int **) malloc(nseason_learn * sizeof(int *));
  if (month_learn == NULL) alloc_error(__FILE__, __LINE__);
  day_learn = (int **) malloc(nseason_learn * sizeof(int *));
  if (day_learn == NULL) alloc_error(__FILE__, __LINE__);
  hour_learn = (int **) malloc(nseason_learn * sizeof(int *));
  if (hour_learn == NULL) alloc_error(__FILE__, __LINE__);
  min_learn = (int **) malloc(nseason_learn * sizeof(int *));
  if (min_learn == NULL) alloc_error(__FILE__, __LINE__);
  sec_learn = (float **) malloc(nseason_learn * sizeof(float *));
  if (sec_learn == NULL) alloc_error(__FILE__, __LINE__);

  for (i=0; i<nseason_learn; i++) {

    /** Dimensions **/
    if (i == 0)
      (void) strcpy(varname, "time_aut");
    else if (i == 1)
      (void) strcpy(varname, "time_hiv");
    else if (i == 2)
      (void) strcpy(varname, "time_pri");
    else if (i == 3)
      (void) strcpy(varname, "time_ete");
    else
      exit(1);
    istat = nc_inq_dimid(ncinid_learn, varname, &(time_learn_dimid[i]));  /* get ID for time_learn dimension */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    istat = nc_inq_dimlen(ncinid_learn, time_learn_dimid[i], &dimval); /* get dimension length */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    ntime_learn[i] = (int) dimval;

    istat = nc_inq_varid(ncinid_learn, varname, &(time_learn_varid[i]));  /* get ID for variable */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

    /* Allocate memory and set start and count */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) ntime_learn[i];
    count[1] = (size_t) 0;
    count[2] = (size_t) 0;
    /* Allocate memory */
    time_learn[i] = (double *) calloc(ntime_learn[i], sizeof(double));
    if (time_learn[i] == NULL) alloc_error(__FILE__, __LINE__);
    
    /* Read values from netCDF variable */
    printf("%s: Reading data from input file %s.\n", __FILE__, filein_learn);
    istat = nc_get_vara_double(ncinid_learn, time_learn_varid[i], start, count, time_learn[i]);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    //    printf("%d %lf\n",i,time_learn[i][0]);

    /* Get variable information */
    istat = nc_inq_var(ncinid_learn, time_learn_varid[i], (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    
    if (varndims != 1) {
      (void) fprintf(stderr, "Error NetCDF type and/or dimensions Line %d.\n", __LINE__);
      (void) banner(basename(argv[0]), "ABORT", "END");
      (void) abort();
    }

    /* Get time units attribute length */
    istat = nc_inq_attlen(ncinid_learn, time_learn_varid[i], "units", &t_len_learn);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    /* Allocate required space before retrieving values */
    time_units_learn = (char *) malloc(t_len_learn * sizeof(char));
    if (time_units_learn == NULL) alloc_error(__FILE__, __LINE__);
    /* Get time units attribute value */
    istat = nc_get_att_text(ncinid_learn, time_learn_varid[i], "units", time_units_learn);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    time_units_learn[t_len_learn-2] = '\0'; /* null terminate */
    
    /* Get calendar type attribute length */
    istat = nc_inq_attlen(ncinid_learn, time_learn_varid[i], "calendar", &t_len_learn);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    /* Allocate required space before retrieving values */
    cal_type_learn = (char *) malloc(t_len_learn + 1);
    if (cal_type_learn == NULL) alloc_error(__FILE__, __LINE__);
    /* Get calendar type attribute value */
    istat = nc_get_att_text(ncinid_learn, time_learn_varid[i], "calendar", cal_type_learn);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    cal_type_learn[t_len_learn] = '\0'; /* null terminate */
    
    /* Compute time info */
    year_learn[i] = (int *) malloc(ntime_learn[i] * sizeof(int));
    if (year_learn[i] == NULL) alloc_error(__FILE__, __LINE__);
    month_learn[i] = (int *) malloc(ntime_learn[i] * sizeof(int));
    if (month_learn[i] == NULL) alloc_error(__FILE__, __LINE__);
    day_learn[i] = (int *) malloc(ntime_learn[i] * sizeof(int));
    if (day_learn[i] == NULL) alloc_error(__FILE__, __LINE__);
    hour_learn[i] = (int *) malloc(ntime_learn[i] * sizeof(int));
    if (hour_learn[i] == NULL) alloc_error(__FILE__, __LINE__);
    min_learn[i] = (int *) malloc(ntime_learn[i] * sizeof(int));
    if (min_learn[i] == NULL) alloc_error(__FILE__, __LINE__);
    sec_learn[i] = (float *) malloc(ntime_learn[i] * sizeof(float));
    if (sec_learn[i] == NULL) alloc_error(__FILE__, __LINE__);

    istat = utScan(time_units_learn, &dataunits_learn);

    for (t=0; t<ntime_learn[i]; t++)
      istat = utCalendar_cal(time_learn[i][t], &dataunits_learn,
                             &(year_learn[i][t]), &(month_learn[i][t]), &(day_learn[i][t]), &(hour_learn[i][t]),
                             &(min_learn[i][t]), &(sec_learn[i][t]), cal_type_learn);

    (void) free(time_units_learn);
    (void) free(cal_type_learn);


  } /* Season loop */

  /* Compute time info */

  year = (int *) malloc(ntime * sizeof(int));
  if (year == NULL) alloc_error(__FILE__, __LINE__);
  month = (int *) malloc(ntime * sizeof(int));
  if (month == NULL) alloc_error(__FILE__, __LINE__);
  day = (int *) malloc(ntime * sizeof(int));
  if (day == NULL) alloc_error(__FILE__, __LINE__);
  hour = (int *) malloc(ntime * sizeof(int));
  if (hour == NULL) alloc_error(__FILE__, __LINE__);
  min = (int *) malloc(ntime * sizeof(int));
  if (min == NULL) alloc_error(__FILE__, __LINE__);
  sec = (float *) malloc(ntime * sizeof(float));
  if (sec == NULL) alloc_error(__FILE__, __LINE__);

  istat = utScan(time_units, &dataunits);
  for (t=0; t<ntime; t++)
    istat = utCalendar_cal(timein[t], &dataunits, &(year[t]), &(month[t]), &(day[t]), &(hour[t]), &(min[t]), &(sec[t]), cal_type);
  (void) free(time_units);
  (void) free(cal_type);

  /* Extract subdomain */
  (void) extract_subdomain(&tas_sub, &nlon_sub, &nlat_sub, tas, lon, lat, minlon, maxlon, minlat, maxlat,
                           nlon, nlat, ntime);

  for (season=0; season<nseason_learn; season++) {

    (void) printf("Season=%d\n", season);

    /* Select sub period of field analyzed */
    /* Assuming season=0 is autumn and the rest in order, that is winter, spring, summer */
    (void) sub_period_common(&tas_subt, &ntime_sub, tas_sub, year, month, day, year_learn[season], month_learn[season], day_learn[season],
                             nlon_sub, nlat_sub, ntime, ntime_learn[season]);

    tas_smean = (double *) malloc(ntime * sizeof(double));
    if (tas_smean == NULL) alloc_error(__FILE__, __LINE__);

    /* Compute mean and variance of spatially-averaged temperature */
    (void) mean_variance_field_spatial(tas_smean, &mean, &var, tas_subt, nlon_sub, nlat_sub, ntime_sub);

    (void) printf("Season: %d  TAS mean=%lf variance=%lf\n", season, mean, var);

    (void) free(tas_subt);
    (void) free(tas_smean);
  }

  /** Close NetCDF files **/
  /* Close the intput netCDF file. */
  istat = ncclose(ncinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = ncclose(ncinid_learn);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  (void) free(time_learn_dimid);
  (void) free(time_learn_varid);
  (void) free(ntime_learn);
  for (i=0; i<nseason_learn; i++) {
    (void) free(time_learn[i]);
    (void) free(year_learn[i]);
    (void) free(month_learn[i]);
    (void) free(day_learn[i]);
    (void) free(hour_learn[i]);
    (void) free(min_learn[i]);
    (void) free(sec_learn[i]);
  }
  (void) free(time_learn);
  (void) free(year_learn);
  (void) free(month_learn);
  (void) free(day_learn);
  (void) free(hour_learn);
  (void) free(min_learn);
  (void) free(sec_learn);

  (void) free(tas);
  (void) free(tas_sub);
  (void) free(lon);
  (void) free(lat);
  (void) free(timein);
  (void) free(filein);
  (void) free(filein_learn);

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
