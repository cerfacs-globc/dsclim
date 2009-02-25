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

Copyright Cerfacs (Christian Page) (2009)

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
                         char *config, char *time_units, char *cal_type, int file_format, int file_compression,
                         info_struct *info, var_struct *obs_var, period_struct *period,
                         double *time_ls, int ntime) {
  /**
     @param[in]   analog_days           Analog days time indexes and dates with corresponding dates being downscaled.
     @param[in]   delta                 Temperature difference to apply to analog day data
     @param[in]   output_month_begin    First month for yearly file output
     @param[in]   output_path           Output path directory
     @param[in]   config                Whole configuration text
     @param[in]   time_units            Output base time units
     @param[in]   cal_type              Output calendar-type
     @param[in]   file_format           File format version for NetCDF
     @param[in]   file_compression      Compression flag for NetCDF-4 file format
     @param[in]   info                  General meta-data information structure for NetCDF output file
     @param[in]   obs_var               Input/output observation variables data structure
     @param[in]   period                Period structure for downscaling output
     @param[in]   time_ls               Time values
     @param[in]   ntime                 Number of times dimension
  */
  
  char **infile = NULL; /* Input filename */
  char **outfile = NULL; /* Output filename */
  char ***outfiles = NULL; /* Output filelist */
  int year1 = 0; /* First year of data input file */
  int year2 = 0; /* End year of data input file */
  double **buf = NULL; /* Temporary data buffer */
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

  int configstrdimid; /* Variable dimension ID for configuration */
  int configstroutid; /* Variable ID for configuration */
  size_t start[1]; /* Start element when writing */
  size_t count[1]; /* Count of elements to write */

  int t; /* Time loop counter */
  int tl; /* Time loop counter */
  int var; /* Variable counter */
  int istat; /* Diagnostic status */
  int f; /* Loop counter for files */
  int i; /* Loop counter */
  int j; /* Loop counter */

  int ncoutid;
  utUnit dataunit;

  double period_begin;
  double period_end;

  int year;
  int month;
  int day;
  int hour;
  int minutes;
  float seconds;

  char *tmpstr = NULL;

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
  found_file = (int *) malloc(obs_var->nobs_var * sizeof(int));
  if (found_file == NULL) alloc_error(__FILE__, __LINE__);
  noutf = (int *) malloc(obs_var->nobs_var * sizeof(int));
  if (noutf == NULL) alloc_error(__FILE__, __LINE__);
  info_tmp = (info_field_struct **) malloc(obs_var->nobs_var * sizeof(info_field_struct *));
  if (info_tmp == NULL) alloc_error(__FILE__, __LINE__);
  
  found = FALSE;
  for (var=0; var<obs_var->nobs_var; var++) {
    infile[var] = (char *) malloc(5000 * sizeof(char));
    if (infile[var] == NULL) alloc_error(__FILE__, __LINE__);
    outfile[var] = (char *) malloc(5000 * sizeof(char));
    if (outfile[var] == NULL) alloc_error(__FILE__, __LINE__);
    found_file[var] = FALSE;
  }
  format = (char *) malloc(5000 * sizeof(char));
  if (format == NULL) alloc_error(__FILE__, __LINE__);

  if (output_month_begin == 1)
    output_month_end = 12;
  else
    output_month_end = output_month_begin - 1;

  if (obs_var->proj->name != NULL)
    (void) free(obs_var->proj->name);
  obs_var->proj->name = NULL;
  
  /* Compute time limits for writing */
  if (period->year_begin != -1) {
    (void) printf("%s: Downscaling output from %02d/%02d/%04d to %02d/%02d/%04d inclusively.\n", __FILE__,
                  period->month_begin, period->day_begin, period->year_begin,
                  period->month_end, period->day_end, period->year_end);
    if (utIsInit() != TRUE)
      istat = utInit("");
    istat = utScan(time_units, &dataunit);
    istat = utInvCalendar(period->year_begin, period->month_begin, period->day_begin, 0, 0, 0.0, &dataunit, &period_begin);
    istat = utInvCalendar(period->year_end, period->month_end, period->day_end, 0, 0, 0.0, &dataunit, &period_end);
    (void) utTerm();
  }
  else {
    if (utIsInit() != TRUE)
      istat = utInit("");
    istat = utScan(time_units,  &dataunit);
    istat = utCalendar(time_ls[0], &dataunit, &year, &month, &day, &hour, &minutes, &seconds);
    (void) printf("%s: Downscaling whole period: %02d/%02d/%04d", __FILE__, month, day, year);
    istat = utCalendar(time_ls[ntime-1], &dataunit, &year, &month, &day, &hour, &minutes, &seconds);
    (void) utTerm();
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
          /* File was not created already */
          outfiles[var] = (char **) realloc(outfiles[var], (noutf[var]+1) * sizeof(char *));
          if (outfiles[var] == NULL) alloc_error(__FILE__, __LINE__);
          outfiles[var][noutf[var]++] = strdup(outfile[var]);
        
          /* Create output file */
          istat = create_netcdf(info->title, info->title_french, info->summary, info->summary_french,
                                info->keywords, info->processor, info->description, info->institution,
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
            return istat;
          }

          /* Add algorithm configuration */
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

          /* End definition mode */
          istat = nc_enddef(ncoutid);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

          /* Write configuration */
          start[0] = 0;
          count[0] = strlen(config) + 1;
          istat = nc_put_vara_text(ncoutid, configstroutid, start, count, config);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

          /* Close the output netCDF file. */
          istat = ncclose(ncoutid);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
        }
      
        /* Complete time metadata time_coverage_end for previous output file and output whole config */
        if (noutf[var] > 1 && t > 0) {
        
          istat = nc_open(outfiles[var][noutf[var]-1], NC_WRITE, &ncoutid);  /* open NetCDF file */
          if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
        
          istat = nc_redef(ncoutid);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
        
          if (utIsInit() != TRUE)
            istat = utInit("");
          istat = utScan(time_units,  &dataunit);
          istat = utCalendar(time_ls[t-1], &dataunit, &year, &month, &day, &hour, &minutes, &seconds);
          (void) utTerm();
        
          tmpstr = (char *) malloc(5000 * sizeof(char));
          if (tmpstr == NULL) alloc_error(__FILE__, __LINE__);
          (void) sprintf(tmpstr, "%04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minutes, (int) seconds);
          istat = nc_put_att_text(ncoutid, NC_GLOBAL, "time_coverage_end", strlen(tmpstr), tmpstr);
          (void) free(tmpstr);

          /* End definition mode */
          istat = nc_enddef(ncoutid);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

          /* Close the output netCDF file. */
          istat = ncclose(ncoutid);
          if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
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
      if (istat < 0) {
        (void) free(outfile[var]);
        for (f=0; f<noutf[var]; f++)
          (void) free(outfiles[var][f]);
        if (noutf[var] > 0)
          (void) free(outfiles[var]);
        (void) free(time_s);
        return -1;
      }
    
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
            
        proj_tmp = (proj_struct *) malloc(sizeof(proj_struct));
        if (proj_tmp == NULL) alloc_error(__FILE__, __LINE__);
        proj_tmp->name = NULL;
        proj_tmp->grid_mapping_name = NULL;
      
        /* Process each variable and read data */
        for (var=0; var<obs_var->nobs_var; var++) {
          info_tmp[var] = (info_field_struct *) malloc(sizeof(info_field_struct));
          if (info_tmp[var] == NULL) alloc_error(__FILE__, __LINE__);
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

        varid_tas = find_str_value("tas", obs_var->netcdfname, obs_var->nobs_var);
        varid_tasmin = find_str_value("tasmin", obs_var->netcdfname, obs_var->nobs_var);
        varid_tasmax = find_str_value("tasmax", obs_var->netcdfname, obs_var->nobs_var);
        varid_prsn = find_str_value("prsn", obs_var->netcdfname, obs_var->nobs_var);
        varid_prr = find_str_value("prr", obs_var->netcdfname, obs_var->nobs_var);
        varid_rlds = find_str_value("rlds", obs_var->netcdfname, obs_var->nobs_var);
        if (varid_tas >= 0) {
          if (fabs(delta[t]) >= 2.0)
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
        if (varid_tasmax >= 0 && varid_tasmin >= 0) {
          if (fabs(delta[t]) >= 2.0)
            for (j=0; j<nlat; j++)
              for (i=0; i<nlon; i++)

                if (buf[varid_tasmax][i+j*nlon] != info_tmp[varid_tasmax]->fillvalue) {

                  /* Save non-corrected mean temperature */
                  curtas = (buf[varid_tasmax][i+j*nlon] + buf[varid_tasmin][i+j*nlon]) / 2.0;
                  /* Compute new temperature */                
                  buf[varid_tasmax][i+j*nlon] += delta[t];
                  buf[varid_tasmin][i+j*nlon] += delta[t];
                  /* New mean temperature */
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

        /* Process each variable for writing */
        for (var=0; var<obs_var->nobs_var; var++)
          /* Write dimensions of field in newly-created NetCDF output file */
          if (found_file[var] == FALSE) {
            /* We just created output file: we need to write dimensions */
            ctimeval[0] = time_ls[t];
            istat = write_netcdf_dims_3d(lon, lat, x, y, ctimeval, cal_type,
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
              (void) free(cal_type_tmp);
              (void) free(time_units_tmp);
              (void) free(timeval);
            
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
              return istat;
            }
          }

        /* Process each variable */
        for (var=0; var<obs_var->nobs_var; var++) {
          if (found_file[var] == FALSE)
            (void) fprintf(stderr, "%s: Writing data to %s\n",__FILE__, outfile[var]);
          /* Write data */
          istat = write_netcdf_var_3d_2d(buf[var], time_ls, info_tmp[var]->fillvalue, outfile[var], obs_var->netcdfname[var],
                                         info_tmp[var]->long_name, info_tmp[var]->units, info_tmp[var]->height, proj_tmp->name, 
                                         obs_var->dimxname, obs_var->dimyname, obs_var->timename,
                                         t, !(found_file[var]), nlon, nlat, ntime_file, FALSE);
        
          /* Free allocated memory */
          (void) free(buf[var]);
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
      
        (void) free(time_s->year);
        (void) free(time_s->month);
        (void) free(time_s->day);
        (void) free(time_s->hour);
        (void) free(time_s->minutes);
        (void) free(time_s->seconds);
      
        (void) free(time_s);
        (void) free(cal_type_tmp);
        (void) free(time_units_tmp);
        (void) free(timeval);
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
        (void) free(cal_type_tmp);
        (void) free(time_units_tmp);
        (void) free(timeval);
      
        return -1;
      }
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
  
  /* Free allocated memory */
  (void) free(infile);
  (void) free(outfile);
  (void) free(format);
  
  /* Success diagnostic */
  return 0;
}
