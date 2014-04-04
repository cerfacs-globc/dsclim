/* ***************************************************** */
/* read_field_subdomain_period Read NetCDF field and     */
/* extract subdomain and subperiod.                      */
/* read_field_subdomain_period.c                         */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: jan 2009                            */
/* Last date of modification: jan 2009                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file read_field_subdomain_period.c
    \brief Read NetCDF field and extract subdomain and subperiod.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2014)

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

/** Read NetCDF field and extract subdomain and subperiod. */
int
read_field_subdomain_period(double **buffer, double **lon, double **lat, double *missing_value, char *varname,
                            int *year, int *month, int *day, double lonmin, double lonmax, double latmin, double latmax,
                            char *coords, char *gridname, char *lonname, char *latname, char *dimxname, char *dimyname,
                            char *timename, char *filename, int *nlon, int *nlat, int ntime) {
  /**
     @param[out]  buffer         Output field 3D array
     @param[out]  lon            Longitude 2D array
     @param[out]  lat            Latitude 2D array
     @param[out]  missing_value  Missing value
     @param[in]   varname        Variable name to read
     @param[in]   year           Year vector for subperiod
     @param[in]   month          Month vector for subperiod
     @param[in]   day            Day vector for subperiod
     @param[in]   lonmin         Minimum longitude for subdomain
     @param[in]   lonmax         Maximum longitude for subdomain
     @param[in]   latmin         Minimum latitude for subdomain
     @param[in]   latmax         Maximum latitude for subdomain
     @param[in]   coords         Coordinates dimensions (1D or 2D)
     @param[in]   gridname       Projection name
     @param[in]   lonname        Longitude field name
     @param[in]   latname        Latitude field name
     @param[in]   dimxname       X Dimension name
     @param[in]   dimyname       Y Dimension name
     @param[in]   timename       Time dimension name
     @param[in]   filename       Input filename
     @param[out]  nlon           Longitude dimension
     @param[out]  nlat           Latitude dimension
     @param[in]   ntime          Time dimension
     
     \return     Status.
  */

  int istat; /* Diagnostic status */
  info_struct *info;
  info_field_struct *info_field;
  double *buf_total = NULL;
  double *buf_sub = NULL;
  double *time_ls = NULL; /* Temporary time information buffer */
  time_vect_struct *time_s = NULL;
  char *cal_type = NULL; /* Calendar type (udunits) */
  char *time_units = NULL; /* Time units (udunits) */
  double *lon_total = NULL;
  double *lat_total = NULL;
  int ntime_file; /* Number of times dimension in input file */
  int nlon_file;
  int nlat_file;
  int ntime_sub;

  int nt;
  int tt;
  int i;
  int j;

  *lon = NULL;
  *lat = NULL;
  *buffer = NULL;

  *nlon = *nlat = -1;

  info = (info_struct *) malloc(sizeof(info_struct));
  if (info == NULL) alloc_error(__FILE__, __LINE__);

  info_field = (info_field_struct *) malloc(sizeof(info_field_struct));
  if (info_field == NULL) alloc_error(__FILE__, __LINE__);

  time_s = (time_vect_struct *) malloc(sizeof(time_vect_struct));
  if (time_s == NULL) alloc_error(__FILE__, __LINE__);

  /* To prevent fetching of not needed info attributes */
  info->title = strdup("none");
  /* Read dimensions */
  istat = read_netcdf_dims_3d(&lon_total, &lat_total, &time_ls, &cal_type, &time_units, &nlon_file, &nlat_file, &ntime_file,
                              info, coords, gridname, lonname, latname, dimxname, dimyname, timename, filename);
  (void) free(info->title);
  (void) free(info);

  /* Compute time information */
  istat = compute_time_info(time_s, time_ls, time_units, cal_type, ntime_file);

  /* Loop over time */
  ntime_sub = 0;
  for (nt=0; nt<ntime; nt++) {
    /* Search in all second time vector times for matching date */
    for (tt=0; tt<ntime_file; tt++) {
      if (year[nt]  == time_s->year[tt] &&
          month[nt] == time_s->month[tt] &&
          day[nt]   == time_s->day[tt]) {
        /* Found common date, process it. */
        istat = read_netcdf_var_3d_2d(&buf_total, info_field, (proj_struct *) NULL, filename, varname, dimxname, dimyname, timename,
                                      tt, nlon, nlat, &ntime_file, FALSE);
        /* Free non-needed variables */
        (void) free(info_field->coordinates);
        (void) free(info_field->grid_mapping);
        (void) free(info_field->units);
        (void) free(info_field->height);
        (void) free(info_field->long_name);
        if (istat != 0) {
          /* In case of failure */
          (void) free(buf_total);
          (void) free(lon_total);
          (void) free(lat_total);
          (void) free(time_ls);
          (void) free(time_units);
          (void) free(cal_type);
          (void) free(time_s->year);
          (void) free(time_s->month);
          (void) free(time_s->day);
          (void) free(time_s->hour);
          (void) free(time_s->minutes);
          (void) free(time_s->seconds);
          (void) free(time_s);
          (void) free(info_field);
          return istat;
        }
        *missing_value = info_field->fillvalue;
        
        /* Extract subdomain */
        if ((*lat) != NULL)
          (void) free(*lat);
        if ((*lon) != NULL)
          (void) free(*lon);
        (void) extract_subdomain(&buf_sub, lat, lon, nlon, nlat, buf_total, lon_total, lat_total,
                                 lonmin, lonmax, latmin, latmax, nlon_file, nlat_file, 1);
        (void) free(buf_total);

        /* Store into output field */
        (*buffer) = realloc((*buffer), (*nlon)*(*nlat)*(ntime_sub+1) * sizeof(double));
        if ((*buffer) == NULL) alloc_error(__FILE__, __LINE__);
        for (j=0; j<(*nlat); j++)
          for (i=0; i<(*nlon); i++)
            (*buffer)[i+j*(*nlon)+(ntime_sub)*(*nlon)*(*nlat)] = buf_sub[i+j*(*nlon)];

        (void) free(buf_sub);
        
        ntime_sub++;

        break;
      }
    }
  }

  if (*nlat == -1 || *nlon == -1) {
    /* In case of failure */
    (void) free(lon_total);
    (void) free(lat_total);
    (void) free(time_ls);
    (void) free(time_units);
    (void) free(cal_type);
    (void) free(time_s->year);
    (void) free(time_s->month);
    (void) free(time_s->day);
    (void) free(time_s->hour);
    (void) free(time_s->minutes);
    (void) free(time_s->seconds);
    (void) free(time_s);
    (void) free(info_field);
    
    (void) fprintf(stderr, "%s: Cannot find any date!! Dates we try to find:: At index 0: %d %d %d, at last index: %d %d %d. Dates we are searching in (in the file):: At index 0: %d %d %d, at last index: %d %d %d. \n", __FILE__, year[0], month[0], day[0], year[ntime-1], month[ntime-1], day[ntime-1], time_s->year[0], time_s->month[0], time_s->day[0], time_s->year[ntime_file-1], time_s->month[ntime_file-1], time_s->day[ntime_file-1]);

    return -1;
  }

  (void) free(lon_total);
  (void) free(lat_total);

  (void) free(time_s->year);
  (void) free(time_s->month);
  (void) free(time_s->day);
  (void) free(time_s->hour);
  (void) free(time_s->minutes);
  (void) free(time_s->seconds);
  (void) free(time_s);

  (void) free(time_ls);
  (void) free(time_units);
  (void) free(cal_type);

  (void) free(info_field);

  /* Diagnostic status */
  return 0;
}
