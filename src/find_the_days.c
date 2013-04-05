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

/** Find analog days given cluster, supplemental large-scale field, and precipitation distances. */
int
find_the_days(analog_day_struct analog_days, double *precip_index, double *precip_index_learn,
              double *sup_field_index, double *sup_field_index_learn, double *sup_field, double *sup_field_learn, short int *mask,
              int *class_clusters, int *class_clusters_learn, int *year, int *month, int *day,
              int *year_learn, int *month_learn, int *day_learn, char *time_units,
              int ntime, int ntime_learn, int *months, int nmonths, int ndays, int ndayschoices, int npts, int shuffle, int sup,
              int sup_choice, int sup_cov, int use_downscaled_year, int only_wt, int nlon, int nlat, int sup_nlon, int sup_nlat) {
  /**
     @param[out]  analog_days           Analog days time indexes and dates, as well as corresponding downscale dates
     @param[in]   precip_index          Precipitation index of days to downscale
     @param[in]   precip_index_learn    Precipitation index of learning period
     @param[in]   sup_field_index       Secondary large-scale field index of days to downscale
     @param[in]   sup_field_index_learn Secondary large-scale field index of learning period
     @param[in]   sup_field             Secondary large-scale field of days to downscale
     @param[in]   sup_field_learn       Secondary large-scale field of learning period
     @param[in]   mask                  Mask for covariance of secondary large-scale field
     @param[in]   class_clusters        Days classification cluster index of days to downscale
     @param[in]   class_clusters_learn  Days classification cluster index of learning period
     @param[in]   year                  years of days to downscale
     @param[in]   month                 month of days to downscale
     @param[in]   day                   day of month of days to downscale
     @param[in]   year_learn            years of days of learning period
     @param[in]   month_learn           month of days of learning period
     @param[in]   day_learn             day of month of days of learning period
     @param[in]   time_units            base time units for downscaling
     @param[in]   ntime                 number of times of days to downscale
     @param[in]   ntime_learn           number of times of learning period
     @param[in]   months                months of the year in each season
     @param[in]   nmonths               number of months in each season
     @param[in]   ndays                 number of +- days to look around day of the year being downscaled
     @param[in]   ndayschoices          number of days to choose in first selection
     @param[in]   npts                  number of regression points of precipitation index
     @param[in]   shuffle               shuffle or not the days of the first selection
     @param[in]   sup                   if we want to use the secondary large-scale field in the final selection of the analog day
     @param[in]   sup_choice            if we want to use the secondary large-scale field in the first selection of the analog day
     @param[in]   sup_cov               if we want to use covariance of fields instead of averaged-field differences
     @param[in]   use_downscaled_year   if we want to also search the analog day in the year of the current downscaled year
     @param[in]   only_wt               if we want to restrict search within the same weather type
     @param[in]   nlon                  longitude dimension
     @param[in]   nlat                  latitude dimension
     @param[in]   sup_nlon              secondary large-scale field longitude dimension (for covariance)
     @param[in]   sup_nlat              secondary large-scale field latitude dimension (for covariance)
  */
  
  int *buf_sub_i = NULL; /* Temporary buffer for time index of subperiod */
  int *buf_learn_sub_i = NULL; /* Temporary buffer for time index of learning subperiod */
  int ntime_sub; /* Number of times in subperiod */
  int ntime_learn_sub; /* Number of times in learning subperiod */

  unsigned long int *random_num = NULL; /* Random number for shuffle */

  const gsl_rng_type *T = NULL; /* Random number generator type for shuffle */
  gsl_rng *rng = NULL; /* Random number generator for shuffle */

  int cur_dayofy; /* Current day of year being downscaled */
  int learn_dayofy; /* Learning day of year being processed */
  int diff_day_learn; /* Difference in terms of day of year between downscaled day of year and learning one */

  double max_metric = 0.0; /* Maximum metric value. The metric is the value used to compare days
                              (cluster distance, index distance, etc.) */
  double max_metric_sup = 0.0; /* Maximum metric value for secondary large-scale field metric. */
  double min_metric = 0.0; /* Minimum metric */

  double precip_diff; /* Squared sum of regressed precipitation difference (between downscaled and learning day) over all points. */
  double diff_precip_pt; /* Regressed precipitation difference (between downscaled and learning day) for 1 point. */
  double sup_diff; /* Secondary large-scale field difference (between downscaled and learning day) */

  double varstd; /* Standard deviation of precipitation index metric */
  double varmean; /* Mean of precipitation index metric */
  double varstd_sup; /* Standard deviation of secondary large-scale field metric */
  double varmean_sup; /* Mean of secondary large-scale field metric */

  double *metric = NULL; /* Precipitation index metric buffer */
  double *metric_norm = NULL; /* Normalized precipitation index metric buffer */
  double *metric_sup = NULL; /* Secondary large-scale field metric */
  double *metric_sup_norm = NULL; /* Normalized secondary large-scale field metric */
  size_t *metric_index = NULL; /* Metric sorted index of ndayschoices days */
  size_t *random_index = NULL; /* Shuffled metric index of ndayschoices days */
  int min_metric_index; /* Index of minimum metric */
  int max_metric_index; /* Index of maximum metric */
  int *clust_diff = NULL; /* Cluster number differences between cluster of learning day and cluster of day being downscaled */

  int ntime_days; /* Number of days in learning period within the +-ndays of downscaled day of year */
  int *ntime_days_learn = NULL; /* Time index of the learning subperiod corresponding to all valid days within the +-ndays of downscaled day of year */

  int ii; /* Loop counter */
  int t; /* Time loop counter */
  int tt; /* Time loop counter */
  int tl; /* Time loop counter */
  int pts; /* Regression points loop counter */

  ut_system *unitSystem = NULL; /* Unit System (udunits) */
  ut_unit *dataunits = NULL; /* udunits variable */
  int istat; /* Return status of functions */
  double timei; /* udunits Time value */

  /* Initialize udunits */
  ut_set_error_message_handler(ut_ignore);
  unitSystem = ut_read_xml(NULL);
  ut_set_error_message_handler(ut_write_to_stderr);
  dataunits = ut_parse(unitSystem, time_units, UT_ASCII);

  /* Initialize random number generator if needed */
  if (shuffle == TRUE) {
    T = gsl_rng_default;
    rng = gsl_rng_alloc(T);
    (void) gsl_rng_set(rng, time(NULL));
    /* Allocate memory */
    random_num = (unsigned long int *) malloc(ndayschoices * sizeof(unsigned long int));
    if (random_num == NULL) alloc_error(__FILE__, __LINE__);
    random_index = (size_t *) malloc(ndayschoices * sizeof(size_t));
    if (random_index == NULL) alloc_error(__FILE__, __LINE__);
  }

  /* Allocate memory for metric index */
  metric_index = (size_t *) malloc(ndayschoices * sizeof(size_t));
  if (metric_index == NULL) alloc_error(__FILE__, __LINE__);

  /* Select correct months for the current season in the time vectors of the downscaled and learning period */
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

  /* Process each downscaled day */
  for (t=0; t<ntime_sub; t++) {
    
#if DEBUG > 7
    printf("%d %d %d %d\n",t,year[buf_sub_i[t]],month[buf_sub_i[t]],day[buf_sub_i[t]]);
#endif

    /* Compute the current downscaled day of year being processed */
    cur_dayofy = dayofclimyear(day[buf_sub_i[t]], month[buf_sub_i[t]]);

    /* Initializing */
    ntime_days = 0;
    max_metric = -9999999.9;
    max_metric_sup = -9999999.9;

    /* Search analog days in learning period */
    for (tl=0; tl<ntime_learn_sub; tl++) {

      /* If use_downscaled_year != 1, check that we don't search the analog day in the downscaled year. */
      if (use_downscaled_year != 0 || (use_downscaled_year == 0 && year_learn[buf_learn_sub_i[tl]] != year[buf_sub_i[t]])) {

        /* Compute the learning period day of year and compute the difference with the current processed day of year */
        learn_dayofy = dayofclimyear(day_learn[buf_learn_sub_i[tl]], month_learn[buf_learn_sub_i[tl]]);
        diff_day_learn = abs(cur_dayofy - learn_dayofy);

        /* We are within the day of year range */
        if (diff_day_learn <= ndays) {
        
          /* Allocate memory */
          metric = (double *) realloc(metric, (ntime_days+1) * sizeof(double));
          if (metric == NULL) alloc_error(__FILE__, __LINE__);
          metric_norm = (double *) realloc(metric_norm, (ntime_days+1) * sizeof(double));
          if (metric_norm == NULL) alloc_error(__FILE__, __LINE__);

          /* Compute precipitation index difference and precipitation index metric */
          precip_diff = 0.0;
          for (pts=0; pts<npts; pts++) {
            diff_precip_pt = precip_index[pts+t*npts] - precip_index_learn[pts+tl*npts];
            precip_diff += (diff_precip_pt*diff_precip_pt);
            /*
              if (t == 4595 && year_learn[buf_learn_sub_i[tl]] == 2005 && month_learn[buf_learn_sub_i[tl]] == 5 && day_learn[buf_learn_sub_i[tl]] == 29) {
              printf("%d %d %lf %lf %lf %lf\n",t,pts,precip_diff,diff_precip_pt,precip_index[pts+t*npts],precip_index_learn[pts+tl*npts]);
              }
            */
            /*          if (t == 4595)
                        printf("%d %d %lf %lf %lf %lf\n",t,pts,precip_diff,diff_precip_pt,precip_index[pts+t*npts],precip_index_learn[pts+tl*npts]);*/
            
          }
          metric[ntime_days] = sqrt(precip_diff);
          //        if (t == (ntime_sub-1))
          //          printf("metric before max %d %lf\n",ntime_days,metric[ntime_days]);
        
          /* Store the maximum metric value and its index */
          if (metric[ntime_days] > max_metric) {
            //          if (t == (ntime_sub-1))
            //            printf("%d %lf %lf\n",ntime_days,max_metric,metric[ntime_days]);
            max_metric = metric[ntime_days];
            max_metric_index = ntime_days;
          }

          /* If we want to also use the secondary large-scale fields in the first selection of days */
          if (sup_choice == TRUE || sup == TRUE) {
            /* Allocate memory */
            metric_sup = (double *) realloc(metric_sup, (ntime_days+1) * sizeof(double));
            if (metric_sup == NULL) alloc_error(__FILE__, __LINE__);
            metric_sup_norm = (double *) realloc(metric_sup_norm, (ntime_days+1) * sizeof(double));
            if (metric_sup_norm == NULL) alloc_error(__FILE__, __LINE__);

            if (sup_cov != TRUE) {
              /* Compute supplemental field index difference */
              sup_diff = sup_field_index[t] - sup_field_index_learn[buf_learn_sub_i[tl]];
              metric_sup[ntime_days] = sqrt(sup_diff * sup_diff);
            }
            else {
              /* Compute covariance of supplemental field */
              if (nlon != sup_nlon || nlat != sup_nlat) {
                (void) fprintf(stderr, "%s: Dimensions of downscaled large-scale secondary field (nlat=%d nlon=%d) are not the same as the learning field (nlat=%d nlon=%d. Cannot proceed...\n", __FILE__, nlat, nlon, sup_nlat, sup_nlon);
                return -1;
              }
              (void) covariance_fields_spatial(&sup_diff, sup_field, sup_field_learn, mask, t, tl, sup_nlon, sup_nlat);
              metric_sup[ntime_days] = sqrt(sup_diff * sup_diff);
            }
            /* Store the maximum value and its index */
            if (metric_sup[ntime_days] > max_metric_sup)
              max_metric_sup = metric_sup[ntime_days];
          
            //            if (t >= (ntime_sub-5))
            // if (year_learn[buf_learn_sub_i[tl]] == 2005 && month_learn[buf_learn_sub_i[tl]] == 5 && day_learn[buf_learn_sub_i[tl]] == 29) {
            //            printf("%d %lf %lf %lf %lf %lf %lf\n",ntime_days,max_metric_sup,metric_sup[ntime_days],sup_field_index[t],sup_field_index_learn[buf_learn_sub_i[tl]],metric[ntime_days],precip_diff);
            //  }
          
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

          /* Count days within day of year range */
          ntime_days++;
        }
      }
    }

    /* If at least one day was in range */
    if (ntime_days > 0) {

      if (only_wt != 0)
        /* Put the maximum value when cluster number is not the same */
        /* Parse each days within range */
        for (tl=0; tl<ntime_days; tl++) {
          if (clust_diff[tl] != 0) {
            metric[tl] = max_metric;
            if (sup_choice == TRUE || sup == TRUE)
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
      
      /** Normalize the two metrics **/
      /* Compute the standard deviation */
      varmean = gsl_stats_mean(metric, 1, (size_t) ntime_days);
      varstd = gsl_stats_sd_m(metric, 1, (size_t) ntime_days, varmean);
      if (sup_choice == TRUE) {
        /* Do the same if needed for secondary large-scale field */
        varmean_sup = gsl_stats_mean(metric_sup, 1, (size_t) ntime_days);
        varstd_sup = gsl_stats_sd_m(metric_sup, 1, (size_t) ntime_days, varmean_sup);
        /* Apply normalization and sum the two metrics if we use the secondary large-scale field in the first selection */
        /* and also in the second and final selection */
        if (sup_choice == TRUE) {
          /* Process each days within range */
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
            metric_norm[tl] = ((metric[tl] - varmean) / varstd) + metric_sup_norm[tl];

            /*
            if (t >= (ntime_sub-5))
              printf("!! %d %lf %lf %lf\n",tl,metric_norm[tl],varmean,varstd);
              if (year_learn[ntime_days_learn[tl]] == 2005 && month_learn[ntime_days_learn[tl]] == 5 && day_learn[ntime_days_learn[tl]] == 29)
              if (clust_diff[tl] == 0)
              printf("%lf\n",metric[tl]);
            */
          }
        }
        else
          for (tl=0; tl<ntime_days; tl++)
            metric_norm[tl] = (metric[tl] - varmean) / varstd;
      }
      else
        for (tl=0; tl<ntime_days; tl++)
          metric_norm[tl] = (metric[tl] - varmean) / varstd;

      /* Sort the vector, retrieve the sorted indexes and select only the first ndayschoices ones */
      //      printf("%d %d\n",ntime_days,ndayschoices);
      (void) gsl_sort_smallest_index(metric_index, (size_t) ndayschoices, metric_norm, 1, (size_t) ntime_days);
      
      if (shuffle == TRUE) {
        /* Shuffle the vector of indexes and choose the first one. This select a random day for the second and final selection */
        for (ii=0; ii<ndayschoices; ii++)
          random_num[ii] = gsl_rng_uniform_int(rng, 100);
        (void) gsl_sort_ulong_index(random_index, random_num, 1, (size_t) ndayschoices);
        
        min_metric = metric_norm[metric_index[random_index[0]]];
        min_metric_index = metric_index[random_index[0]];

        /* Save analog day time index in the learning period */
        analog_days.tindex[t] = ntime_days_learn[metric_index[random_index[0]]];
        analog_days.year[t] = year_learn[analog_days.tindex[t]];
        analog_days.month[t] = month_learn[analog_days.tindex[t]];
        analog_days.day[t] = day_learn[analog_days.tindex[t]];
        analog_days.tindex_all[t] = buf_learn_sub_i[analog_days.tindex[t]];
        istat = utInvCalendar2(analog_days.year[t], analog_days.month[t], analog_days.day[t], 0, 0, 0.0, dataunits, &timei);
        analog_days.time[t] = (int) timei;

        /* Save date of day being downscaled */
        analog_days.year_s[t] = year[buf_sub_i[t]];
        analog_days.month_s[t] = month[buf_sub_i[t]];
        analog_days.day_s[t] = day[buf_sub_i[t]];
        analog_days.tindex_s_all[t] = buf_sub_i[t];

        /* Save all analog days in special time structure */
        analog_days.analog_dayschoice[t] = (tstruct *) malloc(ndayschoices * sizeof(tstruct));
        if (analog_days.analog_dayschoice[t] == NULL) alloc_error(__FILE__, __LINE__);
        analog_days.metric_norm[t] = (float *) malloc(ndayschoices * sizeof(float));
        if (analog_days.metric_norm[t] == NULL) alloc_error(__FILE__, __LINE__);
        for (ii=0; ii<ndayschoices; ii++) {
          analog_days.metric_norm[t][ii] = metric_norm[metric_index[ii]];
          analog_days.analog_dayschoice[t][ii].year = year_learn[ntime_days_learn[metric_index[ii]]];
          analog_days.analog_dayschoice[t][ii].month = month_learn[ntime_days_learn[metric_index[ii]]];
          analog_days.analog_dayschoice[t][ii].day = day_learn[ntime_days_learn[metric_index[ii]]];
          analog_days.analog_dayschoice[t][ii].hour = 0;
          analog_days.analog_dayschoice[t][ii].min = 0;
          analog_days.analog_dayschoice[t][ii].sec = 0;
        }
      }
      else {
        /* Don't shuffle. Instead choose the one having the smallest metric for the best match */

        min_metric = 99999999.9;
        min_metric_index = -1;
        if (sup == TRUE) {
          /* If we use the secondary large-scale field for this final selection */
          for (ii=0; ii<ndayschoices; ii++) {
            if (metric_sup[metric_index[ii]] < min_metric) {
              min_metric_index = metric_index[ii];
              min_metric = metric_sup[metric_index[ii]];
            }
            
            /*            if (t >= (ntime_sub-5)) {
              printf("!!! %d %d %d %d %lf\n",ii,t,tl,(int)metric_index[ii],metric_norm[metric_index[ii]]);
              printf("SUP %d %d %lf %lf %lf %d ",ii,ndayschoices,metric_sup[metric_index[ii]],metric_norm[metric_index[ii]]*2.0,min_metric,(int)metric_index[ii]);
              printf("%d %d %d\n",year_learn[ntime_days_learn[metric_index[ii]]],month_learn[ntime_days_learn[metric_index[ii]]],day_learn[ntime_days_learn[metric_index[ii]]]);
              printf("%d %d\n",sup,sup_choice);
              }*/
            
          }
        }
        else {
          /* We rather use the main large-scale field (precipitation) as the metric for the final selection */
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

        /* Save analog day time index in the learning period */
        analog_days.tindex[t] = ntime_days_learn[min_metric_index];
        analog_days.year[t] = year_learn[analog_days.tindex[t]];
        analog_days.month[t] = month_learn[analog_days.tindex[t]];
        analog_days.day[t] = day_learn[analog_days.tindex[t]];
        analog_days.tindex_all[t] = buf_learn_sub_i[analog_days.tindex[t]];
        istat = utInvCalendar2(analog_days.year[t], analog_days.month[t], analog_days.day[t], 0, 0, 0.0, dataunits, &timei);
        analog_days.time[t] = (int) timei;

        /* Save date of day being downscaled */
        analog_days.year_s[t] = year[buf_sub_i[t]];
        analog_days.month_s[t] = month[buf_sub_i[t]];
        analog_days.day_s[t] = day[buf_sub_i[t]];
        analog_days.tindex_s_all[t] = buf_sub_i[t];

        /* Save all analog days in special time structure */
        analog_days.analog_dayschoice[t] = (tstruct *) malloc(ndayschoices * sizeof(tstruct));
        if (analog_days.analog_dayschoice[t] == NULL) alloc_error(__FILE__, __LINE__);
        analog_days.metric_norm[t] = (float *) malloc(ndayschoices * sizeof(float));
        if (analog_days.metric_norm[t] == NULL) alloc_error(__FILE__, __LINE__);
        for (ii=0; ii<ndayschoices; ii++) {
          analog_days.metric_norm[t][ii] = metric_norm[metric_index[ii]];
          analog_days.analog_dayschoice[t][ii].year = year_learn[ntime_days_learn[metric_index[ii]]];
          analog_days.analog_dayschoice[t][ii].month = month_learn[ntime_days_learn[metric_index[ii]]];
          analog_days.analog_dayschoice[t][ii].day = day_learn[ntime_days_learn[metric_index[ii]]];
          analog_days.analog_dayschoice[t][ii].hour = 0;
          analog_days.analog_dayschoice[t][ii].min = 0;
          analog_days.analog_dayschoice[t][ii].sec = 0;
        }
      }
      
      /* Free memory */
      (void) free(metric);
      (void) free(metric_norm);
      metric = NULL;
      metric_norm = NULL;
      if (sup_choice == TRUE || sup == TRUE) {
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
        if (year[buf_sub_i[t]] == 1999 && month[buf_sub_i[t]] == 5)
          if (month[buf_sub_i[t]] == 3 || month[buf_sub_i[t]] == 4 || month[buf_sub_i[t]] == 5)
            printf("Time downscaled %d: %d %d %d. Analog day: %d %d %d %lf\n", t, year[buf_sub_i[t]], month[buf_sub_i[t]], day[buf_sub_i[t]], year_learn[analog_days.tindex[t]], month_learn[analog_days.tindex[t]], day_learn[analog_days.tindex[t]], min_metric);
        
  }

  /* Free memory */
  if (shuffle == TRUE) {
    (void) gsl_rng_free(rng);
    (void) free(random_num);
    (void) free(random_index);
  }
  (void) free(metric_index);

  (void) free(buf_sub_i);
  (void) free(buf_learn_sub_i);

  (void) ut_free(dataunits);
  (void) ut_free_system(unitSystem);  
      
  return 0;
}
