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

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2015)

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







#include <dsclim.h>

/** Read large-scale fields data from input files. Currently only NetCDF is implemented. */
int
read_large_scale_fields(data_struct *data) {
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
  char **cal_type; /* Calendar type (udunits) */
  char **time_units; /* Time units (udunits) */
  double longitude_min; /* Domain bounding box minimum longitude */
  double longitude_max; /* Domain bounding box maximum longitude */
  double latitude_min; /* Domain bounding box minimum latitude */
  double latitude_max; /* Domain bounding box maximum latitude */
  int ntime; /* Number of times dimension */
  int nlon; /* Longitude dimension for main large-scale fields */
  int nlat; /* Latitude dimension for main large-scale fields */
  int ntime_file; /* Number of times dimension in input file */
  int nlon_file; /* Longitude dimension for main large-scale fields in input file */
  int nlat_file; /* Latitude dimension for main large-scale fields in input file */
  
  int year_begin; /* When fixing time units, year to use as start date. */

  cal_type = (char **) malloc(NCAT * sizeof(char *));
  if (cal_type == NULL) alloc_error(__FILE__, __LINE__);
  time_units = (char **) malloc(NCAT * sizeof(char *));
  if (time_units == NULL) alloc_error(__FILE__, __LINE__);

  /* Loop over all large-scale field categories */
  for (cat=0; cat<NCAT; cat++) {

    cal_type[cat] = NULL;
    time_units[cat] = NULL;

    /* Select proper domain given large-scale field category */
    if (cat == 0 || cat == 1) {
      longitude_min = data->conf->longitude_min;
      longitude_max = data->conf->longitude_max;
      latitude_min = data->conf->latitude_min;
      latitude_max = data->conf->latitude_max;
    }
    else {
      longitude_min = data->conf->secondary_longitude_min;
      longitude_max = data->conf->secondary_longitude_max;
      latitude_min = data->conf->secondary_latitude_min;
      latitude_max = data->conf->secondary_latitude_max;
    }

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
      if (data->field[cat].time_ls == NULL) {
        istat = read_netcdf_dims_3d(&lon, &lat, &time_ls, &(cal_type[cat]), &(time_units[cat]), &nlon, &nlat, &ntime,
                                    data->info, data->field[cat].proj[i].coords, data->field[cat].proj[i].name,
                                    data->field[cat].data[i].lonname, data->field[cat].data[i].latname,
                                    data->field[cat].data[i].dimxname, data->field[cat].data[i].dimyname,
                                    data->field[cat].data[i].timename,
                                    data->field[cat].data[i].filename_ls);
        if (istat < 0) {
          /* In case of failure */
          (void) free(lon);
          (void) free(lat);
          (void) free(time_ls);
          (void) free(time_units[cat]);
          (void) free(cal_type[cat]);
          return istat;
        }
        /* Adjust time units if we want to fix time (set in the configuration file) */
        if (data->conf->fixtime == TRUE) {
          if (cat == FIELD_LS || cat == SEC_FIELD_LS)
            year_begin = data->conf->year_begin_other;
          else
            year_begin = data->conf->year_begin_ctrl;
          if (istat != 1) {
            (void) fprintf(stderr, "\n%s: IMPORTANT WARNING: Time variable values all zero!!! Fixing time variable to index value, STARTING at 0...\n\n", __FILE__);
            for (t=0; t<ntime; t++)
              time_ls[t] = (double) t;
          }
          (void) fprintf(stdout, "%s: Fixing time units using start date %d-01-01 12:00:00.\n", __FILE__, year_begin);
          time_units[cat] = realloc(time_units[cat], 500 * sizeof(char));
          if (time_units[cat] == NULL) alloc_error(__FILE__, __LINE__);
          /* days since 1950-01-01 12:00:00 */
          (void) sprintf(time_units[cat], "days since %d-01-01 12:00:00", year_begin);
        }
      }

      /* For standard calendar data */
      if ( !strcmp(cal_type[cat], "gregorian") || !strcmp(cal_type[cat], "standard") ) {
        
        /* Read data */
        istat = read_netcdf_var_3d(&buf, data->field[cat].data[i].info, &(data->field[cat].proj[i]),
                                   data->field[cat].data[i].filename_ls,
                                   data->field[cat].data[i].nomvar_ls,
                                   data->field[cat].data[i].dimxname, data->field[cat].data[i].dimyname, data->field[cat].data[i].timename,
                                   &nlon_file, &nlat_file, &ntime_file, TRUE);
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
          (void) free(time_units[cat]);
          (void) free(cal_type[cat]);
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

        /* Extraction of subdomain */
        (void) extract_subdomain(&(data->field[cat].data[i].field_ls), &(data->field[cat].lat_ls), &(data->field[cat].lon_ls),
                                 &(data->field[cat].nlon_ls), &(data->field[cat].nlat_ls), buf, lon, lat,
                                 longitude_min, longitude_max, latitude_min, latitude_max, nlon, nlat, ntime);
        (void) free(buf);

        /* Save number of times dimension */
        data->field[cat].ntime_ls = ntime;

        /* If time info not already retrieved for this category, get time information and generate time structure */
        if (data->field[cat].time_ls == NULL) {
          data->field[cat].time_ls = (double *) malloc(data->field[cat].ntime_ls * sizeof(double));
          if (data->field[cat].time_ls == NULL) alloc_error(__FILE__, __LINE__);
          if ( strcmp(time_units[cat], data->conf->time_units) )
            (void) change_date_origin(data->field[cat].time_ls, data->conf->time_units, time_ls, time_units[cat], ntime);
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

        /* Free memory if previously allocated */
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
        /* Read data and fix calendar */
        istat = read_netcdf_var_3d(&(data->field[cat].data[i].field_ls), data->field[cat].data[i].info,
                                   &(data->field[cat].proj[i]), data->field[cat].data[i].filename_ls,
                                   data->field[cat].data[i].nomvar_ls,
                                   data->field[cat].data[i].dimxname, data->field[cat].data[i].dimyname, data->field[cat].data[i].timename,
                                   &nlon_file, &nlat_file, &ntime_file, TRUE);
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
          (void) free(time_units[cat]);
          (void) free(cal_type[cat]);
          return istat;
        }

        /* Extract subdomain of spatial fields */
        (void) extract_subdomain(&buf, &(data->field[cat].lon_ls), &(data->field[cat].lat_ls),
                                 &(data->field[cat].nlon_ls), &(data->field[cat].nlat_ls), data->field[cat].data[i].field_ls, lon, lat,
                                 longitude_min, longitude_max, latitude_min, latitude_max, nlon, nlat, ntime);
        (void) free(data->field[cat].data[i].field_ls);

        /* Adjust calendar to standard calendar */
        istat = data_to_gregorian_cal_d(&(data->field[cat].data[i].field_ls), &dummy, &(data->field[cat].ntime_ls),
                                        buf, time_ls, time_units[cat], data->conf->time_units,
                                        cal_type[cat], data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime);
        if (istat < 0) {
          /* In case of failure */
          (void) free(lon);
          (void) free(lat);
          (void) free(time_ls);
          (void) free(time_units[cat]);
          (void) free(cal_type[cat]);
          (void) free(buf);
          (void) free(data->field[cat].lon_ls);
          (void) free(data->field[cat].lat_ls);
          (void) free(data->field[cat].data[i].field_ls);
          return istat;
        }
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
    if (time_units[cat] != NULL) {
      (void) free(time_units[cat]);
      time_units[cat] = NULL;
    }
    if (cal_type[cat] != NULL) {
      (void) free(cal_type[cat]);
      cal_type[cat] = NULL;
    }
  }

  (void) free(time_units);
  (void) free(cal_type);

  /* Diagnostic status */
  return 0;
}
