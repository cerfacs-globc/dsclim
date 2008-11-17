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

int remove_clim(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  double *bufnoclim = NULL;
  double *clim = NULL;
  tstruct *timein_ts = NULL;

  double fillvalue;

  int ntime_clim;

  int istat = 0;
  int i;
  int j;
  int cat;
  info_field_struct clim_info_field;
  proj_struct clim_proj;
  double *timeclim = NULL;

  /* Remove seasonal cycle:
     - Fix calendar and generate a gregorian calendar
     - Compute climatology including Feb 29th
     - Filter climatology
     - Optionally save climatology in file */

  ntime_clim = 366;

  timeclim = (double *) malloc(ntime_clim * sizeof(double));
  if (timeclim == NULL) alloc_error(__FILE__, __LINE__);
  for (j=0; j<ntime_clim; j++)
    timeclim[j] = (double) (j+1);
  
  for (cat=0; cat<NCAT; cat++) {

    for (i=0; i<data->field[cat].n_ls; i++) {

      bufnoclim = (double *) malloc(data->field[cat].nlon_ls * data->field[cat].nlat_ls * data->field[cat].ntime_ls * sizeof(double));
      if (bufnoclim == NULL) alloc_error(__FILE__, __LINE__);
      
      timein_ts = (tstruct *) malloc(data->field[cat].ntime_ls * sizeof(tstruct));
      if (timein_ts == NULL) alloc_error(__FILE__, __LINE__);
      (void) get_calendar_ts(timein_ts, data->conf->time_units, data->field[cat].time_ls, data->field[cat].ntime_ls);

      if (data->field[cat].data[i].clim_info->clim_remove == 1) {
        if (data->field[cat].data[i].clim_info->clim_provided == 1) {
          if (clim != NULL) {
            (void) free(clim);
            clim = NULL;
          }
          istat = read_netcdf_var_3d(&clim, &clim_info_field, &clim_proj, data->field[cat].data[i].clim_info->clim_filein_ls,
                                     data->field[cat].data[i].clim_info->clim_nomvar_ls,
                                     data->conf->lonname, data->conf->latname, data->conf->timename,
                                     data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime_clim);
          if (istat != 0) {
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim != NULL) (void) free(clim);
            return istat;
          }
          fillvalue = clim_info_field.fillvalue;
        }
        else {
          if (clim == NULL) {
            clim = (double *) malloc(data->field[cat].nlon_ls * data->field[cat].nlat_ls * ntime_clim * sizeof(double));
            if (clim == NULL) alloc_error(__FILE__, __LINE__);
          }
          fillvalue = data->field[cat].data[i].info->fillvalue;
        }
      
        (void) remove_seasonal_cycle(bufnoclim, clim, data->field[cat].data[i].field_ls, timein_ts,
                                     data->field[cat].data[i].info->fillvalue,
                                     data->conf->clim_filter_width, data->conf->clim_filter_type,
                                     data->field[cat].data[i].clim_info->clim_provided,
                                     data->field[cat].nlon_ls, data->field[cat].nlat_ls, data->field[cat].ntime_ls);
      
        if (data->field[cat].data[i].clim_info->clim_save == 1) {
        
          istat = create_netcdf("Computed climatology", "Climatologie calculee", "Computed climatology", "Climatologie calculee",
                                "climatologie,climatology", "C language", "Computed climatology", data->info->institution,
                                data->info->creator_email, data->info->creator_url, data->info->creator_name,
                                data->info->version, data->info->scenario, data->info->scenario_co2, data->info->model,
                                data->info->institution_model, data->info->country, data->info->member,
                                data->info->downscaling_forcing, data->info->contact_email, data->info->contact_name,
                                data->info->other_contact_email, data->info->other_contact_name,
                                data->field[cat].data[i].clim_info->clim_fileout_ls);
          if (istat != 0) {
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim != NULL) (void) free(clim);
            return istat;
          }
          istat = write_netcdf_dims_3d(data->field[cat].lon_ls, data->field[cat].lat_ls, timeclim, data->conf->cal_type,
                                       data->conf->time_units, data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime_clim,
                                       data->info->timestep, data->field[cat].proj[i].name, data->field[cat].proj[i].coords,
                                       data->field[cat].proj[i].grid_mapping_name, data->field[cat].proj[i].latin1,
                                       data->field[cat].proj[i].latin2, data->field[cat].proj[i].lonc, data->field[cat].proj[i].lat0,
                                       data->field[cat].proj[i].false_easting, data->field[cat].proj[i].false_northing,
                                       data->conf->lonname, data->conf->latname, data->conf->timename,
                                       data->field[cat].data[i].clim_info->clim_fileout_ls);
          if (istat != 0) {
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim != NULL) (void) free(clim);
            return istat;
          }
        
          istat = write_netcdf_var_3d(clim, fillvalue, data->field[cat].data[i].clim_info->clim_fileout_ls,
                                      data->field[cat].data[i].clim_info->clim_nomvar_ls, data->field[cat].proj[i].name,
                                      data->conf->lonname, data->conf->latname, data->conf->timename,
                                      data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime_clim);
          if (istat != 0) {
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim != NULL) (void) free(clim);
            return istat;
          }
        }
        if (clim != NULL) {
          (void) free(clim);
          clim = NULL;
        }
      
        (void) memcpy(data->field[cat].data[i].field_ls, bufnoclim,
                      data->field[cat].nlon_ls * data->field[cat].nlat_ls * data->field[cat].ntime_ls * sizeof(double));
      }
      (void) free(bufnoclim);
      (void) free(timein_ts);
    }
  }
  (void) free(timeclim);
  
  return 0;
}
