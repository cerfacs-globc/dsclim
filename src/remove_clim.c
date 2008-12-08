/* ***************************************************** */
/* remove_clim Remove climatology.                       */
/* remove_clim.c                                         */
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
/*! \file remove_clim.c
    \brief Remove climatologies.
*/

#include <dsclim.h>

/** Remove climatologies. */
int remove_clim(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  double *bufnoclim = NULL; /* Temporary buffer for field with climatology removed */
  double *clim = NULL; /* Climatology buffer */
  tstruct *timein_ts = NULL; /* Time info for input field */
  int ntime_clim; /* Number of times for input field */
  int nlon_file; /* Longitude dimension for input field */
  int nlat_file; /* Latitude dimension for input field */
  int ntime_file; /* Time dimension for input field */

  double fillvalue; /* Missing value */

  int istat = 0; /* Diagnostic status */
  int i; /* Loop counter */
  int j; /* Loop counter */
  int cat; /* Loop counter for field category */
  info_field_struct clim_info_field; /* Information structure for climatology field */
  proj_struct clim_proj; /* Spatial projection for climatology field */
  double *timeclim = NULL; /* Time info for climatology field */

  /* Remove seasonal cycle:
     - Fix calendar and generate a gregorian calendar
     - Compute climatology including Feb 29th
     - Filter climatology
     - Optionally save climatology in file */

  /* Climatological year is 366 days */
  ntime_clim = 366;

  /* Time variable for climatological year: day timestep */
  timeclim = (double *) malloc(ntime_clim * sizeof(double));
  if (timeclim == NULL) alloc_error(__FILE__, __LINE__);
  for (j=0; j<ntime_clim; j++)
    timeclim[j] = (double) (j+1);

  /* Loop over all large-scale field categories to process */
  for (cat=0; cat<NCAT; cat++) {

    /* Loop over all large-scale fields */
    for (i=0; i<data->field[cat].n_ls; i++) {

      /* Allocate memory for field with climatology removed */
      bufnoclim = (double *) malloc(data->field[cat].nlon_ls * data->field[cat].nlat_ls * data->field[cat].ntime_ls * sizeof(double));
      if (bufnoclim == NULL) alloc_error(__FILE__, __LINE__);

      /* Allocate memory for temporary time structure */
      timein_ts = (tstruct *) malloc(data->field[cat].ntime_ls * sizeof(tstruct));
      if (timein_ts == NULL) alloc_error(__FILE__, __LINE__);
      /* Get time info and calendar units */
      (void) get_calendar_ts(timein_ts, data->conf->time_units, data->field[cat].time_ls, data->field[cat].ntime_ls);

      /* If we need to remove climatology for that field */
      if (data->field[cat].data[i].clim_info->clim_remove == 1) {
        /* If climatology field is already provided */
        if (data->field[cat].data[i].clim_info->clim_provided == 1) {
          /* Free clim buffer before reading data into it */
          if (clim != NULL) {
            (void) free(clim);
            clim = NULL;
          }
          /* Read climatology from NetCDF file */
          istat = read_netcdf_var_3d(&clim, &clim_info_field, &clim_proj, data->field[cat].data[i].clim_info->clim_filein_ls,
                                     data->field[cat].data[i].clim_info->clim_nomvar_ls,
                                     data->conf->lonname, data->conf->latname, data->conf->timename,
                                     &nlon_file, &nlat_file, &ntime_file);
          if (data->field[cat].nlon_ls != nlon_file || data->field[cat].nlat_ls != nlat_file || ntime_clim != ntime_file) {
            (void) fprintf(stderr, "%s: Problems in dimensions! nlat=%d nlat_file=%d nlon=%d nlon_file=%d ntime=%d ntime_file=%d\n",
                           __FILE__, data->field[cat].nlat_ls, nlat_file, data->field[cat].nlon_ls, nlon_file, ntime_clim, ntime_file);
            istat = -1;
          }
          if (istat != 0) {
            /* In case of error in reading data */
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim != NULL) (void) free(clim);
            return istat;
          }
          /* Get missing value */
          fillvalue = clim_info_field.fillvalue;
        }
        else {
          /* Climatology is not provided: must calculate */
          if (clim == NULL) {
            /* Allocate memory if not already */
            clim = (double *) malloc(data->field[cat].nlon_ls * data->field[cat].nlat_ls * ntime_clim * sizeof(double));
            if (clim == NULL) alloc_error(__FILE__, __LINE__);
          }
          /* Get missing value */
          fillvalue = data->field[cat].data[i].info->fillvalue;
        }
      
        /* Remove seasonal cycle by calculating filtered climatology and substracting from field values */
        (void) remove_seasonal_cycle(bufnoclim, clim, data->field[cat].data[i].field_ls, timein_ts,
                                     data->field[cat].data[i].info->fillvalue,
                                     data->conf->clim_filter_width, data->conf->clim_filter_type,
                                     data->field[cat].data[i].clim_info->clim_provided,
                                     data->field[cat].nlon_ls, data->field[cat].nlat_ls, data->field[cat].ntime_ls);
      
        
        /* If we want to save climatology in NetCDF output file for further use */
        if (data->field[cat].data[i].clim_info->clim_save == 1) {
          istat = create_netcdf("Computed climatology", "Climatologie calculee", "Computed climatology", "Climatologie calculee",
                                "climatologie,climatology", "C language", "Computed climatology", data->info->institution,
                                data->info->creator_email, data->info->creator_url, data->info->creator_name,
                                data->info->version, data->info->scenario, data->info->scenario_co2, data->info->model,
                                data->info->institution_model, data->info->country, data->info->member,
                                data->info->downscaling_forcing, data->info->contact_email, data->info->contact_name,
                                data->info->other_contact_email, data->info->other_contact_name,
                                data->field[cat].data[i].clim_info->clim_fileout_ls, TRUE);
          if (istat != 0) {
            /* In case of failure */
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim != NULL) (void) free(clim);
            return istat;
          }
          /* Write dimensions of climatology field in NetCDF output file */
          istat = write_netcdf_dims_3d(data->field[cat].lon_ls, data->field[cat].lat_ls, timeclim, data->conf->cal_type,
                                       data->conf->time_units, data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime_clim,
                                       data->info->timestep, data->field[cat].proj[i].name, data->field[cat].proj[i].coords,
                                       data->field[cat].proj[i].grid_mapping_name, data->field[cat].proj[i].latin1,
                                       data->field[cat].proj[i].latin2, data->field[cat].proj[i].lonc, data->field[cat].proj[i].lat0,
                                       data->field[cat].proj[i].false_easting, data->field[cat].proj[i].false_northing,
                                       data->conf->lonname, data->conf->latname, data->conf->timename,
                                       data->field[cat].data[i].clim_info->clim_fileout_ls, TRUE);
          if (istat != 0) {
            /* In case of failure */
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim != NULL) (void) free(clim);
            return istat;
          }
        
          /* Write climatology field in NetCDF output file */
          istat = write_netcdf_var_3d(clim, fillvalue, data->field[cat].data[i].clim_info->clim_fileout_ls,
                                      data->field[cat].data[i].clim_info->clim_nomvar_ls, data->field[cat].proj[i].name,
                                      data->conf->lonname, data->conf->latname, data->conf->timename,
                                      data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime_clim);
          if (istat != 0) {
            /* In case of failure */
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim != NULL) (void) free(clim);
            return istat;
          }
        }
        /* Free memory if needed */
        if (clim != NULL) {
          (void) free(clim);
          clim = NULL;
        }

        /* Copy field with climatology removed to proper variable in data structure */
        (void) memcpy(data->field[cat].data[i].field_ls, bufnoclim,
                      data->field[cat].nlon_ls * data->field[cat].nlat_ls * data->field[cat].ntime_ls * sizeof(double));
      }
      /* Free memory */
      (void) free(bufnoclim);
      (void) free(timein_ts);
    }
  }
  /* Free memory */
  (void) free(timeclim);

  /* Success status */
  return 0;
}
