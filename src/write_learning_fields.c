/* ***************************************************** */
/* Write learning fields for later use.                  */
/* write_learning_fields.c                               */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file write_learning_fields.c
    \brief Write learning fields for later use.
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

/** Read analog day data and write it for downscaled period. */
int
write_learning_fields(data_struct *data) {
  /**
     @param[in]   data                  MASTER data structure.
  */

  int istat; /* Diagnostic status */

  int ncoutid; /* NetCDF output file handle ID */
  int *timedimoutid; /* NetCDF time dimension output ID */
  int sdimoutid; /* NetCDF season dimension output ID */
  int eofdimoutid; /* NetCDF EOF dimension output ID */
  int ptsdimoutid; /* NetCDF points dimension output ID */
  int *clustdimoutid; /* NetCDF clusters dimension output ID */
  int *weightdimoutid; /* NetCDF weight dimension output ID */
  int *timeoutid; /* NetCDF time variable ID */
  int *cstoutid; /* NetCDF regression constant variable ID */
  int *regoutid; /* NetCDF regression coefficients variable ID */
  int *rrdoutid; /* NetCDF precipitation index variable ID */
  int *taoutid; /* NetCDF secondary large-scale field index variable ID */
  int pcoutid; /* NetCDF pc_normalized_var variable ID */
  int tamoutid; /* NetCDF secondary large-scale field index mean variable ID */
  int tavoutid; /* NetCDF secondary large-scale field index variance variable ID */
  int *clustoutid; /* NetCDF clusters variable output ID */
  int *weightoutid; /* NetCDF weight variable ID */
  int vardimids[NC_MAX_VAR_DIMS]; /* NetCDF dimension IDs */
  
  size_t start[3]; /* Start element when writing */
  size_t count[3]; /* Count of elements to write */

  char *tmpstr = NULL; /* Temporary string */

  utUnit dataunits; /* Time data units (udunits) */

  double fillvalue;
  char *nomvar = NULL;
  double *timeval = NULL;
  double *tancp_mean = NULL;
  double *tancp_var = NULL;
  double *bufd = NULL;

  int s;
  int t;
  int ii;

  tancp_mean = (double *) malloc(data->conf->nseasons * sizeof(double));
  if (tancp_mean == NULL) alloc_error(__FILE__, __LINE__);
  tancp_var = (double *) malloc(data->conf->nseasons * sizeof(double));
  if (tancp_var == NULL) alloc_error(__FILE__, __LINE__);

  timedimoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (timedimoutid == NULL) alloc_error(__FILE__, __LINE__);
  clustdimoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (clustdimoutid == NULL) alloc_error(__FILE__, __LINE__);
  weightdimoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (weightdimoutid == NULL) alloc_error(__FILE__, __LINE__);

  timeoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (timeoutid == NULL) alloc_error(__FILE__, __LINE__);
  cstoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (cstoutid == NULL) alloc_error(__FILE__, __LINE__);
  regoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (regoutid == NULL) alloc_error(__FILE__, __LINE__);
  rrdoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (rrdoutid == NULL) alloc_error(__FILE__, __LINE__);
  taoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (taoutid == NULL) alloc_error(__FILE__, __LINE__);
  clustoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (clustoutid == NULL) alloc_error(__FILE__, __LINE__);
  weightoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (weightoutid == NULL) alloc_error(__FILE__, __LINE__);

  nomvar = (char *) malloc(200 * sizeof(char));
  if (nomvar == NULL) alloc_error(__FILE__, __LINE__);
  tmpstr = (char *) malloc(200 * sizeof(char));
  if (tmpstr == NULL) alloc_error(__FILE__, __LINE__);

  istat = utInit("");

  /* Open NetCDF file for writing, overwrite and truncate existing file if any */
  istat = nc_create(data->learning->filename_save_learn, NC_CLOBBER, &ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Set global attributes */
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "processor", strlen(data->info->processor), data->info->processor);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "institution", strlen(data->info->institution), data->info->institution);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "creator_email", strlen(data->info->creator_email), data->info->creator_email);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "creator_url", strlen(data->info->creator_url), data->info->creator_url);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "creator_name", strlen(data->info->creator_name), data->info->creator_name);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "contact_email", strlen(data->info->contact_email), data->info->contact_email);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "contact_name", strlen(data->info->contact_name), data->info->contact_name);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "other_contact_email", strlen(data->info->other_contact_email),
                          data->info->other_contact_email);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "other_contact_name", strlen(data->info->other_contact_name),
                          data->info->other_contact_name);

  fillvalue = -9999.9;

  /* Set dimensions */
  istat = nc_def_dim(ncoutid, "season", data->conf->nseasons, &sdimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_def_dim(ncoutid, data->conf->eofname, data->learning->neof, &eofdimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_def_dim(ncoutid, data->conf->ptsname, data->reg->npts, &ptsdimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  for (s=0; s<data->conf->nseasons; s++) {

    /* Define time dimensions and variables */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_time, s+1);
    istat = nc_def_dim(ncoutid, nomvar, data->learning->data[s].ntime, &(timedimoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    vardimids[0] = timedimoutid[s];
    istat = nc_def_var(ncoutid, nomvar, NC_INT, 1, vardimids, &(timeoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
      
    istat = sprintf(tmpstr, "gregorian");
    istat = nc_put_att_text(ncoutid, timeoutid[s], "calendar", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "%s", data->conf->time_units);
    istat = nc_put_att_text(ncoutid, timeoutid[s], "units", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "time in %s", data->conf->time_units);
    istat = nc_put_att_text(ncoutid, timeoutid[s], "long_name", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Define cluster dimensions */
    (void) sprintf(nomvar, "%s_%d", data->conf->clustname, s+1);
    istat = nc_def_dim(ncoutid, nomvar, data->conf->season[s].nclusters, &(clustdimoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Define regression constant variables */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_precip_reg_cst, s+1);
    vardimids[0] = ptsdimoutid;
    istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 1, vardimids, &(cstoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    istat = nc_put_att_double(ncoutid, cstoutid[s], "missing_value", NC_DOUBLE, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_put_att_text(ncoutid, cstoutid[s], "coordinates", strlen(data->conf->ptsname), data->conf->ptsname);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "none");
    istat = nc_put_att_text(ncoutid, cstoutid[s], "units", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Define regression coefficients variables */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_precip_reg, s+1);
    vardimids[0] = clustdimoutid[s];
    vardimids[1] = ptsdimoutid;
    istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 2, vardimids, &(regoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    istat = nc_put_att_double(ncoutid, regoutid[s], "missing_value", NC_DOUBLE, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "%s %s_%d", data->conf->ptsname, data->learning->nomvar_class_clusters, s+1);
    istat = nc_put_att_text(ncoutid, regoutid[s], "coordinates", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "none");
    istat = nc_put_att_text(ncoutid, regoutid[s], "units", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Define precipitation index variables */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_precip_index, s+1);
    vardimids[0] = timedimoutid[s];
    vardimids[1] = ptsdimoutid;
    istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 2, vardimids, &(rrdoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    istat = nc_put_att_double(ncoutid, rrdoutid[s], "missing_value", NC_DOUBLE, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "%s %s_%d", data->conf->ptsname, data->learning->nomvar_time, s+1);
    istat = nc_put_att_text(ncoutid, rrdoutid[s], "coordinates", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "none");
    istat = nc_put_att_text(ncoutid, rrdoutid[s], "units", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Define sup_index (secondary large-scale field index for learning period) */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_sup_index, s+1);
    vardimids[0] = timedimoutid[s];
    istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 1, vardimids, &(taoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    istat = nc_put_att_double(ncoutid, taoutid[s], "missing_value", NC_DOUBLE, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "%s_%d", data->learning->nomvar_time, s+1);
    istat = nc_put_att_text(ncoutid, taoutid[s], "coordinates", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "none");
    istat = nc_put_att_text(ncoutid, taoutid[s], "units", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

  /* Define pc_normalized_var */
  (void) strcpy(nomvar, data->learning->nomvar_pc_normalized_var);
  vardimids[0] = eofdimoutid;
  istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 1, vardimids, &pcoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = nc_put_att_double(ncoutid, pcoutid, "missing_value", NC_DOUBLE, 1, &fillvalue);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_put_att_text(ncoutid, pcoutid, "coordinates", strlen(data->conf->eofname), data->conf->eofname);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = sprintf(tmpstr, "none");
  istat = nc_put_att_text(ncoutid, pcoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define tancp_mean */
  (void) strcpy(nomvar, data->learning->nomvar_sup_index_mean);
  vardimids[0] = sdimoutid;
  istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 1, vardimids, &tamoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = nc_put_att_double(ncoutid, tamoutid, "missing_value", NC_DOUBLE, 1, &fillvalue);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = sprintf(tmpstr, "season");
  istat = nc_put_att_text(ncoutid, tamoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = sprintf(tmpstr, "none");
  istat = nc_put_att_text(ncoutid, tamoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define tancp_var */
  (void) strcpy(nomvar, data->learning->nomvar_sup_index_var);
  vardimids[0] = sdimoutid;
  istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 1, vardimids, &tavoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = nc_put_att_double(ncoutid, tavoutid, "missing_value", NC_DOUBLE, 1, &fillvalue);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = sprintf(tmpstr, "season");
  istat = nc_put_att_text(ncoutid, tavoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = sprintf(tmpstr, "none");
  istat = nc_put_att_text(ncoutid, tavoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* End definition mode */
  istat = nc_enddef(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Write variables */
  timeval = NULL;
  for (s=0; s<data->conf->nseasons; s++) {

    timeval = (double *) realloc(timeval, data->learning->data[s].ntime * sizeof(double));
    if (timeval == NULL) alloc_error(__FILE__, __LINE__);
    
    /* Compute time variable using actual units */
    istat = utScan(data->conf->time_units, &dataunits);
    for (t=0; t<data->learning->data[s].ntime; t++)
      istat = utInvCalendar(data->learning->data[s].time_s->year[t], data->learning->data[s].time_s->month[t],
                            data->learning->data[s].time_s->day[t], data->learning->data[s].time_s->hour[t],
                            data->learning->data[s].time_s->minutes[t], data->learning->data[s].time_s->seconds[t],
                            &dataunits, &(timeval[t]));

    /* Write time */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) data->learning->data[s].ntime;
    count[1] = 0;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, timeoutid[s], start, count, timeval);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Write regression constants */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) data->reg->npts;
    count[1] = 0;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, cstoutid[s], start, count, data->learning->data[s].precip_reg_cst);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Write regressions coefficients */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) data->conf->season[s].nclusters;
    count[1] = (size_t) data->reg->npts;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, regoutid[s], start, count, data->learning->data[s].precip_reg);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Write precipitation index */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) data->learning->data[s].ntime;
    count[1] = (size_t) data->reg->npts;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, rrdoutid[s], start, count, data->learning->data[s].precip_index);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Write secondary field index */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) data->learning->data[s].ntime;
    count[1] = 0;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, taoutid[s], start, count, data->learning->data[s].sup_index);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    tancp_mean[s] = data->learning->data[s].sup_index_mean;
    tancp_var[s] = data->learning->data[s].sup_index_var;
  }

  /* Write pc_normalized_var */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) data->learning->neof;
  count[1] = 0;
  count[2] = 0;
  bufd = (double *) malloc(data->learning->neof * sizeof(double));
  if (bufd == NULL) alloc_error(__FILE__, __LINE__);
  for (ii=0; ii<data->learning->neof; ii++)
    bufd[ii] = sqrt(data->learning->pc_normalized_var[ii]);
  istat = nc_put_vara_double(ncoutid, pcoutid, start, count, bufd);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) free(bufd);

  /* Write tancp_mean */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) data->conf->nseasons;
  count[1] = 0;
  count[2] = 0;
  istat = nc_put_vara_double(ncoutid, tamoutid, start, count, tancp_mean);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Write tancp_var */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) data->conf->nseasons;
  count[1] = 0;
  count[2] = 0;
  istat = nc_put_vara_double(ncoutid, tavoutid, start, count, tancp_var);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    

  /* Close the output netCDF file */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    


  /* Open NetCDF file for writing, overwrite and truncate existing file if any */
  istat = nc_create(data->learning->filename_save_weight, NC_CLOBBER, &ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Set global attributes */
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "processor", strlen(data->info->processor), data->info->processor);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "institution", strlen(data->info->institution), data->info->institution);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "creator_email", strlen(data->info->creator_email), data->info->creator_email);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "creator_url", strlen(data->info->creator_url), data->info->creator_url);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "creator_name", strlen(data->info->creator_name), data->info->creator_name);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "contact_email", strlen(data->info->contact_email), data->info->contact_email);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "contact_name", strlen(data->info->contact_name), data->info->contact_name);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "other_contact_email", strlen(data->info->other_contact_email),
                          data->info->other_contact_email);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "other_contact_name", strlen(data->info->other_contact_name),
                          data->info->other_contact_name);

  fillvalue = -9999.9;

  /* Set dimensions */
  istat = nc_def_dim(ncoutid, "season", data->conf->nseasons, &sdimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_def_dim(ncoutid, data->conf->eofname, data->learning->neof, &eofdimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  for (s=0; s<data->conf->nseasons; s++) {

    /* Define weight dimensions and variables */
    (void) sprintf(nomvar, "%s_%d", data->conf->clustname, s+1);
    istat = nc_def_dim(ncoutid, nomvar, data->conf->season[s].nclusters, &(weightdimoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    vardimids[0] = weightdimoutid[s];
    vardimids[1] = eofdimoutid;
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_weight, s+1);
    istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 2, vardimids, &(weightoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    istat = nc_put_att_double(ncoutid, weightoutid[s], "missing_value", NC_DOUBLE, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "%s %s_%d", data->conf->eofname, data->conf->clustname, s+1);
    istat = nc_put_att_text(ncoutid, weightoutid[s], "coordinates", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "none");
    istat = nc_put_att_text(ncoutid, weightoutid[s], "units", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

  /* End definition mode */
  istat = nc_enddef(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  for (s=0; s<data->conf->nseasons; s++) {
    /* Write weights */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) data->conf->season[s].nclusters;
    count[1] = (size_t) data->learning->neof;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, weightoutid[s], start, count, data->learning->data[s].weight);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }  

  /* Close the output netCDF file */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);


  /* Open NetCDF file for writing, overwrite and truncate existing file if any */
  istat = nc_create(data->learning->filename_save_clust_learn, NC_CLOBBER, &ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Set global attributes */
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "processor", strlen(data->info->processor), data->info->processor);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "institution", strlen(data->info->institution), data->info->institution);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "creator_email", strlen(data->info->creator_email), data->info->creator_email);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "creator_url", strlen(data->info->creator_url), data->info->creator_url);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "creator_name", strlen(data->info->creator_name), data->info->creator_name);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "contact_email", strlen(data->info->contact_email), data->info->contact_email);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "contact_name", strlen(data->info->contact_name), data->info->contact_name);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "other_contact_email", strlen(data->info->other_contact_email),
                          data->info->other_contact_email);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "other_contact_name", strlen(data->info->other_contact_name),
                          data->info->other_contact_name);

  fillvalue = -9999.9;

  /* Set dimensions */
  istat = nc_def_dim(ncoutid, "season", data->conf->nseasons, &sdimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_def_dim(ncoutid, data->conf->eofname, data->learning->neof, &eofdimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  for (s=0; s<data->conf->nseasons; s++) {

    /* Define time dimensions and variables */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_time, s+1);
    istat = nc_def_dim(ncoutid, nomvar, data->learning->data[s].ntime, &(timedimoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    vardimids[0] = timedimoutid[s];
    istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 1, vardimids, &(timeoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    istat = sprintf(tmpstr, "gregorian");
    istat = nc_put_att_text(ncoutid, timeoutid[s], "calendar", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "%s", data->conf->time_units);
    istat = nc_put_att_text(ncoutid, timeoutid[s], "units", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "time in %s", data->conf->time_units);
    istat = nc_put_att_text(ncoutid, timeoutid[s], "long_name", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Define clust_learn variables */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_class_clusters, s+1);
    vardimids[0] = timedimoutid[s];
    istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 1, vardimids, &(clustoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    istat = nc_put_att_double(ncoutid, clustoutid[s], "missing_value", NC_DOUBLE, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "%s_%d", data->learning->nomvar_time, s+1);
    istat = nc_put_att_text(ncoutid, clustoutid[s], "coordinates", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "none");
    istat = nc_put_att_text(ncoutid, clustoutid[s], "units", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

  /* End definition mode */
  istat = nc_enddef(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  for (s=0; s<data->conf->nseasons; s++) {
    /* Write clust_learn */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) data->learning->data[s].ntime;
    count[1] = 0;
    count[2] = 0;
    istat = nc_put_vara_int(ncoutid, clustoutid[s], start, count, data->learning->data[s].class_clusters);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }  

  /* Close the output netCDF file */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  (void) utTerm();

  (void) free(nomvar);
  (void) free(tancp_mean);
  (void) free(tancp_var);
  (void) free(timeval);

  (void) free(timedimoutid);
  (void) free(clustdimoutid);
  (void) free(weightdimoutid);

  (void) free(timeoutid);
  (void) free(cstoutid);
  (void) free(regoutid);
  (void) free(rrdoutid);
  (void) free(taoutid);
  (void) free(clustoutid);
  (void) free(weightoutid);

  (void) free(tmpstr);

  return 0;
}
