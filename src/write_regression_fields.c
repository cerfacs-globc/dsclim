/* ***************************************************** */
/* Write regression-related downscaling fields for       */
/* diagnostics use.                                      */
/* write_regression_fields.c                             */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file write_regression_fields.c
    \brief Write regression-related downscaling fields for diagnostics use.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2010)

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

/** Write regression-related downscaling fields for diagnostics use. */
int
write_regression_fields(data_struct *data, char *filename, double **timeval, int *ntime, double **precip_index, double **distclust,
                        double **sup_index) {
  /**
     @param[in]   data                  MASTER data structure.
     @param[in]   filename              Output filename.
     @param[in]   timeval               Time vector.
     @param[in]   ntime                 Number of times in timeval.
     @param[in]   precip_index          Precipitation index.
     @param[in]   distclust             Distance of days to cluster centres.
     @param[in]   sup_index             Supplemental large-scale field index.
  */

  int istat; /* Diagnostic status */

  int ncoutid; /* NetCDF output file handle ID */
  int *timedimoutid; /* NetCDF time dimension output ID */
  int sdimoutid; /* NetCDF season dimension output ID */
  int ptsdimoutid; /* NetCDF points dimension output ID */
  int *clustdimoutid; /* NetCDF clusters dimension output ID */
  int *timeoutid; /* NetCDF time variable ID */
  int *cstoutid; /* NetCDF regression constant variable ID */
  int *regoutid; /* NetCDF regression coefficients variable ID */
  int *rrdoutid; /* NetCDF precipitation index variable ID */
  int *distclustoutid; /* NetCDF distances to clusters variable ID */
  int *taoutid; /* NetCDF secondary large-scale field index variable ID */
  int *rsqoutid; /* NetCDF regression R^2 variable ID */
  int *acoroutid; /* NetCDF regression autocorrelation variable ID */
  int *vifoutid; /* NetCDF regression VIF variable ID */
  int vardimids[NC_MAX_VAR_DIMS]; /* NetCDF dimension IDs */
  
  size_t start[3]; /* Start element when writing */
  size_t count[3]; /* Count of elements to write */

  char *tmpstr = NULL; /* Temporary string */

  double fillvalue;
  char *nomvar = NULL;

  int s;

  timedimoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (timedimoutid == NULL) alloc_error(__FILE__, __LINE__);
  clustdimoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (clustdimoutid == NULL) alloc_error(__FILE__, __LINE__);

  timeoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (timeoutid == NULL) alloc_error(__FILE__, __LINE__);
  cstoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (cstoutid == NULL) alloc_error(__FILE__, __LINE__);
  regoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (regoutid == NULL) alloc_error(__FILE__, __LINE__);
  rrdoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (rrdoutid == NULL) alloc_error(__FILE__, __LINE__);
  distclustoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (distclustoutid == NULL) alloc_error(__FILE__, __LINE__);
  taoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (taoutid == NULL) alloc_error(__FILE__, __LINE__);
  rsqoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (rsqoutid == NULL) alloc_error(__FILE__, __LINE__);
  acoroutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (acoroutid == NULL) alloc_error(__FILE__, __LINE__);
  vifoutid = (int *) malloc(data->conf->nseasons * sizeof(int));
  if (vifoutid == NULL) alloc_error(__FILE__, __LINE__);

  nomvar = (char *) malloc(200 * sizeof(char));
  if (nomvar == NULL) alloc_error(__FILE__, __LINE__);
  tmpstr = (char *) malloc(200 * sizeof(char));
  if (tmpstr == NULL) alloc_error(__FILE__, __LINE__);

  /* Open NetCDF file for writing, overwrite and truncate existing file if any */
  istat = nc_create(filename, NC_CLOBBER, &ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Set global attributes */
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "processor", strlen(data->info->processor), data->info->processor);
  istat = nc_put_att_text(ncoutid, NC_GLOBAL, "software", strlen(data->info->software), data->info->software);
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
  istat = nc_def_dim(ncoutid, data->conf->ptsname, data->reg->npts, &ptsdimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  for (s=0; s<data->conf->nseasons; s++) {

    /* Define time dimensions and variables */
    (void) sprintf(nomvar, "%s_%d", data->reg->timename, s+1);
    istat = nc_def_dim(ncoutid, nomvar, ntime[s], &(timedimoutid[s]));
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
    istat = sprintf(tmpstr, "%s %s_%d", data->conf->ptsname, data->reg->timename, s+1);
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
    istat = sprintf(tmpstr, "%s_%d", data->reg->timename, s+1);
    istat = nc_put_att_text(ncoutid, taoutid[s], "coordinates", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "none");
    istat = nc_put_att_text(ncoutid, taoutid[s], "units", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Define distances to clusters variable */
    (void) sprintf(nomvar, "cluster_distance_%d", s+1);
    vardimids[0] = clustdimoutid[s];
    vardimids[1] = timedimoutid[s];
    istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 2, vardimids, &(distclustoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    istat = nc_put_att_double(ncoutid, distclustoutid[s], "missing_value", NC_DOUBLE, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "%s_%d %s_%d", data->reg->timename, s+1, data->conf->clustname, s+1);
    istat = nc_put_att_text(ncoutid, distclustoutid[s], "coordinates", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "none");
    istat = nc_put_att_text(ncoutid, distclustoutid[s], "units", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Define regression R^2 diagnostic */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_precip_reg_rsq, s+1);
    vardimids[0] = ptsdimoutid;
    istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 1, vardimids, &(rsqoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    istat = nc_put_att_double(ncoutid, rsqoutid[s], "missing_value", NC_DOUBLE, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_put_att_text(ncoutid, rsqoutid[s], "coordinates", strlen(data->conf->ptsname), data->conf->ptsname);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "none");
    istat = nc_put_att_text(ncoutid, rsqoutid[s], "units", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Define regression autocorrelation diagnostic */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_precip_reg_acor, s+1);
    vardimids[0] = ptsdimoutid;
    istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 1, vardimids, &(acoroutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    istat = nc_put_att_double(ncoutid, acoroutid[s], "missing_value", NC_DOUBLE, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_put_att_text(ncoutid, acoroutid[s], "coordinates", strlen(data->conf->ptsname), data->conf->ptsname);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "none");
    istat = nc_put_att_text(ncoutid, acoroutid[s], "units", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Define regression VIF diagnostic */
    (void) sprintf(nomvar, "%s_%d", data->learning->nomvar_precip_reg_vif, s+1);
    vardimids[0] = clustdimoutid[s];
    istat = nc_def_var(ncoutid, nomvar, NC_DOUBLE, 1, vardimids, &(vifoutid[s]));
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    istat = nc_put_att_double(ncoutid, vifoutid[s], "missing_value", NC_DOUBLE, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = nc_put_att_text(ncoutid, vifoutid[s], "coordinates", strlen(data->learning->nomvar_class_clusters),
                            data->learning->nomvar_class_clusters);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    istat = sprintf(tmpstr, "none");
    istat = nc_put_att_text(ncoutid, vifoutid[s], "units", strlen(tmpstr), tmpstr);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  }

  /* End definition mode */
  istat = nc_enddef(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Write variables */
  for (s=0; s<data->conf->nseasons; s++) {

    /* Write time */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) ntime[s];
    count[1] = 0;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, timeoutid[s], start, count, timeval[s]);
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

    /* Write reconstructed precipitation index */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) ntime[s];
    count[1] = (size_t) data->reg->npts;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, rrdoutid[s], start, count, precip_index[s]);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Write secondary field index */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) ntime[s];
    count[1] = 0;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, taoutid[s], start, count, sup_index[s]);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Write distances to clusters */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) data->conf->season[s].nclusters;
    count[1] = (size_t) ntime[s];
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, distclustoutid[s], start, count, distclust[s]);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Write regression R^2 diagnostic */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) data->reg->npts;
    count[1] = 0;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, rsqoutid[s], start, count, data->learning->data[s].precip_reg_rsq);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Write regression autocorrelation diagnostic */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) data->reg->npts;
    count[1] = 0;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, acoroutid[s], start, count, data->learning->data[s].precip_reg_autocor);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

    /* Write regression VIF diagnostic */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = (size_t) data->conf->season[s].nclusters;
    count[1] = 0;
    count[2] = 0;
    istat = nc_put_vara_double(ncoutid, vifoutid[s], start, count, data->learning->data[s].precip_reg_vif);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

  /* Close the output netCDF file */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    
  (void) free(nomvar);

  (void) free(timedimoutid);
  (void) free(clustdimoutid);

  (void) free(timeoutid);
  (void) free(cstoutid);
  (void) free(regoutid);
  (void) free(rrdoutid);
  (void) free(distclustoutid);
  (void) free(taoutid);
  (void) free(rsqoutid);
  (void) free(acoroutid);
  (void) free(vifoutid);

  (void) free(tmpstr);

  return 0;
}
