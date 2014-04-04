/* ***************************************************** */
/* read_learning_fields Read learning fields.            */
/* read_learning_fields.c                                */
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
/*! \file read_learning_fields.c
    \brief Read Learning data from input files. Currently only NetCDF is implemented.
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

/** Read Learning data from input files. Currently only NetCDF is implemented. */
int
read_learning_fields(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat; /* Diagnostic status */
  int i; /* Loop counter */
  int t; /* Loop counter */
  int ii; /* Loop counter */
  char *nomvar = NULL; /* Variable name in NetCDF file */
  char *nomvar_time = NULL; /* Time variable name in NetCDF file */
  char *nomvar_season = NULL; /* Season variable name in NetCDF file */
  char *name = NULL; /* Dimension name in NetCDF file */
  char *cal_type = NULL; /* Calendar type (udunits) */
  char *time_units = NULL; /* Time units (udunits) */
  double *bufd = NULL; /* Temporary buffer */
  double *time_sort = NULL; /* Temporary time info used for time merging */
  size_t *time_index = NULL; /* Temporary time index used for time merging */
  int total_t; /* Total number of times used for time merging */
  int neof; /* EOF dimension */
  int npts; /* Points dimension */
  int nclusters; /* Clusters dimension */
  int neof_file; /* EOF dimension in input file */

  data->learning->sup_lat = data->learning->sup_lon = NULL;

  /* Allocate memory for temporary strings */
  nomvar = (char *) malloc(500 * sizeof(char));
  if (nomvar == NULL) alloc_error(__FILE__, __LINE__);
  nomvar_time = (char *) malloc(500 * sizeof(char));
  if (nomvar_time == NULL) alloc_error(__FILE__, __LINE__);
  nomvar_season = (char *) malloc(500 * sizeof(char));
  if (nomvar_season == NULL) alloc_error(__FILE__, __LINE__);
  name = (char *) malloc(500 * sizeof(char));
  if (name == NULL) alloc_error(__FILE__, __LINE__);

  /* Initialize season string */
  (void) strcpy(nomvar_season, "season");

  /* Loop over all the seasons */
  for (i=0; i<data->conf->nseasons; i++) {

    if (data->conf->season[i].secondary_cov == TRUE && data->learning->sup_lat == NULL) {
      /* Read lat variable */
      istat = read_netcdf_var_2d(&(data->learning->sup_lat), (info_field_struct *) NULL, (proj_struct *) NULL,
                                 data->learning->filename_open_learn,
                                 data->learning->sup_latname, data->learning->sup_lonname, data->learning->sup_latname,
                                 &(data->learning->sup_nlon), &(data->learning->sup_nlat), TRUE);
      if (istat != 0) {
        /* In case of failure */
        (void) free(nomvar);
        (void) free(nomvar_time);
        (void) free(nomvar_season);
        (void) free(name);
        return istat;
      }
    }
    if (data->conf->season[i].secondary_cov == TRUE && data->learning->sup_lon == NULL) {
      /* Read lon variable */
      istat = read_netcdf_var_2d(&(data->learning->sup_lon), (info_field_struct *) NULL, (proj_struct *) NULL,
                                 data->learning->filename_open_learn,
                                 data->learning->sup_lonname, data->learning->sup_lonname, data->learning->sup_latname,
                                 &(data->learning->sup_nlon), &(data->learning->sup_nlat), TRUE);
      if (istat != 0) {
        /* In case of failure */
        (void) free(nomvar);
        (void) free(nomvar_time);
        (void) free(nomvar_season);
        (void) free(name);
        return istat;
      }
    }

    /* Read time data and info */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_time, i+1);
    (void) sprintf(nomvar_time, "%s_%d", data->learning->nomvar_time, i+1);
    istat = get_time_info(data->learning->data[i].time_s, &(data->learning->data[i].time), &time_units, &cal_type,
                          &(data->learning->data[i].ntime),
                          data->learning->filename_open_learn, nomvar_time, TRUE);
    (void) free(cal_type);
    (void) free(time_units);
    if (istat != 0) {
      /* In case of failure */
      (void) free(nomvar);
      (void) free(nomvar_time);
      (void) free(nomvar_season);
      (void) free(name);
      return istat;
    }

    /* Read weight data */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_weight, i+1);
    (void) sprintf(name, "%s_%d", data->conf->clustname, i+1);
    istat = read_netcdf_var_2d(&(data->learning->data[i].weight), (info_field_struct *) NULL, (proj_struct *) NULL,
                               data->learning->filename_open_weight, nomvar, data->conf->eofname, name,
                               &neof_file, &nclusters, TRUE);
    /* Save EOF dimension for control and model large-scale fields */
    if (data->field[0].n_ls > 0) {
      if (data->field[0].data[0].eof_info->neof_ls != neof_file) {
        (void) fprintf(stderr, "%s: ERROR: Number of EOFs in learning weight datafile (%d) is not equal to number of EOFs specified in XML configuration file for model large-scale fields (%d)!\n", __FILE__, neof_file, data->field[0].data[0].eof_info->neof_ls);
        (void) free(nomvar);
        (void) free(nomvar_time);
        (void) free(nomvar_season);
        (void) free(name);
        return -1;
      }
    }
    if (data->field[1].n_ls > 0)
      if (data->field[1].data[0].eof_info->neof_ls != neof_file) {
        (void) fprintf(stderr, "%s: ERROR: Number of EOFs in learning weight datafile (%d) is not equal to number of EOFs specified in XML configuration file for control large-scale fields (%d)!\n", __FILE__, neof_file, data->field[1].data[0].eof_info->neof_ls);
        (void) free(nomvar);
        (void) free(nomvar_time);
        (void) free(nomvar_season);
        (void) free(name);
        return -1;
      }
    
    /* If clusters dimension is not initialized, use retrieved info from input file */
    if (data->conf->season[i].nclusters == -1)
      data->conf->season[i].nclusters = nclusters;
    /* Else verify that they match */
    else if (data->conf->season[i].nclusters != nclusters) {
      (void) fprintf(stderr, "%s: ERROR: Incorrect number of clusters in NetCDF file. Season %d, nclusters=%d vs configuration file %d.\n",
                     __FILE__, i, nclusters, data->conf->season[i].nclusters);
      (void) free(nomvar);
      (void) free(nomvar_time);
      (void) free(nomvar_season);
      (void) free(name);
      return -1;
    }
    if (istat != 0) {
      /* In case of failure */
      (void) free(nomvar);
      (void) free(nomvar_time);
      (void) free(nomvar_season);
      (void) free(name);
      return istat;
    }

    /* Read precip_reg data (precipitation regression coefficients) */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_precip_reg, i+1);
    (void) sprintf(name, "%s_%d", data->conf->clustname, i+1);
    istat = read_netcdf_var_2d(&(data->learning->data[i].precip_reg), (info_field_struct *) NULL, (proj_struct *) NULL,
                               data->learning->filename_open_learn,
                               nomvar, data->conf->ptsname, name,
                               &npts, &(data->conf->season[i].nreg), TRUE);
    /* Verify that points dimension match configuration value */
    if (npts != data->reg->npts) {
      (void) fprintf(stderr, "%s: ERROR: Incorrect number of points in NetCDF file %d vs configuration file %d.\n",
                     __FILE__, npts, data->reg->npts);
      (void) free(nomvar);
      (void) free(nomvar_time);
      (void) free(nomvar_season);
      (void) free(name);
      return -1;
    }
    if (istat != 0) {
      /* In case of failure */
      (void) free(nomvar);
      (void) free(nomvar_time);
      (void) free(nomvar_season);
      (void) free(name);
      return istat;
    }

    /* Read precip_reg_cst data (precipitation regression constant) */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_precip_reg_cst, i+1);
    istat = read_netcdf_var_1d(&(data->learning->data[i].precip_reg_cst), (info_field_struct *) NULL,
                               data->learning->filename_open_learn, nomvar, data->conf->ptsname, &npts, TRUE);
    /* Verify that points dimension match configuration value */
    if (npts != data->reg->npts) {
      (void) fprintf(stderr, "%s: ERROR: Incorrect number of points in NetCDF file %d vs configuration file %d.\n",
                     __FILE__, npts, data->reg->npts);
      (void) free(nomvar);
      (void) free(nomvar_time);
      (void) free(nomvar_season);
      (void) free(name);
      return -1;
    }
    if (istat != 0) {
      /* In case of failure */
      (void) free(nomvar);
      (void) free(nomvar_time);
      (void) free(nomvar_season);
      (void) free(name);
      return istat;
    }

    /* Read precip_index data (precipitation index for learning period over all regression points) */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_precip_index, i+1);
    istat = read_netcdf_var_2d(&(data->learning->data[i].precip_index), (info_field_struct *) NULL, (proj_struct *) NULL,
                               data->learning->filename_open_learn,
                               nomvar, data->conf->ptsname, nomvar_time,
                               &npts, &(data->learning->data[i].ntime), TRUE);
    /* Verify that points dimension match configuration value */
    if (npts != data->reg->npts) {
      (void) fprintf(stderr, "%s: ERROR: Incorrect number of points in NetCDF file %d vs configuration file %d.\n",
                     __FILE__, npts, data->reg->npts);
      (void) free(nomvar);
      (void) free(nomvar_time);
      (void) free(nomvar_season);
      (void) free(name);
      return -1;
    }
    if (istat != 0) {
      /* In case of failure */
      (void) free(nomvar);
      (void) free(nomvar_time);
      (void) free(nomvar_season);
      (void) free(name);
      return istat;
    }

    /* Read optional precip_reg_err data (regression residuals for learning period over all regression points) */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_precip_reg_err, i+1);
    istat = read_netcdf_var_2d(&(data->learning->data[i].precip_reg_err), (info_field_struct *) NULL, (proj_struct *) NULL,
                               data->learning->filename_open_learn,
                               nomvar, data->conf->ptsname, nomvar_time,
                               &npts, &(data->learning->data[i].ntime), TRUE);
    if (istat != 0) {
      /* In case of failure */
      /* Support the fact that this variable is not in pre-1.5.15 dsclim version output files, so it is optional */
      data->learning->data[i].precip_reg_err = NULL;
      (void) fprintf(stderr, "%s: WARNING: Old learning file without precip_reg_err data.\n", __FILE__);
    }
    else {
      /* Verify that points dimension match configuration value */
      if (npts != data->reg->npts) {
        (void) fprintf(stderr, "%s: ERROR: Incorrect number of points in NetCDF file %d vs configuration file %d.\n",
                       __FILE__, npts, data->reg->npts);
        (void) free(nomvar);
        (void) free(nomvar_time);
        (void) free(nomvar_season);
        (void) free(name);
        return -1;
      }
    }

    /* Read cluster distances data (normalized distances for learning period over all clusters) */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_precip_reg_dist, i+1);
    istat = read_netcdf_var_2d(&(data->learning->data[i].precip_reg_dist), (info_field_struct *) NULL, (proj_struct *) NULL,
                               data->learning->filename_open_learn,
                               nomvar, data->conf->ptsname, nomvar_time,
                               &nclusters, &(data->learning->data[i].ntime), TRUE);
    if (istat != 0) {
      /* In case of failure */
      /* Support the fact that this variable is not in pre-1.5.15 dsclim version output files, so it is optional */
      data->learning->data[i].precip_reg_dist = NULL;
      (void) fprintf(stderr, "%s: WARNING: Old learning file without cluster distances data.\n", __FILE__);
    }
    else {
      /* If clusters dimension is not initialized, use retrieved info from input file */
      if (data->conf->season[i].nclusters == -1)
        data->conf->season[i].nclusters = nclusters;
      /* Else verify that they match */
      else if (data->conf->season[i].nclusters != nclusters) {
        (void) fprintf(stderr, "%s: ERROR: Incorrect number of clusters in NetCDF file. Season %d, nclusters=%d vs configuration file %d.\n",
                       __FILE__, i, nclusters, data->conf->season[i].nclusters);
        (void) free(nomvar);
        (void) free(nomvar_time);
        (void) free(nomvar_season);
        (void) free(name);
        return -1;
      }
    }

    /* Read cluster allocation data */
    bufd = NULL;
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_class_clusters, i+1);
    istat = read_netcdf_var_1d(&bufd, (info_field_struct *) NULL,
                               data->learning->filename_open_clust_learn, nomvar, nomvar_time,
                               &(data->learning->data[i].ntime), TRUE);
    if (istat != 0) {
      /* In case of failure */
      (void) free(nomvar);
      (void) free(nomvar_time);
      (void) free(nomvar_season);
      (void) free(name);
      return istat;
    }
    /* Transfer data into proper data structure */
    data->learning->data[i].class_clusters = malloc(data->learning->data[i].ntime * sizeof(int));
    if (data->learning->data[i].class_clusters == NULL) alloc_error(__FILE__, __LINE__);
    for (ii=0; ii<data->learning->data[i].ntime; ii++)
      data->learning->data[i].class_clusters[ii] = bufd[ii];
    (void) free(bufd);

    /* Read sup_index data (secondary large-scale field index for learning period) */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_sup_index, i+1);
    istat = read_netcdf_var_1d(&(data->learning->data[i].sup_index), (info_field_struct *) NULL,
                               data->learning->filename_open_learn, nomvar, nomvar_time,
                               &(data->learning->data[i].ntime), TRUE);
    if (istat != 0) {
      /* In case of failure */
      (void) free(nomvar);
      (void) free(nomvar_time);
      (void) free(nomvar_season);
      (void) free(name);
      return istat;
    }

    if (data->conf->season[i].secondary_cov == TRUE) {
      (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_sup_val, i+1);
      istat = read_netcdf_var_3d(&(data->learning->data[i].sup_val), (info_field_struct *) NULL, (proj_struct *) NULL,
                                 data->learning->filename_open_learn,
                                 nomvar, data->learning->sup_lonname, data->learning->sup_latname, nomvar_time,
                                 &(data->learning->sup_nlon), &(data->learning->sup_nlat), &(data->learning->data[i].ntime), TRUE);
      if (istat != 0) {
        /* In case of failure */
        (void) free(nomvar);
        (void) free(nomvar_time);
        (void) free(nomvar_season);
        (void) free(name);
        return istat;
      }
    }
    else
      data->learning->data[i].sup_val = NULL;


    /** We read as many season times the same variable to have the same values available for each season! **/

    /* Read sup_index_mean data (secondary large-scale field index spatial mean for learning period) */
    istat = read_netcdf_var_generic_val(&(data->learning->data[i].sup_index_mean), (info_field_struct *) NULL,
                                        data->learning->filename_open_learn, data->learning->nomvar_sup_index_mean, i);
    if (istat != 0) {
      /* In case of failure */
      (void) free(nomvar);
      (void) free(nomvar_time);
      (void) free(nomvar_season);
      (void) free(name);
      return istat;
    }
    
    /* Read sup_index_var data (secondary large-scale field index spatial variance for learning period) */
    istat = read_netcdf_var_generic_val(&(data->learning->data[i].sup_index_var), (info_field_struct *) NULL,
                                        data->learning->filename_open_learn, data->learning->nomvar_sup_index_var, i);
    if (istat != 0) {
      /* In case of failure */
      (void) free(nomvar);
      (void) free(nomvar_time);
      (void) free(nomvar_season);
      (void) free(name);
      return istat;
    }
  }

  /** Create whole period time info from separate season info merging **/

  /* Allocate memory and set pointers to NULL for realloc use */
  time_index = (size_t *) malloc(data->conf->nseasons * sizeof(size_t));
  if (time_index == NULL) alloc_error(__FILE__, __LINE__);
  data->learning->time_s->year = NULL;
  data->learning->time_s->month = NULL;
  data->learning->time_s->day = NULL;
  data->learning->time_s->hour = NULL;
  data->learning->time_s->minutes = NULL;
  data->learning->time_s->seconds = NULL;

  /* Sort the vector, get the sorted vector indexes */
  time_sort = (double *) malloc(data->conf->nseasons * sizeof(double));
  if (time_sort == NULL) alloc_error(__FILE__, __LINE__);
  /* Loop over seasons */
  for (i=0; i<data->conf->nseasons; i++)
    time_sort[i] = data->learning->data[i].time[0];
  /* Sorting */
  (void) gsl_sort_index(time_index, time_sort, 1, (size_t) data->conf->nseasons);
  (void) free(time_sort);

  /* Merge time info */
  total_t = 0;
  for (i=0; i<data->conf->nseasons; i++) {
    for (t=0; t<data->learning->data[time_index[i]].ntime; t++) {
      data->learning->time_s->year = (int *) realloc(data->learning->time_s->year, (total_t+1) * sizeof(int));
      if (data->learning->time_s->year == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->time_s->month = (int *) realloc(data->learning->time_s->month, (total_t+1) * sizeof(int));
      if (data->learning->time_s->month == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->time_s->day = (int *) realloc(data->learning->time_s->day, (total_t+1) * sizeof(int));
      if (data->learning->time_s->day == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->time_s->hour = (int *) realloc(data->learning->time_s->hour, (total_t+1) * sizeof(int));
      if (data->learning->time_s->hour == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->time_s->minutes = (int *) realloc(data->learning->time_s->minutes, (total_t+1) * sizeof(int));
      if (data->learning->time_s->minutes == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->time_s->seconds = (double *) realloc(data->learning->time_s->seconds, (total_t+1) * sizeof(double));
      if (data->learning->time_s->seconds == NULL) alloc_error(__FILE__, __LINE__);
      data->learning->time_s->year[total_t] = data->learning->data[time_index[i]].time_s->year[t];
      data->learning->time_s->month[total_t] = data->learning->data[time_index[i]].time_s->month[t];
      data->learning->time_s->day[total_t] = data->learning->data[time_index[i]].time_s->day[t];
      data->learning->time_s->hour[total_t] = data->learning->data[time_index[i]].time_s->hour[t];
      data->learning->time_s->minutes[total_t] = data->learning->data[time_index[i]].time_s->minutes[t];
      data->learning->time_s->seconds[total_t] = data->learning->data[time_index[i]].time_s->seconds[t];
      total_t++;
    }
  }
  /* Save total number of times */
  data->learning->ntime = total_t;

  /* Free temporary vector */
  (void) free(time_index);

  /* Read pc_normalized_var data (normalized EOF-projected large-scale field variance for learning period) */
  istat = read_netcdf_var_1d(&(data->learning->pc_normalized_var), (info_field_struct *) NULL,
                             data->learning->filename_open_learn, data->learning->nomvar_pc_normalized_var, data->conf->eofname,
                             &neof, TRUE);
  if (neof != neof_file) {
    /* Verify that EOF dimension match configuration value */
    (void) fprintf(stderr, "%s: ERROR: Number of EOFs in learning weight datafile (%d) is not equal to number of EOFs in learning datafile (%d)!\n", __FILE__, neof, neof_file);
    (void) free(nomvar);
    (void) free(nomvar_time);
    (void) free(nomvar_season);
    (void) free(name);
    return -1;
  }
  if (istat != 0) {
    /* In case of failure */
    (void) free(nomvar);
    (void) free(nomvar_time);
    (void) free(nomvar_season);
    (void) free(name);
    return istat;
  }
  /* The square-root of variance is stored: convert back to variance */
  for (ii=0; ii<neof; ii++)
    data->learning->pc_normalized_var[ii] = data->learning->pc_normalized_var[ii] * data->learning->pc_normalized_var[ii];

  /* Free memory */
  (void) free(nomvar);
  (void) free(nomvar_time);
  (void) free(nomvar_season);
  (void) free(name);

  /* Success status */
  return 0;
}
