/* ***************************************************** */
/* read_large_scale_fields Read large-scale fields data  */
/* from input files.                                     */
/* read_large_scale_fields.c                             */
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
/*! \file read_large_scale_fields.c
    \brief Read large-scale fields data from input files. Currently only NetCDF is implemented.
*/

#include <dsclim.h>

/** Read large-scale fields data from input files. Currently only NetCDF is implemented. */
int read_large_scale_fields(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat; /* Diagnostic status */
  int i; /* Loop counter */
  int t; /* Time loop counter */
  int cat; /* Field category loop counter */
  double *buf = NULL; /* Temporary data buffer */
  double *time_ls = NULL; /* Temporary time information buffer */
  double *lat = NULL; /* Temporary latitude buffer for main large-scale fields */
  double *lon = NULL; /* Temporary longitude buffer for main large-scale fields */
  char *cal_type = NULL; /* Calendar type (udunits) */
  char *time_units = NULL; /* Time units (udunits) */
  int ntime; /* Number of times dimension */
  int nlon; /* Longitude dimension for main large-scale fields */
  int nlat; /* Latitude dimension for main large-scale fields */
  int ntime_file; /* Number of times dimension in input file */
  int nlon_file; /* Longitude dimension for main large-scale fields in input file */
  int nlat_file; /* Latitude dimension for main large-scale fields in input file */

  /* Loop over all large-scale field categories */
  for (cat=0; cat<NCAT; cat++) {

    /* Free memory for loop and set pointers to NULL for realloc */
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

    /* Loop over large-scale fields */
    for (i=0; i<data->field[cat].n_ls; i++) {
      /* Retrieve dimensions if time buffer is not already set for this field category */
      /* (assume same dimensions for all field in the same category) */
      if (data->field[cat].time_ls == NULL) {
        istat = read_netcdf_dims_3d(&lon, &lat, &time_ls, &cal_type, &time_units, &nlon, &nlat, &ntime,
                                    data->info, data->field[cat].proj[i].coords, data->field[cat].proj[i].name,
                                    data->conf->lonname, data->conf->latname, data->conf->timename,
                                    data->field[cat].data[i].filename_ls);
        if (istat != 0) {
          /* In case of failure */
          (void) free(lon);
          (void) free(lat);
          (void) free(time_ls);
          (void) free(time_units);
          (void) free(cal_type);
          return istat;
        }
      }

      /* For standard calendar data */
      if ( !strcmp(cal_type, "gregorian") || !strcmp(cal_type, "standard") ) {
        
        /* Read data */
        istat = read_netcdf_var_3d(&buf, data->field[cat].data[i].info, data->field[cat].proj,
                                   data->field[cat].data[i].filename_ls,
                                   data->field[cat].data[i].nomvar_ls, data->conf->lonname, data->conf->latname, data->conf->timename,
                                   &nlon_file, &nlat_file, &ntime_file);
        if (nlon != nlon_file || nlat != nlat_file || ntime != ntime_file) {
          (void) fprintf(stderr, "%s: Problems in dimensions! nlat=%d nlat_file=%d nlon=%d nlon_file=%d ntime=%d ntime_file=%d\n",
                         __FILE__, nlat, nlat_file, nlon, nlon_file, ntime, ntime_file);
          istat = -1;
        }
        if (istat != 0) {
          /* In case of failure */
          (void) free(buf);
          (void) free(lon);
          (void) free(lat);
          (void) free(time_ls);
          (void) free(time_units);
          (void) free(cal_type);
          return istat;
        }
        /* Save number of times dimension */
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
        /* Extraction */
        (void) extract_subdomain(&(data->field[cat].data[i].field_ls), &(data->field[cat].lat_ls), &(data->field[cat].lon_ls),
                                 &(data->field[cat].nlon_ls), &(data->field[cat].nlat_ls), buf, lon, lat,
                                 data->conf->longitude_min, data->conf->longitude_max, data->conf->latitude_min, data->conf->latitude_max,
                                 nlon, nlat, ntime);
        (void) free(buf);

        /* If time info not already retrieved for this category, get time information and generate time structure */
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
        /* Non-standard calendar type */

        double *dummy = NULL;

        /* Read data and fix calendar */
        istat = read_netcdf_var_3d(&(data->field[cat].data[i].field_ls), data->field[cat].data[i].info,
                                   data->field[cat].proj, data->field[cat].data[i].filename_ls,
                                   data->field[cat].data[i].nomvar_ls, data->conf->lonname, data->conf->latname, data->conf->timename,
                                   &nlon_file, &nlat_file, &ntime_file);
        if (nlon != nlon_file || nlat != nlat_file || ntime != ntime_file) {
          (void) fprintf(stderr, "%s: Problems in dimensions! nlat=%d nlat_file=%d nlon=%d nlon_file=%d ntime=%d ntime_file=%d\n",
                         __FILE__, nlat, nlat_file, nlon, nlon_file, ntime, ntime_file);
          istat = -1;
        }
        if (istat != 0) {
          /* In case of failure */
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
        /* Extraction */
        (void) extract_subdomain(&buf, &(data->field[cat].lon_ls), &(data->field[cat].lat_ls),
                                 &(data->field[cat].nlon_ls), &(data->field[cat].nlat_ls), data->field[cat].data[i].field_ls, lon, lat,
                                 data->conf->longitude_min, data->conf->longitude_max, data->conf->latitude_min, data->conf->latitude_max,
                                 nlon, nlat, ntime);
        (void) free(data->field[cat].data[i].field_ls);

        /* Adjust calendar to standard calendar */
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
    /* Free memory */
    if (lat != NULL) {
      (void) free(lat);
      lat = NULL;
    }
    if (lon != NULL) {
      (void) free(lon);
      lon = NULL;
    }
    if (time_ls != NULL) {
      (void) free(time_ls);
      time_ls = NULL;
    }
    if (time_units != NULL) {
      (void) free(time_units);
      time_units = NULL;
    }
    if (cal_type != NULL) {
      (void) free(cal_type);
      cal_type = NULL;
    }
  }

  /* Diagnostic status */
  return 0;
}
