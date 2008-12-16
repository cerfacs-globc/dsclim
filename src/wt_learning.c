/* ***************************************************** */
/* wt_learning Compute learning data needed for          */
/* downscaling climate scenarios using weather typing.   */
/* wt_learning.c                                         */
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
/*! \file wt_learning.c
    \brief Compute or read learning data needed for downscaling climate scenarios using weather typing.
*/

#include <dsclim.h>

/** Compute or read learning data needed for downscaling climate scenarios using weather typing. */
int wt_learning(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  double *buf_learn = NULL;
  double *buf_weight = NULL;
  double *buf_learn_obs = NULL;
  double *buf_learn_rea = NULL;
  double *buf_learn_obs_sub = NULL;
  double *buf_learn_rea_sub = NULL;

  double obs_first_sing;
  double rea_sing;
  double *rea_var_sqrt = NULL;
  double fact_norm;
  //  double rea_first_sing;

  double *mean_dist = NULL;
  double *var_dist = NULL;
  double *dist = NULL;
  int *days_class_clusters = NULL;

  int ntime_learn_all;
  int *ntime_sub = NULL;

  int eof;
  int clust;
  int nt;
  int s;

  int istat; /** Return status. */

  if (data->learning->learning_provided == 1) {
    /** Read learning data **/
    istat = read_learning_fields(data);
    if (istat != 0) return istat;
  }
  else  {
    /** Compute learning data **/
    /** Assume EOFs are already pre-computed **/

    /* Read re-analysis pre-computed EOF and Singular Values */
    istat = read_learning_rea_eof(data);
    if (istat != 0) return istat;

    /* Read observations pre-computed EOF and Singular Values */
    istat = read_learning_obs_eof(data);
    if (istat != 0) return istat;

    /* Select common time period between the re-analysis and the observation data periods */
    (void) sub_period_common(&buf_learn_obs, &ntime_learn_all, data->learning->obs->eof,
                             data->learning->obs->time_s->year, data->learning->obs->time_s->month, data->learning->obs->time_s->day,
                             data->learning->rea->time_s->year, data->learning->rea->time_s->month, data->learning->rea->time_s->day,
                             1, data->learning->neof, 1, data->learning->obs->ntime, data->learning->rea->ntime);
    (void) sub_period_common(&buf_learn_rea, &ntime_learn_all, data->learning->rea->eof,
                             data->learning->rea->time_s->year, data->learning->rea->time_s->month, data->learning->rea->time_s->day,
                             data->learning->obs->time_s->year, data->learning->obs->time_s->month, data->learning->obs->time_s->day,
                             1, data->learning->neof, 1, data->learning->rea->ntime, data->learning->obs->ntime);

    ntime_sub = (int *) malloc(data->conf->nseasons * sizeof(int));
    if (ntime_sub == NULL) alloc_error(__FILE__, __LINE__);

    rea_var_sqrt = (double *) malloc(data->learning->neof * sizeof(double));
    if (rea_var_sqrt == NULL) alloc_error(__FILE__, __LINE__);
    
    /* Read observed precipitation (liquid and solid) */
    
    /* Remove Corsica if needed */
    
    /* Select common time period between the re-analysis and the observation data periods for precipitation and */
    /* secondary large-scale fields */

    /* Normalize precipitation */

    /* Perform spatial mean of observed precipitation around regression points */

    /* Mean secondary large-scale fields spatially */

    /* Loop over each season */
    for (s=0; s<data->conf->nseasons; s++) {
      /* Process separately each season */
      
      /* Select season months in the whole time period and create sub-period fields */
      (void) extract_subperiod_months(&buf_learn_obs_sub, &(ntime_sub[s]), buf_learn_obs,
                                      data->learning->time_s->year, data->learning->time_s->month, data->learning->time_s->day,
                                      data->conf->season[s].month,
                                      1, 1, data->learning->neof, ntime_learn_all,
                                      data->conf->season[s].nmonths);
      (void) extract_subperiod_months(&buf_learn_rea_sub, &(ntime_sub[s]), buf_learn_rea,
                                      data->learning->time_s->year, data->learning->time_s->month, data->learning->time_s->day,
                                      data->conf->season[s].month,
                                      1, 1, data->learning->neof, ntime_learn_all,
                                      data->conf->season[s].nmonths);

      /** Merge observation and reanalysis principal components for clustering algorithm and normalize using first Singular Value **/

      buf_learn = (double *) realloc(buf_learn, ntime_sub[s] * data->learning->neof * 2 * sizeof(double));
      if (buf_learn == NULL) alloc_error(__FILE__, __LINE__);

      /* Verifier avec Julien Boe la normalisation: soit par la variance ou soit par la premiere valeur singuliere */
      //      rea_first_sing = data->learning->rea->sing[0];
      for (eof=0; eof<data->learning->neof; eof++) {
        rea_var_sqrt[eof] = sqrt(gsl_stats_variance(buf_learn_rea_sub, 1, ntime_sub[s]));
        rea_sing = data->learning->rea->sing[eof];        
        fact_norm = rea_sing / rea_var_sqrt[0];
        for (nt=0; nt<ntime_sub[s]; nt++) {
          //          buf_learn[nt+eof*ntime_sub[s]] = buf_learn_rea_sub[nt+eof*ntime_sub[s]] * rea_sing / rea_first_sing;
          buf_learn_rea_sub[nt+eof*ntime_sub[s]] = buf_learn_rea_sub[nt+eof*ntime_sub[s]] * fact_norm;
          buf_learn[nt+eof*ntime_sub[s]] = buf_learn_rea_sub[nt+eof*ntime_sub[s]];
        }
      }      
      obs_first_sing = data->learning->obs->sing[0];
      for (eof=0; eof<data->learning->neof; eof++) {
        for (nt=0; nt<ntime_sub[s]; nt++) {
          buf_learn_obs_sub[nt+eof*ntime_sub[s]] = buf_learn_obs_sub[nt+eof*ntime_sub[s]] / obs_first_sing;
          buf_learn[nt+(eof+data->learning->neof)*ntime_sub[s]] = buf_learn_obs_sub[nt+eof*ntime_sub[s]];
        }
      }

      /* Compute best clusters */
      buf_weight = (double *) realloc(buf_weight, data->conf->season[s].nclusters * data->learning->neof * 2 * sizeof(double));
      if (buf_weight == NULL) alloc_error(__FILE__, __LINE__);
      (void) best_clusters(buf_weight, buf_learn, data->conf->classif_type, data->conf->npartitions,
                           data->conf->nclassifications, data->learning->neof*2, data->conf->season[s].nclusters, ntime_sub[s]);

      /* Keep only first data->learning->neof EOFs */
      data->learning->data[s].weight = (double *) 
        malloc(data->conf->season[s].nclusters*data->learning->neof * sizeof(double));
      if (data->learning->data[s].weight == NULL) alloc_error(__FILE__, __LINE__);
      for (clust=0; clust<data->conf->season[s].nclusters; clust++)
        for (eof=0; eof<data->learning->neof; eof++)
          data->learning->data[s].weight[eof+clust*data->learning->neof] = buf_weight[eof+clust*(data->learning->neof*2)];
      
      /* Classify each day in the current clusters */
      data->learning->data[s].class_clusters = (int *) malloc(ntime_sub[s] * sizeof(int));
      if (data->learning->data[s].class_clusters == NULL) alloc_error(__FILE__, __LINE__);
      (void) class_days_pc_clusters(data->learning->data[s].class_clusters, buf_learn,
                                    data->learning->data[s].weight, data->conf->classif_type,
                                    data->learning->neof, data->conf->season[s].nclusters,
                                    ntime_sub[s]);

      /* Compute mean and variance of distances to clusters */
      mean_dist = (double *) realloc(mean_dist, data->conf->season[s].nclusters * sizeof(double));
      if (mean_dist == NULL) alloc_error(__FILE__, __LINE__);
      var_dist = (double *) realloc(var_dist, data->conf->season[s].nclusters * sizeof(double));
      if (var_dist == NULL) alloc_error(__FILE__, __LINE__);
      (void) mean_variance_dist_clusters(mean_dist, var_dist, buf_learn_rea_sub, data->learning->data[s].weight,
                                         rea_var_sqrt, rea_var_sqrt,
                                         data->learning->neof, data->conf->season[s].nclusters, ntime_sub[s]);

      /* Compute distances to clusters using normalization */
      dist = (double *) realloc(dist, data->conf->season[s].nclusters*ntime_sub[s] * sizeof(double));
      if (dist == NULL) alloc_error(__FILE__, __LINE__);
      (void) dist_clusters_normctrl(dist, buf_learn_rea_sub, data->learning->data[s].weight,
                                    rea_var_sqrt, rea_var_sqrt, mean_dist, var_dist,
                                    data->learning->neof, data->conf->season[s].nclusters, ntime_sub[s]);

      /* Classify each day in the current clusters, preparing for regression calculation */
      days_class_clusters = (int *) realloc(days_class_clusters, ntime_sub[s] * sizeof(int));
      if (days_class_clusters == NULL) alloc_error(__FILE__, __LINE__);
      (void) class_days_pc_clusters(days_class_clusters, buf_learn_rea_sub,
                                    data->learning->data[s].weight, data->conf->classif_type,
                                    data->learning->neof, data->conf->season[s].nclusters, ntime_sub[s]);

      /* Calculate regression between cluster distances and observed precipitation on regression points */


      /* Normalize secondary large-scale fields for re-analysis learning data */


      /* Compute mean and variance of secondary large-scale fields for re-analysis learning data */


      (void) free(buf_learn_rea_sub);
      buf_learn_rea_sub = NULL;
      (void) free(buf_learn_obs_sub);
      buf_learn_obs_sub = NULL;
    }
    (void) free(buf_weight);
    (void) free(buf_learn);
    (void) free(buf_learn_rea);
    (void) free(buf_learn_obs);
    (void) free(ntime_sub);
    (void) free(rea_var_sqrt);
    (void) free(mean_dist);
    (void) free(var_dist);
    (void) free(dist);
    (void) free(days_class_clusters);
  }

  /* Success status */
  return 0;
}
