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

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2013)

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

/** Downscaling climate scenarios program using weather typing. */
int
wt_downscaling(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  double *buf_sub = NULL; /* Temporary buffer for sub-domain or sub-period */
  double *buftmp = NULL; /* Temporary buffer */
  double *buftmpf = NULL; /* Temporary buffer */
  double *mask_subd = NULL; /* Mask covering subdomain in double format when reading */
  short int *mask_sub = NULL; /* Mask covering subdomain in  short int */
  double *lon_mask = NULL; /* Longitudes of mask */
  double *lat_mask = NULL; /* Latitudes of mask */
  double *var_pc_norm_all = NULL; /* Temporary values of the norm of the principal components */
  int **ntime_sub = NULL; /* Number of times for sub-periods. Dimensions number of field categories (NCAT) and number of seasons */
  double **time_ls_sub = NULL; /* Time values used for regression diagnostics output */
  int *merged_itimes = NULL; /* Time values in common merged time vector */
  int ntimes_merged; /* Number of times in one particular season */
  int curindex_merged; /* Current index in merged season vector */
  short int *merged_times_flag = NULL; /* Flag variable for days in the year that are processed */
  double *merged_times = NULL; /* Merge times in udunit */
  int ntime_sub_tmp; /* Number of times for regression diagnostics output */
  int ntime_sub_learn; /* Number of times for learning common sub-period with control run for a specific season */
  int ntime_sub_learn_all; /* Number of times for learning common sub-period with control run for whole period */
  int nlon_mask; /* Longitude dimension for mask subdomain */
  int nlat_mask; /* Latitude dimension for mask subdomain */
  
  int istat; /* Function return diagnostic value */
  int i; /* Loop counter */
  int ii; /* Loop counter */
  int s; /* Loop counter for seasons */
  int cat; /* Loop counter for field categories */
  int beg_cat; /* Beginning category to process in loop */

  char *analog_file = NULL; /* Analog data filename */
  period_struct *period = NULL; /* Period structure for output */

  char *filename = NULL; /* Temporary filename for regression optional output */
  
  if (data->conf->output_only != TRUE) {
  
    /* Allocate memory */
    ntime_sub = (int **) malloc(NCAT * sizeof(int *));
    if (ntime_sub == NULL) alloc_error(__FILE__, __LINE__);

    if (data->reg->reg_save == TRUE) {
      time_ls_sub = (double **) malloc(data->conf->nseasons * sizeof(double *));
      if (time_ls_sub == NULL) alloc_error(__FILE__, __LINE__);
    }
    
    for (cat=0; cat<NCAT; cat++) {
      ntime_sub[cat] = (int *) malloc(data->conf->nseasons * sizeof(int));
      if (ntime_sub[cat] == NULL) alloc_error(__FILE__, __LINE__);
    }

    /** Step 1: Read large-scale fields **/
    istat = read_large_scale_fields(data);
    if (istat != 0) return istat;
    
    /* Prepare optional mask for secondary large-scale fields */
    if (data->secondary_mask->use_mask == TRUE) {
      (void) extract_subdomain(&mask_subd, &lon_mask, &lat_mask, &nlon_mask, &nlat_mask, data->secondary_mask->field,
                               data->secondary_mask->lon, data->secondary_mask->lat,
                               data->conf->secondary_longitude_min, data->conf->secondary_longitude_max,
                               data->conf->secondary_latitude_min, data->conf->secondary_latitude_max, 
                               data->secondary_mask->nlon, data->secondary_mask->nlat, 1);
    
      if (data->conf->period_ctrl->downscale == TRUE)
        beg_cat = CTRL_SEC_FIELD_LS;
      else
        beg_cat = SEC_FIELD_LS;
      /* Loop over secondary field categories (model run and optionally control run) */
      for (cat=beg_cat; cat>=SEC_FIELD_LS; cat--) {
        /* Loop over secondary large-scale fields */
        for (i=0; i<data->field[cat].n_ls; i++)
          if (data->field[cat].nlon_ls != nlon_mask || data->field[cat].nlat_ls != nlat_mask) {
            (void) fprintf(stderr, "%s: The mask for secondary large-scale fields after selecting subdomain has invalid dimensions: nlon=%d nlat=%d. Expected: nlon=%d nlat=%d\nReverting to no-mask processing.", __FILE__,
                           nlon_mask, nlat_mask, data->field[cat].nlon_ls, data->field[cat].nlat_ls);
            mask_sub = (short int *) NULL;
            data->secondary_mask->use_mask = FALSE;
          }
      }
      /* Dimensions are ok and we are using a mask. Get values into short int buffer. */
      if (data->secondary_mask->use_mask == TRUE) {
        mask_sub = (short int *) malloc(data->field[SEC_FIELD_LS].nlon_ls*data->field[SEC_FIELD_LS].nlat_ls * sizeof(short int));
        if (mask_sub == NULL) alloc_error(__FILE__, __LINE__);
        for (i=0; i<data->field[SEC_FIELD_LS].nlon_ls*data->field[SEC_FIELD_LS].nlat_ls; i++)
          mask_sub[i] = (short int) mask_subd[i];
      }
      (void) free(mask_subd);
      (void) free(lon_mask);
      (void) free(lat_mask);
    }
    else
      mask_sub = (short int *) NULL;

    if (mask_sub != NULL)
      printf("%s: Using a mask for secondary large-scale fields.\n", __FILE__);

    /** Step 2: Compute climatologies and remove them from selected large scale fields **/
    istat = remove_clim(data);
    if (istat != 0) return istat;  

    /** Step 3: Project selected large scale fields on EOF **/  

    /* Read EOFs and Singular Values */
    istat = read_large_scale_eof(data);
    if (istat != 0) return istat;
  
    /* Project selected large scale fields on EOF */
    /* Loop over large-scale field categories (Control run and Model run) */
    for (cat=CTRL_FIELD_LS; cat>=FIELD_LS; cat--)
      /* Loop over large-scale fields */
      for (i=0; i<data->field[cat].n_ls; i++) {
        /* Check if we need to project field on EOFs */
        if (data->field[cat].data[i].eof_info->eof_project == TRUE) {
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
      istat = sub_period_common(&buf_sub, &ntime_sub_learn_all, data->field[cat].data[i].field_eof_ls,
                                data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                                data->learning->time_s->year, data->learning->time_s->month,
                                data->learning->time_s->day, 1,
                                data->field[cat].data[i].eof_info->neof_ls, 1, data->field[cat].ntime_ls, data->learning->ntime);
      if (istat != 0) return istat;

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
      //    for (ii=0; ii<9; ii++) printf("%d %lf\n",ii,sqrt(data->field[cat].data[i].down->var_pc_norm[ii]));
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
        istat = sub_period_common(&buf_sub, &ntime_sub_learn, buftmp,
                                  data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                                  data->learning->data[s].time_s->year, data->learning->data[s].time_s->month,
                                  data->learning->data[s].time_s->day, 1,
                                  data->field[cat].data[i].eof_info->neof_ls, 1, data->field[cat].ntime_ls, data->learning->data[s].ntime);
        if (istat != 0) return istat;
      
        /* Compute mean and variance of distances to clusters */
        (void) mean_variance_dist_clusters(data->field[cat].data[i].down->mean_dist[s], data->field[cat].data[i].down->var_dist[s],
                                           buf_sub, data->learning->data[s].weight,
                                           data->learning->pc_normalized_var, data->field[cat].data[i].down->var_pc_norm,
                                           data->field[cat].data[i].eof_info->neof_ls, data->conf->season[s].nclusters, ntime_sub_learn);
        /* Diagnostic output */
        printf("Season: %d\n", s);
        for (ii=0; ii<data->conf->season[s].nclusters; ii++)
          (void) printf("%s: Cluster #%d. Mean and variance of distances to clusters for control run: %lf %lf\n", __FILE__, ii,
                        data->field[cat].data[i].down->mean_dist[s][ii], sqrt(data->field[cat].data[i].down->var_dist[s][ii]));

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

      (void) mean_field_spatial(data->field[cat].data[i].down->smean, data->field[cat].data[i].field_ls, mask_sub,
                                data->field[cat].nlon_ls, data->field[cat].nlat_ls, data->field[cat].ntime_ls);

      for (s=0; s<data->conf->nseasons; s++) {
      
        /* Compute seasonal mean and variance of principal components of selected large-scale fields */
      
        /* Select common time period between the learning period and the model period (control run) */
        istat = sub_period_common(&buf_sub, &ntime_sub_learn, data->field[cat].data[i].field_ls,
                                  data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                                  data->learning->data[s].time_s->year, data->learning->data[s].time_s->month,
                                  data->learning->data[s].time_s->day, 3,
                                  data->field[cat].nlon_ls, data->field[cat].nlat_ls, data->field[cat].ntime_ls,
                                  data->learning->data[s].ntime);
        if (istat != 0) return istat;
      
        /* Compute seasonal mean and variance of spatially-averaged secondary field */
        (void) mean_variance_field_spatial(&(data->field[cat].data[i].down->mean[s]), &(data->field[cat].data[i].down->var[s]), buf_sub,
                                           mask_sub, data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime_sub_learn);
        
        /* Compute mean and variance over time for each point of secondary field */
        data->field[cat].data[i].down->smean_2d[s] = (double *)
          malloc(data->field[cat].nlon_ls*data->field[cat].nlat_ls*ntime_sub_learn * sizeof(double));
        if (data->field[cat].data[i].down->smean_2d[s] == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].data[i].down->svar_2d[s] = (double *)
          malloc(data->field[cat].nlon_ls*data->field[cat].nlat_ls*ntime_sub_learn * sizeof(double));
        if (data->field[cat].data[i].down->svar_2d[s] == NULL) alloc_error(__FILE__, __LINE__);
        (void) time_mean_variance_field_2d(data->field[cat].data[i].down->smean_2d[s], data->field[cat].data[i].down->svar_2d[s],
                                           buf_sub, data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime_sub_learn);

        /* Diagnostic output */
        (void) printf("Control run:: Season: %d  TAS mean=%lf variance=%lf cat=%d field=%d\n", s, data->field[cat].data[i].down->mean[s],
                      sqrt(data->field[cat].data[i].down->var[s]), cat, i);

        /* Free temporary buffer */
        (void) free(buf_sub);
      }
    }


    /** MODEL RUN **/

    /** Step 6: Compute mean secondary large-scale fields for the model run **/

    cat = SEC_FIELD_LS;
    /* Loop over secondary large-scale fields */
    for (i=0; i<data->field[cat].n_ls; i++) {
      /* Compute spatial mean of secondary large-scale fields */
      data->field[cat].data[i].down->smean = (double *) malloc(data->field[cat].ntime_ls * sizeof(double));
      if (data->field[cat].data[i].down->smean == NULL) alloc_error(__FILE__, __LINE__);
      (void) mean_field_spatial(data->field[cat].data[i].down->smean, data->field[cat].data[i].field_ls, mask_sub,
                                data->field[cat].nlon_ls, data->field[cat].nlat_ls, data->field[cat].ntime_ls);
    }
    
    /** Step 7: Compute distance to clusters (model run and optionally control run) **/

    /* Downscale also control run if needed */  
    if (data->conf->period_ctrl->downscale == TRUE)
      beg_cat = CTRL_FIELD_LS;
    else
      beg_cat = FIELD_LS;
    /* Loop over larg-scale field categories (model run and optionally control run) */
    for (cat=beg_cat; cat>=FIELD_LS; cat--) {
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
                                        data->learning->pc_normalized_var, data->field[CTRL_FIELD_LS].data[i].down->var_pc_norm,
                                        data->field[CTRL_FIELD_LS].data[i].down->mean_dist[s],
                                        data->field[CTRL_FIELD_LS].data[i].down->var_dist[s],
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
      beg_cat = CTRL_SEC_FIELD_LS;
    else
      beg_cat = SEC_FIELD_LS;

    /* Loop over secondary field categories (model run and optionally control run) */
    for (cat=beg_cat; cat>=SEC_FIELD_LS; cat--) {
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
                                 data->field[CTRL_SEC_FIELD_LS].data[i].down->mean[s], data->field[CTRL_SEC_FIELD_LS].data[i].down->var[s],
                                 1, 1, ntime_sub[cat][s]);
          /* Free temporary buffer */
          (void) free(buf_sub);

          /* Select season months in the whole time period to create a 2D sub-period buffer */
          (void) extract_subperiod_months(&buf_sub, &(ntime_sub[cat][s]), data->field[cat].data[i].field_ls,
                                          data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                                          data->conf->season[s].month, 3, data->field[cat].nlon_ls, data->field[cat].nlat_ls,
                                          data->field[cat].ntime_ls, data->conf->season[s].nmonths);
          /* Normalize the secondary large-scale fields */
          data->field[cat].data[i].down->sup_val_norm[s] =
            (double *) malloc(data->field[cat].nlon_ls*data->field[cat].nlat_ls*data->field[cat].ntime_ls * sizeof(double));
          if (data->field[cat].data[i].down->sup_val_norm[s] == NULL) alloc_error(__FILE__, __LINE__);
          (void) normalize_field_2d(data->field[cat].data[i].down->sup_val_norm[s], buf_sub,
                                    data->field[CTRL_SEC_FIELD_LS].data[i].down->smean_2d[s],
                                    data->field[CTRL_SEC_FIELD_LS].data[i].down->svar_2d[s],
                                    data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime_sub[cat][s]);
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
      beg_cat = CTRL_FIELD_LS;
    else
      beg_cat = FIELD_LS;

    /* Loop over large-scale field categories (model run and optionally control run) */
    for (cat=beg_cat; cat>=FIELD_LS; cat--) {
      /* Process only if, for this category, at least one large-scale field is available */
      for (i=0; i<data->field[cat].n_ls; i++) {
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
          if (data->reg->reg_save == TRUE)
            /* Select season months in the whole time period and create sub-period time vector */
            (void) extract_subperiod_months(&(time_ls_sub[s]), &ntime_sub_tmp, data->field[cat].time_ls,
                                            data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                                            data->conf->season[s].month,
                                            1, 1, 1, data->field[cat].ntime_ls,
                                            data->conf->season[s].nmonths);
        }
        if (data->reg->reg_save == TRUE) {
          (void) printf("Writing downscaling regression diagnostic fields.\n");
          if (cat == CTRL_FIELD_LS)
            filename = data->reg->filename_save_ctrl_reg;
          else
            filename = data->reg->filename_save_other_reg;
          (void) write_regression_fields(data, filename, time_ls_sub, ntime_sub[cat+2],
                                         data->field[cat].precip_index,
                                         data->field[cat].data[i].down->dist,
                                         data->field[cat+2].data[i].down->smean_norm);
        }
      }
    }
    if (data->reg->reg_save == TRUE) {
      for (s=0; s<data->conf->nseasons; s++)
        (void) free(time_ls_sub[s]);
      (void) free(time_ls_sub);
    }
  
    /** Step 10: Find the days : resampling **/

    /* Select the first large-scale field which must contain the cluster distances */
    /* and the first secondary large-scale fields which must contains its spatial mean */
    i = 0;

    /* Downscale also control run if needed */  
    if (data->conf->period_ctrl->downscale == TRUE)
      beg_cat = CTRL_FIELD_LS;
    else
      beg_cat = FIELD_LS;

    /* Loop over large-scale field categories (model run and optionally control run) */
    for (cat=beg_cat; cat>=FIELD_LS; cat--) {
      /* Process only if, for this category, at least one large-scale field is available */
      if (data->field[cat].n_ls > 0)
        /* Loop over each season */
        for (s=0; s<data->conf->nseasons; s++) {
        
          /* Find the analog days in the learning period given the precipitation index, */
          /* the spatial mean of the secondary large-scale fields and its index, and the cluster classification of the days */
          data->field[cat].analog_days[s].ntime = ntime_sub[cat][s];
          data->field[cat].analog_days[s].time = (int *) malloc(ntime_sub[cat][s] * sizeof(int));
          if (data->field[cat].analog_days[s].time == NULL) alloc_error(__FILE__, __LINE__);
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
          data->field[cat].analog_days[s].ndayschoice = (int *) malloc(ntime_sub[cat][s] * sizeof(int));
          if (data->field[cat].analog_days[s].ndayschoice == NULL) alloc_error(__FILE__, __LINE__);
          data->field[cat].analog_days[s].analog_dayschoice = (tstruct **) malloc(ntime_sub[cat][s] * sizeof(tstruct *));
          if (data->field[cat].analog_days[s].analog_dayschoice == NULL) alloc_error(__FILE__, __LINE__);
          data->field[cat].analog_days[s].metric_norm = (float **) malloc(ntime_sub[cat][s] * sizeof(float *));
          if (data->field[cat].analog_days[s].metric_norm == NULL) alloc_error(__FILE__, __LINE__);
          for (ii=0; ii<ntime_sub[cat][s]; ii++) {
            data->field[cat].analog_days[s].ndayschoice[ii] = data->conf->season[s].ndayschoices;
            data->field[cat].analog_days[s].analog_dayschoice[ii] = (tstruct *) NULL;
            data->field[cat].analog_days[s].metric_norm[ii] = (float *) NULL;            
          }
          (void) printf("%s: Searching analog days for season #%d\n", __FILE__, s);
          istat = find_the_days(data->field[cat].analog_days[s], data->field[cat].precip_index[s], data->learning->data[s].precip_index,
                                data->field[cat+2].data[i].down->smean_norm[s], data->learning->data[s].sup_index,
                                data->field[cat+2].data[i].down->sup_val_norm[s], data->learning->data[s].sup_val, mask_sub,
                                data->field[cat].data[i].down->days_class_clusters[s], data->learning->data[s].class_clusters,
                                data->field[cat].time_s->year, data->field[cat].time_s->month, data->field[cat].time_s->day,
                                data->learning->data[s].time_s->year, data->learning->data[s].time_s->month,
                                data->learning->data[s].time_s->day, data->conf->time_units,
                                data->field[cat].ntime_ls, data->learning->data[s].ntime,
                                data->conf->season[s].month, data->conf->season[s].nmonths,
                                data->conf->season[s].ndays, data->conf->season[s].ndayschoices, data->reg->npts,
                                data->conf->season[s].shuffle, data->conf->season[s].secondary_choice,
                                data->conf->season[s].secondary_main_choice, data->conf->season[s].secondary_cov,
                                data->conf->use_downscaled_year, data->conf->only_wt,
                                data->field[cat+2].nlon_ls, data->field[cat+2].nlat_ls,
                                data->learning->sup_nlon, data->learning->sup_nlat);
          if (istat != 0) return istat;
        }
    }

    /** Step 11: Compute the secondary large-scale fields difference if wanted */
    
    /* Downscale also control run if needed */
    if (data->conf->period_ctrl->downscale == TRUE)
      beg_cat = CTRL_SEC_FIELD_LS;
    else
      beg_cat = SEC_FIELD_LS;
    
    /* Loop over secondary field categories (model run and optionally control run) */
    for (cat=beg_cat; cat>=SEC_FIELD_LS; cat--) {
      /* Process only if, for this category, at least one secondary large-scale field is available */
      for (i=0; i<data->field[cat].n_ls; i++)
        /* Loop over each season */
        for (s=0; s<data->conf->nseasons; s++) {
          data->field[cat].data[i].down->delta[s] = (double *) malloc(ntime_sub[cat][s] * sizeof(double));
          if (data->field[cat].data[i].down->delta[s] == NULL) alloc_error(__FILE__, __LINE__);
          printf("Season %d\n",s);
          (void) compute_secondary_large_scale_diff(data->field[cat].data[i].down->delta[s], data->field[cat-2].analog_days[s],
                                                    data->field[cat].data[i].down->smean_norm[s], data->learning->data[s].sup_index,
                                                    data->field[CTRL_SEC_FIELD_LS].data[i].down->var[s],
                                                    data->learning->data[s].sup_index_var, ntime_sub[cat][s]);
        }
    }
    
  }
  
  /** Step 12: Reconstruct data using chosen resampled days and write output */
  
  /* Downscale also control run if needed */  
  if (data->conf->period_ctrl->downscale == TRUE)
    beg_cat = CTRL_FIELD_LS;
  else
    beg_cat = FIELD_LS;
  
  /* Loop over large-scale field categories (model run and optionally control run) */
  for (cat=beg_cat; cat>=FIELD_LS; cat--) {
    /* Process only if, for this category, at least one large-scale field is available */
    if (data->field[cat].n_ls > 0) {
      /* Process only first large-scale field. Limitation of the current implementation. */
      i = 0;

      if (data->conf->output_only != TRUE) {

        /** Determine the number of elements given the seasons **/
        /* Take into account the fact that it may be possible that the seasons does not span the whole year */
        ntimes_merged = 0;
        merged_times_flag = (short int *) malloc(data->field[cat].ntime_ls * sizeof(short int));
        if (merged_times_flag == NULL) alloc_error(__FILE__, __LINE__);
        for (ii=0; ii<data->field[cat].ntime_ls; ii++) merged_times_flag[ii] = 0;
        /* Flag all times within the processed seasons, and count number of timestep */
        for (s=0; s<data->conf->nseasons; s++)
          for (ii=0; ii<ntime_sub[cat][s]; ii++) {
            /* Retrieve current index */
            curindex_merged = data->field[cat].analog_days[s].tindex_s_all[ii];
            /* Check for bounds */
            if (curindex_merged < 0 || curindex_merged >= data->field[cat].ntime_ls) {
              (void) fprintf(stderr, "%s: Fatal error: index in merged season vector outside bounds! curindex_merged=%d max=%d\n",
                             __FILE__, curindex_merged, data->field[cat].ntime_ls-1);
              return -1;
            }
            merged_times_flag[curindex_merged] = 1;
            ntimes_merged++;
          }
        /* Save time index given flag */
        merged_itimes = (int *) malloc(data->field[cat].ntime_ls * sizeof(int));
        if (merged_itimes == NULL) alloc_error(__FILE__, __LINE__);
        merged_times = (double *) malloc(ntimes_merged * sizeof(double));
        if (merged_times == NULL) alloc_error(__FILE__, __LINE__);
        curindex_merged = 0;
        for (ii=0; ii<data->field[cat].ntime_ls; ii++) {
          if (merged_times_flag[ii] == 1) {
            merged_times[curindex_merged] = data->field[cat].time_ls[ii];
            merged_itimes[ii] = curindex_merged++;
          }
          else
            merged_itimes[ii] = -1;
        }
        (void) free(merged_times_flag);

        data->field[cat].analog_days_year.time = (int *) malloc(ntimes_merged * sizeof(int));
        if (data->field[cat].analog_days_year.time == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days_year.tindex = (int *) malloc(ntimes_merged * sizeof(int));
        if (data->field[cat].analog_days_year.tindex == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days_year.tindex_all = (int *) malloc(ntimes_merged * sizeof(int));
        if (data->field[cat].analog_days_year.tindex_all == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days_year.year = (int *) malloc(ntimes_merged * sizeof(int));
        if (data->field[cat].analog_days_year.year == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days_year.month = (int *) malloc(ntimes_merged * sizeof(int));
        if (data->field[cat].analog_days_year.month == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days_year.day = (int *) malloc(ntimes_merged * sizeof(int));
        if (data->field[cat].analog_days_year.day == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days_year.tindex_s_all = (int *) malloc(ntimes_merged * sizeof(int));
        if (data->field[cat].analog_days_year.tindex_s_all == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days_year.year_s = (int *) malloc(ntimes_merged * sizeof(int));
        if (data->field[cat].analog_days_year.year_s == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days_year.month_s = (int *) malloc(ntimes_merged * sizeof(int));
        if (data->field[cat].analog_days_year.month_s == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days_year.day_s = (int *) malloc(ntimes_merged * sizeof(int));
        if (data->field[cat].analog_days_year.day_s == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days_year.analog_dayschoice = (tstruct **) malloc(ntimes_merged * sizeof(tstruct *));
        if (data->field[cat].analog_days_year.analog_dayschoice == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].analog_days_year.metric_norm = (float **) malloc(ntimes_merged * sizeof(float *));
        if (data->field[cat].analog_days_year.metric_norm == NULL) alloc_error(__FILE__, __LINE__);
        for (ii=0; ii<ntimes_merged; ii++) {
          data->field[cat].analog_days_year.analog_dayschoice[ii] = (tstruct *) NULL;
          data->field[cat].analog_days_year.metric_norm[ii] = (float *) NULL;
        }
        data->field[cat].analog_days_year.ndayschoice = (int *) malloc(ntimes_merged * sizeof(int));
        if (data->field[cat].analog_days_year.ndayschoice == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat+2].data[i].down->delta_all = (double *) malloc(ntimes_merged * sizeof(double));
        if (data->field[cat+2].data[i].down->delta_all == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].data[i].down->dist_all = (double *) malloc(ntimes_merged * sizeof(double));
        if (data->field[cat].data[i].down->dist_all == NULL) alloc_error(__FILE__, __LINE__);
        data->field[cat].data[i].down->days_class_clusters_all = (int *) malloc(ntimes_merged * sizeof(int));
        if (data->field[cat].data[i].down->days_class_clusters_all == NULL) alloc_error(__FILE__, __LINE__);
        /* Loop over each season */
        data->field[cat].analog_days_year.ntime = 0;
        for (s=0; s<data->conf->nseasons; s++) {
          /* Merge all seasons of analog_day data, supplemental field index, and cluster info */
          printf("Season: %d\n",s);
          istat = merge_seasons(data->field[cat].analog_days_year, data->field[cat].analog_days[s],
                                merged_itimes, ntimes_merged, ntime_sub[cat][s]);
          istat = merge_seasonal_data(data->field[cat+2].data[i].down->delta_all,
                                      data->field[cat+2].data[i].down->delta[s],
                                      data->field[cat].analog_days[s], merged_itimes, 1, 1,
                                      ntimes_merged, ntime_sub[cat][s]);
          istat = merge_seasonal_data(data->field[cat].data[i].down->dist_all,
                                      data->field[cat].data[i].down->dist[s],
                                      data->field[cat].analog_days[s], merged_itimes, 1, 1,
                                      ntimes_merged, ntime_sub[cat][s]);
          istat = merge_seasonal_data_i(data->field[cat].data[i].down->days_class_clusters_all,
                                        data->field[cat].data[i].down->days_class_clusters[s],
                                        data->field[cat].analog_days[s], merged_itimes, 1, 1,
                                        ntimes_merged, ntime_sub[cat][s]);
          if (istat != 0) {
            (void) free(merged_times);
            (void) free(merged_itimes);
            return istat;
          }
          data->field[cat].analog_days_year.ntime += ntime_sub[cat][s];
        }
        
        /** Optionally save analog_days information in an output file **/
        if (data->conf->analog_save == TRUE) {
          if (cat == FIELD_LS)
            analog_file = data->conf->analog_file_other;
          else
            analog_file = data->conf->analog_file_ctrl;
          (void) save_analog_data(data->field[cat].analog_days_year, data->field[cat+2].data[i].down->delta_all,
                                  data->field[cat].data[i].down->dist_all, data->field[cat].data[i].down->days_class_clusters_all,
                                  merged_times, analog_file, data);
        }
      }
      else {
        if (cat == FIELD_LS)
          analog_file = data->conf->analog_file_other;
        else
          analog_file = data->conf->analog_file_ctrl;
        (void) printf("%s: Reading analog data from file %s\n", __FILE__, analog_file);
        (void) read_analog_data(&(data->field[cat].analog_days_year), &(data->field[cat+2].data[i].down->delta_all),
                                &merged_times, analog_file, data->conf->obs_var->timename);
        ntimes_merged = data->field[cat].analog_days_year.ntime;
      }
      
      /* Process all data */
      if (data->conf->output == TRUE) {
        if (cat == FIELD_LS) {
          period = data->conf->period;
        }
        else {
          period = data->conf->period_ctrl;
        }
        istat = output_downscaled_analog(data->field[cat].analog_days_year, data->field[cat+2].data[i].down->delta_all,
                                         data->conf->output_month_begin, data->conf->output_path, data->conf->config,
                                         data->conf->time_units, data->conf->cal_type, data->conf->deltat,
                                         data->conf->format, data->conf->compression, data->conf->compression_level,
                                         data->info, data->conf->obs_var, period, merged_times, ntimes_merged);
        if (istat != 0) {
          (void) free(merged_times);
          (void) free(merged_itimes);
          return istat;
        }
      }
      (void) free(merged_times);
      (void) free(merged_itimes);
    }
  }
          
  /* Free memory for specific downscaling buffers */
  if (data->conf->output_only != TRUE) {
    for (cat=0; cat<NCAT; cat++)
      (void) free(ntime_sub[cat]);
    (void) free(ntime_sub);
    
    /* Free mask memory if needed */
    if (mask_sub != NULL)
      (void) free(mask_sub);
  }

  /* Success return */
  return 0;
}
