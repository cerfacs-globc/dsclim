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

/** Free main data structure. */
void
free_main_data(data_struct *data) {

  /**
     @param[in]  data  MASTER data structure.
  */

  int i; /* Loop counter */
  int j; /* Loop counter */
  int s; /* Loop counter */
  int end_cat; /* End category to process */

  if ( (data->conf->analog_save == TRUE || data->conf->output_only == TRUE) && data->conf->period_ctrl->downscale == TRUE )
    (void) free(data->conf->analog_file_ctrl);
  if (data->conf->analog_save == TRUE || data->conf->output_only == TRUE)
    (void) free(data->conf->analog_file_other);

  for (i=0; i<NCAT; i++) {

    for (j=0; j<data->field[i].n_ls; j++) {

      if (data->field[i].data[j].clim_info->clim_provided == TRUE)
        (void) free(data->field[i].data[j].clim_info->clim_filein_ls);
      if (data->field[i].data[j].clim_info->clim_save == TRUE)
        (void) free(data->field[i].data[j].clim_info->clim_fileout_ls);
      if (data->field[i].data[j].clim_info->clim_save == TRUE || data->field[i].data[j].clim_info->clim_provided == TRUE)
        (void) free(data->field[i].data[j].clim_info->clim_nomvar_ls);
      (void) free(data->field[i].data[j].clim_info);

      if (data->field[i].data[j].eof_info->eof_project == TRUE) {
        (void) free(data->field[i].data[j].eof_info->eof_coords);
        (void) free(data->field[i].data[j].eof_info->eof_filein_ls);
        (void) free(data->field[i].data[j].eof_data->eof_nomvar_ls);
        (void) free(data->field[i].data[j].eof_data->sing_nomvar_ls);

        if (i == 0 || i == 1)
          (void) free(data->field[i].data[j].field_eof_ls);
        (void) free(data->field[i].data[j].eof_data->eof_ls);
        (void) free(data->field[i].data[j].eof_data->sing_ls);

        if ((i == 0 || i == 1) && data->conf->output_only != TRUE) {
          (void) free(data->field[i].data[j].eof_info->info->units);
          (void) free(data->field[i].data[j].eof_info->info->height);
          (void) free(data->field[i].data[j].eof_info->info->coordinates);
          (void) free(data->field[i].data[j].eof_info->info->grid_mapping);
          (void) free(data->field[i].data[j].eof_info->info->long_name);
        }
      }
      (void) free(data->field[i].data[j].eof_info->info);
      (void) free(data->field[i].data[j].eof_info);
      (void) free(data->field[i].data[j].eof_data);
      
      if (data->conf->output_only != TRUE) {
        (void) free(data->field[i].data[j].info->coordinates);
        (void) free(data->field[i].data[j].info->grid_mapping);
        (void) free(data->field[i].data[j].info->units);
        (void) free(data->field[i].data[j].info->height);
        (void) free(data->field[i].data[j].info->long_name);
      }

      (void) free(data->field[i].data[j].info);
      (void) free(data->field[i].data[j].nomvar_ls);
      (void) free(data->field[i].data[j].filename_ls);
      (void) free(data->field[i].data[j].dimyname);
      (void) free(data->field[i].data[j].dimxname);
      (void) free(data->field[i].data[j].latname);
      (void) free(data->field[i].data[j].lonname);
      (void) free(data->field[i].data[j].timename);

      if (data->field[i].proj[j].name != NULL)
        (void) free(data->field[i].proj[j].name);
      if (data->field[i].proj[j].grid_mapping_name != NULL)
        (void) free(data->field[i].proj[j].grid_mapping_name);
      if (data->field[i].proj[j].coords != NULL)
        (void) free(data->field[i].proj[j].coords);

      if (data->conf->output_only != TRUE) {
        for (s=0; s<data->conf->nseasons; s++) {
          if (i == 0 || (i == 1 && data->conf->period_ctrl->downscale == TRUE)) {
            (void) free(data->field[i].data[j].down->days_class_clusters[s]);
            (void) free(data->field[i].data[j].down->dist[s]);
          }
          if (i == 1) {
            (void) free(data->field[i].data[j].down->mean_dist[s]);
            (void) free(data->field[i].data[j].down->var_dist[s]);
          }
        }
      }
      if (i == 0 || i == 1) {
        (void) free(data->field[i].data[j].down->mean_dist);
        (void) free(data->field[i].data[j].down->var_dist);
      }
      if ( i == 0 || i == 1) {
        if (i == 0 || (i == 1 && data->conf->period_ctrl->downscale == TRUE)) {
          if (data->conf->output_only != TRUE) {
            (void) free(data->field[i].data[j].down->dist_all);
            (void) free(data->field[i].data[j].down->days_class_clusters_all);
          }
        }
        (void) free(data->field[i].data[j].down->dist);
        (void) free(data->field[i].data[j].down->days_class_clusters);
        (void) free(data->field[i].data[j].down->var_pc_norm);
        if (i == 0 || (i == 1 && data->conf->period_ctrl->downscale == TRUE))
          (void) free(data->field[i+2].data[j].down->delta_all);
      }
      else {
        if (data->conf->period_ctrl->downscale == TRUE || i == 2)
          if (data->conf->output_only != TRUE)
            for (s=0; s<data->conf->nseasons; s++)
              (void) free(data->field[i].data[j].down->smean_norm[s]);
        if (data->conf->output_only != TRUE)
          (void) free(data->field[i].data[j].down->smean);
        
        if (data->conf->period_ctrl->downscale == TRUE || i == 2)
          if (data->conf->output_only != TRUE)
            for (s=0; s<data->conf->nseasons; s++)
              (void) free(data->field[i].data[j].down->delta[s]);

        (void) free(data->field[i].data[j].down->smean_norm);
        (void) free(data->field[i].data[j].down->mean);
        (void) free(data->field[i].data[j].down->var);
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
        if (data->conf->output_only != TRUE) {
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
        }
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
      if (data->conf->output_only != TRUE) {
        (void) free(data->field[i].time_s->year);
        (void) free(data->field[i].time_s->month);
        (void) free(data->field[i].time_s->day);
        (void) free(data->field[i].time_s->hour);
        (void) free(data->field[i].time_s->minutes);
        (void) free(data->field[i].time_s->seconds);
      }
      (void) free(data->field[i].proj);
    }
    (void) free(data->field[i].time_s);
    (void) free(data->field[i].time_ls);    
  }

  for (s=0; s<data->conf->nseasons; s++) {
    if (data->conf->output_only != TRUE) {
      (void) free(data->learning->data[s].time_s->year);
      (void) free(data->learning->data[s].time_s->month);
      (void) free(data->learning->data[s].time_s->day);
      (void) free(data->learning->data[s].time_s->hour);
      (void) free(data->learning->data[s].time_s->minutes);
      (void) free(data->learning->data[s].time_s->seconds);
      (void) free(data->learning->data[s].time);
      (void) free(data->learning->data[s].class_clusters);
    }
    (void) free(data->learning->data[s].time_s);
    if (data->conf->season[s].nmonths > 0)
      (void) free(data->conf->season[s].month);

    if (data->conf->output_only != TRUE) {
      (void) free(data->learning->data[s].weight);
      (void) free(data->learning->data[s].precip_reg);
      (void) free(data->learning->data[s].precip_reg_cst);
      (void) free(data->learning->data[s].precip_index);
      (void) free(data->learning->data[s].sup_index);
    }
  }
  
  if (data->learning->learning_provided == FALSE) {
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

    (void) free(data->learning->nomvar_rea_sup);
    (void) free(data->learning->filename_rea_sup);
    (void) free(data->learning->rea_coords);
    (void) free(data->learning->rea_gridname);
    (void) free(data->learning->rea_dimxname);
    (void) free(data->learning->rea_dimyname);
    (void) free(data->learning->rea_lonname);
    (void) free(data->learning->rea_latname);
    (void) free(data->learning->rea_timename);

    (void) free(data->learning->obs_dimxname);
    (void) free(data->learning->obs_dimyname);
    (void) free(data->learning->obs_lonname);
    (void) free(data->learning->obs_latname);
    (void) free(data->learning->obs_timename);
    (void) free(data->learning->obs_eofname);

    if (data->learning->lon != NULL)
      (void) free(data->learning->lon);
    if (data->learning->lat != NULL)
      (void) free(data->learning->lat);
  }

  if (data->conf->output_only != TRUE) {
    (void) free(data->learning->time_s->year);
    (void) free(data->learning->time_s->month);
    (void) free(data->learning->time_s->day);
    (void) free(data->learning->time_s->hour);
    (void) free(data->learning->time_s->minutes);
    (void) free(data->learning->time_s->seconds);
  }

  (void) free(data->learning->time_s);

  if (data->learning->learning_provided == TRUE) {
    (void) free(data->learning->filename_open_weight);
    (void) free(data->learning->filename_open_learn);
    (void) free(data->learning->filename_open_clust_learn);
  }

  if (data->conf->output_only != TRUE)
    (void) free(data->learning->pc_normalized_var);

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

  if (data->learning->learning_save == TRUE) {
    (void) free(data->learning->filename_save_weight);
    (void) free(data->learning->filename_save_learn);
    (void) free(data->learning->filename_save_clust_learn);
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
  (void) free(data->reg->dimxname);
  (void) free(data->reg->dimyname);
  (void) free(data->reg->lonname);
  (void) free(data->reg->latname);
  (void) free(data->reg->ptsname);
  if (data->reg->reg_save == TRUE) {
    (void) free(data->reg->filename_save_reg);
    (void) free(data->reg->timename);
  }
  if (data->conf->output_only != TRUE) {
    (void) free(data->reg->lat);
    (void) free(data->reg->lon);
  }

  if (data->secondary_mask->use_mask == TRUE) {
    if (data->conf->output_only != TRUE)
      (void) free(data->secondary_mask->field);
    (void) free(data->secondary_mask->filename);
    (void) free(data->secondary_mask->maskname);
    (void) free(data->secondary_mask->lonname);
    (void) free(data->secondary_mask->latname);
    (void) free(data->secondary_mask->coords);
    (void) free(data->secondary_mask->dimxname);
    (void) free(data->secondary_mask->dimyname);
    (void) free(data->secondary_mask->dimcoords);
    (void) free(data->secondary_mask->proj);
    (void) free(data->secondary_mask->lat);
    (void) free(data->secondary_mask->lon);
  }
  (void) free(data->secondary_mask);

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
  (void) free(data->conf->obs_var->frequency);
  (void) free(data->conf->obs_var->template);
  (void) free(data->conf->obs_var->latname);
  (void) free(data->conf->obs_var->lonname);
  (void) free(data->conf->obs_var->timename);
  (void) free(data->conf->obs_var->altitude);
  (void) free(data->conf->obs_var->altitudename);
  (void) free(data->conf->obs_var->proj->name);
  (void) free(data->conf->obs_var->proj->coords);
  (void) free(data->conf->obs_var->proj->grid_mapping_name);
  (void) free(data->conf->obs_var->dimxname);
  (void) free(data->conf->obs_var->dimyname);
  (void) free(data->conf->obs_var->dimcoords);
  (void) free(data->conf->obs_var->proj);
  (void) free(data->conf->obs_var->path);
  (void) free(data->conf->obs_var);
  
  (void) free(data->conf->clim_filter_type);
  (void) free(data->conf->classif_type);
  (void) free(data->conf->time_units);
  (void) free(data->conf->cal_type);
  (void) free(data->conf->dimxname_eof);
  (void) free(data->conf->dimyname_eof);
  (void) free(data->conf->lonname_eof);
  (void) free(data->conf->latname_eof);
  (void) free(data->conf->eofname);
  (void) free(data->conf->ptsname);
  (void) free(data->conf->clustname);
  (void) free(data->conf->output_path);

  if (data->conf->nperiods > 0)
    (void) free(data->conf->period);
  (void) free(data->conf->period_ctrl);
  (void) free(data->conf->season);

  (void) free(data->learning->data);

  (void) free(data->conf->config);

  (void) free(data->conf);
  (void) free(data->info);
  (void) free(data->learning);
  (void) free(data->reg);
  (void) free(data->field);
}
