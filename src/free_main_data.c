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
      (void) free(data->field[i].data[j].eof_info);
      (void) free(data->field[i].data[j].eof_data);

      (void) free(data->field[i].data[j].info);
      (void) free(data->field[i].data[j].down);
      (void) free(data->field[i].data[j].nomvar_ls);
      (void) free(data->field[i].data[j].filename_ls);

      (void) free(data->field[i].proj[j].name);
      (void) free(data->field[i].proj[j].coords);
      
      if (i == 0 || i == 1) {
        (void) free(data->field[i].data[j].down->mean_dist);
        (void) free(data->field[i].data[j].down->var_dist);
        (void) free(data->field[i].data[j].down->dist);
        (void) free(data->field[i].data[j].down->days_class_clusters);
        (void) free(data->field[i].data[j].down->var_pc_norm);
      }
      else {
        (void) free(data->field[i].data[j].down->smean_norm);
        (void) free(data->field[i].data[j].down->mean);
        (void) free(data->field[i].data[j].down->var);
        (void) free(data->field[i].data[j].down->delta);
      }
    }

    for (s=0; s<data->conf->nseasons; s++) {
      (void) free(data->field[i].analog_days[s].tindex);
      (void) free(data->field[i].analog_days[s].year);
      (void) free(data->field[i].analog_days[s].month);
      (void) free(data->field[i].analog_days[s].day);
      (void) free(data->field[i].analog_days[s].year_s);
      (void) free(data->field[i].analog_days[s].month_s);
      (void) free(data->field[i].analog_days[s].day_s);
    }
    (void) free(data->field[i].analog_days);

    (void) free(data->field[i].precip_index);

    (void) free(data->field[i].data);
    (void) free(data->field[i].time_ls);
    (void) free(data->field[i].time_s);
    
    (void) free(data->field[i].proj);
  }

  for (s=0; s<data->conf->nseasons; s++) {
    (void) free(data->learning->data[s].time_s);
    (void) free(data->learning->data[s].class_clusters);
    if (data->conf->season[s].nmonths > 0)
      (void) free(data->conf->season[s].month);
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
  }

  (void) free(data->reg->filename);
  (void) free(data->reg->lonname);
  (void) free(data->reg->latname);
  (void) free(data->reg->ptsname);

  if (data->conf->obs_var->nobs_var > 0)
    for (i=0; i<data->conf->obs_var->nobs_var; i++) {
      (void) free(data->conf->obs_var->acronym);
      (void) free(data->conf->obs_var->netcdfname);
      (void) free(data->conf->obs_var->name);
    }
  (void) free(data->conf->obs_var);
  
  (void) free(data->conf->clim_filter_type);
  (void) free(data->conf->classif_type);
  (void) free(data->conf->time_units);
  (void) free(data->conf->cal_type);
  (void) free(data->conf->lonname);
  (void) free(data->conf->latname);
  (void) free(data->conf->lonname_eof);
  (void) free(data->conf->latname_eof);
  (void) free(data->conf->eofname);
  (void) free(data->conf->timename);
  (void) free(data->conf->ptsname);
  (void) free(data->conf->clustname);

  (void) free(data->conf->period);
  (void) free(data->conf->period_ctrl);
  (void) free(data->conf->season);

  (void) free(data->conf->proj->name);
  (void) free(data->conf->proj->coords);
  (void) free(data->conf->proj->grid_mapping_name);
  (void) free(data->conf->proj);

  (void) free(data->learning->data);

  (void) free(data->conf);
  (void) free(data->info);
  (void) free(data->learning);
  (void) free(data->reg);
  (void) free(data->field);

  (void) free(data);
}
