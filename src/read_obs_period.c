/* ***************************************************** */
/* Read observation data for a given period              */
/* read_obs_period.c                                     */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file read_obs_period.c
    \brief Read observation data for a given period.
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

/** Read observation data for a given period. */
int
read_obs_period(double **buffer, double **lon, double **lat, double *missing_value, data_struct *data, char *varname,
                int *year, int *month, int *day, int *nlon, int *nlat, int ntime) {
  /**
     @param[out]   buffer        Output 2D array
     @param[out]   lon           Output 2D longitude
     @param[out]   lat           Output 2D latitude
     @param[out]   missing_value Missing value
     @param[in]    data          Main data structure
     @param[in]    varname       NetCDF variable name to read
     @param[in]    year          Year vector
     @param[in]    month         Month vector
     @param[in]    day           Day vector
     @param[out]   nlon          Longitude dimension
     @param[out]   nlat          Latitude dimension
     @param[in]    ntime         Time dimension

     \return Status
     
  */
  
  double *buf = NULL; /* Temporary buffer */
  char *infile = NULL; /* Input filename */
  int year1 = 0; /* First year of data input file */
  int year2 = 0; /* End year of data input file */
  double *timeval = NULL; /* Temporary time information buffer */
  char *cal_type = NULL; /* Calendar type (udunits) */
  char *time_units = NULL; /* Time units (udunits) */
  int ntime_obs; /* Number of times dimension in observation database */
  int found = FALSE; /* Used to tag if we found a specific date */
  time_vect_struct *time_s = NULL; /* Time structure for observation database */

  info_field_struct *info = NULL; /* Temporary field information structure */
  proj_struct *proj = NULL; /* Temporary field projection structure */

  int tmpi; /* Temporay integer value */
  char *format = NULL; /* Temporay format string */

  int t; /* Time loop counter */
  int tl; /* Time loop counter */
  int var; /* Variable ID */
  int istat; /* Diagnostic status */
  int i;
  int j;

  int ntime_file;

  char *prev_infile = NULL;

  /* Search variable */
  var = find_str_value(varname, data->conf->obs_var->netcdfname, data->conf->obs_var->nobs_var);
  if (var == -1) return -2;

  infile = (char *) malloc(MAXPATH * sizeof(char));
  if (infile == NULL) alloc_error(__FILE__, __LINE__);
  prev_infile = (char *) malloc(MAXPATH * sizeof(char));
  if (prev_infile == NULL) alloc_error(__FILE__, __LINE__);
  (void) strcpy(prev_infile, "");
  format = (char *) malloc(MAXPATH * sizeof(char));
  if (format == NULL) alloc_error(__FILE__, __LINE__);

  info = (info_field_struct *) malloc(sizeof(info_field_struct));
  if (info == NULL) alloc_error(__FILE__, __LINE__);
  proj = (proj_struct *) malloc(sizeof(proj_struct));
  if (proj == NULL) alloc_error(__FILE__, __LINE__);

  *lat = NULL;
  *lon = NULL;

  if (data->conf->obs_var->proj->name != NULL)
    (void) free(data->conf->obs_var->proj->name);
  data->conf->obs_var->proj->name = NULL;
  proj->name = NULL;
  if (data->conf->obs_var->proj->grid_mapping_name != NULL)
    (void) free(data->conf->obs_var->proj->grid_mapping_name);
  data->conf->obs_var->proj->grid_mapping_name = NULL;
  proj->grid_mapping_name = NULL;

  /* Loop over time */
  for (t=0; t<ntime; t++) {
    
    /* Create input filename for reading data */
    (void) strcpy(format, "%s/%s/");
    (void) strcat(format, data->conf->obs_var->template);
    if (data->conf->obs_var->month_begin != 1) {
      /* Months in observation files *does not* begin in January: must have 2 years in filename */
      if (month[t] < data->conf->obs_var->month_begin)
        year1 = year[t] - 1;
      else
        year1 = year[t];
      year2 = year1 + 1;
      if (data->conf->obs_var->year_digits == 4)
        (void) sprintf(infile, format, data->conf->obs_var->path, data->conf->obs_var->frequency,
                       data->conf->obs_var->acronym[var], year1, year2);
      else {
        tmpi = year1 / 100;
        year1 = year1 - (tmpi*100);
        tmpi = year2 / 100;
        year2 = year2 - (tmpi*100);
        (void) sprintf(infile, format, data->conf->obs_var->path, data->conf->obs_var->frequency,
                       data->conf->obs_var->acronym[var], year1, year2);
      }
    }
    else {
      /* Months in observation files begins in January: must have 1 year in filename */
      if (data->conf->obs_var->year_digits == 4) {
        year1 = year[t];
        (void) sprintf(infile, format, data->conf->obs_var->path, data->conf->obs_var->frequency,
                       data->conf->obs_var->acronym[var], year1);
      }
      else {
        tmpi = year1 / 100;
        year1 = year1 - (tmpi*100);
        (void) sprintf(infile, format, data->conf->obs_var->path, data->conf->obs_var->frequency,
                       data->conf->obs_var->acronym[var], year1);
      }
    }
    
    /* Get time information for this input file if needed */
    if ( strcmp(prev_infile, infile) ) {
      (void) printf("%s: Reading observation data %s from %s\n", __FILE__, varname, infile);
      if (time_s != NULL) {
        (void) free(time_s->year);
        (void) free(time_s->month);
        (void) free(time_s->day);
        (void) free(time_s->hour);
        (void) free(time_s->minutes);
        (void) free(time_s->seconds);
        
        (void) free(time_s);
        (void) free(cal_type);
        (void) free(time_units);
        (void) free(timeval);
      }
      
      time_s = (time_vect_struct *) malloc(sizeof(time_vect_struct));
      if (time_s == NULL) alloc_error(__FILE__, __LINE__);
      
      istat = get_time_info(time_s, &timeval, &time_units, &cal_type, &ntime_obs, infile, data->conf->obs_var->timename, FALSE);
      if (istat < 0) {
        (void) free(time_s);
        (void) free(infile);
        (void) free(prev_infile);
        (void) free(format);
        (void) free(info);
        (void) free(proj);
        return -1;
      }
    }
    
    /* Find date in observation database */
    found = FALSE;
    tl = 0;
    while (tl<ntime_obs && found == FALSE) {
      if (year[t] == time_s->year[tl] && month[t] == time_s->month[tl] && day[t] == time_s->day[tl])
        found = TRUE;
      tl++;
    }
    
    if (found == TRUE) {
      
      tl--;
          
      /* Read data */
      istat = read_netcdf_var_3d_2d(&buf, info, proj, infile, data->conf->obs_var->acronym[var],
                                    data->conf->obs_var->dimxname, data->conf->obs_var->dimyname, data->conf->obs_var->timename,
                                    tl, nlon, nlat, &ntime_file, FALSE);
      *missing_value = info->fillvalue;

      if (data->conf->obs_var->proj->name == NULL) {
        /* Retrieve observation grid parameters if not done already */
        data->conf->obs_var->proj->name = strdup(proj->name);
        data->conf->obs_var->proj->grid_mapping_name = strdup(proj->grid_mapping_name);
        data->conf->obs_var->proj->latin1 = proj->latin1;
        data->conf->obs_var->proj->latin2 = proj->latin2;
        data->conf->obs_var->proj->lonc = proj->lonc;
        data->conf->obs_var->proj->lat0 = proj->lat0;
        data->conf->obs_var->proj->false_easting = proj->false_easting;
        data->conf->obs_var->proj->false_northing = proj->false_northing;
      }
            
      if ( (*lat) == NULL && (*lon) == NULL ) {
        /* Get latitude and longitude coordinates information */
        istat = read_netcdf_latlon(lon, lat, nlon, nlat, data->conf->obs_var->dimcoords, data->conf->obs_var->proj->coords,
                                   data->conf->obs_var->proj->name, data->conf->obs_var->lonname,
                                   data->conf->obs_var->latname, data->conf->obs_var->dimxname,
                                   data->conf->obs_var->dimyname, infile);
            
        /* Allocate buffer memory given dimensions */
        *buffer = (double *) malloc((*nlon)*(*nlat)*ntime * sizeof(double));
        if ( (*buffer) == NULL) alloc_error(__FILE__, __LINE__);
      }

      /* Transfer data */
      for (j=0; j<(*nlat); j++)
        for (i=0; i<(*nlon); i++)
          if (buf[i+j*(*nlon)] != (*missing_value))
            (*buffer)[i+j*(*nlon)+t*(*nlon)*(*nlat)] = (buf[i+j*(*nlon)] * data->conf->obs_var->factor[var]) +
              data->conf->obs_var->delta[var];
          else
            (*buffer)[i+j*(*nlon)+t*(*nlon)*(*nlat)] = (*missing_value);
                    
      /* Free allocated memory */
      (void) free(proj->name);
      (void) free(proj->grid_mapping_name);
          
      (void) free(info->grid_mapping);
      (void) free(info->units);
      (void) free(info->height);
      (void) free(info->coordinates);
      (void) free(info->long_name);

      (void) free(buf);
    }
    else {
      (void) fprintf(stderr, "%s: Fatal error in algorithm: date not found: %d %d %d %d!!\n", __FILE__, t, year[t],month[t],day[t]);
          
      /* Fatal error */
      (void) free(infile);
      (void) free(format);
          
      (void) free(info);
      (void) free(proj);
          
      if (time_s != NULL) {
        (void) free(time_s->year);
        (void) free(time_s->month);
        (void) free(time_s->day);
        (void) free(time_s->hour);
        (void) free(time_s->minutes);
        (void) free(time_s->seconds);
          
        (void) free(time_s);
        (void) free(cal_type);
        (void) free(time_units);
        (void) free(timeval);
      }
          
      return -1;
    }
    (void) strcpy(prev_infile, infile);
  }

  /* Free allocated memory */
  if (time_s != NULL) {
    (void) free(time_s->year);
    (void) free(time_s->month);
    (void) free(time_s->day);
    (void) free(time_s->hour);
    (void) free(time_s->minutes);
    (void) free(time_s->seconds);

    (void) free(time_s);
    (void) free(cal_type);
    (void) free(time_units);
    (void) free(timeval);
  }

  (void) free(info);
  (void) free(proj);
          
  (void) free(infile);
  (void) free(prev_infile);
  (void) free(format);
  
  /* Success diagnostic */
  return 0;
}
