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
  double *lat = NULL;
  double *lon = NULL;
  char *time_units = NULL;
  char *cal_type = NULL;
  int nlon;
  int nlat;

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

    if (data->field->time_ls == NULL) {
      istat = read_netcdf_dims_3d(&lon, &lat, &time_ls, &cal_type, &time_units, &nlon, &nlat, &ntime,
                                  data->info, data->field->proj[i].coords, data->field->proj[i].name,
                                  data->conf->lonname, data->conf->latname, data->conf->timename,
                                  data->field->filename_ls[i]);
      if (istat != 0) {
        (void) free(lon);
        (void) free(lat);
        (void) free(time_ls);
        (void) free(time_units);
        (void) free(cal_type);
        return istat;
      }
    }

    if ( !strcmp(cal_type, "gregorian") || !strcmp(cal_type, "standard") ) {

      /* Read data */
      istat = read_netcdf_var_3d(&buf, &(data->field->info_field[i]), &(data->field->proj[i]),
                                 data->field->filename_ls[i],
                                 data->field->nomvar_ls[i], data->conf->lonname, data->conf->latname, data->conf->timename,
                                 nlon, nlat, ntime);
      if (istat != 0) {
        (void) free(buf);
        (void) free(lon);
        (void) free(lat);
        (void) free(time_ls);
        (void) free(time_units);
        (void) free(cal_type);
        return istat;
      }
      data->field->ntime_ls = ntime;

      /* Extract subdomain of spatial fields */
      if (data->field->lon_ls != NULL) {
        (void) free(data->field->lon_ls);
        data->field->lon_ls = NULL;
      }
      if (data->field->lat_ls != NULL) {
        (void) free(data->field->lat_ls);
        data->field->lat_ls = NULL;
      }
      if (data->field->field_ls[i] != NULL) {
        (void) free(data->field->field_ls[i]);
        data->field->field_ls[i] = NULL;
      }
      (void) extract_subdomain(&(data->field->field_ls[i]), &(data->field->lon_ls), &(data->field->lat_ls),
                               &(data->field->nlon_ls), &(data->field->nlat_ls), buf, lon, lat,
                               data->conf->longitude_min, data->conf->longitude_max, data->conf->latitude_min, data->conf->latitude_max,
                               nlon, nlat, ntime);
      (void) free(buf);

      if (data->field->time_ls == NULL) {
        data->field->time_ls = (double *) malloc(data->field->ntime_ls * sizeof(double));
        if (data->field->time_ls == NULL) alloc_error(__FILE__, __LINE__);
        if ( strcmp(time_units, data->conf->time_units) )
          (void) change_date_origin(data->field->time_ls, data->conf->time_units, time_ls, time_units, ntime);
        else
          for (t=0; t<data->field->ntime_ls; t++)
            data->field->time_ls[t] = time_ls[t];
      }
    }
    else {
      double *dummy = NULL;

      /* Read data and fix calendar */
      istat = read_netcdf_var_3d(&(data->field->field_ls[i]), &(data->field->info_field[i]),
                                 &(data->field->proj[i]), data->field->filename_ls[i],
                                 data->field->nomvar_ls[i], data->conf->lonname, data->conf->latname, data->conf->timename,
                                 nlon, nlat, ntime);
      if (istat != 0) {
        (void) free(lon);
        (void) free(lat);
        (void) free(time_ls);
        (void) free(time_units);
        (void) free(cal_type);
        return istat;
      }

      /* Extract subdomain of spatial fields */
      if (data->field->lon_ls != NULL) {
        (void) free(data->field->lon_ls);
        data->field->lon_ls = NULL;
      }
      if (data->field->lat_ls != NULL) {
        (void) free(data->field->lat_ls);
        data->field->lat_ls = NULL;
      }
      if (data->field->field_ls[i] != NULL) {
        (void) free(data->field->field_ls[i]);
        data->field->field_ls[i] = NULL;
      }
      (void) extract_subdomain(&buf, &(data->field->lon_ls), &(data->field->lat_ls),
                               &(data->field->nlon_ls), &(data->field->nlat_ls), data->field->field_ls[i], lon, lat,
                               data->conf->longitude_min, data->conf->longitude_max, data->conf->latitude_min, data->conf->latitude_max,
                               nlon, nlat, ntime);
      (void) free(data->field->field_ls[i]);

      /* Adjust calendar */
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

  (void) free(lat);
  (void) free(lon);
  
  (void) free(time_ls);
  (void) free(time_units);
  (void) free(cal_type);

  return 0;
}
