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

/** Downscaling climate scenarios program using weather typing. */
int wt_downscaling(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  double *buf_sub = NULL; /* Temporary buffer for sub-domain or sub-period */
  double *buftmp = NULL; /* Temporary buffer */
  double *buftmpf = NULL; /* Temporary buffer */
  double *var_pc_norm_all = NULL; /* Temporary values of the norm of the principal components */
  int **ntime_sub = NULL; /* Number of times for sub-periods. Dimensions number of field categories (NCAT) and number of seasons */
  int ntime_sub_learn; /* Number of times for learning common sub-period with control run for a specific season */
  int ntime_sub_learn_all; /* Number of times for learning common sub-period with control run for whole period */
  
  int istat; /* Function return diagnostic value */
  int i; /* Loop counter */
  int ii; /* Loop counter */
  int s; /* Loop counter for seasons */
  int cat; /* Loop counter for field categories */
  int end_cat; /* End category to process in loop */
  
  /* Allocate memory */
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
  /* Loop over large-scale field categories (Control run and Model run) */
  for (cat=0; cat<2; cat++)
    /* Loop over large-scale fields */
    for (i=0; i<data->field[cat].n_ls; i++) {
      /* Check if we need to project field on EOFs */
      if (data->field[cat].data[i].eof_info->eof_project == 1) {
        /* Allocate memory for projected large-scale field */
        data->field[cat].data[i].field_eof_ls = (double *) malloc(data->field[cat].ntime_ls * data->field[cat].data[i].eof_info->neof_ls *
                                                                            sizeof(double));
        if (data->field[cat].data[i].field_eof_ls == NULL) alloc_error(__FILE__, __LINE__);
        /* Project large-scale field on EOFs */
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
  /* Process control run only */
  cat = CTRL_FIELD_LS;
  /* Loop over large-scale fields */
  for (i=0; i<data->field[cat].n_ls; i++) {

    /* Allocate memory for temporary buffer */
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

    /* Allocate memory for temporary buffer */
    buftmpf = (double *) malloc(ntime_sub_learn_all*data->field[cat].data[i].eof_info->neof_ls * sizeof(double));
    if (buftmpf == NULL) alloc_error(__FILE__, __LINE__); 

    //    for (s=0; s<data->field[cat].ntime_ls; s++)
    //      printf("%d %lf\n",s,data->field[cat].data[i].field_eof_ls[s]);

    /* Compute the norm and the variance of the first EOF of the control run as references */
    printf("Compute the norm and the variance of the first EOF of the control run as references\n");
    /* Only when first_variance is -9999.9999, the variance of the first EOF will be computed */
    data->field[cat].data[i].first_variance = -9999.9999;
    (void) normalize_pc(data->field[cat].data[i].down->var_pc_norm, &(data->field[cat].data[i].first_variance),
                        buftmpf, buf_sub, data->field[cat].data[i].eof_info->neof_ls,
                        ntime_sub_learn_all);
    /* Free temporary buffers */
    (void) free(buf_sub);
    (void) free(buftmpf);

    /* Normalize the large-scale field given the reference norm and variance */
    printf("Normalize the large-scale field given the reference norm and variance.\n");
    /* Allocate memory for temporary buffer */
    var_pc_norm_all = (double *) malloc(data->field[cat].data[i].eof_info->neof_ls * sizeof(double));
    if (var_pc_norm_all == NULL) alloc_error(__FILE__, __LINE__);
    /* Normalize EOF-projected large-scale fields */
    (void) normalize_pc(var_pc_norm_all, &(data->field[cat].data[i].first_variance),
                        buftmp, data->field[cat].data[i].field_eof_ls, data->field[cat].data[i].eof_info->neof_ls,
                        data->field[cat].ntime_ls);
    /* Free temporary buffer */
    (void) free(var_pc_norm_all);

    /* Loop over each season */
    for (s=0; s<data->conf->nseasons; s++) {
      /* Compute mean and variance of principal components of selected large-scale fields */

      /* Allocate memory for season-specific mean and variance of distances to clusters */
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
      /* Diagnostic output */
      printf("Season: %d\n", s);
      for (ii=0; ii<data->conf->season[s].nclusters; ii++)
        (void) printf("%s: Cluster #%d. Mean and variance of distances to clusters: %lf %lf\n", __FILE__, ii,
                      data->field[cat].data[i].down->mean_dist[s][ii], data->field[cat].data[i].down->var_dist[s][ii]);

      /* Free temporary buffer */
      (void) free(buf_sub);
    }
    /* Free temporary buffer */
    (void) free(buftmp);
  }
      
  /** Step 5: Compute mean and variance of secondary large-scale fields for the control run **/

  /* Process only secondary field of control run. */
  cat = CTRL_SEC_FIELD_LS;
  /* Loop over secondary large-scale fields */
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

      /* Diagnostic output */
      (void) printf("Season: %d  TAS mean=%lf variance=%lf\n", s, data->field[cat].data[i].down->mean[s],
                    data->field[cat].data[i].down->var[s]);

      /* Free temporary buffer */
      (void) free(buf_sub);
    }
  }


  /** MODEL RUN **/

  /** Step 6: Compute mean secondary large-scale fields for the model run **/

  /* Downscale also control run if needed */
  if (data->conf->period_ctrl->downscale == TRUE)
    end_cat = CTRL_SEC_FIELD_LS;
  else
    end_cat = SEC_FIELD_LS;
  /* Loop over secondary field categories (model run and optionally control run) */
  for (cat=SEC_FIELD_LS; cat <= end_cat; cat++)
    /* Loop over secondary large-scale fields */
    for (i=0; i<data->field[cat].n_ls; i++) {
      /* Compute spatial mean of secondary large-scale fields */
      data->field[cat].data[i].down->smean = (double *) malloc(data->field[cat].ntime_ls * sizeof(double));
      if (data->field[cat].data[i].down->smean == NULL) alloc_error(__FILE__, __LINE__);
      (void) mean_field_spatial(data->field[cat].data[i].down->smean, data->field[cat].data[i].field_ls,
                                data->field[cat].nlon_ls, data->field[cat].nlat_ls, data->field[cat].ntime_ls);
    }
  
  /** Step 7: Compute distance to clusters for the model run **/

  /* Downscale also control run if needed */  
  if (data->conf->period_ctrl->downscale == TRUE)
    end_cat = CTRL_FIELD_LS;
  else
    end_cat = FIELD_LS;
  /* Loop over larg-scale field categories (model run and optionally control run) */
  for (cat=FIELD_LS; cat <= end_cat; cat++) {
    /* Loop over large-scale fields */
    for (i=0; i<data->field[cat].n_ls; i++) {
      
      /* Allocate memory for temporary buffer */
      buftmp = (double *) malloc(data->field[cat].ntime_ls*data->field[cat].data[i].eof_info->neof_ls * sizeof(double));
      if (buftmp == NULL) alloc_error(__FILE__, __LINE__); 
      
      /* Normalisation of the principal component by the square root of the variance of the control run */
      var_pc_norm_all = (double *) malloc(data->field[cat].data[i].eof_info->neof_ls * sizeof(double));
      if (var_pc_norm_all == NULL) alloc_error(__FILE__, __LINE__);
      (void) normalize_pc(var_pc_norm_all, &(data->field[CTRL_FIELD_LS].data[i].first_variance), buftmp,
                          data->field[cat].data[i].field_eof_ls, data->field[cat].data[i].eof_info->neof_ls,
                          data->field[cat].ntime_ls);
      (void) free(var_pc_norm_all);
      
      /* Loop over seasons */
      for (s=0; s<data->conf->nseasons; s++) {

        /* Select season months in the whole time period and create sub-period large-scale field buffer */
        (void) extract_subperiod_months(&buf_sub, &(ntime_sub[cat][s]), buftmp,
                                        data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                                        data->conf->season[s].month,
                                        1, 1, data->field[cat].data[i].eof_info->neof_ls, data->field[cat].ntime_ls,
                                        data->conf->season[s].nmonths);

        /* Compute distances to clusters using normalization and against the control reference run */
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
        /* Free temporary buffer */
        (void) free(buf_sub);
      }
      /* Free temporary buffer */
      (void) free(buftmp);
    }
  }
  
  /** Step 8: Normalize the secondary large-scale fields by control-run mean and variance **/

  /* Downscale also control run if needed */
  if (data->conf->period_ctrl->downscale == TRUE)
    end_cat = CTRL_SEC_FIELD_LS;
  else
    end_cat = SEC_FIELD_LS;

  /* Loop over secondary field categories (model run and optionally control run) */
  for (cat=SEC_FIELD_LS; cat <= end_cat; cat++) {
    /* Loop over secondary large-scale fields */
    for (i=0; i<data->field[cat].n_ls; i++)
      /* Loop over each season */
      for (s=0; s<data->conf->nseasons; s++) {
        /* Select season months in the whole time period to create a sub-period buffer */
        (void) extract_subperiod_months(&buf_sub, &(ntime_sub[cat][s]), data->field[cat].data[i].down->smean,
                                        data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                                        data->conf->season[s].month, 3, 1, 1, data->field[cat].ntime_ls, data->conf->season[s].nmonths);
        /* Normalize the spatial mean of secondary large-scale fields */
        data->field[cat].data[i].down->smean_norm[s] = (double *) malloc(data->field[cat].ntime_ls * sizeof(double));
        if (data->field[cat].data[i].down->smean_norm[s] == NULL) alloc_error(__FILE__, __LINE__);
        (void) normalize_field(data->field[cat].data[i].down->smean_norm[s], buf_sub,
                               data->field[cat].data[i].down->mean[s], data->field[cat].data[i].down->var[s],
                               1, 1, ntime_sub[cat][s]);
        /* Free temporary buffer */
        (void) free(buf_sub);
      }
  }

  /** Step 9: Compute the precipitation using the pre-computed regressions for the model run for each season **/

  /* Select the first large-scale field which must contain the cluster distances */
  /* and the first secondary large-scale fields which must contains its spatial mean */
  i = 0;

  /* Downscale also control run if needed */
  if (data->conf->period_ctrl->downscale == TRUE)
    end_cat = CTRL_FIELD_LS;
  else
    end_cat = FIELD_LS;

  /* Loop over larg-scale field categories (model run and optionally control run) */
  for (cat=FIELD_LS; cat <= end_cat; cat++) {
    /* Process only if, for this category, at least one large-scale field is available */
    if (data->field[cat].n_ls > 0)
      /* Loop over each season */
      for (s=0; s<data->conf->nseasons; s++) {
        /* Apply the regression coefficients to calculate precipitation using the cluster distances */
        /* and the normalized spatial mean of the corresponding secondary large-scale field */
        data->field[cat].precip_index[s] = (double *) malloc(data->reg->npts*ntime_sub[cat+2][s] * sizeof(double));
        if (data->field[cat].precip_index[s] == NULL) alloc_error(__FILE__, __LINE__);
        (void) apply_regression(data->field[cat].precip_index[s], data->learning->data[s].precip_reg,
                                data->learning->data[s].precip_reg_cst,
                                data->field[cat].data[i].down->dist[s], data->field[cat+2].data[i].down->smean_norm[s],
                                data->reg->npts, ntime_sub[cat+2][s], data->conf->season[s].nclusters, data->conf->season[s].nreg);
      }
  }
  
  /** Step 10: Find the days : resampling **/

  /* Select the first large-scale field which must contain the cluster distances */
  /* and the first secondary large-scale fields which must contains its spatial mean */
  i = 0;

  /* Downscale also control run if needed */  
  if (data->conf->period_ctrl->downscale == TRUE)
    end_cat = CTRL_FIELD_LS;
  else
    end_cat = FIELD_LS;

  /* Loop over large-scale field categories (model run and optionally control run) */
  for (cat=FIELD_LS; cat <= end_cat; cat++) {
    /* Process only if, for this category, at least one large-scale field is available */
    if (data->field[cat].n_ls > 0)
      /* Loop over each season */
      for (s=0; s<data->conf->nseasons; s++) {
        
        /* Find the analog days in the learning period given the precipitation index, */
        /* the spatial mean of the secondary large-scale fields and its index, and the cluster classification of the days */
        data->field[cat].analog_days[s].tindex = (int *) malloc(ntime_sub[cat][s] * sizeof(int));
        if (data->field[cat].analog_days[s].tindex == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days[s].tindex_all = (int *) malloc(ntime_sub[cat][s] * sizeof(int));
        if (data->field[cat].analog_days[s].tindex_all == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days[s].year = (int *) malloc(ntime_sub[cat][s] * sizeof(int));
        if (data->field[cat].analog_days[s].year == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days[s].month = (int *) malloc(ntime_sub[cat][s] * sizeof(int));
        if (data->field[cat].analog_days[s].month == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days[s].day = (int *) malloc(ntime_sub[cat][s] * sizeof(int));
        if (data->field[cat].analog_days[s].day == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days[s].tindex_s_all = (int *) malloc(ntime_sub[cat][s] * sizeof(int));
        if (data->field[cat].analog_days[s].tindex_s_all == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days[s].year_s = (int *) malloc(ntime_sub[cat][s] * sizeof(int));
        if (data->field[cat].analog_days[s].year_s == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days[s].month_s = (int *) malloc(ntime_sub[cat][s] * sizeof(int));
        if (data->field[cat].analog_days[s].month_s == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days[s].day_s = (int *) malloc(ntime_sub[cat][s] * sizeof(int));
        if (data->field[cat].analog_days[s].day_s == NULL) alloc_error(__FILE__, __LINE__);
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

  /* Downscale also control run if needed */
  if (data->conf->period_ctrl->downscale == TRUE)
    end_cat = CTRL_SEC_FIELD_LS;
  else
    end_cat = SEC_FIELD_LS;

  /* Loop over secondary field categories (model run and optionally control run) */
  for (cat=SEC_FIELD_LS; cat <= end_cat; cat++) {
    /* Process only if, for this category, at least one secondary large-scale field is available */
    if (data->field[cat].n_ls > 0)
      /* Loop over each season */
      for (s=0; s<data->conf->nseasons; s++)
        (void) compute_secondary_large_scale_diff(data->field[cat].data[i].down->delta[s], data->field[cat].analog_days[s],
                                                  data->field[cat].data[i].down->smean_norm[s], data->learning->data[s].sup_index,
                                                  data->field[cat].data[i].down->var[s], data->learning->data[s].sup_index_var,
                                                  ntime_sub[cat][s]);
    /** Optionally save analog_days information in an output file **/
    /** TODO **/
  }
        
  /** Step 12: Reconstruct data using chosen resampled days and write output */
  
  /* Downscale also control run if needed */  
  if (data->conf->period_ctrl->downscale == TRUE)
    end_cat = CTRL_FIELD_LS;
  else
    end_cat = FIELD_LS;
  
  /* Loop over large-scale field categories (model run and optionally control run) */
  for (cat=FIELD_LS; cat <= end_cat; cat++) {
    /* Process only if, for this category, at least one large-scale field is available */
    if (data->field[cat].n_ls > 0) {
      /* Loop over each season */
      for (s=0; s<data->conf->nseasons; s++) {
        /* Merge all seasons of analog_day data */
        istat = merge_seasons(data->field[cat].analog_days_year, data->field[cat].analog_days[s],
                              data->field[cat].ntime_ls, ntime_sub[cat][s]);
        if (istat != 0) return istat;
      }
      /* Process all data */
      //      istat = output_downscaled_analog(data->field[cat].analog_days_year, data->field[cat].ntime_ls);
    }
  }
          
  /* Free memory for specific downscaling buffers */
  for (cat=0; cat<NCAT; cat++)
    (void) free(ntime_sub[cat]);
  (void) free(ntime_sub);

  /* Success return */
  return 0;
}
