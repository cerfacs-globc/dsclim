/* ********************************************************* */
/* test_mean_variance_dist_clusters test program.            */
/* test_mean_variance_dist_clusters.c                        */
/* ********************************************************* */
/* Author: Christian Page, CERFACS, Toulouse, France.        */
/* ********************************************************* */
/*! \file test_mean_variance_dist_clusters.c
    \brief Test program for mean_variance_dist_clusters
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

#include <udunits.h>

#include <utils.h>
#include <filter.h>
#include <clim.h>
#include <classif.h>
#include <pceof.h>

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

  //  int nlat;
  // int nlon;
  //  int nlat_sub;
  //int nlon_sub;
  int ntime;
  int neof;
  int neof_poid;
  int neof_learn;
  int npts_learn;
  int nseason_learn;
  int *nclust_learn = NULL;
  int *ntime_learn = NULL;
  int *nclust = NULL;

  size_t dimval;

  char *filein = NULL;
  char *filein_poid = NULL;
  char *filein_learn = NULL;

  int istat, ncinid, ncinid_poid, ncinid_learn;
  int varinid, timeinid, timediminid;
  //  int loninid, londiminid, latinid, latdiminid;
  int eofdiminid;
  int eofdiminid_poid;
  int eofdiminid_learn, ptsdiminid_learn, seasondiminid_learn;
  int *clust_dimid = NULL;
  int *time_learn_dimid = NULL;
  int *clust_learn_dimid = NULL;
  int *time_learn_varid = NULL;
  int *cst_varid = NULL;
  int *reg_varid = NULL;
  int *rrd_varid = NULL;
  int *ta_varid = NULL;
  int eca_pc_learn_varid;
  int *clust_varid = NULL;
  int *poid_varid = NULL;
  int tancp_mean_varid, tancp_var_varid;
  nc_type vartype_main;
  nc_type vartype_time;
  int varndims;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  size_t start[3];
  size_t count[3];

  size_t t_len;
  char *time_units = NULL;
  char *cal_type = NULL;
  utUnit dataunits;

  char attname[1000];
  float valf;

  double *psl_pc = NULL;
  double *buf_sub = NULL;
  double *buftmp = NULL;
  int ntime_sub;
  double *timein = NULL;
  int *year = NULL;
  int *month = NULL;
  int *day = NULL;
  int *hour = NULL;
  int *min = NULL;
  float *sec = NULL;
  //  double *lat = NULL;
  // double *lon = NULL;

  double **poid = NULL;
  double **time_learn = NULL;
  int **year_learn = NULL;
  int **month_learn = NULL;
  int **day_learn = NULL;
  int **hour_learn = NULL;
  int **min_learn = NULL;
  float **sec_learn = NULL;
  double **cst_learn = NULL;
  double **reg_learn = NULL;
  double **rrd_learn = NULL;
  double **ta_learn = NULL;
  double *eca_pc_learn = NULL;
  double *tancp_mean = NULL;
  double *tancp_var = NULL;

  double **mean_dist = NULL;
  double **var_dist = NULL;

  double *var_pc_norm_all = NULL;

  double fillvalue_poid;
  double fillvalue_cst;
  double fillvalue_reg;
  double fillvalue_rrd;
  double fillvalue_ta;

  double first_variance = -9999.9999;
  
  char varname[500];

  int nseason = 4;

  int i;
  int t;
  int ii;

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
    else if ( !strcmp(argv[i], "-i_poid") ) {
      filein_poid = (char *) malloc((strlen(argv[++i])+1) * sizeof(char));
      if (filein_poid == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(filein_poid, argv[i]);
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

  istat = nc_inq_dimid(ncinid, "eof", &eofdiminid);  /* get ID for eof dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid, eofdiminid, &dimval); /* get lon length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  neof = (int) dimval;
  
  istat = nc_inq_varid(ncinid, "time", &timeinid);  /* get ID for time variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_varid(ncinid, "pc_proj", &varinid); /* get variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /** Read dimensions variables **/
  /* Allocate memory and set start and count */

  /* Get time dimensions and type */
  istat = nc_inq_var(ncinid, timeinid, (char *) NULL, &vartype_time, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  
  if (varndims != 1) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions Line %d.\n", __LINE__);
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

  if (varndims != 2) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions Line %d.\n", __LINE__);
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }

  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) ntime;
  count[1] = (size_t) neof;
  count[2] = 0;
  /* Allocate memory */
  psl_pc = (double *) calloc(ntime*neof, sizeof(double));
  if (psl_pc == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  printf("%s: Reading data from input file %s.\n", __FILE__, filein);
  istat = nc_get_vara_double(ncinid, varinid, start, count, psl_pc);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);


  /* Get time units attribute length */
  istat = nc_inq_attlen(ncinid, timeinid, "units", &t_len);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  /* Allocate required space before retrieving values */
  time_units = (char *) malloc(t_len * sizeof(char));
  if (time_units == NULL) alloc_error(__FILE__, __LINE__);
  /* Get time units attribute value */
  istat = nc_get_att_text(ncinid, timeinid, "units", time_units);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  time_units[t_len-2] = '\0'; /* null terminate */

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

  /* Read data in NetCDF file */
  printf("%s: Reading info from input file %s.\n", __FILE__, filein_poid);
  istat = nc_open(filein_poid, NC_NOWRITE, &ncinid_poid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_dimid(ncinid_poid, "eof", &eofdiminid_poid);  /* get ID for eof dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid_poid, eofdiminid_poid, &dimval); /* get eof length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  neof_poid = (int) dimval;

  clust_dimid = malloc(nseason * sizeof(int));
  if (clust_dimid == NULL) alloc_error(__FILE__, __LINE__);
  poid_varid = malloc(nseason * sizeof(int));
  if (poid_varid == NULL) alloc_error(__FILE__, __LINE__);

  clust_varid = malloc(nseason * sizeof(int));
  if (clust_varid == NULL) alloc_error(__FILE__, __LINE__);

  nclust = malloc(nseason * sizeof(int));
  if (nclust == NULL) alloc_error(__FILE__, __LINE__);

  poid = (double **) malloc(nseason * sizeof(double *));
  if (poid == NULL) alloc_error(__FILE__, __LINE__);

  for (i=0; i<nseason; i++) {
    if (i == 0)
      (void) strcpy(varname, "clust_aut");
    else if (i == 1)
      (void) strcpy(varname, "clust_hiv");
    else if (i == 2)
      (void) strcpy(varname, "clust_pri");
    else if (i == 3)
      (void) strcpy(varname, "clust_ete");
    else
      exit(1);
    istat = nc_inq_dimid(ncinid_poid, varname, &(clust_dimid[i]));  /* get ID for clust_aut dimension */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    istat = nc_inq_dimlen(ncinid_poid, clust_dimid[i], &dimval); /* get dimension length */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    nclust[i] = (int) dimval;
  
    if (i == 0)
      (void) strcpy(varname, "poid_aut");
    else if (i == 1)
      (void) strcpy(varname, "poid_hiv");
    else if (i == 2)
      (void) strcpy(varname, "poid_pri");
    else if (i == 3)
      (void) strcpy(varname, "poid_ete");
    else
      exit(1);
    istat = nc_inq_varid(ncinid_poid, varname, &(poid_varid[i])); /* get poid variable ID */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

    /* Get variable information */
    istat = nc_inq_var(ncinid_poid, poid_varid[i], (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    
    if (varndims != 2) {
      (void) fprintf(stderr, "Error NetCDF type and/or dimensions Line %d.\n", __LINE__);
      (void) banner(basename(argv[0]), "ABORT", "END");
      (void) abort();
    }

    /* Get missing value and assume the same for all seasons */
    if (i == 0) {
      (void) strcpy(attname, "missing_value");
      if (vartype_main == NC_FLOAT) {
        istat = nc_get_att_float(ncinid_poid, poid_varid[0], attname, &valf);
        if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
        fillvalue_poid = (double) valf;
      }
      else if (vartype_main == NC_DOUBLE) {
        istat = nc_get_att_double(ncinid_poid, poid_varid[0], attname, &fillvalue_poid);
        if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
      }
    }

    /* Allocate memory and set start and count */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) nclust[i];
    count[1] = (size_t) neof_poid;
    count[2] = (size_t) 0;
    /* Allocate memory */
    poid[i] = (double *) calloc(nclust[i] * neof_poid, sizeof(double));
    if (poid[i] == NULL) alloc_error(__FILE__, __LINE__);
    
    /* Read values from netCDF variable */
    printf("%s: Reading data from input file %s.\n", __FILE__, filein_poid);
    istat = nc_get_vara_double(ncinid_poid, poid_varid[i], start, count, poid[i]);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  }


  /** Close NetCDF files **/
  /* Close the intput netCDF file. */
  istat = ncclose(ncinid_poid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);


  /* Read data in NetCDF file */
  printf("%s: Reading info from input file %s.\n", __FILE__, filein_learn);
  istat = nc_open(filein_learn, NC_NOWRITE, &ncinid_learn);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_dimid(ncinid_learn, "eof", &eofdiminid_learn);  /* get ID for eof dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid_learn, eofdiminid_learn, &dimval); /* get eof length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  neof_learn = (int) dimval;

  istat = nc_inq_dimid(ncinid_learn, "pts", &ptsdiminid_learn);  /* get ID for pts dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncinid_learn, ptsdiminid_learn, &dimval); /* get pts length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  npts_learn = (int) dimval;

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
  clust_learn_dimid = malloc(nseason_learn * sizeof(int));
  if (clust_learn_dimid == NULL) alloc_error(__FILE__, __LINE__);

  cst_varid = malloc(nseason_learn * sizeof(int));
  if (cst_varid == NULL) alloc_error(__FILE__, __LINE__);
  reg_varid = malloc(nseason_learn * sizeof(int));
  if (reg_varid == NULL) alloc_error(__FILE__, __LINE__);
  rrd_varid = malloc(nseason_learn * sizeof(int));
  if (rrd_varid == NULL) alloc_error(__FILE__, __LINE__);
  ta_varid = malloc(nseason_learn * sizeof(int));
  if (ta_varid == NULL) alloc_error(__FILE__, __LINE__);

  nclust_learn = malloc(nseason_learn * sizeof(int));
  if (nclust_learn == NULL) alloc_error(__FILE__, __LINE__);
  ntime_learn = malloc(nseason_learn * sizeof(int));
  if (ntime_learn == NULL) alloc_error(__FILE__, __LINE__);

  /** Variables function of season **/
  time_learn = (double **) malloc(nseason_learn * sizeof(double *));
  if (time_learn == NULL) alloc_error(__FILE__, __LINE__);
  cst_learn = (double **) malloc(nseason_learn * sizeof(double *));
  if (cst_learn == NULL) alloc_error(__FILE__, __LINE__);
  reg_learn = (double **) malloc(nseason_learn * sizeof(double *));
  if (reg_learn == NULL) alloc_error(__FILE__, __LINE__);
  rrd_learn = (double **) malloc(nseason_learn * sizeof(double *));
  if (rrd_learn == NULL) alloc_error(__FILE__, __LINE__);
  ta_learn = (double **) malloc(nseason_learn * sizeof(double *));
  if (ta_learn == NULL) alloc_error(__FILE__, __LINE__);

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
    istat = nc_inq_attlen(ncinid_learn, time_learn_varid[i], "units", &t_len);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    /* Allocate required space before retrieving values */
    time_units = (char *) malloc(t_len * sizeof(char));
    if (time_units == NULL) alloc_error(__FILE__, __LINE__);
    /* Get time units attribute value */
    istat = nc_get_att_text(ncinid_learn, time_learn_varid[i], "units", time_units);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    time_units[t_len-2] = '\0'; /* null terminate */
    
    /* Get calendar type attribute length */
    istat = nc_inq_attlen(ncinid_learn, time_learn_varid[i], "calendar", &t_len);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    /* Allocate required space before retrieving values */
    cal_type = (char *) malloc(t_len + 1);
    if (cal_type == NULL) alloc_error(__FILE__, __LINE__);
    /* Get calendar type attribute value */
    istat = nc_get_att_text(ncinid_learn, time_learn_varid[i], "calendar", cal_type);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    cal_type[t_len] = '\0'; /* null terminate */
    
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

    istat = utScan(time_units, &dataunits);

    for (t=0; t<ntime_learn[i]; t++)
      istat = utCalendar_cal(time_learn[i][t], &dataunits,
                             &(year_learn[i][t]), &(month_learn[i][t]), &(day_learn[i][t]), &(hour_learn[i][t]),
                             &(min_learn[i][t]), &(sec_learn[i][t]), cal_type);

    (void) free(time_units);
    (void) free(cal_type);


    if (i == 0)
      (void) strcpy(varname, "clust_aut");
    else if (i == 1)
      (void) strcpy(varname, "clust_hiv");
    else if (i == 2)
      (void) strcpy(varname, "clust_pri");
    else if (i == 3)
      (void) strcpy(varname, "clust_ete");
    else
      exit(1);
    istat = nc_inq_dimid(ncinid_learn, varname, &(clust_learn_dimid[i]));  /* get ID for clust_learn dimension */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    istat = nc_inq_dimlen(ncinid_learn, clust_learn_dimid[i], &dimval); /* get dimension length */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    nclust_learn[i] = (int) dimval;
  

    /** Variables **/
    if (i == 0)
      (void) strcpy(varname, "cst_aut");
    else if (i == 1)
      (void) strcpy(varname, "cst_hiv");
    else if (i == 2)
      (void) strcpy(varname, "cst_pri");
    else if (i == 3)
      (void) strcpy(varname, "cst_ete");
    else
      exit(1);
    istat = nc_inq_varid(ncinid_learn, varname, &(cst_varid[i])); /* get cst variable ID */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

    /* Get variable information */
    istat = nc_inq_var(ncinid_learn, cst_varid[i], (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    
    if (varndims != 1) {
      (void) fprintf(stderr, "Error NetCDF type and/or dimensions Line %d.\n", __LINE__);
      (void) banner(basename(argv[0]), "ABORT", "END");
      (void) abort();
    }

    /* Get missing value and assume the same for all seasons */
    if (i == 0) {
      (void) strcpy(attname, "missing_value");
      if (vartype_main == NC_FLOAT) {
        istat = nc_get_att_float(ncinid_learn, cst_varid[0], attname, &valf);
        if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
        fillvalue_cst = (double) valf;
      }
      else if (vartype_main == NC_DOUBLE) {
        istat = nc_get_att_double(ncinid_learn, cst_varid[0], attname, &fillvalue_cst);
        if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
      }
    }

    /* Allocate memory and set start and count */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) npts_learn;
    count[1] = (size_t) 0;
    count[2] = (size_t) 0;
    /* Allocate memory */
    cst_learn[i] = (double *) calloc(npts_learn, sizeof(double));
    if (cst_learn[i] == NULL) alloc_error(__FILE__, __LINE__);
    
    /* Read values from netCDF variable */
    printf("%s: Reading data from input file %s.\n", __FILE__, filein_learn);
    istat = nc_get_vara_double(ncinid_learn, cst_varid[i], start, count, cst_learn[i]);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);


    if (i == 0)
      (void) strcpy(varname, "reg_aut");
    else if (i == 1)
      (void) strcpy(varname, "reg_hiv");
    else if (i == 2)
      (void) strcpy(varname, "reg_pri");
    else if (i == 3)
      (void) strcpy(varname, "reg_ete");
    else
      exit(1);
    istat = nc_inq_varid(ncinid_learn, varname, &(reg_varid[i])); /* get reg variable ID */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

    /* Get variable information */
    istat = nc_inq_var(ncinid_learn, reg_varid[i], (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    
    if (varndims != 2) {
      (void) fprintf(stderr, "Error NetCDF type and/or dimensions Line %d.\n", __LINE__);
      (void) banner(basename(argv[0]), "ABORT", "END");
      (void) abort();
    }

    /* Get missing value and assume the same for all seasons */
    if (i == 0) {
      (void) strcpy(attname, "missing_value");
      if (vartype_main == NC_FLOAT) {
        istat = nc_get_att_float(ncinid_learn, reg_varid[0], attname, &valf);
        if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
        fillvalue_reg = (double) valf;
      }
      else if (vartype_main == NC_DOUBLE) {
        istat = nc_get_att_double(ncinid_learn, reg_varid[0], attname, &fillvalue_reg);
        if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
      }
    }

    /* Allocate memory and set start and count */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) nclust_learn[i];
    count[1] = (size_t) npts_learn;
    count[2] = (size_t) 0;
    /* Allocate memory */
    reg_learn[i] = (double *) calloc(nclust_learn[i] * npts_learn, sizeof(double));
    if (reg_learn[i] == NULL) alloc_error(__FILE__, __LINE__);
    
    /* Read values from netCDF variable */
    printf("%s: Reading data from input file %s.\n", __FILE__, filein_learn);
    istat = nc_get_vara_double(ncinid_learn, reg_varid[i], start, count, reg_learn[i]);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);


    if (i == 0)
      (void) strcpy(varname, "rrd_aut");
    else if (i == 1)
      (void) strcpy(varname, "rrd_hiv");
    else if (i == 2)
      (void) strcpy(varname, "rrd_pri");
    else if (i == 3)
      (void) strcpy(varname, "rrd_ete");
    else
      exit(1);
    istat = nc_inq_varid(ncinid_learn, varname, &(rrd_varid[i])); /* get rrd variable ID */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

    /* Get variable information */
    istat = nc_inq_var(ncinid_learn, rrd_varid[i], (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    if (varndims != 2) {
      (void) fprintf(stderr, "Error NetCDF type and/or dimensions Line %d.\n", __LINE__);
      (void) banner(basename(argv[0]), "ABORT", "END");
      (void) abort();
    }

    /* Get missing value and assume the same for all seasons */
    if (i == 0) {
      (void) strcpy(attname, "missing_value");
      if (vartype_main == NC_FLOAT) {
        istat = nc_get_att_float(ncinid_learn, rrd_varid[0], attname, &valf);
        if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
        fillvalue_rrd = (double) valf;
      }
      else if (vartype_main == NC_DOUBLE) {
        istat = nc_get_att_double(ncinid_learn, rrd_varid[0], attname, &fillvalue_rrd);
        if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
      }
    }

    /* Allocate memory and set start and count */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) ntime_learn[i];
    count[1] = (size_t) npts_learn;
    count[2] = (size_t) 0;
    /* Allocate memory */
    rrd_learn[i] = (double *) calloc(ntime_learn[i] * npts_learn, sizeof(double));
    if (rrd_learn[i] == NULL) alloc_error(__FILE__, __LINE__);
    
    /* Read values from netCDF variable */
    printf("%s: Reading data from input file %s.\n", __FILE__, filein_learn);
    istat = nc_get_vara_double(ncinid_learn, rrd_varid[i], start, count, rrd_learn[i]);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);


    if (i == 0)
      (void) strcpy(varname, "ta_aut");
    else if (i == 1)
      (void) strcpy(varname, "ta_hiv");
    else if (i == 2)
      (void) strcpy(varname, "ta_pri");
    else if (i == 3)
      (void) strcpy(varname, "ta_ete");
    else
      exit(1);
    istat = nc_inq_varid(ncinid_learn, varname, &(ta_varid[i])); /* get ta variable ID */
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

    /* Get variable information */
    istat = nc_inq_var(ncinid_learn, ta_varid[i], (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
    
    if (varndims != 1) {
      (void) fprintf(stderr, "Error NetCDF type and/or dimensions Line %d.\n", __LINE__);
      (void) banner(basename(argv[0]), "ABORT", "END");
      (void) abort();
    }

    /* Get missing value and assume the same for all seasons */
    if (i == 0) {
      (void) strcpy(attname, "missing_value");
      if (vartype_main == NC_FLOAT) {
        istat = nc_get_att_float(ncinid_learn, ta_varid[0], attname, &valf);
        if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
        fillvalue_ta = (double) valf;
      }
      else if (vartype_main == NC_DOUBLE) {
        istat = nc_get_att_double(ncinid_learn, ta_varid[0], attname, &fillvalue_ta);
        if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
      }
    }

    /* Allocate memory and set start and count */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) ntime_learn[i];
    count[1] = (size_t) 0;
    count[2] = (size_t) 0;
    /* Allocate memory */
    ta_learn[i] = (double *) calloc(ntime_learn[i], sizeof(double));
    if (ta_learn[i] == NULL) alloc_error(__FILE__, __LINE__);
    
    /* Read values from netCDF variable */
    printf("%s: Reading data from input file %s.\n", __FILE__, filein_learn);
    istat = nc_get_vara_double(ncinid_learn, ta_varid[i], start, count, ta_learn[i]);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  } /* Season loop */

  istat = nc_inq_varid(ncinid_learn, "eca_pc_learn", &eca_pc_learn_varid); /* get variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  
  /* Get variable information */
  istat = nc_inq_var(ncinid_learn, eca_pc_learn_varid, (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  
  if (varndims != 1) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions Line %d.\n", __LINE__);
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }
  
  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) neof_learn;
  count[1] = (size_t) 0;
  count[2] = (size_t) 0;
  /* Allocate memory */
  eca_pc_learn = (double *) calloc(neof_learn, sizeof(double));
  if (eca_pc_learn == NULL) alloc_error(__FILE__, __LINE__);
  
  /* Read values from netCDF variable */
  printf("%s: Reading data from input file %s.\n", __FILE__, filein_learn);
  istat = nc_get_vara_double(ncinid_learn, eca_pc_learn_varid, start, count, eca_pc_learn);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);


  istat = nc_inq_varid(ncinid_learn, "tancp_mean", &tancp_mean_varid); /* get variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  
  /* Get variable information */
  istat = nc_inq_var(ncinid_learn, tancp_mean_varid, (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  
  if (varndims != 1) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions Line %d.\n", __LINE__);
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }
  
  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) nseason_learn;
  count[1] = (size_t) 0;
  count[2] = (size_t) 0;
  /* Allocate memory */
  tancp_mean = (double *) calloc(nseason_learn, sizeof(double));
  if (tancp_mean == NULL) alloc_error(__FILE__, __LINE__);
  
  /* Read values from netCDF variable */
  printf("%s: Reading data from input file %s.\n", __FILE__, filein_learn);
  istat = nc_get_vara_double(ncinid_learn, tancp_mean_varid, start, count, tancp_mean);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);


  istat = nc_inq_varid(ncinid_learn, "tancp_var", &tancp_var_varid); /* get variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  
  /* Get variable information */
  istat = nc_inq_var(ncinid_learn, tancp_var_varid, (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  
  if (varndims != 1) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions Line %d.\n", __LINE__);
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }
  
  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) nseason_learn;
  count[1] = (size_t) 0;
  count[2] = (size_t) 0;
  /* Allocate memory */
  tancp_var = (double *) calloc(nseason_learn, sizeof(double));
  if (tancp_var == NULL) alloc_error(__FILE__, __LINE__);
  
  /* Read values from netCDF variable */
  printf("%s: Reading data from input file %s.\n", __FILE__, filein_learn);
  istat = nc_get_vara_double(ncinid_learn, tancp_var_varid, start, count, tancp_var);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  

  /***** BEGIN Compute mean and variance of distances to clusters *****/

  (void) printf("Computing mean and variance of distances to clusters.\n");

  mean_dist = (double **) malloc(nseason * sizeof(double *));
  if (mean_dist == NULL) alloc_error(__FILE__, __LINE__);
  var_dist = (double **) malloc(nseason * sizeof(double *));
  if (var_dist == NULL) alloc_error(__FILE__, __LINE__);

  for (i=0; i<nseason; i++) {

    mean_dist[i] = (double *) malloc(nclust[i] * sizeof(double));
    if (mean_dist[i] == NULL) alloc_error(__FILE__, __LINE__);
    var_dist[i] = (double *) malloc(nclust[i] * sizeof(double));
    if (var_dist[i] == NULL) alloc_error(__FILE__, __LINE__);

    /* Normalisation of the principal component by the square root of the variance of the first one */

    var_pc_norm_all = (double *) malloc(neof * sizeof(double));
    if (var_pc_norm_all == NULL) alloc_error(__FILE__, __LINE__);
    buftmp = (double *) malloc(ntime*neof * sizeof(double));
    if (buftmp == NULL) alloc_error(__FILE__, __LINE__);

    first_variance = -9999.9999;
    (void) normalize_pc(var_pc_norm_all, &first_variance, buftmp, psl_pc, neof, ntime);

    /* Select sub period of field analyzed */
    /* Assuming season=0 is autumn and the rest in order, that is winter, spring, summer */
    (void) sub_period_common(&buf_sub, &ntime_sub, buftmp, year, month, day, year_learn[i], month_learn[i], day_learn[i], 1,
                             neof, 1, ntime, ntime_learn[i]);
    (void) free(buftmp);
    
    (void) mean_variance_dist_clusters(mean_dist[i], var_dist[i], buf_sub, poid[i], eca_pc_learn, var_pc_norm_all,
                                       neof, nclust[i], ntime_sub);
    (void) free(buf_sub);
    (void) free(var_pc_norm_all);

    printf("Season: %d\n", i);
    for (ii=0; ii<nclust[i]; ii++)
      printf("Cluster #%d. Mean and variance of distances to clusters: %lf %lf\n", ii, mean_dist[i][ii], var_dist[i][ii]);

  }

  /***** END Compute mean and variance of distances to clusters *****/


  /** Close NetCDF files **/
  /* Close the intput netCDF file. */
  istat = ncclose(ncinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  (void) utTerm();

  (void) free(psl_pc);

  (void) free(timein);
  (void) free(filein);
  (void) free(filein_poid);
  (void) free(filein_learn);

  for (i=0; i<nseason; i++) {
    (void) free(poid[i]);
    (void) free(mean_dist[i]);
    (void) free(var_dist[i]);
  }
  for (i=0; i<nseason_learn; i++) {
    (void) free(time_learn[i]);
    (void) free(cst_learn[i]);
    (void) free(reg_learn[i]);
    (void) free(rrd_learn[i]);
    (void) free(ta_learn[i]);
    (void) free(year_learn[i]);
    (void) free(month_learn[i]);
    (void) free(day_learn[i]);
    (void) free(hour_learn[i]);
    (void) free(min_learn[i]);
    (void) free(sec_learn[i]);
  }
  (void) free(poid);
  (void) free(mean_dist);
  (void) free(var_dist);
  (void) free(time_learn);
  (void) free(cst_learn);
  (void) free(reg_learn);
  (void) free(rrd_learn);
  (void) free(ta_learn);
  (void) free(year_learn);
  (void) free(month_learn);
  (void) free(day_learn);
  (void) free(hour_learn);
  (void) free(min_learn);
  (void) free(sec_learn);

  (void) free(clust_dimid);
  (void) free(poid_varid);
  (void) free(clust_varid);
  (void) free(time_learn_dimid);
  (void) free(time_learn_varid);
  (void) free(clust_learn_dimid);

  (void) free(cst_varid);
  (void) free(reg_varid);
  (void) free(rrd_varid);
  (void) free(ta_varid);

  (void) free(nclust);

  (void) free(nclust_learn);
  (void) free(ntime_learn);
  (void) free(eca_pc_learn);
  (void) free(tancp_mean);
  (void) free(tancp_var);

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
