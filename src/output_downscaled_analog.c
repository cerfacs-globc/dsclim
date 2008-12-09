/* ***************************************************** */
/* Read analog day data and write it for downscaled      */
/* period.                                               */
/* output_downscaled_analog.c                            */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file output_downscaled_analog.c
    \brief Read analog day data and write it for downscaled period.
*/

#include <dsclim.h>

/** Read analog day data and write it for downscaled period. */
int output_downscaled_analog(analog_day_struct analog_days, data_struct *data, double *time_ls, int ntime) {
  /**
     @param[in]   analog_days           Analog days time indexes and dates with corresponding dates being downscaled.
     @param[in]   data                  MASTER data structure.
     @param[in]   time_ls               Time values
     @param[in]   ntime                 Number of times dimension
  */
  
  char *infile = NULL; /* Input filename */
  char *outfile = NULL; /* Output filename */
  char **outfiles = NULL; /* Output filelist */
  int year1 = 0; /* First year of data input file */
  int year2 = 0; /* End year of data input file */
  double *buf = NULL; /* Temporary data buffer */
  double *timeval = NULL; /* Temporary time information buffer */
  double *lat = NULL; /* Temporary latitude buffer */
  double *lon = NULL; /* Temporary longitude buffer */
  double *y = NULL; /* Temporary Y buffer */
  double *x = NULL; /* Temporary X buffer */
  char *cal_type = NULL; /* Calendar type (udunits) */
  char *time_units = NULL; /* Time units (udunits) */
  double ctimeval[1]; /* Dummy time info */
  int ntime_file; /* Number of times dimension */
  int ntime_obs; /* Number of times dimension in observation database */
  int nlon; /* Longitude dimension */
  int nlat; /* Latitude dimension */
  int noutf; /* Number of files in filelist */
  int found = FALSE; /* Used to tag if we found a specific date */
  int found_file = FALSE; /* Used to tag if we found a specific filename in the filelist */
  int output_month_end; /* Ending month for observation database */
  time_struct *time_s = NULL; /* Time structure for observation database */

  info_field_struct *info = NULL; /* Temporary field information structure */
  proj_struct *proj = NULL; /* Temporary field projection structure */

  int tmpi; /* Temporay integer value */
  char *format = NULL; /* Temporay format string */

  int t; /* Time loop counter */
  int tl; /* Time loop counter */
  int var; /* Variable counter */
  int istat; /* Diagnostic status */
  int f; /* Loop counter for files */

  int ncoutid;
  utUnit dataunit;

  int year;
  int month;
  int day;
  int hour;
  int minutes;
  float seconds;

  char *tmpstr = NULL;

  /*                                         J   F   M   A   M   J   J   A   S   O   N   D    */
  static int days_per_month_reg_year[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  infile = (char *) malloc(5000 * sizeof(char));
  if (infile == NULL) alloc_error(__FILE__, __LINE__);
  outfile = (char *) malloc(5000 * sizeof(char));
  if (outfile == NULL) alloc_error(__FILE__, __LINE__);
  format = (char *) malloc(5000 * sizeof(char));
  if (format == NULL) alloc_error(__FILE__, __LINE__);

  if (data->conf->output_month_begin == 1)
    output_month_end = 12;
  else
    output_month_end = data->conf->output_month_begin - 1;

  data->conf->obs_var->proj->name = (char *) NULL;
  
  /* Process each variable */
  for (var=0; var<data->conf->obs_var->nobs_var; var++) {
    
    /** Retrieve dimensions for this variable if needed, using the first timestep **/

    /* Create input filename for retrieving dimensions */
    (void) strcpy(format, "%s/%s/");
    (void) strcat(format, data->conf->obs_var->template);
    if (data->conf->obs_var->month_begin != 1) {
      /* Months in observation files *does not* begin in January: must have 2 years in filename */
      if (analog_days.month[0] < data->conf->obs_var->month_begin)
        year1 = analog_days.year[0] - 1;
      else
        year1 = analog_days.year[0];
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
      if (data->conf->obs_var->year_digits == 4)
        (void) sprintf(infile, format, data->conf->obs_var->path, data->conf->obs_var->frequency,
                       data->conf->obs_var->acronym[var], year1);
      else {
        tmpi = year1 / 100;
        year1 = year1 - (tmpi*100);
        (void) sprintf(infile, format, data->conf->obs_var->path, data->conf->obs_var->frequency,
                       data->conf->obs_var->acronym[var], year1);
      }
    }

    /* Process each downscaled day */
    noutf = 0;
    outfiles = NULL;
    for (t=0; t<ntime; t++) {
      /* Create output filename for writing data */
      if (analog_days.month_s[t] < data->conf->output_month_begin)
        year1 = analog_days.year_s[t] - 1;
      else
        year1 = analog_days.year_s[t];
      if (data->conf->output_month_begin != 1)
        year2 = year1 + 1;
      else
        year2 = year1;
      /* Example: evapn_1d_19790801_19800731.nc */
      (void) sprintf(outfile, "%s/%s_1d_%04d%02d%02d_%04d%02d%02d.nc", data->conf->output_path, data->conf->obs_var->netcdfname[var],
                     year1, data->conf->output_month_begin, 1,
                     year2, output_month_end, days_per_month_reg_year[output_month_end-1]);
      
      /* Check if output file has already been created */
      found_file = FALSE;
      f = 0;
      while (f < noutf && found_file == FALSE) {
        if ( !strcmp(outfiles[f], outfile) ) {
          found_file = TRUE;
          break;
        }
        f++;
      }
      if (found_file == FALSE) {
        /* File was not created already */
        outfiles = (char **) realloc(outfiles, (noutf+1) * sizeof(char *));
        if (outfiles == NULL) alloc_error(__FILE__, __LINE__);
        outfiles[noutf++] = strdup(outfile);

        /* Create output file */
        istat = create_netcdf(data->info->title, data->info->title_french, data->info->summary, data->info->summary_french,
                              data->info->keywords, "C language", data->info->description, data->info->institution,
                              data->info->creator_email, data->info->creator_url, data->info->creator_name,
                              data->info->version, data->info->scenario, data->info->scenario_co2, data->info->model,
                              data->info->institution_model, data->info->country, data->info->member,
                              data->info->downscaling_forcing, data->info->contact_email, data->info->contact_name,
                              data->info->other_contact_email, data->info->other_contact_name,
                              outfile, TRUE);
        if (istat != 0) {
          /* In case of failure */
          (void) free(infile);
          (void) free(outfile);
          for (f=0; f<noutf; f++)
            (void) free(outfiles[f]);
          if (noutf > 0)
            (void) free(outfiles);
          return istat;
        }

        /* Complete time metadata time_coverage_end for previous output file */
        if (noutf > 1 && t > 0) {

          istat = nc_open(outfiles[noutf-2], NC_WRITE, &ncoutid);  /* open NetCDF file */
          if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

          istat = nc_redef(ncoutid);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

          if (utIsInit() != TRUE)
            istat = utInit("");

          istat = utScan(data->conf->time_units,  &dataunit);

          istat = utCalendar(time_ls[t-1], &dataunit, &year, &month, &day, &hour, &minutes, &seconds);

          tmpstr = (char *) malloc(5000 * sizeof(char));
          if (tmpstr == NULL) alloc_error(__FILE__, __LINE__);

          (void) sprintf(tmpstr, "%04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minutes, (int) seconds);
          istat = nc_put_att_text(ncoutid, NC_GLOBAL, "time_coverage_end", strlen(tmpstr), tmpstr);

          (void) utTerm();

          (void) free(tmpstr);

          /* End definition mode */
          istat = nc_enddef(ncoutid);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
        }
      }

      /* Create input filename for reading data */
      (void) strcpy(format, "%s/%s/");
      (void) strcat(format, data->conf->obs_var->template);
      if (data->conf->obs_var->month_begin != 1) {
        /* Months in observation files *does not* begin in January: must have 2 years in filename */
        if (analog_days.month[t] < data->conf->obs_var->month_begin)
          year1 = analog_days.year[t] - 1;
        else
          year1 = analog_days.year[t];
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
        if (data->conf->obs_var->year_digits == 4)
          (void) sprintf(infile, format, data->conf->obs_var->path, data->conf->obs_var->frequency,
                         data->conf->obs_var->acronym[var], year1);
        else {
          tmpi = year1 / 100;
          year1 = year1 - (tmpi*100);
          (void) sprintf(infile, format, data->conf->obs_var->path, data->conf->obs_var->frequency,
                         data->conf->obs_var->acronym[var], year1);
        }
      }

      /* Get time information for this input file */
      time_s = (time_struct *) malloc(sizeof(time_struct));
      if (time_s == NULL) alloc_error(__FILE__, __LINE__);
      istat = get_time_info(time_s, &timeval, &time_units, &cal_type, &ntime_obs, infile, data->conf->timename, FALSE);

      /* Find date in observation database */
      found = FALSE;
      tl = 0;
#if DEBUG > 7
      (void) printf("Processing %d %d %d %d\n",t,analog_days.year_s[t],analog_days.month_s[t],analog_days.day_s[t]);
#endif
      while (tl<ntime_obs && found == FALSE) {
#if DEBUG > 7
        (void) printf("%d %d %d %d\n",tl,time_s->year[tl],time_s->month[tl],time_s->day[tl]);
#endif
        if (analog_days.year[t] == time_s->year[tl] && analog_days.month[t] == time_s->month[tl] &&
            analog_days.day[t] == time_s->day[tl]) {
          found = TRUE;
#if DEBUG > 7
          (void) printf("Found analog %d %d %d %d\n",tl,analog_days.year[t],analog_days.month[t],analog_days.day[t]);
#endif
        }
        tl++;
      }

      if (found == TRUE) {

        tl--;
        
        /* Read data */
        info = (info_field_struct *) malloc(sizeof(info_field_struct));
        if (info == NULL) alloc_error(__FILE__, __LINE__);
        proj = (proj_struct *) malloc(sizeof(proj_struct));
        if (proj == NULL) alloc_error(__FILE__, __LINE__);
        istat = read_netcdf_var_3d_2d(&buf, info, proj, infile, data->conf->obs_var->acronym[var],
                                      data->conf->obs_var->dimxname, data->conf->obs_var->dimyname, data->conf->obs_var->timename,
                                      tl, &nlon, &nlat, &ntime_file, FALSE);
        
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

          /* Get latitude and longitude coordinates information */
          istat = read_netcdf_latlon(&lon, &lat, &nlon, &nlat, data->conf->obs_var->dimcoords, data->conf->obs_var->proj->coords,
                                     data->conf->obs_var->proj->name, data->conf->obs_var->lonname,
                                     data->conf->obs_var->latname, data->conf->obs_var->dimxname,
                                     data->conf->obs_var->dimyname, infile);
          istat = read_netcdf_xy(&x, &y, &nlon, &nlat, data->conf->obs_var->dimxname, data->conf->obs_var->dimyname, 
                                 data->conf->obs_var->dimxname, data->conf->obs_var->dimyname, infile);
        }
        
        /* Apply modifications to data if needed */
        
        /* Write dimensions of field in newly-created NetCDF output file */
        if (found_file == FALSE) {
          /* We just created output file: we need to write dimensions */
          ctimeval[0] = time_ls[t];
          istat = write_netcdf_dims_3d(lon, lat, x, y, ctimeval, data->conf->cal_type,
                                       data->conf->time_units, nlon, nlat, 0,
                                       data->info->timestep, data->conf->obs_var->proj->name, data->conf->coords,
                                       data->conf->obs_var->proj->grid_mapping_name, data->conf->obs_var->proj->latin1,
                                       data->conf->obs_var->proj->latin2, data->conf->obs_var->proj->lonc, data->conf->obs_var->proj->lat0,
                                       data->conf->obs_var->proj->false_easting, data->conf->obs_var->proj->false_northing,
                                       data->conf->lonname, data->conf->latname, data->conf->timename,
                                       outfile, FALSE);
          if (istat != 0) {
            /* In case of failure */
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

            (void) free(infile);
            (void) free(outfile);
            (void) free(info->grid_mapping);
            (void) free(info->units);
            (void) free(info->height);
            (void) free(info->coordinates);
            (void) free(info->long_name);
            (void) free(info);
            (void) free(proj->name);
            (void) free(proj->grid_mapping_name);
            (void) free(proj);
            for (f=0; f<noutf; f++)
              (void) free(outfiles[f]);
            if (noutf > 0)
              (void) free(outfiles);
            return istat;
          }
        }
        
        /* Write data */
        istat = write_netcdf_var_3d_2d(buf, time_ls, info->fillvalue, outfile, data->conf->obs_var->netcdfname[var],
                                       info->long_name, info->units, info->height, proj->name, 
                                       data->conf->obs_var->dimxname, data->conf->obs_var->dimyname, data->conf->obs_var->timename,
                                       t, !found_file, nlon, nlat, ntime_file, FALSE);
        
        /* Free allocated memory */
        (void) free(buf);
        (void) free(info->grid_mapping);
        (void) free(info->units);
        (void) free(info->height);
        (void) free(info->coordinates);
        (void) free(info->long_name);
        (void) free(info);
        (void) free(proj->name);
        (void) free(proj->grid_mapping_name);
        (void) free(proj);

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
      else {
        (void) fprintf(stderr, "%s: Fatal error in algorithm: analog date %d %d %d %d not found in database!!\n", __FILE__, t,
                       analog_days.year[t],analog_days.month[t],analog_days.day[t]);
        /* Fatal error */
        for (f=0; f<noutf; f++)
          (void) free(outfiles[f]);
        if (noutf > 0)
          (void) free(outfiles);
        
        (void) free(lat);
        (void) free(lon);
                
        (void) free(x);
        (void) free(y);

        (void) free(infile);
        (void) free(outfile);
        (void) free(format);

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

        return -1;
      }
    }

    /* Free allocated memory */
    for (f=0; f<noutf; f++)
      (void) free(outfiles[f]);
    if (noutf > 0)
      (void) free(outfiles);

    (void) free(x);
    (void) free(y);

    (void) free(lat);
    (void) free(lon);
  }

  /* Free allocated memory */
  (void) free(infile);
  (void) free(outfile);
  (void) free(format);

  /* Success diagnostic */
  return 0;
}
