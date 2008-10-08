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

short int remove_clim(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  double *bufnoclim = NULL;
  double *clim = NULL;
  tstruct *timein_ts = NULL;

  double fillvalue;

  int ntime_clim;

  short int istat = 0;
  int i;
  info_field_struct clim_info_field;
  proj_struct clim_proj;

  /* Remove seasonal cycle:
     - Fix calendar and generate a gregorian calendar
     - Compute climatology including Feb 29th
     - Filter climatology
     - Optionally save climatology in file */

  ntime_clim = 366;

  bufnoclim = (double *) malloc(data->field->nlon_ls * data->field->nlat_ls * data->field->ntime_ls * sizeof(double));
  if (bufnoclim == NULL) alloc_error(__FILE__, __LINE__);

  timein_ts = (tstruct *) malloc(data->field->ntime_ls * sizeof(tstruct));
  if (timein_ts == NULL) alloc_error(__FILE__, __LINE__);
  (void) get_calendar_ts(timein_ts, data->conf->time_units, data->field->time_ls, data->field->ntime_ls);

  for (i=0; i<data->field->n_ls; i++) {

    if (data->field->clim_provided[i] == 1) {
      if (clim != NULL) {
        (void) free(clim);
        clim = NULL;
      }
      istat = read_netcdf_var_3d(&clim, &clim_info_field, &clim_proj, data->field->clim_filein_ls[i],
                                 data->field->clim_nomvar_ls[i],
                                 data->conf->lonname, data->conf->latname, data->conf->timename,
                                 data->field->nlon_ls, data->field->nlat_ls, ntime_clim);
      if (istat != 0) {
        (void) free(bufnoclim);
        (void) free(timein_ts);
        if (clim != NULL) (void) free(clim);
        return istat;
      }
      fillvalue = clim_info_field.fillvalue;
    }
    else {
      if (clim == NULL) {
        clim = (double *) malloc(data->field->nlon_ls * data->field->nlat_ls * ntime_clim * sizeof(double));
        if (clim == NULL) alloc_error(__FILE__, __LINE__);
      }
      fillvalue = data->field->info_field[i].fillvalue;
    }

    (void) remove_seasonal_cycle(bufnoclim, clim, data->field->field_ls[i], timein_ts, data->field->info_field[i].fillvalue,
                                 data->conf->clim_filter_width, data->conf->clim_filter_type, data->field->clim_provided[i],
                                 data->field->nlon_ls, data->field->nlat_ls, data->field->ntime_ls);
    
    if (data->field->clim_save[i] == 1) {

      istat = create_netcdf("Computed climatology", "Climatologie calculee", "Computed climatology", "Climatologie calculee",
                            "climatologie,climatology", "C language", "Computed climatology", data->info->institution,
                            data->info->creator_email, data->info->creator_url, data->info->creator_name,
                            data->info->version, data->info->scenario, data->info->scenario_co2, data->info->model,
                            data->info->institution_model, data->info->country, data->info->member,
                            data->info->downscaling_forcing, data->info->contact_email, data->info->contact_name,
                            data->info->other_contact_email, data->info->other_contact_name, data->field->clim_fileout_ls[i]);
      if (istat != 0) {
        (void) free(bufnoclim);
        (void) free(timein_ts);
        if (clim != NULL) (void) free(clim);
        return istat;
      }

      istat = write_netcdf_dims_3d(data->field->lon_ls, data->field->lat_ls,  data->field->time_ls, data->conf->cal_type,
                                   data->conf->time_units, data->field->nlon_ls, data->field->nlat_ls, ntime_clim,
                                   data->info->timestep, data->field->proj[i].name, data->field->proj[i].coords,
                                   data->field->proj[i].grid_mapping_name, data->field->proj[i].latin1,
                                   data->field->proj[i].latin2, data->field->proj[i].lonc, data->field->proj[i].lat0,
                                   data->field->proj[i].false_easting, data->field->proj[i].false_northing,
                                   data->conf->lonname, data->conf->latname, data->conf->timename,
                                   data->field->clim_fileout_ls[i]);
      if (istat != 0) {
        (void) free(bufnoclim);
        (void) free(timein_ts);
        if (clim != NULL) (void) free(clim);
        return istat;
      }

      istat = write_netcdf_var_3d(clim, fillvalue, data->field->clim_fileout_ls[i],
                                  data->field->clim_nomvar_ls[i], data->field->proj[i].name, 
                                  data->conf->lonname, data->conf->latname, data->conf->timename,
                                  data->field->nlon_ls, data->field->nlat_ls, ntime_clim);
      if (istat != 0) {
        (void) free(bufnoclim);
        (void) free(timein_ts);
        if (clim != NULL) (void) free(clim);
        return istat;
      }
    }
    if (clim != NULL)
      (void) free(clim);
    
    (void) free(data->field->field_ls[i]);
    data->field->field_ls[i] = bufnoclim;
    bufnoclim = NULL;
  }

  return 0;
}

