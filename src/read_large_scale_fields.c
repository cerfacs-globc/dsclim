/* ***************************************************** */
/* read_learning_fields Read learning fields.            */
/* read_learning_fields.c                                */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: oct 2008                            */
/* Last date of modification: oct 2008                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file read_learning_fields.c
    \brief Read Learning data from input files. Currently only NetCDF is implemented.
*/

#include <dsclim.h>

int read_large_scale_fields(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat;
  int i;
  int t;
  int ntime;
  double *buf = NULL;
  double *time_ls = NULL;
  double *lat = NULL;
  double *lon = NULL;
  double *lat_sup_ls = NULL;
  double *lon_sup_ls = NULL;
  char *cal_type = NULL;
  char *time_units = NULL;
  int nlon;
  int nlat;
  int nlon_sup_ls;
  int nlat_sup_ls;
  int cat;

  time_units = (char *) malloc(5000 * sizeof(char));
  if (time_units == NULL) alloc_error(__FILE__, __LINE__);

  for (cat=0; cat<NCAT; cat++) {
    
    if (data->field[cat].time_ls != NULL) {
      (void) free(data->field[cat].time_ls);
      data->field[cat].time_ls = NULL;
    }
    if (data->field[cat].lat_ls != NULL) {
      (void) free(data->field[cat].lat_ls);
      data->field[cat].lat_ls = NULL;
    }
    if (data->field[cat].lon_ls != NULL)  {
      (void) free(data->field[cat].lon_ls);
      data->field[cat].lon_ls = NULL;
    }

    for (i=0; i<data->field[cat].n_ls; i++) {
      if (data->field[cat].time_ls == NULL) {
        istat = read_netcdf_dims_3d(&lon, &lat, &time_ls, &cal_type, &time_units, &nlon, &nlat, &ntime,
                                    data->info, data->field[cat].proj[i].coords, data->field[cat].proj[i].name,
                                    data->conf->lonname, data->conf->latname, data->conf->timename,
                                    data->field[cat].data[i].filename_ls);
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
        istat = read_netcdf_var_3d(&buf, data->field[cat].data[i].info, data->field[cat].proj,
                                   data->field[cat].data[i].filename_ls,
                                   data->field[cat].data[i].nomvar_ls, data->conf->lonname, data->conf->latname, data->conf->timename,
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
        data->field[cat].ntime_ls = ntime;
        
        /* Extract subdomain of spatial fields */
        if (data->field[cat].lon_ls != NULL) {
          (void) free(data->field[cat].lon_ls);
          data->field[cat].lon_ls = NULL;
        }
        if (data->field[cat].lat_ls != NULL) {
          (void) free(data->field[cat].lat_ls);
          data->field[cat].lat_ls = NULL;
        }
        if (data->field[cat].data[i].field_ls != NULL) {
          (void) free(data->field[cat].data[i].field_ls);
          data->field[cat].data[i].field_ls = NULL;
        }
        (void) extract_subdomain(&(data->field[cat].data[i].field_ls), &(data->field[cat].lat_ls), &(data->field[cat].lon_ls),
                                 &(data->field[cat].nlon_ls), &(data->field[cat].nlat_ls), buf, lon, lat,
                                 data->conf->longitude_min, data->conf->longitude_max, data->conf->latitude_min, data->conf->latitude_max,
                                 nlon, nlat, ntime);
        (void) free(buf);

        if (data->field[cat].time_ls == NULL) {
          data->field[cat].time_ls = (double *) malloc(data->field[cat].ntime_ls * sizeof(double));
          if (data->field[cat].time_ls == NULL) alloc_error(__FILE__, __LINE__);
          if ( strcmp(time_units, data->conf->time_units) )
            (void) change_date_origin(data->field[cat].time_ls, data->conf->time_units, time_ls, time_units, ntime);
          else
            for (t=0; t<data->field[cat].ntime_ls; t++)
              data->field[cat].time_ls[t] = time_ls[t];
          istat = compute_time_info(data->field[cat].time_s, data->field[cat].time_ls, data->conf->time_units, data->conf->cal_type,
                                    data->field[cat].ntime_ls);
        }
      }
      else {
        double *dummy = NULL;

        /* Read data and fix calendar */
        istat = read_netcdf_var_3d(&(data->field[cat].data[i].field_ls), data->field[cat].data[i].info,
                                   data->field[cat].proj, data->field[cat].data[i].filename_ls,
                                   data->field[cat].data[i].nomvar_ls, data->conf->lonname, data->conf->latname, data->conf->timename,
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
        if (data->field[cat].lon_ls != NULL) {
          (void) free(data->field[cat].lon_ls);
          data->field[cat].lon_ls = NULL;
        }
        if (data->field[cat].lat_ls != NULL) {
          (void) free(data->field[cat].lat_ls);
          data->field[cat].lat_ls = NULL;
        }
        if (data->field[cat].data[i].field_ls != NULL) {
          (void) free(data->field[cat].data[i].field_ls);
          data->field[cat].data[i].field_ls = NULL;
        }
        (void) extract_subdomain(&buf, &(data->field[cat].lon_ls), &(data->field[cat].lat_ls),
                                 &(data->field[cat].nlon_ls), &(data->field[cat].nlat_ls), data->field[cat].data[i].field_ls, lon, lat,
                                 data->conf->longitude_min, data->conf->longitude_max, data->conf->latitude_min, data->conf->latitude_max,
                                 nlon, nlat, ntime);
        (void) free(data->field[cat].data[i].field_ls);

        /* Adjust calendar */
        (void) data_to_gregorian_cal_d(&(data->field[cat].data[i].field_ls), &dummy, &(data->field[cat].ntime_ls),
                                       buf, time_ls, time_units, data->conf->time_units,
                                       cal_type, data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime);
        if (data->field[cat].time_ls == NULL) {
          data->field[cat].time_ls = (double *) malloc(data->field[cat].ntime_ls * sizeof(double));
          if (data->field[cat].time_ls == NULL) alloc_error(__FILE__, __LINE__);
          for (t=0; t<data->field[cat].ntime_ls; t++)
            data->field[cat].time_ls[t] = dummy[t];
          istat = compute_time_info(data->field[cat].time_s, data->field[cat].time_ls, data->conf->time_units, data->conf->cal_type,
                                    data->field[cat].ntime_ls);
        }
        (void) free(dummy);
        (void) free(buf);
      }
    }
  }
  (void) free(lat);
  (void) free(lon);
  
  (void) free(time_ls);
  (void) free(time_units);
  (void) free(cal_type);

  return 0;
}
