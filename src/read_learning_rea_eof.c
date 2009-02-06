/* ***************************************************** */
/* read_learning_rea_eof Read reanalysis data EOFs for   */
/* learning period.                                      */
/* read_learning_rea_eof.c                               */
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
/*! \file read_learning_rea_eof.c
    \brief Read reanalysis data EOFs for learning period. Currently only NetCDF is implemented.
*/

#include <dsclim.h>

/** Read reanalysis data EOFs for learning period. Currently only NetCDF is implemented. */
int read_learning_rea_eof(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat; /* Diagnostic status */
  int neof; /* EOF dimension for main large-scale fields in input file */
  int ntime; /* Time dimension for main large-scale fields in input file */
  double *time;
  char *cal_type = NULL; /* Calendar type (udunits) */
  char *time_units = NULL; /* Time units (udunits) */
  double *buf = NULL;
  int t;
  int eof;

  /* Read EOF principal components */
  istat = read_netcdf_var_2d(&buf, (info_field_struct *) NULL, (proj_struct *) NULL,
                             data->learning->rea->filename_eof, data->learning->rea->nomvar_eof,
                             data->conf->eofname, data->learning->rea_timename, &neof, &ntime, TRUE);
  if (istat != 0) {
    /* In case of failure */
    return istat;
  }
  /* Re-order array with time as fastest varying dimension */
  data->learning->rea->eof = malloc(neof*ntime * sizeof(double));
  if (data->learning->rea->eof == NULL) alloc_error(__FILE__, __LINE__);
  for (eof=0; eof<neof; eof++)
    for (t=0; t<ntime; t++)
      data->learning->rea->eof[t+eof*ntime] = buf[eof+t*neof];
  (void) free(buf);

  /* Get time information */
  istat = get_time_info(data->learning->rea->time_s, &time, &time_units, &cal_type,
                        &ntime, data->learning->rea->filename_eof, data->learning->nomvar_time, TRUE);
  (void) free(cal_type);
  (void) free(time_units);
  (void) free(time);

  data->learning->rea->ntime = ntime;

  /* Read Singular Values */
  istat = read_netcdf_var_1d(&(data->learning->rea->sing), (info_field_struct *) NULL,
                             data->learning->rea->filename_eof, data->learning->rea->nomvar_sing,
                             data->conf->eofname, &neof, TRUE);
  if (istat != 0) {
    /* In case of failure */
    return istat;
  }

  /* Diagnostic status */
  return 0;
}
