/* ***************************************************** */
/* free_main_data Free main data structure               */
/* free_main-data.c                                      */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: nov 2008                            */
/* Last date of modification: nov 2008                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file free_main_data.c
    \brief Free main data structure.
*/

#include <dsclim.h>

/** Free main data structure. */
void free_main_data(data_struct *data) {

  /**
     @param[in]  data  MASTER data structure.
  */

  int i; /* Loop counter */
  int j; /* Loop counter */
  int s; /* Loop counter */
  int end_cat; /* End category to process */

  for (i=0; i<NCAT; i++) {

    for (j=0; j<data->field[i].n_ls; j++) {

      if (data->field[i].data[j].clim_info->clim_provided == 1)
        (void) free(data->field[i].data[j].clim_info->clim_filein_ls);
      if (data->field[i].data[j].clim_info->clim_save == 1)
        (void) free(data->field[i].data[j].clim_info->clim_fileout_ls);
      if (data->field[i].data[j].clim_info->clim_save == 1 || data->field[i].data[j].clim_info->clim_provided == 1)
        (void) free(data->field[i].data[j].clim_info->clim_nomvar_ls);
      (void) free(data->field[i].data[j].clim_info);

      if (data->field[i].data[j].eof_info->eof_provided == 1) {
        (void) free(data->field[i].data[j].eof_info->eof_coords);
        (void) free(data->field[i].data[j].eof_info->eof_filein_ls);
      }
      if (data->field[i].data[j].eof_info->eof_save == 1)
        (void) free(data->field[i].data[j].eof_info->eof_fileout_ls);
      if (data->field[i].data[j].eof_info->eof_provided == 1 || data->field[i].data[j].eof_info->eof_save == 1) {
        (void) free(data->field[i].data[j].eof_data->eof_nomvar_ls);
        (void) free(data->field[i].data[j].eof_data->sing_nomvar_ls);
      }

      if (data->field[i].data[j].eof_info->eof_project == 1) {
        if (i == 0 || i == 1)
          (void) free(data->field[i].data[j].field_eof_ls);
        (void) free(data->field[i].data[j].eof_data->eof_ls);
        (void) free(data->field[i].data[j].eof_data->sing_ls);
      }

      if (data->field[i].data[j].eof_info->eof_project == 1)
        if (i == 0 || i == 1) {
          (void) free(data->field[i].data[j].eof_info->info->units);
          (void) free(data->field[i].data[j].eof_info->info->height);
          (void) free(data->field[i].data[j].eof_info->info->coordinates);
          (void) free(data->field[i].data[j].eof_info->info->grid_mapping);
          (void) free(data->field[i].data[j].eof_info->info->long_name);
        }
      (void) free(data->field[i].data[j].eof_info->info);
      (void) free(data->field[i].data[j].eof_info);
      (void) free(data->field[i].data[j].eof_data);

      (void) free(data->field[i].data[j].info->coordinates);
      (void) free(data->field[i].data[j].info->grid_mapping);
      (void) free(data->field[i].data[j].info->units);
      (void) free(data->field[i].data[j].info->height);
      (void) free(data->field[i].data[j].info->long_name);

      (void) free(data->field[i].data[j].info);
      (void) free(data->field[i].data[j].nomvar_ls);
      (void) free(data->field[i].data[j].filename_ls);

      (void) free(data->field[i].proj[j].name);
      (void) free(data->field[i].proj[j].coords);      
      
      if (i == 0 || i == 1) {
        for (s=0; s<data->conf->nseasons; s++) {
          (void) free(data->field[i].data[j].down->days_class_clusters[s]);
          (void) free(data->field[i].data[j].down->dist[s]);
          (void) free(data->field[i].data[j].down->mean_dist[s]);
          (void) free(data->field[i].data[j].down->var_dist[s]);
        }
        (void) free(data->field[i].data[j].down->mean_dist);
        (void) free(data->field[i].data[j].down->var_dist);
        (void) free(data->field[i].data[j].down->dist);
        (void) free(data->field[i].data[j].down->days_class_clusters);
        (void) free(data->field[i].data[j].down->var_pc_norm);
      }
      else {
        for (s=0; s<data->conf->nseasons; s++)
          (void) free(data->field[i].data[j].down->smean_norm[s]);
        (void) free(data->field[i].data[j].down->smean_norm);
        (void) free(data->field[i].data[j].down->smean);
        (void) free(data->field[i].data[j].down->mean);
        (void) free(data->field[i].data[j].down->var);
        for (s=0; s<data->conf->nseasons; s++)
          (void) free(data->field[i].data[j].down->delta[s]);
        (void) free(data->field[i].data[j].down->delta);
      }

      (void) free(data->field[i].data[j].down);

      (void) free(data->field[i].data[j].field_ls);
    }

    (void) free(data->field[i].lat_ls);
    (void) free(data->field[i].lon_ls);

    if (data->field[i].lon_eof_ls != NULL)
      (void) free(data->field[i].lon_eof_ls);
    if (data->field[i].lat_eof_ls != NULL)
      (void) free(data->field[i].lat_eof_ls);

    if (data->conf->period_ctrl->downscale == TRUE)
      end_cat = CTRL_FIELD_LS;
    else
      end_cat = FIELD_LS;
    if (i >= FIELD_LS && i <= end_cat) {
      if (data->field[i].n_ls > 0) {
        for (s=0; s<data->conf->nseasons; s++) {
          (void) free(data->field[i].precip_index[s]);
          (void) free(data->field[i].analog_days[s].tindex);
          (void) free(data->field[i].analog_days[s].tindex_all);
          (void) free(data->field[i].analog_days[s].tindex_s_all);
          (void) free(data->field[i].analog_days[s].year);
          (void) free(data->field[i].analog_days[s].month);
          (void) free(data->field[i].analog_days[s].day);
          (void) free(data->field[i].analog_days[s].year_s);
          (void) free(data->field[i].analog_days[s].month_s);
          (void) free(data->field[i].analog_days[s].day_s);
        }
        (void) free(data->field[i].analog_days_year.tindex);
        (void) free(data->field[i].analog_days_year.tindex_all);
        (void) free(data->field[i].analog_days_year.tindex_s_all);
        (void) free(data->field[i].analog_days_year.year);
        (void) free(data->field[i].analog_days_year.month);
        (void) free(data->field[i].analog_days_year.day);
        (void) free(data->field[i].analog_days_year.year_s);
        (void) free(data->field[i].analog_days_year.month_s);
        (void) free(data->field[i].analog_days_year.day_s);
      }
    }

    (void) free(data->field[i].precip_index);
    (void) free(data->field[i].analog_days);

    if (data->field[i].n_ls > 0) {
      (void) free(data->field[i].data);
      (void) free(data->field[i].time_s->year);
      (void) free(data->field[i].time_s->month);
      (void) free(data->field[i].time_s->day);
      (void) free(data->field[i].time_s->hour);
      (void) free(data->field[i].time_s->minutes);
      (void) free(data->field[i].time_s->seconds);
      (void) free(data->field[i].proj);
    }
    (void) free(data->field[i].time_s);
    (void) free(data->field[i].time_ls);    
  }

  for (s=0; s<data->conf->nseasons; s++) {
    (void) free(data->learning->data[s].time_s->year);
    (void) free(data->learning->data[s].time_s->month);
    (void) free(data->learning->data[s].time_s->day);
    (void) free(data->learning->data[s].time_s->hour);
    (void) free(data->learning->data[s].time_s->minutes);
    (void) free(data->learning->data[s].time_s->seconds);
    (void) free(data->learning->data[s].time_s);
    (void) free(data->learning->data[s].time);
    (void) free(data->learning->data[s].class_clusters);
    if (data->conf->season[s].nmonths > 0)
      (void) free(data->conf->season[s].month);
    
    if (data->learning->learning_provided == 1) {
      (void) free(data->learning->data[s].weight);
      (void) free(data->learning->data[s].precip_reg);
      (void) free(data->learning->data[s].precip_reg_cst);
      (void) free(data->learning->data[s].precip_index);
      (void) free(data->learning->data[s].sup_index);
    }
    else {
      (void) free(data->learning->obs->filename_eof);
      (void) free(data->learning->obs->nomvar_eof);
      (void) free(data->learning->obs->nomvar_sing);
      (void) free(data->learning->obs->eof);
      (void) free(data->learning->obs->sing);

      (void) free(data->learning->obs->time_s->year);
      (void) free(data->learning->obs->time_s->month);
      (void) free(data->learning->obs->time_s->day);
      (void) free(data->learning->obs->time_s->hour);
      (void) free(data->learning->obs->time_s->minutes);
      (void) free(data->learning->obs->time_s->seconds);      
      (void) free(data->learning->obs->time_s);

      (void) free(data->learning->rea->time_s->year);
      (void) free(data->learning->rea->time_s->month);
      (void) free(data->learning->rea->time_s->day);
      (void) free(data->learning->rea->time_s->hour);
      (void) free(data->learning->rea->time_s->minutes);
      (void) free(data->learning->rea->time_s->seconds);      
      (void) free(data->learning->rea->time_s);

      (void) free(data->learning->rea->filename_eof);
      (void) free(data->learning->rea->nomvar_eof);
      (void) free(data->learning->rea->nomvar_sing);
      (void) free(data->learning->rea->eof);
      (void) free(data->learning->rea->sing);

      (void) free(data->learning->obs);
      (void) free(data->learning->rea);
    }
  }

  (void) free(data->learning->time_s->year);
  (void) free(data->learning->time_s->month);
  (void) free(data->learning->time_s->day);
  (void) free(data->learning->time_s->hour);
  (void) free(data->learning->time_s->minutes);
  (void) free(data->learning->time_s->seconds);

  (void) free(data->learning->time_s);

  if (data->learning->learning_provided == 1) {
    (void) free(data->learning->filename_weight);
    (void) free(data->learning->filename_learn);
    (void) free(data->learning->filename_clust_learn);
    (void) free(data->learning->nomvar_time);
    (void) free(data->learning->nomvar_weight);
    (void) free(data->learning->nomvar_class_clusters);
    (void) free(data->learning->nomvar_precip_reg);
    (void) free(data->learning->nomvar_precip_reg_cst);
    (void) free(data->learning->nomvar_precip_index);
    (void) free(data->learning->nomvar_sup_index);
    (void) free(data->learning->nomvar_sup_index_mean);
    (void) free(data->learning->nomvar_sup_index_var);
    (void) free(data->learning->nomvar_pc_normalized_var);
    (void) free(data->learning->pc_normalized_var);
  }

  if (data->info->title != NULL) {
    (void) free(data->info->title);
    (void) free(data->info->title_french);
    (void) free(data->info->summary);
    (void) free(data->info->summary_french);
    (void) free(data->info->keywords);
    (void) free(data->info->processor);
    (void) free(data->info->description);
    (void) free(data->info->institution);
    (void) free(data->info->creator_email);
    (void) free(data->info->creator_url);
    (void) free(data->info->creator_name);
    (void) free(data->info->version);
    (void) free(data->info->scenario);
    (void) free(data->info->scenario_co2);
    (void) free(data->info->model);
    (void) free(data->info->institution_model);
    (void) free(data->info->country);
    (void) free(data->info->member);
    (void) free(data->info->downscaling_forcing);
    (void) free(data->info->timestep);
    (void) free(data->info->contact_email);
    (void) free(data->info->contact_name);
    (void) free(data->info->other_contact_email);
    (void) free(data->info->other_contact_name);
  }

  (void) free(data->reg->filename);
  (void) free(data->reg->lonname);
  (void) free(data->reg->latname);
  (void) free(data->reg->ptsname);
  (void) free(data->reg->lat);
  (void) free(data->reg->lon);

  if (data->conf->obs_var->nobs_var > 0) {
    for (i=0; i<data->conf->obs_var->nobs_var; i++) {
      (void) free(data->conf->obs_var->acronym[i]);
      (void) free(data->conf->obs_var->netcdfname[i]);
      (void) free(data->conf->obs_var->name[i]);
    }
    (void) free(data->conf->obs_var->acronym);
    (void) free(data->conf->obs_var->netcdfname);
    (void) free(data->conf->obs_var->name);
    (void) free(data->conf->obs_var->factor);
    (void) free(data->conf->obs_var->delta);
  }
  (void) free(data->conf->obs_var->template);
  (void) free(data->conf->obs_var->latname);
  (void) free(data->conf->obs_var->lonname);
  (void) free(data->conf->obs_var->timename);
  (void) free(data->conf->obs_var->proj->name);
  (void) free(data->conf->obs_var->proj->coords);
  (void) free(data->conf->obs_var->proj->grid_mapping_name);
  (void) free(data->conf->obs_var->dimxname);
  (void) free(data->conf->obs_var->dimyname);
  (void) free(data->conf->obs_var->dimcoords);
  (void) free(data->conf->obs_var->proj);
  (void) free(data->conf->obs_var);
  
  (void) free(data->conf->clim_filter_type);
  (void) free(data->conf->classif_type);
  (void) free(data->conf->time_units);
  (void) free(data->conf->cal_type);
  (void) free(data->conf->lonname);
  (void) free(data->conf->latname);
  (void) free(data->conf->coords);
  (void) free(data->conf->lonname_eof);
  (void) free(data->conf->latname_eof);
  (void) free(data->conf->eofname);
  (void) free(data->conf->timename);
  (void) free(data->conf->ptsname);
  (void) free(data->conf->clustname);
  (void) free(data->conf->output_path);

  if (data->conf->nperiods > 0)
    (void) free(data->conf->period);
  (void) free(data->conf->period_ctrl);
  (void) free(data->conf->season);

  (void) free(data->learning->data);

  (void) free(data->conf);
  (void) free(data->info);
  (void) free(data->learning);
  (void) free(data->reg);
  (void) free(data->field);

  (void) free(data);
}
