/* ***************************************************** */
/* read_large_scale_fields Read large scale fields.      */
/* read_large_scale_fields.c                             */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: sep 2008                            */
/* Last date of modification: sep 2008                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file read_large_scale_fields.c
    \brief Read Large-Scale fields from input files. Currently only NetCDF is implemented.
*/

#include <dsclim.h>

short int read_large_scale_fields(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  short int istat;
  int i;
  int t;
  int ntime;
  double *buf = NULL;
  double *time_ls = NULL;
  char *time_units = NULL;
  char *cal_type = NULL;

  time_units = (char *) malloc(5000 * sizeof(char));
  if (time_units == NULL) alloc_error(__FILE__, __LINE__);
  cal_type = (char *) malloc(5000 * sizeof(char));
  if (cal_type == NULL) alloc_error(__FILE__, __LINE__);

  if (data->field->time_ls != NULL) {
    (void) free(data->field->time_ls);
    data->field->time_ls = NULL;
  }
  if (data->field->lat_ls != NULL) {
    (void) free(data->field->lat_ls);
    data->field->lat_ls = NULL;
  }
  if (data->field->lon_ls != NULL)  {
    (void) free(data->field->lon_ls);
    data->field->lon_ls = NULL;
  }

  for (i=0; i<data->field->n_ls; i++) {

    if (data->field->time_ls == NULL)
      istat = read_netcdf_dims_3d(&(data->field->lon_ls), &(data->field->lat_ls), &time_ls,
                                  &cal_type, &time_units,
                                  &(data->field->nlon_ls), &(data->field->nlat_ls), &ntime,
                                  data->conf->lonname, data->conf->latname, data->conf->timename,
                                  data->field->filename_ls[i]);

    if ( !strcmp(cal_type, "gregorian") || !strcmp(cal_type, "standard") ) {

      /* Read data */
      istat = read_netcdf_var_3d(&(data->field->field_ls[i]), &(data->field->field_ls_fillvalue[i]), data->field->filename_ls[i],
                                 data->field->fname_ls[i], data->conf->lonname, data->conf->latname, data->conf->timename,
                                 data->field->nlon_ls, data->field->nlat_ls, ntime);

      data->field->ntime_ls = ntime;

      if (data->field->time_ls == NULL) {
        data->field->time_ls = (double *) malloc(data->field->ntime_ls * sizeof(double));
        if (data->field->time_ls == NULL) alloc_error(__FILE__, __LINE__);
        for (t=0; t<data->field->ntime_ls; t++)
          data->field->time_ls[t] = time_ls[t];
      }
    }
    else {
      double *dummy = NULL;

      /* Read data and fix calendar */
      istat = read_netcdf_var_3d(&buf, &(data->field->field_ls_fillvalue[i]), data->field->filename_ls[i],
                                 data->field->fname_ls[i], data->conf->lonname, data->conf->latname, data->conf->timename,
                                 data->field->nlon_ls, data->field->nlat_ls, ntime);

      (void) data_to_gregorian_cal_d(&(data->field->field_ls[i]), &dummy, &(data->field->ntime_ls),
                                     buf, time_ls, time_units, data->conf->time_units,
                                     cal_type, data->field->nlon_ls, data->field->nlat_ls, ntime);
      if (data->field->time_ls == NULL) {
        data->field->time_ls = (double *) malloc(data->field->ntime_ls * sizeof(double));
        if (data->field->time_ls == NULL) alloc_error(__FILE__, __LINE__);
        for (t=0; t<data->field->ntime_ls; t++)
          data->field->time_ls[t] = dummy[t];
      }
      (void) free(dummy);
      (void) free(buf);
    }
  }
  
  (void) free(time_ls);
  (void) free(time_units);
  (void) free(cal_type);

  return 0;
}
