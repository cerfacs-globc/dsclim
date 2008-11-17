/* ***************************************************** */
/* Find analog days given cluster, supplemental          */
/* large-scale field, and precipitation distances.       */
/* find_the_days.c                                       */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file find_the_days.c
    \brief Find analog days given cluster, supplemental large-scale field, and precipitation distances.
*/

#include <dsclim.h>

void find_the_days(int *analog_days, double *precip_index, double *precip_index_learn, double *sup_field, double *sup_index,
                   int *class_clusters, int *class_clusters_learn, int *year, int *month, int *day,
                   int *year_learn, int *month_learn, int *day_learn,
                   int ntime, int ntime_learn, int *months, int nmonths, int ndays, int ndayschoices, int npts, int shuffle, int sup,
                   int sup_choice) {
  
  int *buf_sub_i = NULL;
  int *buf_learn_sub_i = NULL;
  int ntime_sub;
  int ntime_learn_sub;

  unsigned long int *random_num;

  const gsl_rng_type *T;
  gsl_rng *rng;

  int cur_dayofy;
  int learn_dayofy;
  int diff_day_learn;

  int ntime_days;

  double max_metric;
  double max_metric_sup;
  double min_metric;

  double precip_diff;
  double diff_precip_pt;
  double sup_diff;

  double varstd;
  double varmean;
  double varstd_sup;
  double varmean_sup;

  double *metric = NULL;
  double *metric_norm = NULL;
  double *metric_sup = NULL;
  double *metric_sup_norm = NULL;
  size_t *metric_index = NULL;
  size_t *random_index = NULL;
  int min_metric_index;
  int max_metric_index;
  int *clust_diff = NULL;

  int *ntime_days_learn = NULL;

  int i;
  int ii;
  int j;
  int t;
  int tt;
  int tl;
  int pts;

  /* Initialize random number generator */
  T = gsl_rng_default;
  rng = gsl_rng_alloc(T);
  (void) gsl_rng_set(rng, time(NULL));

  if (shuffle == TRUE) {
    random_num = (unsigned long int *) malloc(ndayschoices * sizeof(unsigned long int));
    if (random_num == NULL) alloc_error(__FILE__, __LINE__);
  }

  metric_index = (size_t *) malloc(ndayschoices * sizeof(size_t));
  if (metric_index == NULL) alloc_error(__FILE__, __LINE__);
  random_index = (size_t *) malloc(ndayschoices * sizeof(size_t));
  if (random_index == NULL) alloc_error(__FILE__, __LINE__);

  /* Select correct months for the current season in the time vectors */
  ntime_sub = 0;
  for (t=0; t<ntime; t++)
    for (tt=0; tt<nmonths; tt++)
      if (month[t] == months[tt]) {
        buf_sub_i = (int *) realloc(buf_sub_i, (ntime_sub+1) * sizeof(int));
        if (buf_sub_i == NULL) alloc_error(__FILE__, __LINE__);
        buf_sub_i[ntime_sub++] = t;
      }

  ntime_learn_sub = 0;
  for (t=0; t<ntime_learn; t++)
    for (tt=0; tt<nmonths; tt++)
      if (month_learn[t] == months[tt]) {
        buf_learn_sub_i = (int *) realloc(buf_learn_sub_i, (ntime_learn_sub+1) * sizeof(int));
        if (buf_learn_sub_i == NULL) alloc_error(__FILE__, __LINE__);
        buf_learn_sub_i[ntime_learn_sub++] = t;
      }

  for (t=0; t<ntime_sub; t++) {

    /* Compute the current model day of year processed */
    cur_dayofy = dayofclimyear(day[buf_sub_i[t]], month[buf_sub_i[t]]);

    ntime_days = 0;
    max_metric = -9999999.9;
    if (sup == TRUE)
      max_metric_sup = -9999999.9;
    for (tl=0; tl<ntime_learn_sub; tl++) {

      /* Compute the learning period day of year and compute the difference with the current processed day of year */
      learn_dayofy = dayofclimyear(day_learn[buf_learn_sub_i[tl]], month_learn[buf_learn_sub_i[tl]]);
      diff_day_learn = abs(cur_dayofy - learn_dayofy);

      /* We are within the day of year range */
      if (diff_day_learn <= ndays) {
        
        metric = (double *) realloc(metric, (ntime_days+1) * sizeof(double));
        if (metric == NULL) alloc_error(__FILE__, __LINE__);
        metric_norm = (double *) realloc(metric_norm, (ntime_days+1) * sizeof(double));
        if (metric_norm == NULL) alloc_error(__FILE__, __LINE__);

        /* Compute precipitation index difference */
        precip_diff = 0.0;
        for (pts=0; pts<npts; pts++) {
          diff_precip_pt = precip_index[pts+t*npts] - precip_index_learn[pts+tl*npts];
          precip_diff += (diff_precip_pt*diff_precip_pt);
          /*
            if (t == 4595 && year_learn[buf_learn_sub_i[tl]] == 2005 && month_learn[buf_learn_sub_i[tl]] == 5 && day_learn[buf_learn_sub_i[tl]] == 29) {
            printf("%d %d %lf %lf %lf %lf\n",t,pts,precip_diff,diff_precip_pt,precip_index[pts+t*npts],precip_index_learn[pts+tl*npts]);
            }
          */
        }
        metric[ntime_days] = sqrt(precip_diff);
        //        if (t == (ntime_sub-1))
        //          printf("metric before max %d %lf\n",ntime_days,metric[ntime_days]);
        
        /* Store the maximum value */
        if (metric[ntime_days] > max_metric) {
          //          if (t == (ntime_sub-1))
          //            printf("%d %lf %lf\n",ntime_days,max_metric,metric[ntime_days]);
          max_metric = metric[ntime_days];
          max_metric_index = ntime_days;
        }

        if (sup == TRUE) {
          metric_sup = (double *) realloc(metric_sup, (ntime_days+1) * sizeof(double));
          if (metric_sup == NULL) alloc_error(__FILE__, __LINE__);
          metric_sup_norm = (double *) realloc(metric_sup_norm, (ntime_days+1) * sizeof(double));
          if (metric_sup_norm == NULL) alloc_error(__FILE__, __LINE__);

          /* Compute supplemental field index difference */
          sup_diff = sup_field[t] - sup_index[buf_learn_sub_i[tl]];
          metric_sup[ntime_days] = sqrt(sup_diff * sup_diff);
          /* Store the maximum value */
          if (metric_sup[ntime_days] > max_metric_sup)
            max_metric_sup = metric_sup[ntime_days];
          /*
            if (t >= (ntime_sub-5))
            if (year_learn[buf_learn_sub_i[tl]] == 2005 && month_learn[buf_learn_sub_i[tl]] == 5 && day_learn[buf_learn_sub_i[tl]] == 29) {
            printf("%d %lf %lf %lf %lf %lf %lf\n",ntime_days,max_metric_sup,metric_sup[ntime_days],sup_field[t],sup_index[buf_learn_sub_i[tl]],metric[ntime_days],precip_diff);
            }
          */
        }

        /* Compute cluster difference */
        clust_diff = (int *) realloc(clust_diff, (ntime_days+1) * sizeof(int));
        if (clust_diff == NULL) alloc_error(__FILE__, __LINE__);
        clust_diff[ntime_days] = class_clusters_learn[tl] - class_clusters[t];

        /* Store the index in the time vector of the selected day */
        ntime_days_learn = (int *) realloc(ntime_days_learn, (ntime_days+1) * sizeof(int));
        if (ntime_days_learn == NULL) alloc_error(__FILE__, __LINE__);

        ntime_days_learn[ntime_days] = buf_learn_sub_i[tl];
        /*        
        if (t == (ntime_sub-1)) {
          printf("!clust %d %d %d %d %d %d day=%d\n",year_learn[buf_learn_sub_i[tl]], month_learn[buf_learn_sub_i[tl]], day_learn[buf_learn_sub_i[tl]], clust_diff[ntime_days], class_clusters_learn[tl], class_clusters[t],t);
          if (year_learn[buf_learn_sub_i[tl]] == 1986 && month_learn[buf_learn_sub_i[tl]] == 5 && day_learn[buf_learn_sub_i[tl]] == 25)
            for (pts=0; pts<npts; pts++)
              printf("!!! %d %d %d %d %d %lf %lf %lf %lf %lf\n",pts,ntime_days,year_learn[buf_learn_sub_i[tl]], month_learn[buf_learn_sub_i[tl]], day_learn[buf_learn_sub_i[tl]], (precip_index[pts+t*npts] - precip_index_learn[pts+tl*npts]), (precip_index[pts+t*npts] - precip_index_learn[pts+tl*npts])*(precip_index[pts+t*npts] - precip_index_learn[pts+tl*npts]), metric[ntime_days], precip_index[pts+t*npts],precip_index_learn[pts+tl*npts]);
        }
        */

        ntime_days++;
      }
    }

    if (ntime_days > 0) {

      /* Put the maximum value when cluster is not the same */
      for (tl=0; tl<ntime_days; tl++) {
        if (clust_diff[tl] != 0) {
          metric[tl] = max_metric;
          if (sup == TRUE)
            metric_sup[tl] = max_metric_sup;
        }
      }

      /*
        for (tl=0; tl<ntime_days; tl++)
        if (year_learn[ntime_days_learn[tl]] == 2005 && month_learn[ntime_days_learn[tl]] == 5 && day_learn[ntime_days_learn[tl]] == 29) {
        if (clust_diff[tl] == 0)
        printf("after max metric %lf max_metric=%lf %d\n",metric[tl],max_metric,max_metric_index);
        }
      */
      
      /* Normalize the two metrics */
      /* Compute the standard deviation */
      varmean = gsl_stats_mean(metric, 1, (size_t) ntime_days);
      varstd = gsl_stats_sd_m(metric, 1, (size_t) ntime_days, varmean);
      if (sup == TRUE) {
        varmean_sup = gsl_stats_mean(metric_sup, 1, (size_t) ntime_days);
        varstd_sup = gsl_stats_sd_m(metric_sup, 1, (size_t) ntime_days, varmean_sup);
      }
      /* Apply normalization and sum the two metrics */
      if (sup == TRUE && sup_choice == TRUE) {
        for (tl=0; tl<ntime_days; tl++) {
          /*
            if (tl == 0)
            printf("std mean %lf %lf\n",varstd,varmean);
            if (year_learn[ntime_days_learn[tl]] == 2005 && month_learn[ntime_days_learn[tl]] == 5 && day_learn[ntime_days_learn[tl]] == 29) {
            if (clust_diff[tl] == 0)
            printf("after norm metric %lf ",metric[tl]);
            }
          */
          metric_sup_norm[tl] = (metric_sup[tl] - varmean_sup) / varstd_sup;
          metric_norm[tl] = ((metric[tl] - varmean) / varstd) + metric_sup[tl];
          /*
          if (year_learn[ntime_days_learn[tl]] == 2005 && month_learn[ntime_days_learn[tl]] == 5 && day_learn[ntime_days_learn[tl]] == 29)
            if (clust_diff[tl] == 0)
              printf("%lf\n",metric[tl]);
          */
        }
      }
      else
        for (tl=0; tl<ntime_days; tl++)
          metric_norm[tl] = (metric[tl] - varmean) / varstd;

      /* Sort the vector, retrieve the sortex indexes and select only the first ndayschoices ones */
      (void) gsl_sort_smallest_index(metric_index, (size_t) ndayschoices, metric_norm, 1, (size_t) ntime_days);
      
      if (shuffle == TRUE) {
        /* Shuffle the vector of indexes */
        for (ii=0; ii<ndayschoices; ii++)
          random_num[ii] = gsl_rng_uniform_int(rng, 100);
        (void) gsl_sort_int_index(random_index, random_num, 1, (size_t) ndayschoices);
        
        min_metric = metric_norm[metric_index[random_index[0]]];
        min_metric_index = metric_index[random_index[0]];

        analog_days[t] = ntime_days_learn[metric_index[random_index[0]]];
      }
      else {
        min_metric = 99999999.9;
        min_metric_index = -1;
        if (sup == TRUE) {
          for (ii=0; ii<ndayschoices; ii++) {
            if (metric_sup[metric_index[ii]] < min_metric) {
              min_metric_index = metric_index[ii];
              min_metric = metric_sup[metric_index[ii]];
            }
            /*
            if (t >= (ntime_sub-5)) {
              printf("SUP %d %d %lf %lf %d ",ii,ndayschoices,metric_sup[metric_index[ii]],metric_norm[metric_index[ii]]*2.0,min_metric,metric_index[ii]);
              printf("%d %d %d\n",year_learn[ntime_days_learn[metric_index[ii]]],month_learn[ntime_days_learn[metric_index[ii]]],day_learn[ntime_days_learn[metric_index[ii]]]);
            }
            */
          }
        }
        else {
          for (ii=0; ii<ndayschoices; ii++) {
            //            if (t == (ntime_sub-1)) {
            //              printf("%d %d %lf %lf\n",ii,ndayschoices,metric[metric_index[ii]],min_metric);
            //              printf("%d %d %d\n",year_learn[ntime_days_learn[metric_index[ii]]],month_learn[ntime_days_learn[metric_index[ii]]],day_learn[ntime_days_learn[metric_index[ii]]]);
            //            }
            if (metric_norm[metric_index[ii]] < min_metric) {
              min_metric_index = metric_index[ii];
              min_metric = metric_norm[metric_index[ii]];
            }
          }
        }
        analog_days[t] = ntime_days_learn[min_metric_index];
      }
      
      /* Free memory */
      (void) free(metric);
      (void) free(metric_norm);
      metric = NULL;
      metric_norm = NULL;
      if (sup == TRUE) {
        (void) free(metric_sup);
        metric_sup = NULL;
        (void) free(metric_sup_norm);
        metric_sup_norm = NULL;
      }
      (void) free(clust_diff);
      clust_diff = NULL;
      (void) free(ntime_days_learn);
      ntime_days_learn = NULL;
    }
    //    if (year[buf_sub_i[t]] == 1999 && month[buf_sub_i[t]] == 5 && day[buf_sub_i[t]] == 31)
    //    if (month[buf_sub_i[t]] == 3 || month[buf_sub_i[t]] == 4 || month[buf_sub_i[t]] == 5)
    //    printf("Time downscaled %d: %d %d %d. Analog day: %d %d %d %lf\n", t, year[buf_sub_i[t]], month[buf_sub_i[t]], day[buf_sub_i[t]], year_learn[analog_days[t]], month_learn[analog_days[t]], day_learn[analog_days[t]], min_metric);
    
  }

  /* Free memory */
  if (shuffle == TRUE) {
    (void) gsl_rng_free(rng);
    (void) free(random_num);
  }
  (void) free(metric_index);
  (void) free(random_index);

  (void) free(buf_sub_i);
  (void) free(buf_learn_sub_i);
}
