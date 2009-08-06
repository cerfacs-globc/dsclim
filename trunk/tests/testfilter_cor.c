/* ***************************************************** */
/* testfilter_cor Test hanning filter implementation and */
/*                compute correlation                    */
/* testfilter_cor.c                                      */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file testfilter_cor.c
    \brief Test hanning filter implementation and compute correlation.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2009)

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
#include <filter.h>

#include <gsl/gsl_statistics.h>

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

  int nlat2;
  int nlon2;
  int ntime2;

  size_t dimval;
  int i;

  char *filein = NULL;
  char *filein2 = NULL;
  char *fileout = NULL;
  FILE *outptr;

  int istat;

  int ncinid;
  int varinid, timeinid, timediminid, londiminid, latdiminid;
  nc_type vartype_main;
  int varndims;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  int ncinid2;
  int varinid2, timeinid2, timediminid2, londiminid2, latdiminid2;
  nc_type vartype_main2;
  int varndims2;
  int vardimids2[NC_MAX_VAR_DIMS];    /* dimension ids */

  size_t start[3];
  size_t count[3];

  size_t t_len;
  char *time_units = NULL;
  size_t t_len2;
  char *time_units2 = NULL;

  int *year = NULL;
  int *month = NULL;
  int *day = NULL;
  int *hour = NULL;
  int *minutes = NULL;
  float *seconds = NULL;

  int *year2 = NULL;
  int *month2 = NULL;
  int *day2 = NULL;
  int *hour2 = NULL;
  int *minutes2 = NULL;
  float *seconds2 = NULL;

  float *buf = NULL;
  float *buf2 = NULL;
  double *timein = NULL;
  double *timein2 = NULL;

  double *invect;
  double *invect2;
  double *outvect;
  double *outvect2;

  double correl;
  int width[8];
  int wd;

  int firstt;
  int firstt2;
  int lastt;
  int lastt2;

  int t;
  int x;
  int y;

  int byear;
  int bmonth;
  int bday;
  int eyear;
  int emonth;
  int eday;

  char *varname = NULL;
  char *varname2 = NULL;

  int ntime_sub;
  double mean_cor;

  double missing_value;
  double missing_value2;

  int countpt;

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
    else if ( !strcmp(argv[i], "-i2") ) {
      filein2 = (char *) malloc((strlen(argv[++i])+1) * sizeof(char));
      if (filein2 == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(filein2, argv[i]);
    }
    else if ( !strcmp(argv[i], "-o") ) {
      fileout = (char *) malloc((strlen(argv[++i])+1) * sizeof(char));
      if (fileout == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(fileout, argv[i]);
    }
    else if ( !strcmp(argv[i], "-v1") ) {
      varname = (char *) malloc((strlen(argv[++i])+1) * sizeof(char));
      if (varname == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(varname, argv[i]);
    }
    else if ( !strcmp(argv[i], "-v2") ) {
      varname2 = (char *) malloc((strlen(argv[++i])+1) * sizeof(char));
      if (varname2 == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(varname2, argv[i]);
    }
    else if ( !strcmp(argv[i], "-byear") ) {
      (void) sscanf(argv[++i], "%d", &byear);
    }
    else if ( !strcmp(argv[i], "-bmonth") ) {
      (void) sscanf(argv[++i], "%d", &bmonth);
    }
    else if ( !strcmp(argv[i], "-bday") ) {
      (void) sscanf(argv[++i], "%d", &bday);
    }
    else if ( !strcmp(argv[i], "-eyear") ) {
      (void) sscanf(argv[++i], "%d", &eyear);
    }
    else if ( !strcmp(argv[i], "-emonth") ) {
      (void) sscanf(argv[++i], "%d", &emonth);
    }
    else if ( !strcmp(argv[i], "-eday") ) {
      (void) sscanf(argv[++i], "%d", &eday);
    }
    else {
      (void) fprintf(stderr, "%s:: Wrong arg %s.\n\n", basename(argv[0]), argv[i]);
      (void) show_usage(basename(argv[0]));
      (void) banner(basename(argv[0]), "ABORT", "END");
      (void) abort();
    }
  }

  /** First file **/

  /* Read data in NetCDF file */
  istat = nc_open(filein, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_dimid(ncinid, "time", &timediminid);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, timediminid, &dimval); /* get time length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  ntime = (int) dimval;

  istat = nc_inq_dimid(ncinid, "y", &latdiminid);  /* get ID for lat dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, latdiminid, &dimval); /* get lat length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  nlat = (int) dimval;

  istat = nc_inq_dimid(ncinid, "x", &londiminid);  /* get ID for lon dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, londiminid, &dimval); /* get lon length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  nlon = (int) dimval;
  
  istat = nc_inq_varid(ncinid, "time", &timeinid);  /* get ID for time variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_varid(ncinid, varname, &varinid); /* get buf variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Get missing_value attribute value */
  istat = nc_get_att_double(ncinid, varinid, "missing_value", &missing_value);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  printf("Missing value=%lf\n", missing_value);

  /* Get time units attribute length */
  istat = nc_inq_attlen(ncinid, timeinid, "units", &t_len);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  /* Allocate required space before retrieving values */
  time_units = (char *) malloc(t_len);
  if (time_units == NULL) alloc_error(__FILE__, __LINE__);
  /* Get time units attribute value */
  istat = nc_get_att_text(ncinid, timeinid, "units", time_units);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  time_units[t_len-2] = '\0'; /* null terminate */

  /* Allocate memory and set start and count */
  start[0] = 0;
  count[0] = (size_t) ntime;
  timein = malloc(ntime * sizeof(double));
  if (timein == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid, timeinid, start, count, timein);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  year = malloc(ntime * sizeof(int));
  if (year == NULL) alloc_error(__FILE__, __LINE__);
  month = malloc(ntime * sizeof(int));
  if (month == NULL) alloc_error(__FILE__, __LINE__);
  day = malloc(ntime * sizeof(int));
  if (day == NULL) alloc_error(__FILE__, __LINE__);
  hour = malloc(ntime * sizeof(int));
  if (hour == NULL) alloc_error(__FILE__, __LINE__);
  minutes = malloc(ntime * sizeof(int));
  if (minutes == NULL) alloc_error(__FILE__, __LINE__);
  seconds = malloc(ntime * sizeof(float));
  if (seconds == NULL) alloc_error(__FILE__, __LINE__);
  istat = get_calendar(year, month, day, hour, minutes, seconds, time_units, timein, ntime);

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
  buf = (float *) calloc(nlat*nlon*ntime, sizeof(float));
  if (buf == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  printf("%s: Reading data from input file %s.\n", __FILE__, filein);
  istat = nc_get_vara(ncinid, varinid, start, count, buf);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  
  /** Close NetCDF files **/
  /* Close the intput netCDF file. */
  istat = ncclose(ncinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /** Second file **/

  /* Read data in NetCDF file */
  istat = nc_open(filein2, NC_NOWRITE, &ncinid2);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_dimid(ncinid2, "time", &timediminid2);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid2, timediminid2, &dimval); /* get time length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  ntime2 = (int) dimval;

  istat = nc_inq_dimid(ncinid2, "y", &latdiminid2);  /* get ID for lat dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid2, latdiminid2, &dimval); /* get lat length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  nlat2 = (int) dimval;

  istat = nc_inq_dimid(ncinid2, "x", &londiminid2);  /* get ID for lon dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid2, londiminid2, &dimval); /* get lon length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  nlon2 = (int) dimval;
  
  istat = nc_inq_varid(ncinid2, "time", &timeinid2);  /* get ID for time variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_varid(ncinid2, varname2, &varinid2); /* get buf variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Get missing_value attribute value */
  istat = nc_get_att_double(ncinid2, varinid2, "missing_value", &missing_value2);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Get time units attribute length */
  istat = nc_inq_attlen(ncinid2, timeinid2, "units", &t_len2);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  /* Allocate required space before retrieving values */
  time_units2 = (char *) malloc(t_len2);
  if (time_units2 == NULL) alloc_error(__FILE__, __LINE__);
  /* Get time units attribute value */
  istat = nc_get_att_text(ncinid2, timeinid2, "units", time_units2);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  time_units2[t_len2-2] = '\0'; /* null terminate */

  /* Allocate memory and set start and count */
  start[0] = 0;
  count[0] = (size_t) ntime2;
  timein2 = malloc(ntime2 * sizeof(double));
  if (timein2 == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncinid2, timeinid2, start, count, timein2);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  year2 = malloc(ntime2 * sizeof(int));
  if (year2 == NULL) alloc_error(__FILE__, __LINE__);
  month2 = malloc(ntime2 * sizeof(int));
  if (month2 == NULL) alloc_error(__FILE__, __LINE__);
  day2 = malloc(ntime2 * sizeof(int));
  if (day2 == NULL) alloc_error(__FILE__, __LINE__);
  hour2 = malloc(ntime2 * sizeof(int));
  if (hour2 == NULL) alloc_error(__FILE__, __LINE__);
  minutes2 = malloc(ntime2 * sizeof(int));
  if (minutes2 == NULL) alloc_error(__FILE__, __LINE__);
  seconds2 = malloc(ntime2 * sizeof(float));
  if (seconds2 == NULL) alloc_error(__FILE__, __LINE__);
  istat = get_calendar(year2, month2, day2, hour2, minutes2, seconds2, time_units2, timein2, ntime2);

  /** Read data variable **/
  
  /* Get variable information */
  istat = nc_inq_var(ncinid2, varinid2, (char *) NULL, &vartype_main2, &varndims2, vardimids2, (int *) NULL);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  if (varndims2 != 3) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions.\n");
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }

  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) ntime2;
  count[1] = (size_t) nlat2;
  count[2] = (size_t) nlon2;
  /* Allocate memory */
  buf2 = (float *) calloc(nlat2*nlon2*ntime2, sizeof(float));
  if (buf2 == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  printf("%s: Reading data from input file %s.\n", __FILE__, filein2);
  istat = nc_get_vara(ncinid2, varinid2, start, count, buf2);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);  

  /** Close NetCDF files **/
  /* Close the intput netCDF file. */
  istat = ncclose(ncinid2);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);


  /**** Compute statistics ****/

  outptr = fopen(fileout, "w");
  if (outptr == NULL) {
    (void) fprintf(stderr, "Cannot open output file : %s.\n", fileout);
    (void) abort();
  }

  invect = malloc(ntime * sizeof(double));
  if (invect == NULL) alloc_error(__FILE__, __LINE__);
  invect2 = malloc(ntime2 * sizeof(double));
  if (invect == NULL) alloc_error(__FILE__, __LINE__);

  outvect = malloc(ntime * sizeof(double));
  if (outvect == NULL) alloc_error(__FILE__, __LINE__);
  outvect2 = malloc(ntime2 * sizeof(double));
  if (outvect == NULL) alloc_error(__FILE__, __LINE__);
  
  firstt = -1;
  lastt = -1;
  for (t=0; t<ntime; t++) {
    if (year[t] == byear && month[t] == bmonth && day[t] == bday) {
      firstt = t;
      t = ntime;
    }
  }
  for (t=0; t<ntime; t++) {
    if (year[t] == eyear && month[t] == emonth && day[t] == eday) {
      lastt = t;
      t = ntime;
    }
  }
  if (firstt == -1 || lastt == -1) {
    (void) fprintf(stderr, "Error in searching for time boundaries! %d %d\n", firstt, lastt);
    (void) exit(1);
  }
  ntime_sub = lastt - firstt + 1;

  firstt2 = -1;
  lastt2 = -1;
  for (t=0; t<ntime2; t++) {
    if (year2[t] == byear && month2[t] == bmonth && day2[t] == bday) {
      firstt2 = t;
      t = ntime2;
    }
  }
  for (t=0; t<ntime2; t++) {
    if (year2[t] == eyear && month2[t] == emonth && day2[t] == eday) {
      lastt2 = t;
      t = ntime2;
    }
  }
  if (firstt2 == -1 || lastt2 == -1) {
    (void) fprintf(stderr, "Error in searching for time boundaries! %d %d\n", firstt2, lastt2);
    (void) exit(1);
  }

  if (ntime_sub != (lastt2 - firstt2 + 1)) {
    (void) fprintf(stderr, "Error: cannot compare time series of different lengths %d vs %d! %d %d %d %d\n", ntime_sub, lastt2 - firstt2 + 1, firstt, lastt, firstt2, lastt2);
    (void) exit(1);
  }

  width[0] = 4;
  width[1] = 30;
  width[2] = 60;
  width[3] = 90;
  width[4] = 120;
  width[5] = 182;
  width[6] = 365;
  for (wd=0; wd<7; wd++) {

    mean_cor = 0.0;

    (void) fprintf(stdout, "Filter width=%d\n", width[wd]);

    countpt = 0;
    for (y=0; y<nlat; y++)
      for (x=0; x<nlon; x++) {

        if (buf[x+y*nlon] != missing_value) {
          
          for (t=firstt; t<=lastt; t++) {
            invect[t] = buf[x+y*nlon+t*nlat*nlon];
          }
          for (t=firstt2; t<=lastt2; t++)
            invect2[t] = buf2[x+y*nlon2+t*nlat2*nlon2];
          
          filter(outvect, invect, "hanning", width[wd], 1, 1, ntime_sub);
          /*          for (t=firstt; t<=lastt; t++)
                      printf("%d %lf %lf\n",t,invect[t],outvect[t]);*/
          filter(outvect2, invect2, "hanning", width[wd], 1, 1, ntime_sub);
          correl = gsl_stats_correlation(outvect, 1, outvect2, 1, ntime_sub);
          /*          printf("%lf\n",correl);*/
          mean_cor = mean_cor + correl;

          countpt++;
        }
      }

    mean_cor = mean_cor / (float) (countpt);
    printf("%d %lf %d\n", width[wd], mean_cor, countpt);
    (void) fprintf(outptr, "%d %lf\n", width[wd], mean_cor);

  }

  (void) fclose(outptr);

  /**** End statistics ****/

  (void) free(buf);
  (void) free(filein);

  (void) free(buf2);
  (void) free(filein2);

  (void) free(fileout);

  (void) free(timein);
  (void) free(timein2);

  (void) free(invect);
  (void) free(invect2);

  (void) free(outvect);
  (void) free(outvect2);

  (void) free(year);
  (void) free(month);
  (void) free(day);
  (void) free(hour);
  (void) free(minutes);
  (void) free(seconds);

  (void) free(year2);
  (void) free(month2);
  (void) free(day2);
  (void) free(hour2);
  (void) free(minutes2);
  (void) free(seconds2);

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
