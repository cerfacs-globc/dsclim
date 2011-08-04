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

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2011)

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

/** Read analog day data and write it for downscaled period. */
int
output_downscaled_analog(analog_day_struct analog_days, double *delta, int output_month_begin, char *output_path,
                         char *config, char *time_units, char *cal_type,
                         double deltat, int file_format, int file_compression, int file_compression_level,
                         info_struct *info, var_struct *obs_var, period_struct *period,
                         double *time_ls, int ntime) {
  /**
     @param[in]   analog_days            Analog days time indexes and dates with corresponding dates being downscaled.
     @param[in]   delta                  Temperature difference to apply to analog day data
     @param[in]   output_month_begin     First month for yearly file output
     @param[in]   output_path            Output path directory
     @param[in]   config                 Whole configuration text
     @param[in]   time_units             Output base time units
     @param[in]   cal_type               Output calendar-type
     @param[in]   deltat                 Absolute difference of large-scale temperature threshold to apply as a correction
     @param[in]   file_format            File format version for NetCDF
     @param[in]   file_compression       Compression flag for NetCDF-4 file format
     @param[in]   file_compression_level Compression level for NetCDF-4 file format
     @param[in]   info                   General meta-data information structure for NetCDF output file
     @param[in]   obs_var                Input/output observation variables data structure
     @param[in]   period                 Period structure for downscaling output
     @param[in]   time_ls                Time values
     @param[in]   ntime                  Number of times dimension
  */
  
  char **infile = NULL; /* Input filename */
  char *infile_alt = NULL; /* Input filename for optional altitudes */
  char **outfile = NULL; /* Output filename */
  char ***outfiles = NULL; /* Output filelist */
  int year1 = 0; /* First year of data input file */
  int year2 = 0; /* End year of data input file */
  double **buf = NULL; /* Temporary data buffer */
  double **bufsave = NULL; /* Temporary data buffer for averaging hourly data */
  double *buftmp = NULL; /* Temporary buffer for mean temperature */
  double *alt = NULL; /* Altitudes of observation points (optional) */
  double *pmsl = NULL; /* Standard Pressure of observation points (optional) */
  double *timeval = NULL; /* Temporary time information buffer */
  double *lat = NULL; /* Temporary latitude buffer */
  double *lon = NULL; /* Temporary longitude buffer */
  double *y = NULL; /* Temporary Y buffer */
  double *x = NULL; /* Temporary X buffer */
  char *cal_type_tmp = NULL; /* Input observations calendar type (udunits) */
  char *time_units_tmp = NULL; /* Input observations time units (udunits) */
  double ctimeval[1]; /* Dummy time info */
  int ntime_file; /* Number of times dimension */
  int ntime_obs; /* Number of times dimension in observation database */
  int nlon; /* Longitude dimension */
  int nlat; /* Latitude dimension */
  int *noutf = NULL; /* Number of files in filelist */
  int found = FALSE; /* Used to tag if we found a specific date */
  int *found_file = NULL; /* Used to tag if we found a specific filename in the filelist */
  int output_month_end; /* Ending month for observation database */
  time_vect_struct *time_s = NULL; /* Time structure for observation database */

  info_field_struct **info_tmp = NULL; /* Temporary field information structure */
  proj_struct *proj_tmp = NULL; /* Temporary field projection structure */

  int tmpi; /* Temporay integer value */
  double curtas; /* Current temperature value */
  double newcurtas; /* New current temperature value */
  char *format = NULL; /* Temporay format string */

  int varid_tas; /* Variable index ID */
  int varid_tasmax; /* Variable index ID */
  int varid_tasmin; /* Variable index ID */
  int varid_prsn; /* Variable index ID */
  int varid_prr; /* Variable index ID */
  int varid_rlds; /* Variable index ID */
  int varid_rsds; /* Variable index ID */
  int varid_hur; /* Variable index ID */
  int varid_hus; /* Variable index ID */
  int varid_husmin; /* Variable index ID */
  int varid_husmax; /* Variable index ID */
  int varid_etp; /* Variable index ID */
  int varid_uvas; /* Variable index ID */
  int varid_prtot; /* Variable index ID */

  int configstrdimid; /* Variable dimension ID for configuration */
  int configstroutid; /* Variable ID for configuration */
  size_t start[1]; /* Start element when writing */
  size_t count[1]; /* Count of elements to write */

  int t; /* Time loop counter */
  int tl; /* Time loop counter */
  int var; /* Variable counter */
  int vare; /* Variable counter */
  int istat; /* Diagnostic status */
  int f; /* Loop counter for files */
  int i; /* Loop counter */
  int j; /* Loop counter */

  double curtime;

  int ncoutid;
  ut_system *unitSystem = NULL; /* Unit System (udunits) */
  ut_unit *dataunits = NULL; /* udunits variable */

  double period_begin;
  double period_end;

  int year;
  int month;
  int day;
  int hour;
  int minutes;
  double seconds;

  int yy;
  int mm;
  int dd;
  int hh;

  int minh;
  int maxh;

  char *tmpstr = NULL;
  short int tas_correction = TRUE;

  /*                                       J   F   M   A   M   J   J   A   S   O   N   D    */
  static int days_per_month_reg_year[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  infile = (char **) malloc(obs_var->nobs_var * sizeof(char *));
  if (infile == NULL) alloc_error(__FILE__, __LINE__);
  outfile = (char **) malloc(obs_var->nobs_var * sizeof(char *));
  if (outfile == NULL) alloc_error(__FILE__, __LINE__);
  outfiles = (char ***) malloc(obs_var->nobs_var * sizeof(char **));
  if (outfiles == NULL) alloc_error(__FILE__, __LINE__);
  buf = (double **) malloc(obs_var->nobs_var * sizeof(double *));
  if (buf == NULL) alloc_error(__FILE__, __LINE__);
  for (i=0; i<obs_var->nobs_var; i++)
    buf[i] = NULL;
  if ( !strcmp(info->timestep, "daily") && !strcmp(obs_var->frequency, "hourly") ) {
    bufsave = (double **) malloc(obs_var->nobs_var * sizeof(double *));
    if (bufsave == NULL) alloc_error(__FILE__, __LINE__);
  }
  found_file = (int *) malloc(obs_var->nobs_var * sizeof(int));
  if (found_file == NULL) alloc_error(__FILE__, __LINE__);
  noutf = (int *) malloc(obs_var->nobs_var * sizeof(int));
  if (noutf == NULL) alloc_error(__FILE__, __LINE__);
  info_tmp = (info_field_struct **) malloc(obs_var->nobs_var * sizeof(info_field_struct *));
  if (info_tmp == NULL) alloc_error(__FILE__, __LINE__);
  
  found = FALSE;
  for (var=0; var<obs_var->nobs_var; var++) {
    infile[var] = (char *) malloc(MAXPATH * sizeof(char));
    if (infile[var] == NULL) alloc_error(__FILE__, __LINE__);
    outfile[var] = (char *) malloc(MAXPATH * sizeof(char));
    if (outfile[var] == NULL) alloc_error(__FILE__, __LINE__);
    found_file[var] = FALSE;
  }
  format = (char *) malloc(MAXPATH * sizeof(char));
  if (format == NULL) alloc_error(__FILE__, __LINE__);

  if (output_month_begin == 1)
    output_month_end = 12;
  else
    output_month_end = output_month_begin - 1;

  if (obs_var->proj->name != NULL)
    (void) free(obs_var->proj->name);
  obs_var->proj->name = NULL;

  /* Initialize udunits */
  ut_set_error_message_handler(ut_ignore);
  unitSystem = ut_read_xml(NULL);
  ut_set_error_message_handler(ut_write_to_stderr);
  dataunits = ut_parse(unitSystem, time_units, UT_ASCII);

  /* Read altitudes if available, and compute pressure using standard atmosphere */
  if ( strcmp(obs_var->altitude, "") ) {
    infile_alt = (char *) malloc(MAXPATH * sizeof(char));
    if (infile_alt == NULL) alloc_error(__FILE__, __LINE__);
    (void) sprintf(infile_alt, "%s/%s", obs_var->path, obs_var->altitude);
    istat = read_netcdf_var_2d(&alt, (info_field_struct *) NULL, (proj_struct *) NULL, infile_alt, obs_var->altitudename,
                               obs_var->dimxname, obs_var->dimyname, &nlon, &nlat, FALSE);
    if (istat < 0)
      (void) fprintf(stderr, "%s: WARNING: Cannot read observation altitude field in file %s.\n", __FILE__, infile_alt);
    else {
      pmsl = (double *) malloc(nlon*nlat*sizeof(double));
      if (pmsl == NULL) alloc_error(__FILE__, __LINE__);
      (void) alt_to_press(pmsl, alt, nlon, nlat);
    }
    (void) free(infile_alt);
  }
  
  /* Compute time limits for writing */
  if (period->year_begin != -1) {
    (void) printf("%s: Downscaling output from %02d/%02d/%04d to %02d/%02d/%04d inclusively.\n", __FILE__,
                  period->month_begin, period->day_begin, period->year_begin,
                  period->month_end, period->day_end, period->year_end);
    istat = utInvCalendar2(period->year_begin, period->month_begin, period->day_begin, 0, 0, 0.0, dataunits, &period_begin);
    istat = utInvCalendar2(period->year_end, period->month_end, period->day_end, 23, 59, 0.0, dataunits, &period_end);
  }
  else {
    istat = utCalendar2(time_ls[0], dataunits, &year, &month, &day, &hour, &minutes, &seconds);
    (void) printf("%s: Downscaling whole period: %02d/%02d/%04d", __FILE__, month, day, year);
    istat = utCalendar2(time_ls[ntime-1], dataunits, &year, &month, &day, &hour, &minutes, &seconds);
    (void) printf(" to %02d/%02d/%04d inclusively.\n", month, day, year);
    period_begin = time_ls[0];
    period_end = time_ls[ntime-1];
  }

  /* Process each downscaled day */
  for (var=0; var<obs_var->nobs_var; var++) {
    noutf[var] = 0;
    outfiles[var] = NULL;
  }
  for (t=0; t<ntime; t++) {

    /* Check if we want to write data for this date */
    if (time_ls[t] >= period_begin && time_ls[t] <= period_end) {
      
      /* Create output filename for writing data */
      if (analog_days.month_s[t] < output_month_begin)
        year1 = analog_days.year_s[t] - 1;
      else
        year1 = analog_days.year_s[t];
      if (output_month_begin != 1)
        year2 = year1 + 1;
      else
        year2 = year1;
      /* Process each variable and create output filenames, and output files if necessary */
      for (var=0; var<obs_var->nobs_var; var++) {
        /* Example: evapn_1d_19790801_19800731.nc */
        (void) sprintf(outfile[var], "%s/%s_1d_%04d%02d%02d_%04d%02d%02d.nc", output_path, obs_var->netcdfname[var],
                       year1, output_month_begin, 1,
                       year2, output_month_end, days_per_month_reg_year[output_month_end-1]);
        /* Check if output file has already been created */
        found_file[var] = FALSE;
        f = 0;
        while (f < noutf[var] && found_file[var] == FALSE) {
          if ( !strcmp(outfiles[var][f], outfile[var]) ) {
            found_file[var] = TRUE;
            break;
          }
          f++;
        }
        if (found_file[var] == FALSE) {

          if ( !strcmp(obs_var->output[var], "yes") ) {

            /* File was not created already by this algorithm run */
            outfiles[var] = (char **) realloc(outfiles[var], (noutf[var]+1) * sizeof(char *));
            if (outfiles[var] == NULL) alloc_error(__FILE__, __LINE__);
            outfiles[var][noutf[var]++] = strdup(outfile[var]);
            
            /* Verify if file exists and if we can write into it */
            istat = nc_open(outfile[var], NC_WRITE, &ncoutid);
            
            if (istat != NC_NOERR) {
              /* File does not exists */
              
              /* Create output file */
              istat = create_netcdf(info->title, info->title_french, info->summary, info->summary_french,
                                    info->keywords, info->processor, info->software,
                                    info->description, info->institution,
                                    info->creator_email, info->creator_url, info->creator_name,
                                    info->version, info->scenario, info->scenario_co2, info->model,
                                    info->institution_model, info->country, info->member,
                                    info->downscaling_forcing, info->contact_email, info->contact_name,
                                    info->other_contact_email, info->other_contact_name,
                                    outfile[var], TRUE, file_format, file_compression);
              if (istat != 0) {
                /* In case of failure */
                (void) free(outfile[var]);
                for (f=0; f<noutf[var]; f++)
                  (void) free(outfiles[var][f]);
                if (noutf[var] > 0)
                  (void) free(outfiles[var]);
                if (pmsl != NULL) (void) free(pmsl);
                if (alt != NULL) (void) free(alt);
                (void) ut_free(dataunits);
                (void) ut_free_system(unitSystem);  
                return istat;
              }
            
              /** Add algorithm configuration **/

              istat = nc_open(outfile[var], NC_WRITE, &ncoutid);  /* open NetCDF file */
              if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
              
              /* Go into redefine mode */
              istat = nc_redef(ncoutid);
              if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
              
              /* Define configuration */
              istat = nc_def_dim(ncoutid, "configstr", strlen(config)+1, &configstrdimid);
              if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
              istat = nc_def_var(ncoutid, "dsclim_configuration", NC_CHAR, 1, &configstrdimid, &configstroutid);
              if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

              /* Update also time_coverage_end and time_coverage_start global attribute */
              tmpstr = (char *) malloc(MAXPATH * sizeof(char));
              if (tmpstr == NULL) alloc_error(__FILE__, __LINE__);
              hour=0;
              minutes=0;
              seconds=0;
              (void) sprintf(tmpstr, "%04d-%02d-%02dT%02d:%02d:%02dZ", year1, output_month_begin, 1, hour, minutes, (int) seconds);
              istat = nc_put_att_text(ncoutid, NC_GLOBAL, "time_coverage_start", strlen(tmpstr), tmpstr);
              hour=23;
              minutes=59;
              seconds=59;
              (void) sprintf(tmpstr, "%04d-%02d-%02dT%02d:%02d:%02dZ", year2, output_month_end, days_per_month_reg_year[output_month_end-1],
                             hour, minutes, (int) seconds);
              istat = nc_put_att_text(ncoutid, NC_GLOBAL, "time_coverage_end", strlen(tmpstr), tmpstr);
              (void) free(tmpstr);
        
              /* End definition mode */
              istat = nc_enddef(ncoutid);
              if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
              
              /* Write configuration */
              start[0] = 0;
              count[0] = strlen(config) + 1;
              istat = nc_put_vara_text(ncoutid, configstroutid, start, count, config);
              if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
            }
            else
              found_file[var] = TRUE;
            
            /* Close the output netCDF file. */
            istat = ncclose(ncoutid);
            if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
          }
        }
      }
          
      /* Create input filename for reading data */
      (void) strcpy(format, "%s/%s/");
      (void) strcat(format, obs_var->template);
      if (obs_var->month_begin != 1) {
        /* Months in observation files *does not* begin in January: must have 2 years in filename */
        if (analog_days.month[t] < obs_var->month_begin)
          year1 = analog_days.year[t] - 1;
        else
          year1 = analog_days.year[t];
        year2 = year1 + 1;
        if (obs_var->year_digits == 4)
          /* Process each variable and create input filenames */
          for (var=0; var<obs_var->nobs_var; var++)
            (void) sprintf(infile[var], format, obs_var->path, obs_var->frequency,
                           obs_var->acronym[var], year1, year2);
        else {
          tmpi = year1 / 100;
          year1 = year1 - (tmpi*100);
          tmpi = year2 / 100;
          year2 = year2 - (tmpi*100);
          /* Process each variable and create input filenames */
          for (var=0; var<obs_var->nobs_var; var++)
            (void) sprintf(infile[var], format, obs_var->path, obs_var->frequency,
                           obs_var->acronym[var], year1, year2);
        }
      }
      else {
        /* Months in observation files begins in January: must have 1 year in filename */
        if (obs_var->year_digits == 4)
          /* Process each variable and create input filenames */
          for (var=0; var<obs_var->nobs_var; var++)
            (void) sprintf(infile[var], format, obs_var->path, obs_var->frequency,
                           obs_var->acronym[var], year1);
        else {
          tmpi = year1 / 100;
          year1 = year1 - (tmpi*100);
          /* Process each variable and create input filenames */
          for (var=0; var<obs_var->nobs_var; var++) 
            (void) sprintf(infile[var], format, obs_var->path, obs_var->frequency,
                           obs_var->acronym[var], year1);
        }
      }
      
      /* Get time information for first input observation file and assume all files are alike */
      time_s = (time_vect_struct *) malloc(sizeof(time_vect_struct));
      if (time_s == NULL) alloc_error(__FILE__, __LINE__);
      istat = get_time_info(time_s, &timeval, &time_units_tmp, &cal_type_tmp, &ntime_obs, infile[0], obs_var->timename, FALSE);
      (void) free(cal_type_tmp);
      (void) free(time_units_tmp);
      (void) free(timeval);
      if (istat < 0) {
        for (var=0; var<obs_var->nobs_var; var++) {
          (void) free(outfile[var]);
          for (f=0; f<noutf[var]; f++) {
            (void) free(outfiles[var][f]);
          }
          if (noutf[var] > 0) {
            (void) free(outfiles[var]);
          }
        }
        (void) free(time_s);
        if (pmsl != NULL) (void) free(pmsl);
        if (alt != NULL) (void) free(alt);
        (void) ut_free(dataunits);
        (void) ut_free_system(unitSystem);  
        return istat;
      }

      /* Find date in observation database */
#if DEBUG > 7
      (void) printf("Processing %d %d %d %d\n",t,analog_days.year_s[t],analog_days.month_s[t],analog_days.day_s[t]);
#endif
      
      if ( !strcmp(obs_var->frequency, "hourly") ) {
        /* For hourly frequency data, find hours from 0 to 23 */
        minh = 0;
        maxh = 23;
      }
      else {
        /* For daily data, only read data for one day */
        minh = 0;
        maxh = 0;
      }

      /* Loop over hours if needed */
      for (hour=minh; hour<=maxh; hour++) {
        found = FALSE;
        tl = 0;
        
        if ( !strcmp(obs_var->frequency, "hourly") ) {
          while (tl<ntime_obs && found == FALSE) {
#if DEBUG > 7
            (void) printf("%d %d %d %d %d\n",tl,time_s->year[tl],time_s->month[tl],time_s->day[tl],time_s->hour[tl]);
#endif
            if (analog_days.year[t] == time_s->year[tl] && analog_days.month[t] == time_s->month[tl] &&
                analog_days.day[t] == time_s->day[tl] && hour == time_s->hour[tl]) {
              found = TRUE;
#if DEBUG > 7
              (void) printf("Found analog %d %d %d %d\n",tl,analog_days.year[t],analog_days.month[t],analog_days.day[t]);
#endif
            }
            tl++;
          }
        }
        else {
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
        }
        
        if (found == TRUE) {
          
          tl--;
          
          proj_tmp = (proj_struct *) malloc(sizeof(proj_struct));
          if (proj_tmp == NULL) alloc_error(__FILE__, __LINE__);
          proj_tmp->name = NULL;
          proj_tmp->grid_mapping_name = NULL;
          
          /* Process each variable and read data */
          for (var=0; var<obs_var->nobs_var; var++) {
            info_tmp[var] = (info_field_struct *) malloc(sizeof(info_field_struct));
            if (info_tmp[var] == NULL) alloc_error(__FILE__, __LINE__);
            /* Don't read variables which will be calculated : read only variables already available in datafiles */
            if ( !strcmp(obs_var->post[var], "no") ) {
              if (proj_tmp->name != NULL) {
                (void) free(proj_tmp->name);
                proj_tmp->name = NULL;
              }
              if (proj_tmp->grid_mapping_name != NULL) {
                (void) free(proj_tmp->grid_mapping_name);
                proj_tmp->grid_mapping_name = NULL;
              }
              istat = read_netcdf_var_3d_2d(&(buf[var]), info_tmp[var], proj_tmp, infile[var], obs_var->acronym[var],
                                            obs_var->dimxname, obs_var->dimyname, obs_var->timename,
                                            tl, &nlon, &nlat, &ntime_file, FALSE);
              /* Overwrite units and height if it was specified in configuration file. In that case, the value is not unknown. */
              if ( strcmp(obs_var->units[var], "unknown")) {
                (void) free(info_tmp[var]->units);
                info_tmp[var]->units = strdup(obs_var->units[var]);
              }
              if ( strcmp(obs_var->height[var], "unknown")) {
                (void) free(info_tmp[var]->height);
                info_tmp[var]->height = strdup(obs_var->height[var]);
              }
            }
            else {
              /* For post-processing variables, must fill in the info field structure info_tmp.
                 The projection structure proj_tmp used is the one of the previous variable,
                 because the first variable in the list is enforced to be a non post-processing variable
                 when loading the configuration file. */
              info_tmp[var]->fillvalue = info_tmp[0]->fillvalue;
              info_tmp[var]->coordinates = strdup(info_tmp[0]->coordinates);
              info_tmp[var]->grid_mapping = strdup(info_tmp[0]->grid_mapping);
              info_tmp[var]->units = strdup(obs_var->units[var]);
              info_tmp[var]->height = strdup(obs_var->height[var]);
              info_tmp[var]->long_name = strdup(obs_var->name[var]);
            }              
          }

          if (obs_var->proj->name == NULL) {
            /* Retrieve observation grid parameters if not done already */
            obs_var->proj->name = strdup(proj_tmp->name);
            obs_var->proj->grid_mapping_name = strdup(proj_tmp->grid_mapping_name);
            obs_var->proj->latin1 = proj_tmp->latin1;
            obs_var->proj->latin2 = proj_tmp->latin2;
            obs_var->proj->lonc = proj_tmp->lonc;
            obs_var->proj->lat0 = proj_tmp->lat0;
            obs_var->proj->false_easting = proj_tmp->false_easting;
            obs_var->proj->false_northing = proj_tmp->false_northing;
            
            /* Get latitude and longitude coordinates information from first file */
            istat = read_netcdf_latlon(&lon, &lat, &nlon, &nlat, obs_var->dimcoords, obs_var->proj->coords,
                                       obs_var->proj->name, obs_var->lonname,
                                       obs_var->latname, obs_var->dimxname,
                                       obs_var->dimyname, infile[0]);
            if ( !strcmp(obs_var->proj->name, "list") )
              /* List of lat + lon points only : keep only X dimension */
              nlat = 0;
            else
              /* Read coordinates information */
              istat = read_netcdf_xy(&x, &y, &nlon, &nlat, obs_var->dimxname, obs_var->dimyname, 
                                     obs_var->dimxname, obs_var->dimyname, infile[0]);
          }
          
          /*** Apply modifications to data ***/
          /** Retrieve temperature change and apply to analog day temperature and other variables **/
          
          /* Find known variable IDs for correction or calculation */
          varid_tas = find_str_value("tas", obs_var->netcdfname, obs_var->nobs_var);
          varid_tasmin = find_str_value("tasmin", obs_var->netcdfname, obs_var->nobs_var);
          varid_tasmax = find_str_value("tasmax", obs_var->netcdfname, obs_var->nobs_var);
          varid_prsn = find_str_value("prsn", obs_var->netcdfname, obs_var->nobs_var);
          varid_prr = find_str_value("prr", obs_var->netcdfname, obs_var->nobs_var);
          varid_rlds = find_str_value("rlds", obs_var->netcdfname, obs_var->nobs_var);
          varid_rsds = find_str_value("rsds", obs_var->netcdfname, obs_var->nobs_var);
          varid_hus = find_str_value("hus", obs_var->netcdfname, obs_var->nobs_var);
          varid_husmin = find_str_value("husmin", obs_var->netcdfname, obs_var->nobs_var);
          varid_husmax = find_str_value("husmax", obs_var->netcdfname, obs_var->nobs_var);
          varid_uvas = find_str_value("uvas", obs_var->netcdfname, obs_var->nobs_var);
          varid_hur = find_str_value("hur", obs_var->netcdfname, obs_var->nobs_var);
          varid_etp = find_str_value("evapn", obs_var->netcdfname, obs_var->nobs_var);
          varid_prtot = find_str_value("prtot", obs_var->netcdfname, obs_var->nobs_var);

          tas_correction = TRUE;

          if ( (varid_tasmax >= 0 || varid_tasmin >= 0 || varid_husmin >= 0 || varid_husmax >= 0) && !strcmp(obs_var->frequency, "hourly")) {
            (void) fprintf(stderr, "%s: WARNING: Cannot mix min and/or max observation variables with hourly data! Min and/or max variables will be ignored! \n", __FILE__);
            varid_tasmin = -1;
            varid_tasmax = -1;
            varid_husmin = -1;
            varid_husmax = -1;
          }

          if ( !strcmp(obs_var->frequency, "daily") ) {
            if (varid_tas < 0 && ( varid_tasmin < 0 || varid_tasmax < 0 ) )
              tas_correction = FALSE;
          }
          else {
            if (varid_tas < 0)
              tas_correction = FALSE;
          }

          if (tas_correction == FALSE)
            (void) fprintf(stderr, "%s: WARNING: No temperature correction can be done to precipitation partition or infra-red radiation required temperature variables are not available! It needs at least either average daily or hourly temperature, or, with daily data, min and max temperatures.\n", __FILE__);

          /* Correct average temperature and related variables (precipitation partition, infra-red radiation) */
          if (varid_tas >= 0 && tas_correction == TRUE) {
            if (fabs(delta[t]) >= deltat)
              for (j=0; j<nlat; j++)
                for (i=0; i<nlon; i++)
                  
                  if (buf[varid_tas][i+j*nlon] != info_tmp[varid_tas]->fillvalue) {
                    
                    /* Save non-corrected temperature */
                    curtas = buf[varid_tas][i+j*nlon];
                    /* Compute new temperature */                
                    buf[varid_tas][i+j*nlon] += delta[t];
                    
                    /* Compute new rain/snow partition, if needed */
                    if (varid_prsn != -1 && varid_prr != -1)
                      if (buf[varid_prsn][i+j*nlon] != info_tmp[varid_prsn]->fillvalue &&
                          buf[varid_prr][i+j*nlon] != info_tmp[varid_prr]->fillvalue)
                        if ( buf[varid_tas][i+j*nlon] >= (K_TKELVIN + 1.5) ) {
                          buf[varid_prr][i+j*nlon] += buf[varid_prsn][i+j*nlon];
                          buf[varid_prsn][i+j*nlon] = 0.0;
                        }
                    
                    /* Compute new infra-red radiation, if needed */
                    if (varid_rlds != -1)
                      if (buf[varid_rlds][i+j*nlon] != info_tmp[varid_rlds]->fillvalue)
                        buf[varid_rlds][i+j*nlon] += (4.0 * delta[t] / curtas ) * buf[varid_rlds][i+j*nlon];
                    
                  }
          }

          /* Correct min and max temperatures and related variables when having daily data */
          if (varid_tasmax >= 0 && varid_tasmin >= 0 && tas_correction == TRUE && !strcmp(obs_var->frequency, "daily")) {
            if (fabs(delta[t]) >= deltat)
              for (j=0; j<nlat; j++)
                for (i=0; i<nlon; i++)
                  
                  if (buf[varid_tasmax][i+j*nlon] != info_tmp[varid_tasmax]->fillvalue) {
                    
                    /* Save non-corrected mean temperature */
                    curtas = (buf[varid_tasmax][i+j*nlon] + buf[varid_tasmin][i+j*nlon]) / 2.0;
                    /* Compute new temperature */                
                    buf[varid_tasmax][i+j*nlon] += delta[t];
                    buf[varid_tasmin][i+j*nlon] += delta[t];
                    /* New averaged temperature */
                    newcurtas = (buf[varid_tasmax][i+j*nlon] + buf[varid_tasmin][i+j*nlon]) / 2.0;

                    /* Do not perform correction twice! */
                    if (varid_tas < 0) {
                      /* Compute new rain/snow partition, if needed */
                      if (varid_prsn != -1 && varid_prr != -1)
                        if (buf[varid_prsn][i+j*nlon] != info_tmp[varid_prsn]->fillvalue &&
                            buf[varid_prr][i+j*nlon] != info_tmp[varid_prr]->fillvalue)
                          if ( newcurtas >= (K_TKELVIN + 1.5) ) {
                            buf[varid_prr][i+j*nlon] += buf[varid_prsn][i+j*nlon];
                            buf[varid_prsn][i+j*nlon] = 0.0;
                          }
                  
                      /* Compute new infra-red radiation, if needed */
                      if (varid_rlds != -1)
                        if (buf[varid_rlds][i+j*nlon] != info_tmp[varid_rlds]->fillvalue)
                          buf[varid_rlds][i+j*nlon] += (4.0 * delta[t] / curtas ) * buf[varid_rlds][i+j*nlon];
                    }
                  }
          }

          /* Calculate only known post-processed variables */

          if (varid_hur >= 0) {
            /* Relative humidity */
            if ( !strcmp(obs_var->post[varid_hur], "yes") ) {
              if ( varid_hus >= 0 && (varid_tas >= 0 || (varid_tasmax >= 0 && varid_tasmin >= 0 )) && pmsl != NULL ) {
                /* Calculate relative humidity from temperature and specific humidity */
                buf[varid_hur] = (double *) malloc(nlat*nlon * sizeof(double));
                if (buf[varid_hur] == NULL) alloc_error(__FILE__, __LINE__);
                if (varid_tas >= 0)
                  info_tmp[varid_hur]->fillvalue = info_tmp[varid_tas]->fillvalue;
                else if (varid_tasmax >= 0)
                  info_tmp[varid_hur]->fillvalue = info_tmp[varid_tasmax]->fillvalue;
                else
                  info_tmp[varid_hur]->fillvalue = -9999.0;
                /* Create mean temperature temporary matrix when having only min and max temperature */
                if (varid_tas < 0) {
                  buftmp = (double *) malloc(nlat*nlon* sizeof(double));
                  if (buftmp == NULL) alloc_error(__FILE__, __LINE__);
                  info_tmp[varid_tas]->fillvalue = info_tmp[varid_tasmax]->fillvalue;
                  for (i=0; i<(nlon*nlat); i++)
                    if ((buf[varid_tasmax][i] != info_tmp[varid_tasmax]->fillvalue) &&
                        (buf[varid_tasmin][i] != info_tmp[varid_tasmin]->fillvalue))
                      buftmp[i] = (buf[varid_tasmax][i] + buf[varid_tasmin][i]) / 2.0;
                    else
                      buftmp[i] = info_tmp[varid_tas]->fillvalue;
                }
                else
                  buftmp = buf[varid_tas];
                (void) spechum_to_hr(buf[varid_hur], buftmp, buf[varid_hus], pmsl, info_tmp[varid_hur]->fillvalue, nlon, nlat);
                if (varid_tas < 0)
                  (void) free(buftmp);
              }
              else {
                (void) fprintf(stderr, "%s: WARNING: Cannot calculate Relative Humidity because needed variables are not available: Specific Humidity; Averaged temperature or Min/Max temperature, Standard Pressure from altitude.\n", __FILE__);                
                buf[varid_hur] = NULL;
              }
            }
          }

          if (varid_prsn >= 0 && varid_prr >= 0 && varid_prtot >= 0) {
            /* Total precipitation */
            if ( !strcmp(obs_var->post[varid_prtot], "yes") ) {
              /* Calculate total precipitation from liquid and solid precipitation */
                buf[varid_prtot] = (double *) malloc(nlat*nlon * sizeof(double));
                if (buf[varid_prtot] == NULL) alloc_error(__FILE__, __LINE__);
                info_tmp[varid_prtot]->fillvalue = info_tmp[varid_prr]->fillvalue;
                for (i=0; i<(nlon*nlat); i++) {
                  if ( (buf[varid_prr][i] != info_tmp[varid_prr]->fillvalue) && (buf[varid_prsn][i] != info_tmp[varid_prsn]->fillvalue))
                    buf[varid_prtot][i] = buf[varid_prr][i] + buf[varid_prsn][i];
                  else
                    buf[varid_prtot][i] = info_tmp[varid_prtot]->fillvalue;
                }
              }
            else {
                (void) fprintf(stderr, "%s: WARNING: Cannot calculate Total Precipitation because needed variables are not available: Liquid and Solid Precipitation.\n", __FILE__);                
                buf[varid_prtot] = NULL;
            }
          }

          if (varid_etp >= 0) {
            /* ETP */
            if ( !strcmp(obs_var->post[varid_etp], "yes") ) {
              if ( varid_hus >= 0 && (varid_tas >= 0 || (varid_tasmax >= 0 && varid_tasmin >= 0 )) && varid_rsds >= 0 && varid_rlds >= 0 &&
                   varid_uvas >= 0 && pmsl != NULL ) {
                /* Calculate ETP */
                buf[varid_etp] = (double *) malloc(nlat*nlon * sizeof(double));
                if (buf[varid_etp] == NULL) alloc_error(__FILE__, __LINE__);
                if (varid_tas >= 0)
                  info_tmp[varid_etp]->fillvalue = info_tmp[varid_tas]->fillvalue;
                else if (varid_tasmax >= 0)
                  info_tmp[varid_etp]->fillvalue = info_tmp[varid_tasmax]->fillvalue;
                else
                  info_tmp[varid_etp]->fillvalue = -9999.0;
                /* Create mean temperature temporary matrix when having only min and max temperature */
                if (varid_tas < 0) {
                  buftmp = (double *) malloc(nlat*nlon* sizeof(double));
                  if (buftmp == NULL) alloc_error(__FILE__, __LINE__);
                  for (i=0; i<(nlon*nlat); i++) {
                    if ((buf[varid_tasmax][i] != info_tmp[varid_tasmax]->fillvalue) &&
                        (buf[varid_tasmin][i] != info_tmp[varid_tasmin]->fillvalue))
                      buftmp[i] = (buf[varid_tasmax][i] + buf[varid_tasmin][i]) / 2.0;
                    else
                      buftmp[i] = info_tmp[varid_tasmax]->fillvalue;
                  }
                }
                else
                  buftmp = buf[varid_tas];
                (void) calc_etp_mf(buf[varid_etp], buftmp, buf[varid_hus], buf[varid_rsds], buf[varid_rlds], buf[varid_uvas],
                                   pmsl, info_tmp[varid_etp]->fillvalue, nlon, nlat);
                if (varid_tas < 0)
                  (void) free(buftmp);
              }
              else {
                (void) fprintf(stderr, "%s: WARNING: Cannot calculate ETP because needed variables are not available: Specific Humidity; Averaged Temperature or Min/Max Temperature; Short and Long Wave Radiation; Wind Module, Standard Pressure from altitude.\n", __FILE__);
                buf[varid_etp] = NULL;
              }
            }
          }

          /* Process each variable for writing */
          for (var=0; var<obs_var->nobs_var; var++) {
            if ( !strcmp(obs_var->output[var], "yes") ) {
              /* Write dimensions of field in newly-created NetCDF output file */
              if (found_file[var] == FALSE && hour == minh && buf[var] != NULL) {
                /* We just created output file: we need to write dimensions */
                ctimeval[0] = time_ls[t];
                istat = write_netcdf_dims_3d(lon, lat, x, y, alt, ctimeval, cal_type,
                                             time_units, nlon, nlat, 0,
                                             info->timestep, obs_var->proj->name, obs_var->proj->coords,
                                             obs_var->proj->grid_mapping_name, obs_var->proj->latin1,
                                             obs_var->proj->latin2, obs_var->proj->lonc, obs_var->proj->lat0,
                                             obs_var->proj->false_easting, obs_var->proj->false_northing,
                                             obs_var->lonname, obs_var->latname, obs_var->timename,
                                             outfile[var], FALSE);
                if (istat != 0) {
                  /* In case of failure */
                  (void) free(time_s->year);
                  (void) free(time_s->month);
                  (void) free(time_s->day);
                  (void) free(time_s->hour);
                  (void) free(time_s->minutes);
                  (void) free(time_s->seconds);
                  
                  (void) free(time_s);
            
                  (void) free(infile[var]);
                  (void) free(outfile[var]);
                  (void) free(info_tmp[var]->grid_mapping);
                  (void) free(info_tmp[var]->units);
                  (void) free(info_tmp[var]->height);
                  (void) free(info_tmp[var]->coordinates);
                  (void) free(info_tmp[var]->long_name);
                  (void) free(info_tmp[var]);
                  (void) free(proj_tmp->name);
                  (void) free(proj_tmp->grid_mapping_name);
                  (void) free(proj_tmp);
                  for (f=0; f<noutf[var]; f++)
                    (void) free(outfiles[var][f]);
                  if (noutf[var] > 0)
                    (void) free(outfiles[var]);
                  (void) free(outfiles);
                  if (pmsl != NULL) (void) free(pmsl);
                  if (alt != NULL) (void) free(alt);
                  (void) ut_free(dataunits);
                  (void) ut_free_system(unitSystem);  
                  return istat;
                }
              }
            }
          }

          /* Compute time if output timestep is hourly and not daily */
          if ( !strcmp(info->timestep, "hourly") ) {
            istat = utCalendar2(time_ls[t], dataunits, &yy, &mm, &dd, &hh, &minutes, &seconds);
            istat = utInvCalendar2(yy, mm, dd, hour, 0, 0.0, dataunits, &curtime);
            (void) ut_free(dataunits);
            (void) ut_free_system(unitSystem);  
          }
          else
            curtime = time_ls[t];

          /* Process each variable */
          for (var=0; var<obs_var->nobs_var; var++) {
            if (buf[var] != NULL && !strcmp(obs_var->output[var], "yes")) {
              if ( !strcmp(info->timestep, obs_var->frequency) ) {
                /* Output and input data are at same frequency */
                if (found_file[var] == FALSE && hour == minh)
                  (void) fprintf(stderr, "%s: Writing data to %s\n", __FILE__, outfile[var]);
                /* Write data */
                istat = write_netcdf_var_3d_2d(buf[var], &curtime, info_tmp[var]->fillvalue, outfile[var], obs_var->netcdfname[var],
                                               info_tmp[var]->long_name, info_tmp[var]->units, info_tmp[var]->height, proj_tmp->name, 
                                               obs_var->dimxname, obs_var->dimyname, obs_var->timename,
                                               0, !(found_file[var]), file_format, file_compression_level,
                                               nlon, nlat, ntime_file, FALSE);
                found_file[var] = TRUE;
              }
              else if ( !strcmp(info->timestep, "daily") && !strcmp(obs_var->frequency, "hourly") ) {
                if (hour == maxh) {
                  /* Last hour of day */
                  for (i=0; i<nlon*nlat; i++)
                    /* Average data */
                    buf[var][i] = (bufsave[var][i] + buf[var][i]) / 24.0;
                  /* Free memory */
                  (void) free(bufsave[var]);
                  bufsave[var] = NULL;
                  if (found_file[var] == FALSE && hour == minh)
                    (void) fprintf(stderr, "%s: Writing data to %s\n",__FILE__, outfile[var]);
                  /* Write data */
                  istat = write_netcdf_var_3d_2d(buf[var], &curtime, info_tmp[var]->fillvalue, outfile[var], obs_var->netcdfname[var],
                                                 info_tmp[var]->long_name, info_tmp[var]->units, info_tmp[var]->height, proj_tmp->name, 
                                                 obs_var->dimxname, obs_var->dimyname, obs_var->timename,
                                                 0, !(found_file[var]),  file_format, file_compression_level,
                                                 nlon, nlat, ntime_file, FALSE);
                  found_file[var] = TRUE;
                }
                else {
                  /* Allocate memory if first hour accumulating */
                  if (bufsave[var] == NULL) {
                    bufsave[var] = (double *) calloc(nlat*nlon, sizeof(double));
                    if (bufsave[var] == NULL) alloc_error(__FILE__, __LINE__);
                  }
                  /* Accumulate data to compute average when input data is hourly and output is daily */
                  for (i=0; i<nlon*nlat; i++)
                    bufsave[var][i] += buf[var][i];
                }
              }
              else {
                (void) fprintf(stderr, "%s: Fatal error in configuration of output timestep and observation variables frequency! Output timestep = %s    Observation variables frequency = %s\n", __FILE__, info->timestep, obs_var->frequency);
                
                /* Fatal error */
                if (buf[var] != NULL) (void) free(buf[var]);
                (void) free(info_tmp[var]->grid_mapping);
                (void) free(info_tmp[var]->units);
                (void) free(info_tmp[var]->height);
                (void) free(info_tmp[var]->coordinates);
                (void) free(info_tmp[var]->long_name);
                (void) free(info_tmp[var]);
                
                for (vare=0; vare<=var; vare++) {
                  for (f=0; f<noutf[vare]; f++)
                    (void) free(outfiles[vare][f]);
                  if (noutf[vare] > 0)
                    (void) free(outfiles[vare]);
                  (void) free(infile[vare]);
                  (void) free(outfile[vare]);
                }
                (void) free(outfiles);
                (void) free(noutf);
                (void) free(found_file);
                (void) free(info_tmp);
                (void) free(buf);
                
                (void) free(lat);
                (void) free(lon);
                
                (void) free(x);
                (void) free(y);
                
                (void) free(format);
                
                (void) free(proj_tmp->name);
                (void) free(proj_tmp->grid_mapping_name);
                (void) free(proj_tmp);
                
                (void) free(time_s->year);
                (void) free(time_s->month);
                (void) free(time_s->day);
                (void) free(time_s->hour);
                (void) free(time_s->minutes);
                (void) free(time_s->seconds);
                
                (void) free(time_s);

                if (alt != NULL) (void) free(alt);

                (void) ut_free(dataunits);
                (void) ut_free_system(unitSystem);  

                return -3;
              }
            }
            
            /* Free allocated memory */
            if (buf[var] != NULL) (void) free(buf[var]);
            (void) free(info_tmp[var]->grid_mapping);
            (void) free(info_tmp[var]->units);
            (void) free(info_tmp[var]->height);
            (void) free(info_tmp[var]->coordinates);
            (void) free(info_tmp[var]->long_name);
            (void) free(info_tmp[var]);
          }

          /* Free allocated memory */
          (void) free(proj_tmp->name);
          (void) free(proj_tmp->grid_mapping_name);
          (void) free(proj_tmp);        
        }
        else {
          (void) fprintf(stderr, "%s: Fatal error in algorithm: analog date %d %d %d %d not found in database!!\n", __FILE__, t,
                         analog_days.year[t],analog_days.month[t],analog_days.day[t]);
          /* Fatal error */
          for (var=0; var<obs_var->nobs_var; var++) {
            for (f=0; f<noutf[var]; f++)
              (void) free(outfiles[var][f]);
            if (noutf[var] > 0)
              (void) free(outfiles[var]);
            (void) free(infile[var]);
            (void) free(outfile[var]);
          }
          (void) free(outfiles);
          (void) free(noutf);
          (void) free(found_file);
          (void) free(info_tmp);
          (void) free(buf);
      
          (void) free(lat);
          (void) free(lon);
      
          (void) free(x);
          (void) free(y);
      
          (void) free(format);
      
          (void) free(proj_tmp->name);
          (void) free(proj_tmp->grid_mapping_name);
          (void) free(proj_tmp);
      
          (void) free(time_s->year);
          (void) free(time_s->month);
          (void) free(time_s->day);
          (void) free(time_s->hour);
          (void) free(time_s->minutes);
          (void) free(time_s->seconds);
      
          (void) free(time_s);
      
          if (alt != NULL) (void) free(alt);

          (void) ut_free(dataunits);
          (void) ut_free_system(unitSystem);  

          return -1;
        }
      }
      (void) free(time_s->year);
      (void) free(time_s->month);
      (void) free(time_s->day);
      (void) free(time_s->hour);
      (void) free(time_s->minutes);
      (void) free(time_s->seconds);
      
      (void) free(time_s);
    }
  }
  
  /* Free allocated memory */
  for (var=0; var<obs_var->nobs_var; var++) {
    for (f=0; f<noutf[var]; f++)
      (void) free(outfiles[var][f]);
    if (noutf[var] > 0)
      (void) free(outfiles[var]);
    (void) free(infile[var]);
    (void) free(outfile[var]);
  }
  (void) free(outfiles);
  (void) free(noutf);
  (void) free(found_file);
  (void) free(info_tmp);
  (void) free(buf);
  
  (void) free(x);
  (void) free(y);
  
  (void) free(lat);
  (void) free(lon);

  if (pmsl != NULL) (void) free(pmsl);
  if (alt != NULL) (void) free(alt);

  (void) free(infile);
  (void) free(outfile);
  (void) free(format);
  
  (void) ut_free(dataunits);
  (void) ut_free_system(unitSystem);  

  /* Success diagnostic */
  return 0;
}
