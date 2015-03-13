/* ***************************************************** */
/* Save analog data information for further use.         */
/* save_analog_data.c                                    */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file save_analog_data.c
    \brief Save analog data information for further use.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2015)

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

/** Save analog data information for further use. */
void
save_analog_data(analog_day_struct analog_days, double *delta, double **delta_dayschoice, double *dist, int *cluster, double *time_ls,
                 char *filename, data_struct *data) {
  /**
     @param[in]   analog_days           Analog days time indexes and dates with corresponding dates being downscaled.
     @param[in]   delta                 Temperature difference to apply to analog day data.
     @param[in]   delta_dayschoice      Temperature difference to apply to analog day data, for all ndayschoice analogs.
     @param[in]   dist                  Distance to cluster associated with each downscaled/analog day.
     @param[in]   cluster               Cluster number associated with each downscaled/analog day.
     @param[in]   time_ls               Time values in udunit
     @param[in]   filename              Analog days output filename.
     @param[in]   data                  MASTER data structure.
  */

  int istat; /* Diagnostic status */

  int ncoutid; /* NetCDF output file handle ID */
  int timedimoutid; /* NetCDF time dimension output ID */
  int timeoutid; /* NetCDF time variable ID */
  int ndayschoicedimoutid; /* NetCDF ndayschoice dimension output ID */
  int ndayschoiceoutid; /* NetCDF ndayschoice variable ID */
  int analogoutid; /* NetCDF analog dates variable ID */
  int analogyearoutid; /* NetCDF analog dates variable ID */
  int analogmonthoutid; /* NetCDF analog dates variable ID */
  int analogdayoutid; /* NetCDF analog dates variable ID */
  int analogyearndaysoutid; /* NetCDF analog ndayschoice dates variable ID */
  int analogmonthndaysoutid; /* NetCDF analog ndayschoice dates variable ID */
  int analogdayndaysoutid; /* NetCDF analog ndayschoice dates variable ID */
  int metricoutid; /* NetCDF analog normalized metric variable ID */
  int downscaledyearoutid; /* NetCDF downscaled dates variable ID */
  int downscaledmonthoutid; /* NetCDF downscaled dates variable ID */
  int downscaleddayoutid; /* NetCDF downscaled dates variable ID */
  int distoutid; /* NetCDF cluster distance variable ID */
  int clusteroutid; /* NetCDF cluster number variable ID */
  int deltatoutid; /* NetCDF delta T variable ID */
  int deltatndaysoutid; /* NetCDF delta T ndayschoice variable ID */
  int vardimids[NC_MAX_VAR_DIMS]; /* NetCDF dimension IDs */
  
  int *buftmp = NULL; /* Temporary int buffer for writing data */
  float *buftmpf = NULL; /* Temporary float buffer for writing data */
  //  double *buftmpd = NULL; /* Temporary double buffer for writing data */
  int maxndays; /* Maximum number of days selected for any particular date */
    
  size_t start[2]; /* Start element when writing */
  size_t count[2]; /* Count of elements to write */  

  int fillvaluei; /* Missing value */
  float fillvaluef; /* Missing value */

  char *tmpstr = NULL; /* Temporary string */

  int t; /* Time loop counter */
  int i; /* Loop counter */

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

  /* Set dimensions */
  istat = nc_def_dim(ncoutid, "time", NC_UNLIMITED, &timedimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  
  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "time", NC_INT, 1, vardimids, &timeoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  
  istat = sprintf(tmpstr, "gregorian");
  istat = nc_put_att_text(ncoutid, timeoutid, "calendar", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = sprintf(tmpstr, "%s", data->conf->time_units);
  istat = nc_put_att_text(ncoutid, timeoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = sprintf(tmpstr, "time in %s", data->conf->time_units);
  istat = nc_put_att_text(ncoutid, timeoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Find maximum number of days in the first selection of analog days to have constant dimension size */
  maxndays = analog_days.ndayschoice[0];
  for (t=0; t<analog_days.ntime; t++)
    if (maxndays < analog_days.ndayschoice[t])
      maxndays = analog_days.ndayschoice[t];
  istat = nc_def_dim(ncoutid, "ndayschoice", maxndays, &ndayschoicedimoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "ndayschoice", NC_INT, 1, vardimids, &ndayschoiceoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = sprintf(tmpstr, "Number of analog days selected");
  istat = nc_put_att_text(ncoutid, ndayschoiceoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Set variables */

  /* Define downscaled day variable: year */
  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "downscaled_date_year", NC_INT, 1, vardimids, &downscaledyearoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluei = -1;
  istat = nc_put_att_int(ncoutid, downscaledyearoutid, "missing_value", NC_INT, 1, &fillvaluei);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time");
  istat = nc_put_att_text(ncoutid, downscaledyearoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "%s", "year");
  istat = nc_put_att_text(ncoutid, downscaledyearoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Downscaled date: year");
  istat = nc_put_att_text(ncoutid, downscaledyearoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define downscaled day variable: month */
  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "downscaled_date_month", NC_INT, 1, vardimids, &downscaledmonthoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluei = -1;
  istat = nc_put_att_int(ncoutid, downscaledmonthoutid, "missing_value", NC_INT, 1, &fillvaluei);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time");
  istat = nc_put_att_text(ncoutid, downscaledmonthoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "%s", "month");
  istat = nc_put_att_text(ncoutid, downscaledmonthoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Downscaled date: month");
  istat = nc_put_att_text(ncoutid, downscaledmonthoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define downscaled day variable: day */
  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "downscaled_date_day", NC_INT, 1, vardimids, &downscaleddayoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluei = -1;
  istat = nc_put_att_int(ncoutid, downscaleddayoutid, "missing_value", NC_INT, 1, &fillvaluei);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time");
  istat = nc_put_att_text(ncoutid, downscaleddayoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "%s", "day");
  istat = nc_put_att_text(ncoutid, downscaleddayoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Downscaled date: day");
  istat = nc_put_att_text(ncoutid, downscaleddayoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define analog day time variable */
  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "analog_date", NC_INT, 1, vardimids, &analogoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluei = -1;
  istat = nc_put_att_int(ncoutid, analogoutid, "missing_value", NC_INT, 1, &fillvaluei);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time");
  istat = nc_put_att_text(ncoutid, analogoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "%s", data->conf->time_units);
  istat = nc_put_att_text(ncoutid, analogoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Analog date");
  istat = nc_put_att_text(ncoutid, analogoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define analog day variable: year */
  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "analog_date_year", NC_INT, 1, vardimids, &analogyearoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluei = -1;
  istat = nc_put_att_int(ncoutid, analogyearoutid, "missing_value", NC_INT, 1, &fillvaluei);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time");
  istat = nc_put_att_text(ncoutid, analogyearoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "%s", "year");
  istat = nc_put_att_text(ncoutid, analogyearoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Analog date: year");
  istat = nc_put_att_text(ncoutid, analogyearoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define analog day variable: month */
  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "analog_date_month", NC_INT, 1, vardimids, &analogmonthoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluei = -1;
  istat = nc_put_att_int(ncoutid, analogmonthoutid, "missing_value", NC_INT, 1, &fillvaluei);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time");
  istat = nc_put_att_text(ncoutid, analogmonthoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "%s", "month");
  istat = nc_put_att_text(ncoutid, analogmonthoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Analog date: month");
  istat = nc_put_att_text(ncoutid, analogmonthoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define analog day variable: day */
  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "analog_date_day", NC_INT, 1, vardimids, &analogdayoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluei = -1;
  istat = nc_put_att_int(ncoutid, analogdayoutid, "missing_value", NC_INT, 1, &fillvaluei);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time");
  istat = nc_put_att_text(ncoutid, analogdayoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "%s", "day");
  istat = nc_put_att_text(ncoutid, analogdayoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Analog date: day");
  istat = nc_put_att_text(ncoutid, analogdayoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define analog delta Temperature variable */
  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "analog_delta_t", NC_FLOAT, 1, vardimids, &deltatoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluef = -9999.0;
  istat = nc_put_att_float(ncoutid, deltatoutid, "missing_value", NC_FLOAT, 1, &fillvaluef);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time");
  istat = nc_put_att_text(ncoutid, deltatoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "K");
  istat = nc_put_att_text(ncoutid, deltatoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Delta of Temperature");
  istat = nc_put_att_text(ncoutid, deltatoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  
  /* Define analog delta Temperature ndayschoice variable */
  vardimids[0] = timedimoutid;
  vardimids[1] = ndayschoicedimoutid;
  istat = nc_def_var(ncoutid, "analog_ndays_delta_t", NC_FLOAT, 2, vardimids, &deltatndaysoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluef = -9999.0;
  istat = nc_put_att_float(ncoutid, deltatndaysoutid, "missing_value", NC_FLOAT, 1, &fillvaluef);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time ndayschoice");
  istat = nc_put_att_text(ncoutid, deltatndaysoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "K");
  istat = nc_put_att_text(ncoutid, deltatndaysoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Delta of Temperature ndays");
  istat = nc_put_att_text(ncoutid, deltatndaysoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  
  /* Define cluster distance variable */
  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "cluster_distance", NC_FLOAT, 1, vardimids, &distoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluef = -9999.0;
  istat = nc_put_att_float(ncoutid, distoutid, "missing_value", NC_FLOAT, 1, &fillvaluef);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time");
  istat = nc_put_att_text(ncoutid, distoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "none");
  istat = nc_put_att_text(ncoutid, distoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Normalized distance to cluster");
  istat = nc_put_att_text(ncoutid, distoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define cluster number variable */
  vardimids[0] = timedimoutid;
  istat = nc_def_var(ncoutid, "cluster", NC_INT, 1, vardimids, &clusteroutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluei = -1;
  istat = nc_put_att_int(ncoutid, clusteroutid, "missing_value", NC_INT, 1, &fillvaluei);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time");
  istat = nc_put_att_text(ncoutid, clusteroutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "none");
  istat = nc_put_att_text(ncoutid, clusteroutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Cluster number");
  istat = nc_put_att_text(ncoutid, clusteroutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define ndayschoice analog day variable: year */
  vardimids[0] = timedimoutid;
  vardimids[1] = ndayschoicedimoutid;
  istat = nc_def_var(ncoutid, "analog_ndays_date_year", NC_INT, 2, vardimids, &analogyearndaysoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluei = 0;
  istat = nc_put_att_int(ncoutid, analogyearndaysoutid, "missing_value", NC_INT, 1, &fillvaluei);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time ndayschoice");
  istat = nc_put_att_text(ncoutid, analogyearndaysoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "%s", "year");
  istat = nc_put_att_text(ncoutid, analogyearndaysoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Analog ndays date: year");
  istat = nc_put_att_text(ncoutid, analogyearndaysoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define ndayschoice analog day variable: month */
  vardimids[0] = timedimoutid;
  vardimids[1] = ndayschoicedimoutid;
  istat = nc_def_var(ncoutid, "analog_ndays_date_month", NC_INT, 2, vardimids, &analogmonthndaysoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluei = 0;
  istat = nc_put_att_int(ncoutid, analogmonthndaysoutid, "missing_value", NC_INT, 1, &fillvaluei);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time ndayschoice");
  istat = nc_put_att_text(ncoutid, analogmonthndaysoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "%s", "month");
  istat = nc_put_att_text(ncoutid, analogmonthndaysoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Analog ndays date: month");
  istat = nc_put_att_text(ncoutid, analogmonthndaysoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define ndayschoice analog day variable: day */
  vardimids[0] = timedimoutid;
  vardimids[1] = ndayschoicedimoutid;
  istat = nc_def_var(ncoutid, "analog_ndays_date_day", NC_INT, 2, vardimids, &analogdayndaysoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluei = 0;
  istat = nc_put_att_int(ncoutid, analogdayndaysoutid, "missing_value", NC_INT, 1, &fillvaluei);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time ndayschoice");
  istat = nc_put_att_text(ncoutid, analogdayndaysoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "%s", "day");
  istat = nc_put_att_text(ncoutid, analogdayndaysoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Analog ndays date: day");
  istat = nc_put_att_text(ncoutid, analogdayndaysoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Define ndayschoice normalized metric */
  vardimids[0] = timedimoutid;
  vardimids[1] = ndayschoicedimoutid;
  istat = nc_def_var(ncoutid, "analog_metric_norm", NC_FLOAT, 2, vardimids, &metricoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  fillvaluef = 0.0;
  istat = nc_put_att_float(ncoutid, metricoutid, "missing_value", NC_FLOAT, 1, &fillvaluef);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "time ndayschoice");
  istat = nc_put_att_text(ncoutid, metricoutid, "coordinates", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) sprintf(tmpstr, "%s", "metric");
  istat = nc_put_att_text(ncoutid, metricoutid, "units", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) strcpy(tmpstr, "Analog normalized metric");
  istat = nc_put_att_text(ncoutid, metricoutid, "long_name", strlen(tmpstr), tmpstr);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* End definition mode */
  istat = nc_enddef(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Write variables */  /* Write time */
  start[0] = 0;
  start[1] = 0;
  count[0] = (size_t) analog_days.ntime;
  count[1] = 0;
  istat = nc_put_vara_double(ncoutid, timeoutid, start, count, time_ls);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Write ndayschoice */
  start[0] = 0;
  start[1] = 0;
  count[0] = (size_t) analog_days.ntime;
  count[1] = 0;
  istat = nc_put_vara_int(ncoutid, ndayschoiceoutid, start, count, analog_days.ndayschoice);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Write downscaled dates */
  start[0] = 0;
  start[1] = 0;
  count[0] = (size_t) analog_days.ntime;
  count[1] = 0;
  istat = nc_put_vara_int(ncoutid, downscaledyearoutid, start, count, analog_days.year_s);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_put_vara_int(ncoutid, downscaledmonthoutid, start, count, analog_days.month_s);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_put_vara_int(ncoutid, downscaleddayoutid, start, count, analog_days.day_s);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Write analog dates */
  start[0] = 0;
  start[1] = 0;
  count[0] = (size_t) analog_days.ntime;
  count[1] = 0;
  istat = nc_put_vara_int(ncoutid, analogoutid, start, count, analog_days.time);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_put_vara_int(ncoutid, analogyearoutid, start, count, analog_days.year);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_put_vara_int(ncoutid, analogmonthoutid, start, count, analog_days.month);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_put_vara_int(ncoutid, analogdayoutid, start, count, analog_days.day);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  
  /* Write ndayschoice analog dates */
  start[0] = 0;
  start[1] = 0;
  count[0] = (size_t) analog_days.ntime;
  count[1] = (size_t) maxndays;
  /* Build 2D array */
  buftmp = (int *) calloc(analog_days.ntime * maxndays, sizeof(int));
  if (buftmp == NULL) alloc_error(__FILE__, __LINE__);
  for (t=0; t<analog_days.ntime; t++)
    for (i=0; i<analog_days.ndayschoice[t]; i++)
      buftmp[i+t*maxndays] = analog_days.analog_dayschoice[t][i].year;
  istat = nc_put_vara_int(ncoutid, analogyearndaysoutid, start, count, buftmp);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  for (t=0; t<analog_days.ntime; t++)
    for (i=0; i<analog_days.ndayschoice[t]; i++)
      buftmp[i+t*maxndays] = analog_days.analog_dayschoice[t][i].month;
  istat = nc_put_vara_int(ncoutid, analogmonthndaysoutid, start, count, buftmp);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  for (t=0; t<analog_days.ntime; t++)
    for (i=0; i<analog_days.ndayschoice[t]; i++)
      buftmp[i+t*maxndays] = analog_days.analog_dayschoice[t][i].day;
  istat = nc_put_vara_int(ncoutid, analogdayndaysoutid, start, count, buftmp);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) free(buftmp);

  /* Write analog normalized metric */
  start[0] = 0;
  start[1] = 0;
  count[0] = (size_t) analog_days.ntime;
  count[1] = (size_t) maxndays;
  buftmpf = (float *) calloc(analog_days.ntime * maxndays, sizeof(float));
  if (buftmpf == NULL) alloc_error(__FILE__, __LINE__);
  for (t=0; t<analog_days.ntime; t++)
    for (i=0; i<analog_days.ndayschoice[t]; i++)
      buftmpf[i+t*maxndays] = analog_days.metric_norm[t][i];
  istat = nc_put_vara_float(ncoutid, metricoutid, start, count, buftmpf);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) free(buftmpf);
 
  /* Write delta of temperature */
  start[0] = 0;
  start[1] = 0;
  count[0] = (size_t) analog_days.ntime;
  count[1] = 0;
  istat = nc_put_vara_double(ncoutid, deltatoutid, start, count, delta);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Write ndayschoice delta of temperature */
  start[0] = 0;
  start[1] = 0;
  count[0] = (size_t) analog_days.ntime;
  count[1] = (size_t) maxndays;
  buftmpf = (float *) calloc(analog_days.ntime * maxndays, sizeof(float));
  if (buftmpf == NULL) alloc_error(__FILE__, __LINE__);
  for (t=0; t<analog_days.ntime; t++)
    for (i=0; i<analog_days.ndayschoice[t]; i++)
      buftmpf[i+t*maxndays] = (float) delta_dayschoice[t][i];
  istat = nc_put_vara_float(ncoutid, deltatndaysoutid, start, count, buftmpf);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  (void) free(buftmpf);

  /* Write cluster distance */
  start[0] = 0;
  start[1] = 0;
  count[0] = (size_t) analog_days.ntime;
  count[1] = 0;
  istat = nc_put_vara_double(ncoutid, distoutid, start, count, dist);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  
  /* Write cluster number */
  start[0] = 0;
  start[1] = 0;
  count[0] = (size_t) analog_days.ntime;
  count[1] = 0;
  istat = nc_put_vara_int(ncoutid, clusteroutid, start, count, cluster);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  
  /* Close the output netCDF file */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  (void) free(tmpstr);
}
