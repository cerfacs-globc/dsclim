/* ***************************************************** */
/* read_learning_obs_eof Read observation data EOFs for  */
/* learning period.                                      */
/* read_learning_obs_eof.c                               */
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
/*! \file read_learning_obs_eof.c
    \brief Read observation data EOFs for learning period. Currently only NetCDF is implemented.
*/

#include <dsclim.h>

/** Read observation data EOFs for learning period. Currently only NetCDF is implemented. */
int read_learning_obs_eof(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat; /* Diagnostic status */
  int neof; /* EOF dimension for main large-scale fields in input file */
  int ntime; /* Time dimension for main large-scale fields in input file */
  double *time;
  info_field_struct info_eof;
  char *cal_type = NULL; /* Calendar type (udunits) */
  char *time_units = NULL; /* Time units (udunits) */
  int i;

  /* Read EOF */
  istat = read_netcdf_var_2d(&(data->learning->obs->eof), &info_eof, (proj_struct *) NULL,
                             data->learning->obs->filename_eof, data->learning->obs->nomvar_eof,
                             data->conf->eofname, data->conf->timename, &neof, &ntime, TRUE);
  if (istat != 0) {
    /* In case of failure */
    return istat;
  }
  
  /* Get time information */
  istat = get_time_info(data->learning->obs->time_s, &time, &time_units, &cal_type,
                        &ntime, data->learning->obs->filename_eof, data->learning->nomvar_time, TRUE);
  (void) free(cal_type);
  (void) free(time_units);

  data->learning->ntime = ntime;
  data->learning->obs->ntime = ntime;

  /* Copy into main time structure */
  data->learning->time_s->year = (int *) malloc(ntime * sizeof(int));
  if (data->learning->time_s->year == NULL) alloc_error(__FILE__, __LINE__);
  data->learning->time_s->month = (int *) malloc(ntime * sizeof(int));
  if (data->learning->time_s->month == NULL) alloc_error(__FILE__, __LINE__);
  data->learning->time_s->day = (int *) malloc(ntime * sizeof(int));
  if (data->learning->time_s->day == NULL) alloc_error(__FILE__, __LINE__);
  data->learning->time_s->hour = (int *) malloc(ntime * sizeof(int));
  if (data->learning->time_s->hour == NULL) alloc_error(__FILE__, __LINE__);
  data->learning->time_s->minutes = (int *) malloc(ntime * sizeof(int));
  if (data->learning->time_s->minutes == NULL) alloc_error(__FILE__, __LINE__);
  data->learning->time_s->seconds = (float *) malloc(ntime * sizeof(float));
  if (data->learning->time_s->seconds == NULL) alloc_error(__FILE__, __LINE__);

  for (i=0; i<ntime; i++) {
    data->learning->time_s->year[i] = data->learning->obs->time_s->year[i];
    data->learning->time_s->month[i] = data->learning->obs->time_s->month[i];
    data->learning->time_s->day[i] = data->learning->obs->time_s->day[i];
    data->learning->time_s->hour[i] = data->learning->obs->time_s->hour[i];
    data->learning->time_s->minutes[i] = data->learning->obs->time_s->minutes[i];
    data->learning->time_s->seconds[i] = data->learning->obs->time_s->seconds[i];
  }

  /* Read Singular Values */
  istat = read_netcdf_var_1d(&(data->learning->obs->sing), (info_field_struct *) NULL,
                             data->learning->obs->filename_eof, data->learning->obs->nomvar_sing,
                             data->conf->eofname, &neof, TRUE);
  if (istat != 0) {
    /* In case of failure */
    return istat;
  }

  /* Diagnostic status */
  return 0;
}
