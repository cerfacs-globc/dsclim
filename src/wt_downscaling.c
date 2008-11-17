/* ***************************************************** */
/* wt_downscaling Downscaling climate scenarios using    */
/* weather typing.                                       */
/* wt_downscaling.c                                      */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: sep 2008                            */
/* Last date of modification: sep 2008                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file wt_downscaling.c
    \brief Downscaling climate scenarios program using weather typing.
*/

#include <dsclim.h>

int wt_downscaling(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  double *buf_sub = NULL;
  double *buftmp = NULL;
  double *buftmpf = NULL;
  double *var_pc_norm_all = NULL;
  double *var_pc_norm_all_tmp = NULL;
  int **ntime_sub = NULL;
  int ntime_sub_learn;
  int ntime_sub_learn_all;
  
  int istat;
  int i;
  int ii;
  int s;
  int cat;
  int end_cat;
  
  ntime_sub = (int **) malloc(NCAT * sizeof(int *));
  if (ntime_sub == NULL) alloc_error(__FILE__, __LINE__);
  
  for (cat=0; cat<NCAT; cat++) {
    ntime_sub[cat] = (int *) malloc(data->conf->nseasons * sizeof(int));
    if (ntime_sub[cat] == NULL) alloc_error(__FILE__, __LINE__);
  }
  
  /** Step 1: Read large-scale fields **/
  istat = read_large_scale_fields(data);
  if (istat != 0) return istat;

  /** Step 2: Compute climatologies and remove them from selected large scale fields **/
  istat = remove_clim(data);
  if (istat != 0) return istat;  

  /** Step 3: Project selected large scale fields on EOF **/  
  /* Read EOFs and Singular Values */
  istat = read_large_scale_eof(data);
  if (istat != 0) return istat;
  
  /* Project selected large scale fields on EOF */
  for (cat=0; cat<2; cat++)
    for (i=0; i<data->field[cat].n_ls; i++) {
      if (data->field[cat].data[i].eof_info->eof_project == 1) {
        data->field[cat].data[i].field_eof_ls = (double *) malloc(data->field[cat].ntime_ls * data->field[cat].data[i].eof_info->neof_ls *
                                                                            sizeof(double));
        if (data->field[cat].data[i].field_eof_ls == NULL) alloc_error(__FILE__, __LINE__);
        istat = project_field_eof(data->field[cat].data[i].field_eof_ls, data->field[cat].data[i].field_ls,
                                  data->field[cat].data[i].eof_data->eof_ls, data->field[cat].data[i].eof_data->sing_ls,
                                  data->field[cat].data[i].eof_info->info->fillvalue, data->field[cat].data[i].eof_info->eof_scale,
                                  data->field[cat].nlon_eof_ls, data->field[cat].nlat_eof_ls, data->field[cat].ntime_ls,
                                  data->field[cat].data[i].eof_info->neof_ls);
        if (istat != 0) return istat;
      }
    }

  /** CONTROL RUN **/

  /** Step 4: Compute distance to clusters for the control run **/
  cat = CTRL_FIELD_LS;
  for (i=0; i<data->field[cat].n_ls; i++) {

    buftmp = (double *) malloc(data->field[cat].ntime_ls*data->field[cat].data[i].eof_info->neof_ls * sizeof(double));
    if (buftmp == NULL) alloc_error(__FILE__, __LINE__); 

    /* Normalisation of the principal component by the square root of the variance of the first one */
    /* Select common time period between the learning period and the model period (control run) */
    /* for first variance calculation */
    (void) sub_period_common(&buf_sub, &ntime_sub_learn_all, data->field[cat].data[i].field_eof_ls,
                             data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                             data->learning->time_s->year, data->learning->time_s->month,
                             data->learning->time_s->day, 1,
                             data->field[cat].data[i].eof_info->neof_ls, 1, data->field[cat].ntime_ls, data->learning->ntime);

    buftmpf = (double *) malloc(ntime_sub_learn_all*data->field[cat].data[i].eof_info->neof_ls * sizeof(double));
    if (buftmpf == NULL) alloc_error(__FILE__, __LINE__); 

    //    for (s=0; s<data->field[cat].ntime_ls; s++)
    //      printf("%d %lf\n",s,data->field[cat].data[i].field_eof_ls[s]);
    data->field[cat].data[i].first_variance = -9999.9999;
    (void) normalize_pc(data->field[cat].data[i].down->var_pc_norm, &(data->field[cat].data[i].first_variance),
                        buftmpf, buf_sub, data->field[cat].data[i].eof_info->neof_ls,
                        ntime_sub_learn_all);
    (void) free(buf_sub);
    (void) free(buftmpf);

    printf("Second norm\n");
    var_pc_norm_all = (double *) malloc(data->field[cat].data[i].eof_info->neof_ls * sizeof(double));
    if (var_pc_norm_all == NULL) alloc_error(__FILE__, __LINE__);
    (void) normalize_pc(var_pc_norm_all, &(data->field[cat].data[i].first_variance),
                        buftmp, data->field[cat].data[i].field_eof_ls, data->field[cat].data[i].eof_info->neof_ls,
                        data->field[cat].ntime_ls);
    (void) free(var_pc_norm_all);

    for (s=0; s<data->conf->nseasons; s++) {

      /* Compute mean and variance of principal components of selected large-scale fields */
      data->field[cat].data[i].down->mean_dist[s] = (double *) malloc(data->conf->season[s].nclusters * sizeof(double));
      if (data->field[cat].data[i].down->mean_dist[s] == NULL) alloc_error(__FILE__, __LINE__);
      data->field[cat].data[i].down->var_dist[s] = (double *) malloc(data->conf->season[s].nclusters * sizeof(double));
      if (data->field[cat].data[i].down->var_dist[s] == NULL) alloc_error(__FILE__, __LINE__);
      
      /* Select common time period between the learning period and the model period (control run) */
      (void) sub_period_common(&buf_sub, &ntime_sub_learn, buftmp,
                               data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                               data->learning->data[s].time_s->year, data->learning->data[s].time_s->month,
                               data->learning->data[s].time_s->day, 1,
                               data->field[cat].data[i].eof_info->neof_ls, 1, data->field[cat].ntime_ls, data->learning->data[s].ntime);
      
      /* Compute mean and variance of distances to clusters */
      (void) mean_variance_dist_clusters(data->field[cat].data[i].down->mean_dist[s], data->field[cat].data[i].down->var_dist[s],
                                         buf_sub, data->learning->data[s].weight,
                                         data->learning->pc_normalized_var, data->field[cat].data[i].down->var_pc_norm,
                                         data->field[cat].data[i].eof_info->neof_ls, data->conf->season[s].nclusters, ntime_sub_learn);
      printf("Season: %d\n", s);
      for (ii=0; ii<data->conf->season[s].nclusters; ii++)
        (void) printf("%s: Cluster #%d. Mean and variance of distances to clusters: %lf %lf\n", __FILE__, ii,
                      data->field[cat].data[i].down->mean_dist[s][ii], data->field[cat].data[i].down->var_dist[s][ii]);

      (void) free(buf_sub);
    }
    (void) free(buftmp);
  }
      
  /** Step 5: Compute mean and variance of secondary large-scale fields for the control run **/
  cat = CTRL_SEC_FIELD_LS;
  for (i=0; i<data->field[cat].n_ls; i++) {

    /* Compute spatial mean of secondary large-scale fields */
    data->field[cat].data[i].down->smean = (double *) malloc(data->field[cat].ntime_ls * sizeof(double));
    if (data->field[cat].data[i].down->smean == NULL) alloc_error(__FILE__, __LINE__);
    (void) mean_field_spatial(data->field[cat].data[i].down->smean, data->field[cat].data[i].field_ls,
                              data->field[cat].nlon_ls, data->field[cat].nlat_ls, data->field[cat].ntime_ls);
    
    for (s=0; s<data->conf->nseasons; s++) {
      
      /* Compute seasonal mean and variance of principal components of selected large-scale fields */
      
      /* Select common time period between the learning period and the model period (control run) */
      (void) sub_period_common(&buf_sub, &ntime_sub_learn, data->field[cat].data[i].field_ls,
                               data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                               data->learning->data[s].time_s->year, data->learning->data[s].time_s->month,
                               data->learning->data[s].time_s->day, 3,
                               data->field[cat].nlon_ls, data->field[cat].nlat_ls, data->field[cat].ntime_ls,
                               data->learning->data[s].ntime);
      
      /* Compute seasonal mean and variance of spatially-averaged secondary field */
      (void) mean_variance_field_spatial(&(data->field[cat].data[i].down->mean[s]), &(data->field[cat].data[i].down->var[s]), buf_sub,
                                         data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime_sub_learn);
      (void) printf("Season: %d  TAS mean=%lf variance=%lf\n", s, data->field[cat].data[i].down->mean[s],
                    data->field[cat].data[i].down->var[s]);

      (void) free(buf_sub);
    }
  }

  /** MODEL RUN **/

  /** Step 6: Compute mean secondary large-scale fields for the model run **/
  if (data->conf->period_ctrl->downscale == TRUE)
    end_cat = CTRL_SEC_FIELD_LS;
  else
    end_cat = SEC_FIELD_LS;
  for (cat=SEC_FIELD_LS; cat <= end_cat; cat++)
    for (i=0; i<data->field[cat].n_ls; i++) {
      /* Compute spatial mean of secondary large-scale fields */
      data->field[cat].data[i].down->smean = (double *) malloc(data->field[cat].ntime_ls * sizeof(double));
      if (data->field[cat].data[i].down->smean == NULL) alloc_error(__FILE__, __LINE__);
      (void) mean_field_spatial(data->field[cat].data[i].down->smean, data->field[cat].data[i].field_ls,
                                data->field[cat].nlon_ls, data->field[cat].nlat_ls, data->field[cat].ntime_ls);
    }
  
  /** Step 7: Compute distance to clusters for the model run **/
  if (data->conf->period_ctrl->downscale == TRUE)
    end_cat = CTRL_FIELD_LS;
  else
    end_cat = FIELD_LS;
  for (cat=FIELD_LS; cat <= end_cat; cat++) {
    for (i=0; i<data->field[cat].n_ls; i++) {
      
      buftmp = (double *) malloc(data->field[cat].ntime_ls*data->field[cat].data[i].eof_info->neof_ls * sizeof(double));
      if (buftmp == NULL) alloc_error(__FILE__, __LINE__); 
      
      /* Normalisation of the principal component by the square root of the variance of the control run */
      var_pc_norm_all = (double *) malloc(data->field[cat].data[i].eof_info->neof_ls * sizeof(double));
      if (var_pc_norm_all == NULL) alloc_error(__FILE__, __LINE__);
      (void) normalize_pc(var_pc_norm_all, &(data->field[CTRL_FIELD_LS].data[i].first_variance), buftmp,
                          data->field[cat].data[i].field_eof_ls, data->field[cat].data[i].eof_info->neof_ls,
                          data->field[cat].ntime_ls);
      (void) free(var_pc_norm_all);
      
      for (s=0; s<data->conf->nseasons; s++) {
        /* Select season months in the whole time period */
        (void) extract_subperiod_months(&buf_sub, &(ntime_sub[cat][s]), buftmp,
                                        data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                                        data->conf->season[s].month,
                                        1, 1, data->field[cat].data[i].eof_info->neof_ls, data->field[cat].ntime_ls,
                                        data->conf->season[s].nmonths);
        /* Compute distances to clusters */
        data->field[cat].data[i].down->dist[s] = (double *) 
          malloc(data->conf->season[s].nclusters*ntime_sub[cat][s] * sizeof(double));
        if (data->field[cat].data[i].down->dist[s] == NULL) alloc_error(__FILE__, __LINE__);
        (void) dist_clusters_normctrl(data->field[cat].data[i].down->dist[s], buf_sub, data->learning->data[s].weight,
                                      data->learning->pc_normalized_var, data->field[cat].data[i].down->var_pc_norm,
                                      data->field[cat].data[i].down->mean_dist[s], data->field[cat].data[i].down->var_dist[s],
                                      data->field[cat].data[i].eof_info->neof_ls, data->conf->season[s].nclusters,
                                      ntime_sub[cat][s]);
        /* Classify each day in the current clusters */
        data->field[cat].data[i].down->days_class_clusters[s] = (int *) malloc(ntime_sub[cat][s] * sizeof(int));
        if (data->field[cat].data[i].down->days_class_clusters[s] == NULL) alloc_error(__FILE__, __LINE__);
        (void) class_days_pc_clusters(data->field[cat].data[i].down->days_class_clusters[s], buf_sub,
                                      data->learning->data[s].weight, data->conf->classif_type,
                                      data->field[cat].data[i].eof_info->neof_ls, data->conf->season[s].nclusters,
                                      ntime_sub[cat][s]);
        (void) free(buf_sub);
      }
      (void) free(buftmp);
    }
  }
  
  /** Step 8: Normalize the secondary large-scale fields by control-run mean and variance **/
  if (data->conf->period_ctrl->downscale == TRUE)
    end_cat = CTRL_SEC_FIELD_LS;
  else
    end_cat = SEC_FIELD_LS;
  for (cat=SEC_FIELD_LS; cat <= end_cat; cat++) {
    for (i=0; i<data->field[cat].n_ls; i++)
      for (s=0; s<data->conf->nseasons; s++) {
        /* Select season months in the whole time period */
        (void) extract_subperiod_months(&buf_sub, &(ntime_sub[cat][s]), data->field[cat].data[i].down->smean,
                                        data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                                        data->conf->season[s].month, 3, 1, 1, data->field[cat].ntime_ls, data->conf->season[s].nmonths);
        /* Normalize the spatial mean of secondary large-scale fields */
        data->field[cat].data[i].down->smean_norm[s] = (double *) malloc(data->field[cat].ntime_ls * sizeof(double));
        if (data->field[cat].data[i].down->smean_norm[s] == NULL) alloc_error(__FILE__, __LINE__);
        (void) normalize_field(data->field[cat].data[i].down->smean_norm[s], buf_sub,
                               data->field[cat].data[i].down->mean[s], data->field[cat].data[i].down->var[s],
                               1, 1, ntime_sub[cat][s]);
        (void) free(buf_sub);
      }
  }

  /** Step 9: Compute the precipitation using the pre-computed regressions for the model run for each season **/
  i = 0;
  if (data->conf->period_ctrl->downscale == TRUE)
    end_cat = CTRL_FIELD_LS;
  else
    end_cat = FIELD_LS;
  for (cat=FIELD_LS; cat <= end_cat; cat++) {
    if (data->field[cat].n_ls > 0)
      for (s=0; s<data->conf->nseasons; s++) {
        data->field[cat].precip_index[s] = (double *) malloc(data->reg->npts*ntime_sub[cat+2][s] * sizeof(double));
        if (data->field[cat].precip_index[s] == NULL) alloc_error(__FILE__, __LINE__);
        (void) apply_regression(data->field[cat].precip_index[s], data->learning->data[s].precip_reg,
                                data->learning->data[s].precip_reg_cst,
                                data->field[cat].data[i].down->dist[s], data->field[cat+2].data[i].down->smean_norm[s],
                                data->reg->npts, ntime_sub[cat+2][s], data->conf->season[s].nclusters, data->conf->season[s].nreg);
      }
  }
  
  /** Step 10: Find the days : resampling **/
  if (data->conf->period_ctrl->downscale == TRUE)
    end_cat = CTRL_FIELD_LS;
  else
    end_cat = FIELD_LS;
  for (cat=FIELD_LS; cat <= end_cat; cat++) {
    if (data->field[cat].n_ls > 0)
      for (s=0; s<data->conf->nseasons; s++) {
        
        data->field[cat].analog_days[s] = (int *) malloc(ntime_sub[cat][s] * sizeof(int));
        if (data->field[cat].analog_days[s] == NULL) alloc_error(__FILE__, __LINE__);
        (void) printf("%s: Searching analog days for season #%d\n", __FILE__, s);
        (void) find_the_days(data->field[cat].analog_days[s], data->field[cat].precip_index[s], data->learning->data[s].precip_index,
                             data->field[cat+2].data[i].down->smean_norm[s], data->learning->data[s].sup_index,
                             data->field[cat].data[i].down->days_class_clusters[s], data->learning->data[s].class_clusters,
                             data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                             data->learning->data[s].time_s->year, data->learning->data[s].time_s->month,
                             data->learning->data[s].time_s->day,
                             data->field[cat].ntime_ls, data->learning->data[s].ntime,
                             data->conf->season[s].month, data->conf->season[s].nmonths,
                             data->conf->season[s].ndays, data->conf->season[s].ndayschoices, data->reg->npts,
                             data->conf->season[s].shuffle, data->conf->season[s].secondary_choice,
                             data->conf->season[s].secondary_main_choice);
      }
  }
  
  /** Step 11: Compute the secondary large-scale fields difference if wanted */
  
        
  /** Step 12: Reconstruct data using chosen resampled days and write output */

  if (data->conf->period_ctrl->downscale == TRUE)
    end_cat = CTRL_SEC_FIELD_LS;
  else
    end_cat = SEC_FIELD_LS;

  for (cat=0; cat<NCAT; cat++)
    (void) free(ntime_sub[cat]);
  (void) free(ntime_sub);

  return 0;
}
