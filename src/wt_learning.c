/* ***************************************************** */
/* wt_learning Compute learning data needed for          */
/* downscaling climate scenarios using weather typing.   */
/* wt_learning.c                                         */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: oct 2008                            */
/* Last date of modification: jul 2011                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision: 1.1                                 */
/* Adapted to udunits2                                   */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file wt_learning.c
    \brief Compute or read learning data needed for downscaling climate scenarios using weather typing.
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

/** Compute or read learning data needed for downscaling climate scenarios using weather typing. */
int
wt_learning(data_struct *data) {
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
  double *buf_learn_pc = NULL;
  double *buf_learn_pc_sub = NULL;

  double *precip_liquid_obs = NULL;
  double *precip_solid_obs = NULL;
  double *precip_obs = NULL;
  double *mean_precip = NULL;
  double *mean_precip_sub = NULL;

  double *precip_reg = NULL;
  double *precip_err = NULL;
  double *precip_index = NULL;
  double *dist_reg = NULL;
  double *vif = NULL;
  double chisq;
  double rsq;
  double autocor;

  double obs_first_sing;
  double rea_sing;
  double obs_sing;
  double *rea_var = NULL;
  double rea_first_sing;

  double *tas_rea = NULL;
  double *tas_rea_sub = NULL;
  double *tas_rea_mean = NULL;
  double *tas_rea_mean_sub = NULL;

  double missing_value;
  double missing_value_precip;

  double *mean_dist = NULL;
  double *var_dist = NULL;
  double *dist = NULL;
  double dist_pt;

  double *mask_subd = NULL;
  short int *mask_sub = NULL;
  int nlon_mask;
  int nlat_mask;
  double *lon_mask = NULL;
  double *lat_mask = NULL;

  int ntime_learn_all;
  int *ntime_sub = NULL;

  double *sup_mean = NULL;
  double *sup_var = NULL;

  double meanvif = 0.0;

  int eof;
  int clust;
  int nt;
  int ntt;
  int t;
  int s;
  int i;
  int j;
  int pt;
  int term;
  int *npt = NULL;

  /* udunits variables */
  ut_system *unitSystem = NULL; /* Unit System (udunits) */
  ut_unit *dataunits = NULL; /* Data units (udunits) */

  int niter = 2;

  int istat; /** Return status. */

  if (data->learning->learning_provided == TRUE) {
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
    if (data->learning->obs_neof != 0) {
      istat = sub_period_common(&buf_learn_obs, &ntime_learn_all, data->learning->obs->eof,
                                data->learning->obs->time_s->year, data->learning->obs->time_s->month, data->learning->obs->time_s->day,
                                data->learning->rea->time_s->year, data->learning->rea->time_s->month, data->learning->rea->time_s->day,
                                1, data->learning->obs_neof, 1, data->learning->obs->ntime, data->learning->rea->ntime);
      if (istat != 0) return istat;
    }
    istat = sub_period_common(&buf_learn_rea, &ntime_learn_all, data->learning->rea->eof,
                              data->learning->rea->time_s->year, data->learning->rea->time_s->month, data->learning->rea->time_s->day,
                              data->learning->obs->time_s->year, data->learning->obs->time_s->month, data->learning->obs->time_s->day,
                              1, data->learning->rea_neof, 1, data->learning->rea->ntime, data->learning->obs->ntime);
    if (istat != 0) return istat;

    rea_var = (double *) malloc(data->learning->rea_neof * sizeof(double));
    if (rea_var == NULL) alloc_error(__FILE__, __LINE__);

    /* Compute normalisation factor of EOF of large-scale field for the whole period */

    data->learning->pc_normalized_var = (double *) malloc(data->learning->rea_neof * sizeof(double));
    if (data->learning->pc_normalized_var == NULL) alloc_error(__FILE__, __LINE__);
    buf_learn_pc = (double *) malloc(data->learning->rea_neof * ntime_learn_all * sizeof(double));
    if (buf_learn_pc == NULL) alloc_error(__FILE__, __LINE__);

    for (eof=0; eof<data->learning->rea_neof; eof++) {

      for (nt=0; nt<ntime_learn_all; nt++)
        buf_learn_pc[nt+eof*ntime_learn_all] = buf_learn_rea[nt+eof*ntime_learn_all] * data->learning->rea->sing[eof];

      rea_var[eof] = gsl_stats_variance(&(buf_learn_pc[eof*ntime_learn_all]), 1, ntime_learn_all);
      if (rea_var[eof] == 0.0) {
        (void) fprintf(stderr, "%s: ERROR: Variance of the projection of the large-scale field onto EOF is 0.0. You probably have too many EOFs for your field. EOF number=%d. Variance=%f. Must abort...\n",
                       __FILE__, eof, rea_var[eof]);
        return -1;
      }

      /* Renormalize EOF of large-scale field for the whole period using the first EOF norm and the Singular Value */
      for (nt=0; nt<ntime_learn_all; nt++)
        buf_learn_pc[nt+eof*ntime_learn_all] = buf_learn_pc[nt+eof*ntime_learn_all] / sqrt(rea_var[0]);

      /* Recompute normalization factor using normalized field */
      data->learning->pc_normalized_var[eof] = gsl_stats_variance(&(buf_learn_pc[eof*ntime_learn_all]), 1, ntime_learn_all);
      if (data->learning->pc_normalized_var[eof] == 0.0) {
        (void) fprintf(stderr, "%s: ERROR: Normalized variance of the projection of the large-scale field onto EOF is 0.0. You probably have too many EOFs for your field. EOF number=%d. Variance=%f. Must abort...\n",
                       __FILE__, eof, data->learning->pc_normalized_var[eof]);
        return -1;
      }
    }
                                                    
    ntime_sub = (int *) malloc(data->conf->nseasons * sizeof(int));
    if (ntime_sub == NULL) alloc_error(__FILE__, __LINE__);

    /* Read observed precipitation (liquid and solid) */
    istat = read_obs_period(&precip_liquid_obs, &(data->learning->lon), &(data->learning->lat), &missing_value_precip, data, "prr",
                            data->learning->obs->time_s->year, data->learning->obs->time_s->month, data->learning->obs->time_s->day,
                            &(data->learning->nlon), &(data->learning->nlat), data->learning->obs->ntime);
    (void) free(data->learning->lon);
    (void) free(data->learning->lat);
    if (istat == -1) return -1;
    istat = read_obs_period(&precip_solid_obs, &(data->learning->lon), &(data->learning->lat), &missing_value_precip, data, "prsn",
                            data->learning->obs->time_s->year, data->learning->obs->time_s->month, data->learning->obs->time_s->day,
                            &(data->learning->nlon), &(data->learning->nlat), data->learning->obs->ntime);
    if (istat == -1) return -1;

    /* Calculate total precipitation */
    (void) printf("%s: Calculating total precipitation from solid and liquid.\n", __FILE__);
    precip_obs = (double *) malloc(data->learning->nlon*data->learning->nlat*data->learning->obs->ntime * sizeof(double));
    if (precip_obs == NULL) alloc_error(__FILE__, __LINE__);
    for (t=0; t<data->learning->obs->ntime; t++)
      for (j=0; j<data->learning->nlat; j++)
        for (i=0; i<data->learning->nlon; i++)
          if (precip_liquid_obs[i+j*data->learning->nlon+t*data->learning->nlon*data->learning->nlat] != missing_value_precip)
            precip_obs[i+j*data->learning->nlon+t*data->learning->nlon*data->learning->nlat] =
              (precip_liquid_obs[i+j*data->learning->nlon+t*data->learning->nlon*data->learning->nlat] +
               precip_solid_obs[i+j*data->learning->nlon+t*data->learning->nlon*data->learning->nlat]) * 86400.0;
          else
            precip_obs[i+j*data->learning->nlon+t*data->learning->nlon*data->learning->nlat] = missing_value_precip;
    (void) free(precip_liquid_obs);
    (void) free(precip_solid_obs);

    /* Apply mask for learning data */
    if (data->conf->learning_maskfile->use_mask == TRUE) {
      /* Allocate memory */
      mask_sub = (short int *) malloc(data->learning->nlat*data->learning->nlon * sizeof(short int));
      if (mask_sub == NULL) alloc_error(__FILE__, __LINE__);
      for (i=0; i<data->learning->nlat*data->learning->nlon; i++)
        mask_sub[i] = (short int) data->conf->learning_maskfile->field[i];
      /* Apply mask */
      (void) printf("%s: Masking points using mask file for regression analysis.\n", __FILE__);
      (void) mask_points(precip_obs, missing_value_precip, mask_sub,
                         data->learning->nlon, data->learning->nlat, data->learning->obs->ntime);
      /* Free memory of mask_sub */
      (void) free(mask_sub);
      mask_sub = NULL;
    }

    /* Mask region if needed using domain bounding box */
    if (data->conf->learning_mask_longitude_min != -999.0 &&
        data->conf->learning_mask_longitude_max != -999.0 &&
        data->conf->learning_mask_latitude_min != -999.0 &&
        data->conf->learning_mask_latitude_max != -999.0) {
      (void) printf("%s: Masking region for regression analysis.\n", __FILE__);
      (void) mask_region(precip_obs, missing_value_precip, data->learning->lon, data->learning->lat,
                         data->conf->learning_mask_longitude_min, data->conf->learning_mask_longitude_max,
                         data->conf->learning_mask_latitude_min, data->conf->learning_mask_latitude_max,
                         data->learning->nlon, data->learning->nlat, data->learning->obs->ntime);
    }

    /* Perform spatial mean of observed precipitation around regression points, normalize precip */
    (void) printf("%s: Perform spatial mean of observed precipitation around regression points.\n", __FILE__);
    mean_precip = (double *) malloc(data->reg->npts * data->learning->obs->ntime * sizeof(double));
    if (mean_precip == NULL) alloc_error(__FILE__, __LINE__);
    npt = (int *) malloc(data->learning->obs->ntime * sizeof(int));
    if (npt == NULL) alloc_error(__FILE__, __LINE__);
    for (pt=0; pt<data->reg->npts; pt++) {
      for (t=0; t<data->learning->obs->ntime; t++) {
        mean_precip[t+pt*data->learning->obs->ntime] = 0.0;
        npt[t] = 0;
      }
      for (j=0; j<data->learning->nlat; j++)
        for (i=0; i<data->learning->nlon; i++) {
          dist_pt = distance_point(data->reg->lon[pt], data->reg->lat[pt],
                                   data->learning->lon[i+j*data->learning->nlon], data->learning->lat[i+j*data->learning->nlon]);
          if (dist_pt <= data->reg->dist)
            for (t=0; t<data->learning->obs->ntime; t++)
              if (precip_obs[i+j*data->learning->nlon+t*data->learning->nlon*data->learning->nlat] != missing_value_precip) {
                mean_precip[t+pt*data->learning->obs->ntime] +=
                  precip_obs[i+j*data->learning->nlon+t*data->learning->nlon*data->learning->nlat];
                npt[t]++;
              }
        }
      for (t=0; t<data->learning->obs->ntime; t++)
        if (npt[t] == 0) {
          (void) fprintf(stderr, "%s: ERROR: There are no point of observation in the vicinity of the regression point #%d at a minimum distance of at least %f meters! Verify your regression points, or the configuration of your coordinate variable names in your configuration file. Time=%d. Must abort...\n",
                         __FILE__, pt, data->reg->dist, t);
          return -1;
        }
      for (t=0; t<data->learning->obs->ntime; t++)
        mean_precip[t+pt*data->learning->obs->ntime] = sqrt(mean_precip[t+pt*data->learning->obs->ntime] / (double) npt[t]);
    }
    (void) free(npt);
    (void) free(precip_obs);

    /* Select common time period between the re-analysis and the observation data periods for */
    /* secondary large-scale field and extract subdomain */
    istat = read_field_subdomain_period(&tas_rea, &(data->learning->sup_lon), &(data->learning->sup_lat),
                                        &missing_value, data->learning->nomvar_rea_sup,
                                        data->learning->obs->time_s->year, data->learning->obs->time_s->month,
                                        data->learning->obs->time_s->day,
                                        data->conf->secondary_longitude_min, data->conf->secondary_longitude_max,
                                        data->conf->secondary_latitude_min, data->conf->secondary_latitude_max, 
                                        data->learning->rea_coords, data->learning->rea_gridname,
                                        data->learning->rea_lonname, data->learning->rea_latname,
                                        data->learning->rea_dimxname, data->learning->rea_dimyname,
                                        data->learning->rea_timename, data->learning->filename_rea_sup,
                                        &(data->learning->sup_nlon), &(data->learning->sup_nlat), data->learning->obs->ntime);

    /* Perform spatial mean of secondary large-scale fields */
    tas_rea_mean = (double *) malloc(data->learning->obs->ntime * sizeof(double));
    if (tas_rea_mean == NULL) alloc_error(__FILE__, __LINE__);
    /* Prepare mask */
    if (data->secondary_mask->use_mask == TRUE) {
      (void) extract_subdomain(&mask_subd, &lon_mask, &lat_mask, &nlon_mask, &nlat_mask, data->secondary_mask->field,
                               data->secondary_mask->lon, data->secondary_mask->lat,
                               data->conf->secondary_longitude_min, data->conf->secondary_longitude_max,
                               data->conf->secondary_latitude_min, data->conf->secondary_latitude_max, 
                               data->secondary_mask->nlon, data->secondary_mask->nlat, 1);
      if (data->learning->sup_nlon != nlon_mask || data->learning->sup_nlat != nlat_mask) {
        (void) fprintf(stderr, "%s: IMPORTANT WARNING: The mask for secondary large-scale fields after selecting subdomain has invalid dimensions: nlon=%d nlat=%d. Expected: nlon=%d nlat=%d\nReverting to no-mask processing.", __FILE__, nlon_mask, nlat_mask,
                       data->learning->sup_nlon, data->learning->sup_nlat);
        mask_sub = (short int *) NULL;
      }
      else {
        mask_sub = (short int *) malloc(data->learning->sup_nlat*data->learning->sup_nlon * sizeof(short int));
        if (mask_sub == NULL) alloc_error(__FILE__, __LINE__);
        for (i=0; i<data->learning->sup_nlat*data->learning->sup_nlon; i++)
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

    (void) mean_field_spatial(tas_rea_mean, tas_rea, mask_sub, data->learning->sup_nlon, data->learning->sup_nlat,
                              data->learning->obs->ntime);
    if (mask_sub != NULL)
      (void) free(mask_sub);

    /* Loop over each season */
    (void) printf("Extract data for each season separately and process each season.\n");

    for (s=0; s<data->conf->nseasons; s++) {
      /* Process separately each season */

      /* Select season months in the whole time period and create sub-period fields */
      if (data->learning->obs_neof != 0) {
        (void) extract_subperiod_months(&buf_learn_obs_sub, &(ntime_sub[s]), buf_learn_obs,
                                        data->learning->time_s->year, data->learning->time_s->month, data->learning->time_s->day,
                                        data->conf->season[s].month,
                                        1, 1, data->learning->obs_neof, ntime_learn_all,
                                        data->conf->season[s].nmonths);
      }
      (void) extract_subperiod_months(&buf_learn_rea_sub, &(ntime_sub[s]), buf_learn_rea,
                                      data->learning->time_s->year, data->learning->time_s->month, data->learning->time_s->day,
                                      data->conf->season[s].month,
                                      1, 1, data->learning->rea_neof, ntime_learn_all,
                                      data->conf->season[s].nmonths);
      (void) extract_subperiod_months(&buf_learn_pc_sub, &(ntime_sub[s]), buf_learn_pc,
                                      data->learning->time_s->year, data->learning->time_s->month, data->learning->time_s->day,
                                      data->conf->season[s].month,
                                      1, 1, data->learning->rea_neof, ntime_learn_all,
                                      data->conf->season[s].nmonths);
      (void) extract_subperiod_months(&tas_rea_mean_sub, &(ntime_sub[s]), tas_rea_mean,
                                      data->learning->time_s->year, data->learning->time_s->month, data->learning->time_s->day,
                                      data->conf->season[s].month,
                                      1, 1, 1, ntime_learn_all,
                                      data->conf->season[s].nmonths);
      (void) extract_subperiod_months(&tas_rea_sub, &(ntime_sub[s]), tas_rea,
                                      data->learning->time_s->year, data->learning->time_s->month, data->learning->time_s->day,
                                      data->conf->season[s].month,
                                      1, data->learning->sup_nlon, data->learning->sup_nlat, ntime_learn_all,
                                      data->conf->season[s].nmonths);
      (void) extract_subperiod_months(&mean_precip_sub, &(ntime_sub[s]), mean_precip,
                                      data->learning->time_s->year, data->learning->time_s->month, data->learning->time_s->day,
                                      data->conf->season[s].month,
                                      1, 1, data->reg->npts, ntime_learn_all,
                                      data->conf->season[s].nmonths);

      /** Normalize secondary large-scale fields for re-analysis learning data **/
      data->learning->data[s].sup_index = (double *) malloc(ntime_sub[s] * sizeof(double));
      if (data->learning->data[s].sup_index == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].sup_val = (double *) malloc(data->learning->sup_nlon*data->learning->sup_nlat*ntime_sub[s] * sizeof(double));
      if (data->learning->data[s].sup_val == NULL) alloc_error(__FILE__, __LINE__);

      /* Compute mean and variance over time */
      data->learning->data[s].sup_index_mean = gsl_stats_mean(tas_rea_mean_sub, 1, ntime_sub[s]);
      data->learning->data[s].sup_index_var = gsl_stats_variance(tas_rea_mean_sub, 1, ntime_sub[s]);

      /* Normalize using mean and variance */
      (void) normalize_field(data->learning->data[s].sup_index, tas_rea_mean_sub, data->learning->data[s].sup_index_mean,
                             data->learning->data[s].sup_index_var, 1, 1, ntime_sub[s]);

      /* Compute mean and variance over time for each point */
      sup_mean = (double *) malloc(data->learning->sup_nlon*data->learning->sup_nlat*ntime_sub[s] * sizeof(double));
      if (sup_mean == NULL) alloc_error(__FILE__, __LINE__);
      sup_var = (double *) malloc(data->learning->sup_nlon*data->learning->sup_nlat*ntime_sub[s] * sizeof(double));
      if (sup_var == NULL) alloc_error(__FILE__, __LINE__);
      (void) time_mean_variance_field_2d(sup_mean, sup_var, tas_rea_sub, data->learning->sup_nlon, data->learning->sup_nlat, ntime_sub[s]);

      /* Normalize whole secondary 2D field using mean and variance at each point */
      (void) normalize_field_2d(data->learning->data[s].sup_val, tas_rea_sub, sup_mean,
                                sup_var, data->learning->sup_nlon, data->learning->sup_nlat, ntime_sub[s]);
      
      (void) free(sup_mean);
      sup_mean = NULL;
      (void) free(sup_var);
      sup_var = NULL;

      /** Construct time vectors **/
      data->learning->data[s].ntime = ntime_sub[s];
      data->learning->data[s].time = (double *) malloc(ntime_sub[s] * sizeof(double));
      if (data->learning->data[s].time == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].time_s->year = (int *) malloc(ntime_sub[s] * sizeof(int));
      if (data->learning->data[s].time_s->year == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].time_s->month = (int *) malloc(ntime_sub[s] * sizeof(int));
      if (data->learning->data[s].time_s->month == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].time_s->day = (int *) malloc(ntime_sub[s] * sizeof(int));
      if (data->learning->data[s].time_s->day == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].time_s->hour = (int *) malloc(ntime_sub[s] * sizeof(int));
      if (data->learning->data[s].time_s->hour == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].time_s->minutes = (int *) malloc(ntime_sub[s] * sizeof(int));
      if (data->learning->data[s].time_s->minutes == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].time_s->seconds = (double *) malloc(ntime_sub[s] * sizeof(double));
      if (data->learning->data[s].time_s->seconds == NULL) alloc_error(__FILE__, __LINE__);

      /* Retrieve time index spanning selected months and assign time structure values */
      t = 0;

      /* Initialize udunits */
      ut_set_error_message_handler(ut_ignore);
      unitSystem = ut_read_xml(NULL);
      ut_set_error_message_handler(ut_write_to_stderr);
      dataunits = ut_parse(unitSystem, data->conf->time_units, UT_ASCII);

      for (nt=0; nt<ntime_learn_all; nt++)
        for (ntt=0; ntt<data->conf->season[s].nmonths; ntt++)
          if (data->learning->time_s->month[nt] == data->conf->season[s].month[ntt]) {
            data->learning->data[s].time_s->year[t] = data->learning->time_s->year[nt];
            data->learning->data[s].time_s->month[t] = data->learning->time_s->month[nt];
            data->learning->data[s].time_s->day[t] = data->learning->time_s->day[nt];
            data->learning->data[s].time_s->hour[t] = data->learning->time_s->hour[nt];
            data->learning->data[s].time_s->minutes[t] = data->learning->time_s->minutes[nt];
            data->learning->data[s].time_s->seconds[t] = data->learning->time_s->seconds[nt];
            istat = utInvCalendar2(data->learning->data[s].time_s->year[t], data->learning->data[s].time_s->month[t],
                                   data->learning->data[s].time_s->day[t], data->learning->data[s].time_s->hour[t],
                                   data->learning->data[s].time_s->minutes[t], data->learning->data[s].time_s->seconds[t],
                                   dataunits, &(data->learning->data[s].time[t]));
            t++;
          }

      (void) ut_free(dataunits);
      (void) ut_free_system(unitSystem);  
      
      /** Merge observation and reanalysis principal components for clustering algorithm and normalize using first Singular Value **/

      buf_learn = (double *) realloc(buf_learn, ntime_sub[s] * (data->learning->rea_neof + data->learning->obs_neof) * sizeof(double));
      if (buf_learn == NULL) alloc_error(__FILE__, __LINE__);

      /* Normalisation by the first Singular Value */
      rea_first_sing = data->learning->rea->sing[0];
      for (eof=0; eof<data->learning->rea_neof; eof++) {
        rea_sing = data->learning->rea->sing[eof];        
        for (nt=0; nt<ntime_sub[s]; nt++) {
          buf_learn_rea_sub[nt+eof*ntime_sub[s]] = buf_learn_rea_sub[nt+eof*ntime_sub[s]] * rea_sing / rea_first_sing;
          buf_learn[nt+eof*ntime_sub[s]] = buf_learn_rea_sub[nt+eof*ntime_sub[s]];
        }
      }      
      if (data->learning->obs_neof != 0) {
        obs_first_sing = data->learning->obs->sing[0];
        for (eof=0; eof<data->learning->obs_neof; eof++) {
          obs_sing = data->learning->obs->sing[eof];        
          for (nt=0; nt<ntime_sub[s]; nt++) {
            buf_learn_obs_sub[nt+eof*ntime_sub[s]] = buf_learn_obs_sub[nt+eof*ntime_sub[s]] * obs_sing / obs_first_sing;
            buf_learn[nt+(eof+data->learning->rea_neof)*ntime_sub[s]] = buf_learn_obs_sub[nt+eof*ntime_sub[s]];
          }
        }
      }

      /* Compute best clusters */
      buf_weight = (double *) realloc(buf_weight, data->conf->season[s].nclusters * (data->learning->rea_neof + data->learning->obs_neof) *
                                      sizeof(double));
      if (buf_weight == NULL) alloc_error(__FILE__, __LINE__);
      niter = best_clusters(buf_weight, buf_learn, data->conf->classif_type, data->conf->npartitions,
                            data->conf->nclassifications, data->learning->rea_neof + data->learning->obs_neof,
                            data->conf->season[s].nclusters, ntime_sub[s]);

      /* Keep only first data->learning->rea_neof EOFs */
      data->learning->data[s].weight = (double *) 
        malloc(data->conf->season[s].nclusters*data->learning->rea_neof * sizeof(double));
      if (data->learning->data[s].weight == NULL) alloc_error(__FILE__, __LINE__);
      for (clust=0; clust<data->conf->season[s].nclusters; clust++)
        for (eof=0; eof<data->learning->rea_neof; eof++)
          data->learning->data[s].weight[eof+clust*data->learning->rea_neof] =
            buf_weight[eof+clust*(data->learning->rea_neof+data->learning->obs_neof)];
      
      /* Classify each day in the current clusters */
      data->learning->data[s].class_clusters = (int *) malloc(ntime_sub[s] * sizeof(int));
      if (data->learning->data[s].class_clusters == NULL) alloc_error(__FILE__, __LINE__);
      (void) class_days_pc_clusters(data->learning->data[s].class_clusters, buf_learn,
                                    data->learning->data[s].weight, data->conf->classif_type,
                                    data->learning->rea_neof, data->conf->season[s].nclusters,
                                    ntime_sub[s]);

      /* Set mean and variance of distances to clusters to 1.0 because we first need to compute distances and */
      /* we don't have a control run in learning mode */
      mean_dist = (double *) realloc(mean_dist, data->conf->season[s].nclusters * sizeof(double));
      if (mean_dist == NULL) alloc_error(__FILE__, __LINE__);
      var_dist = (double *) realloc(var_dist, data->conf->season[s].nclusters * sizeof(double));
      if (var_dist == NULL) alloc_error(__FILE__, __LINE__);
      for (clust=0; clust<data->conf->season[s].nclusters; clust++) {
        mean_dist[clust] = 1.0;
        var_dist[clust] = 1.0;
      }

      /* Compute distances to clusters using normalization */
      dist = (double *) realloc(dist, data->conf->season[s].nclusters*ntime_sub[s] * sizeof(double));
      if (dist == NULL) alloc_error(__FILE__, __LINE__);
      (void) dist_clusters_normctrl(dist, buf_learn_pc_sub, data->learning->data[s].weight,
                                    data->learning->pc_normalized_var, data->learning->pc_normalized_var, mean_dist, var_dist,
                                    data->learning->rea_neof, data->conf->season[s].nclusters, ntime_sub[s]);
      /* Normalize */
      for (clust=0; clust<data->conf->season[s].nclusters; clust++) {
        /* Calculate mean over time */
        mean_dist[clust] = gsl_stats_mean(&(dist[clust*ntime_sub[s]]), 1, ntime_sub[s]);
        /* Calculate variance over time */
        var_dist[clust] = gsl_stats_variance(&(dist[clust*ntime_sub[s]]), 1, ntime_sub[s]);
        /* Normalize */
        for (nt=0; nt<ntime_sub[s]; nt++)
          dist[nt+clust*ntime_sub[s]] = ( dist[nt+clust*ntime_sub[s]] - mean_dist[clust] ) / sqrt(var_dist[clust]);
      }

      /* Classify each day in the current clusters */
      /* data->learning->data[s].class_clusters */
      /*      data->learning->data[s].class_clusters = (int *) malloc(ntime_sub[s] * sizeof(int));
      if (data->learning->data[s].class_clusters == NULL) alloc_error(__FILE__, __LINE__);
      (void) class_days_pc_clusters(data->learning->data[s].class_clusters, buf_learn,
      data->learning->data[s].weight, data->conf->classif_type,
      data->learning->rea_neof, data->conf->season[s].nclusters, ntime_sub[s]);*/

      /* Allocate memory for regression */
      precip_reg = (double *) malloc(data->conf->season[s].nreg * sizeof(double));
      if (precip_reg == NULL) alloc_error(__FILE__, __LINE__);
      precip_index = (double *) malloc(ntime_sub[s] * sizeof(double));
      if (precip_index == NULL) alloc_error(__FILE__, __LINE__);
      precip_err = (double *) malloc(ntime_sub[s] * sizeof(double));
      if (precip_err == NULL) alloc_error(__FILE__, __LINE__);
      dist_reg = (double *) malloc(data->conf->season[s].nreg*ntime_sub[s] * sizeof(double));
      if (dist_reg == NULL) alloc_error(__FILE__, __LINE__);
      vif = (double *) malloc(data->conf->season[s].nreg * sizeof(double));
      if (vif == NULL) alloc_error(__FILE__, __LINE__);

      /* Create variable to hold values of x vector for regression */
      /* Begin with distances to clusters */
      for (clust=0; clust<data->conf->season[s].nclusters; clust++)
        for (t=0; t<ntime_sub[s]; t++)
          dist_reg[t+clust*ntime_sub[s]] = dist[t+clust*ntime_sub[s]];

      /* For special seasons using secondary field in the regression, append values to x vector for regression */
      if (data->conf->season[s].nreg == (data->conf->season[s].nclusters+1)) {
        clust = data->conf->season[s].nclusters;
        for (t=0; t<ntime_sub[s]; t++)
          dist_reg[t+clust*ntime_sub[s]] = data->learning->data[s].sup_index[t];
      }

      data->learning->data[s].precip_reg_cst = (double *) malloc(data->reg->npts * sizeof(double));
      if (data->learning->data[s].precip_reg_cst == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].precip_reg = (double *) malloc(data->reg->npts*data->conf->season[s].nreg * sizeof(double));
      if (data->learning->data[s].precip_reg == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].precip_reg_dist = (double *)
        malloc(data->conf->season[s].nclusters*ntime_sub[s] * sizeof(double));
      if (data->learning->data[s].precip_reg_dist == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].precip_index = (double *) malloc(data->reg->npts*ntime_sub[s] * sizeof(double));
      if (data->learning->data[s].precip_index == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].precip_index_obs = (double *) malloc(data->reg->npts*ntime_sub[s] * sizeof(double));
      if (data->learning->data[s].precip_index_obs == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].precip_reg_err = (double *) malloc(data->reg->npts*ntime_sub[s] * sizeof(double));
      if (data->learning->data[s].precip_reg_err == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].precip_reg_rsq = (double *) malloc(data->reg->npts * sizeof(double));
      if (data->learning->data[s].precip_reg_rsq == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].precip_reg_vif = (double *) malloc(data->conf->season[s].nreg * sizeof(double));
      if (data->learning->data[s].precip_reg_vif == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->data[s].precip_reg_autocor = (double *) malloc(data->reg->npts * sizeof(double));
      if (data->learning->data[s].precip_reg_autocor == NULL) alloc_error(__FILE__, __LINE__);

      /* Save distances */
      for (t=0; t<ntime_sub[s]; t++)
        for (clust=0; clust<data->conf->season[s].nclusters; clust++)
          data->learning->data[s].precip_reg_dist[clust+t*data->conf->season[s].nclusters] = dist[t+clust*ntime_sub[s]];

      for (pt=0; pt<data->reg->npts; pt++) {
        /* Compute regression and save regression constant */
        istat = regress(precip_reg, dist_reg, &(mean_precip_sub[pt*ntime_sub[s]]), &(data->learning->data[s].precip_reg_cst[pt]),
                        precip_index, precip_err, &chisq, &rsq, vif, &autocor, data->conf->season[s].nreg, ntime_sub[s]);
        /* Save R^2 */
        data->learning->data[s].precip_reg_rsq[pt] = rsq;
        /* Save residuals */
        for (t=0; t<ntime_sub[s]; t++)
          data->learning->data[s].precip_reg_err[pt+t*data->reg->npts] = precip_err[t];
        /* Save autocorrelation of residuals */
        data->learning->data[s].precip_reg_autocor[pt] = autocor;
        /* Save Variance Inflation Factor VIF, and compute mean VIF */
        if (pt == 0) {
          meanvif = 0.0;
          for (term=0; term<data->conf->season[s].nreg; term++) {
            data->learning->data[s].precip_reg_vif[term] = vif[term];
            meanvif += vif[term];
          }
          meanvif = meanvif / (double) data->conf->season[s].nreg;
        }

        //        (void) fprintf(stdout, "%s: pt=%d R^2=%lf CHI^2=%lf ACOR=%lf\n", __FILE__, pt, rsq, chisq, autocor);

        /* Save regression coefficients */
        for (clust=0; clust<data->conf->season[s].nreg; clust++)
          data->learning->data[s].precip_reg[pt+clust*data->reg->npts] = precip_reg[clust];

        /* Save precipitation index */
        for (t=0; t<ntime_sub[s]; t++)
          data->learning->data[s].precip_index[pt+t*data->reg->npts] = precip_index[t];

        /* Save observed precipitation index */
        for (t=0; t<ntime_sub[s]; t++)
          data->learning->data[s].precip_index_obs[pt+t*data->reg->npts] = mean_precip_sub[t+pt*ntime_sub[s]];
      }

      (void) fprintf(stdout, "%s: MeanVIF=%lf\n", __FILE__, meanvif);

      (void) free(precip_reg);
      (void) free(precip_index);
      (void) free(precip_err);
      (void) free(dist_reg);
      (void) free(vif);

      (void) free(buf_learn_rea_sub);
      buf_learn_rea_sub = NULL;
      if (data->learning->obs_neof != 0) {
        (void) free(buf_learn_obs_sub);
        buf_learn_obs_sub = NULL;
      }
      (void) free(buf_learn_pc_sub);
      buf_learn_pc_sub = NULL;
      (void) free(tas_rea_mean_sub);
      tas_rea_mean_sub = NULL;
      (void) free(tas_rea_sub);
      tas_rea_sub = NULL;
      (void) free(mean_precip_sub);
      mean_precip_sub = NULL;
    }

    (void) free(tas_rea);
    (void) free(tas_rea_mean);
    (void) free(mean_precip);
    (void) free(buf_weight);
    (void) free(buf_learn);
    (void) free(buf_learn_rea);
    if (data->learning->obs_neof != 0) (void) free(buf_learn_obs);
    (void) free(buf_learn_pc);
    (void) free(ntime_sub);
    (void) free(rea_var);
    (void) free(mean_dist);
    (void) free(var_dist);
    (void) free(dist);

    /* If wanted, write learning data to files for later use */
    if (data->learning->learning_save == TRUE) {
      (void) printf("Writing learning fields.\n");
      istat = write_learning_fields(data);
    }
    if (niter == 1) {
      (void) fprintf(stderr, "%s: ERROR: In one classification, only 1 iteration was needed! Probably an error in your EOF data or configuration. Must abort...\n",
                     __FILE__);
      return -1;
    }
  }

  /* Success status */
  return 0;
}
