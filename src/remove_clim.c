/* ***************************************************** */
/* remove_clim Remove climatology.                       */
/* remove_clim.c                                         */
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
/*! \file remove_clim.c
    \brief Remove climatologies.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2011)

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

/** Remove climatologies. */
int
remove_clim(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  double *bufnoclim = NULL; /* Temporary buffer for field with climatology removed */
  double **clim = NULL; /* Climatology buffer */
  tstruct *timein_ts = NULL; /* Time info for input field */
  int ntime_clim; /* Number of times for input field */
  int nlon_file; /* Longitude dimension for input field */
  int nlat_file; /* Latitude dimension for input field */
  int ntime_file; /* Time dimension for input field */

  double fillvalue; /* Missing value */

  int istat = 0; /* Diagnostic status */
  int i; /* Loop counter */
  int j; /* Loop counter */
  int cat; /* Loop counter for field category */
  int ii; /* Loop counter */
  info_field_struct clim_info_field; /* Information structure for climatology field */
  double *timeclim = NULL; /* Time info for climatology field */

  /* Remove seasonal cycle:
     - Fix calendar and generate a gregorian calendar
     - Compute climatology including Feb 29th
     - Filter climatology
     - Optionally save climatology in file */

  /* Climatological year is 366 days */
  ntime_clim = 366;

  /* Time variable for climatological year: day timestep */
  timeclim = (double *) malloc(ntime_clim * sizeof(double));
  if (timeclim == NULL) alloc_error(__FILE__, __LINE__);
  for (j=0; j<ntime_clim; j++)
    timeclim[j] = (double) (j+1);

  /* Climatology variable */
  clim = (double **) malloc(NCAT * sizeof(double *));
  if (clim == NULL) alloc_error(__FILE__, __LINE__);
  for (cat=0; cat<NCAT; cat++)
    clim[cat] = NULL;

  /* Loop over all control-run large-scale field categories to process */
  /* Always remove climatology from the control run and apply to corresponding fields for other downscaled runs */
  for (cat=1; cat<NCAT; cat=cat+2) {

    /* Loop over all large-scale fields */
    for (i=0; i<data->field[cat].n_ls; i++) {

      /* Allocate memory for field with climatology removed */
      bufnoclim = (double *) malloc(data->field[cat].nlon_ls * data->field[cat].nlat_ls * data->field[cat].ntime_ls * sizeof(double));
      if (bufnoclim == NULL) alloc_error(__FILE__, __LINE__);

      /* Allocate memory for temporary time structure */
      timein_ts = (tstruct *) malloc(data->field[cat].ntime_ls * sizeof(tstruct));
      if (timein_ts == NULL) alloc_error(__FILE__, __LINE__);
      /* Get time info and calendar units */
      istat = get_calendar_ts(timein_ts, data->conf->time_units, data->field[cat].time_ls, data->field[cat].ntime_ls);
      if (istat < 0) {
        (void) free(timein_ts);
        (void) free(bufnoclim);
        (void) free(timeclim);
        return -1;
      }

      /* If we need to remove climatology for that field */
      if (data->field[cat].data[i].clim_info->clim_remove == TRUE) {
        /* If climatology field is already provided */
        if (data->field[cat].data[i].clim_info->clim_provided == TRUE) {
          /* Read climatology from NetCDF file */
          istat = read_netcdf_var_3d(&(clim[cat]), &clim_info_field, (proj_struct *) NULL,
                                     data->field[cat].data[i].clim_info->clim_filein_ls,
                                     data->field[cat].data[i].clim_info->clim_nomvar_ls,
                                     data->field[cat].data[i].dimxname, data->field[cat].data[i].dimyname,
                                     data->field[cat].data[i].timename,
                                     &nlon_file, &nlat_file, &ntime_file, TRUE);
          if (data->field[cat].nlon_ls != nlon_file || data->field[cat].nlat_ls != nlat_file || ntime_clim != ntime_file) {
            (void) fprintf(stderr, "%s: Problems in dimensions! nlat=%d nlat_file=%d nlon=%d nlon_file=%d ntime=%d ntime_file=%d\n",
                           __FILE__, data->field[cat].nlat_ls, nlat_file, data->field[cat].nlon_ls, nlon_file, ntime_clim, ntime_file);
            istat = -1;
          }
          if (istat != 0) {
            /* In case of error in reading data */
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim[cat] != NULL) (void) free(clim[cat]);
            return istat;
          }
          /* Get missing value */
          fillvalue = clim_info_field.fillvalue;
          /* Free memory */
          (void) free(clim_info_field.height);
          (void) free(clim_info_field.coordinates);
          (void) free(clim_info_field.grid_mapping);
          (void) free(clim_info_field.units);
          (void) free(clim_info_field.long_name);
        }
        else {
          /* Climatology is not provided: must calculate */
          if (clim[cat] == NULL) {
            /* Allocate memory if not already */
            clim[cat] = (double *) malloc(data->field[cat].nlon_ls * data->field[cat].nlat_ls * ntime_clim * sizeof(double));
            if (clim[cat] == NULL) alloc_error(__FILE__, __LINE__);
          }
          /* Get missing value */
          fillvalue = data->field[cat].data[i].info->fillvalue;
        }
      
        /* Remove seasonal cycle by calculating filtered climatology and substracting from field values */
        (void) remove_seasonal_cycle(bufnoclim, clim[cat], data->field[cat].data[i].field_ls, timein_ts,
                                     data->field[cat].data[i].info->fillvalue,
                                     data->conf->clim_filter_width, data->conf->clim_filter_type,
                                     data->field[cat].data[i].clim_info->clim_provided,
                                     data->field[cat].nlon_ls, data->field[cat].nlat_ls, data->field[cat].ntime_ls);
      
        /* If we want to save climatology in NetCDF output file for further use */
        if (data->field[cat].data[i].clim_info->clim_save == TRUE) {
          istat = create_netcdf("Computed climatology", "Climatologie calculee", "Computed climatology", "Climatologie calculee",
                                "climatologie,climatology", "C language", data->info->software,
                                "Computed climatology", data->info->institution,
                                data->info->creator_email, data->info->creator_url, data->info->creator_name,
                                data->info->version, data->info->scenario, data->info->scenario_co2, data->info->model,
                                data->info->institution_model, data->info->country, data->info->member,
                                data->info->downscaling_forcing, data->info->contact_email, data->info->contact_name,
                                data->info->other_contact_email, data->info->other_contact_name,
                                data->field[cat].data[i].clim_info->clim_fileout_ls, TRUE, data->conf->format, data->conf->compression);
          if (istat != 0) {
            /* In case of failure */
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim[cat] != NULL) (void) free(clim[cat]);
            return istat;
          }
          /* Write dimensions of climatology field in NetCDF output file */
          istat = write_netcdf_dims_3d(data->field[cat].lon_ls, data->field[cat].lat_ls, (double *) NULL, (double *) NULL,
                                       (double *) NULL, timeclim, data->conf->cal_type,
                                       data->conf->time_units, data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime_clim,
                                       "daily", data->field[cat].proj[i].name, data->field[cat].proj[i].coords,
                                       data->field[cat].proj[i].grid_mapping_name, data->field[cat].proj[i].latin1,
                                       data->field[cat].proj[i].latin2, data->field[cat].proj[i].lonc, data->field[cat].proj[i].lat0,
                                       data->field[cat].proj[i].false_easting, data->field[cat].proj[i].false_northing,
                                       data->field[cat].data[i].lonname, data->field[cat].data[i].latname,
                                       data->field[cat].data[i].timename,
                                       data->field[cat].data[i].clim_info->clim_fileout_ls, TRUE);
          if (istat != 0) {
            /* In case of failure */
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim[cat] != NULL) (void) free(clim[cat]);
            return istat;
          }
        
          /* Write climatology field in NetCDF output file */
          istat = write_netcdf_var_3d(clim[cat], fillvalue, data->field[cat].data[i].clim_info->clim_fileout_ls,
                                      data->field[cat].data[i].clim_info->clim_nomvar_ls, data->field[cat].proj[i].name,
                                      data->field[cat].data[i].lonname, data->field[cat].data[i].latname,
                                      data->field[cat].data[i].timename,
                                      data->conf->format, data->conf->compression_level,
                                      data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime_clim, TRUE);
          if (istat != 0) {
            /* In case of failure */
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim[cat] != NULL) (void) free(clim[cat]);
            return istat;
          }
        }

        /* Copy field with climatology removed to proper variable in data structure */
        for (ii=0; ii<(data->field[cat].nlon_ls * data->field[cat].nlat_ls * data->field[cat].ntime_ls); ii++)
          data->field[cat].data[i].field_ls[ii] = bufnoclim[ii];
      }
      /* Free memory */
      (void) free(bufnoclim);
      (void) free(timein_ts);
    }
  }

  /* Loop over all non-control-run large-scale field categories to process */
  /* Always remove climatology calculated with the control run and apply to corresponding fields for other downscaled runs */
  for (cat=0; cat<NCAT; cat=cat+2) {

    /* Loop over all large-scale fields */
    for (i=0; i<data->field[cat].n_ls; i++) {

      /* Allocate memory for field with climatology removed */
      bufnoclim = (double *) malloc(data->field[cat].nlon_ls * data->field[cat].nlat_ls * data->field[cat].ntime_ls * sizeof(double));
      if (bufnoclim == NULL) alloc_error(__FILE__, __LINE__);

      /* Allocate memory for temporary time structure */
      timein_ts = (tstruct *) malloc(data->field[cat].ntime_ls * sizeof(tstruct));
      if (timein_ts == NULL) alloc_error(__FILE__, __LINE__);
      /* Get time info and calendar units */
      istat = get_calendar_ts(timein_ts, data->conf->time_units, data->field[cat].time_ls, data->field[cat].ntime_ls);
      if (istat < 0) {
        (void) free(timein_ts);
        (void) free(bufnoclim);
        (void) free(timeclim);
        return -1;
      }

      /* If we need to remove climatology for that field */
      if (data->field[cat].data[i].clim_info->clim_remove == TRUE) {
        /* If climatology field is already provided */
        if (data->field[cat].data[i].clim_info->clim_provided == TRUE) {
          /* Read climatology from NetCDF file */
          istat = read_netcdf_var_3d(&(clim[cat]), &clim_info_field, (proj_struct *) NULL,
                                     data->field[cat].data[i].clim_info->clim_filein_ls,
                                     data->field[cat].data[i].clim_info->clim_nomvar_ls,
                                     data->field[cat].data[i].dimxname, data->field[cat].data[i].dimyname,
                                     data->field[cat].data[i].timename,
                                     &nlon_file, &nlat_file, &ntime_file, TRUE);
          if (data->field[cat].nlon_ls != nlon_file || data->field[cat].nlat_ls != nlat_file || ntime_clim != ntime_file) {
            (void) fprintf(stderr, "%s: Problems in dimensions! nlat=%d nlat_file=%d nlon=%d nlon_file=%d ntime=%d ntime_file=%d\n",
                           __FILE__, data->field[cat].nlat_ls, nlat_file, data->field[cat].nlon_ls, nlon_file, ntime_clim, ntime_file);
            istat = -1;
          }
          if (istat != 0) {
            /* In case of error in reading data */
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim[cat] != NULL) (void) free(clim[cat]);
            return istat;
          }
          /* Get missing value */
          fillvalue = clim_info_field.fillvalue;
          /* Free memory */
          (void) free(clim_info_field.height);
          (void) free(clim_info_field.coordinates);
          (void) free(clim_info_field.grid_mapping);
          (void) free(clim_info_field.units);
          (void) free(clim_info_field.long_name);
        }
        else {
          /* Climatology is not provided: must use the one calculated with control-run data */
          /* Get missing value */
          fillvalue = data->field[cat].data[i].info->fillvalue;
        }
      
        /* Remove seasonal cycle by substracting control-run climatology from field values (not the clim[cat+1] */
        (void) remove_seasonal_cycle(bufnoclim, clim[cat+1], data->field[cat].data[i].field_ls, timein_ts,
                                     data->field[cat].data[i].info->fillvalue,
                                     data->conf->clim_filter_width, data->conf->clim_filter_type,
                                     TRUE,
                                     data->field[cat].nlon_ls, data->field[cat].nlat_ls, data->field[cat].ntime_ls);
      
        /* If we want to save climatology in NetCDF output file for further use */
        if (data->field[cat].data[i].clim_info->clim_save == TRUE) {
          istat = create_netcdf("Computed climatology", "Climatologie calculee", "Computed climatology", "Climatologie calculee",
                                "climatologie,climatology", "C language", data->info->software,
                                "Computed climatology", data->info->institution,
                                data->info->creator_email, data->info->creator_url, data->info->creator_name,
                                data->info->version, data->info->scenario, data->info->scenario_co2, data->info->model,
                                data->info->institution_model, data->info->country, data->info->member,
                                data->info->downscaling_forcing, data->info->contact_email, data->info->contact_name,
                                data->info->other_contact_email, data->info->other_contact_name,
                                data->field[cat].data[i].clim_info->clim_fileout_ls, TRUE, data->conf->format, data->conf->compression);
          if (istat != 0) {
            /* In case of failure */
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim[cat+1] != NULL) (void) free(clim[cat+1]);
            return istat;
          }
          /* Write dimensions of climatology field in NetCDF output file */
          istat = write_netcdf_dims_3d(data->field[cat].lon_ls, data->field[cat].lat_ls, (double *) NULL, (double *) NULL,
                                       (double *) NULL, timeclim, data->conf->cal_type,
                                       data->conf->time_units, data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime_clim,
                                       "daily", data->field[cat].proj[i].name, data->field[cat].proj[i].coords,
                                       data->field[cat].proj[i].grid_mapping_name, data->field[cat].proj[i].latin1,
                                       data->field[cat].proj[i].latin2, data->field[cat].proj[i].lonc, data->field[cat].proj[i].lat0,
                                       data->field[cat].proj[i].false_easting, data->field[cat].proj[i].false_northing,
                                       data->field[cat].data[i].lonname, data->field[cat].data[i].latname,
                                       data->field[cat].data[i].timename,
                                       data->field[cat].data[i].clim_info->clim_fileout_ls, TRUE);
          if (istat != 0) {
            /* In case of failure */
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim[cat+1] != NULL) (void) free(clim[cat+1]);
            return istat;
          }
        
          /* Write climatology field in NetCDF output file */
          istat = write_netcdf_var_3d(clim[cat+1], fillvalue, data->field[cat].data[i].clim_info->clim_fileout_ls,
                                      data->field[cat].data[i].clim_info->clim_nomvar_ls, data->field[cat].proj[i].name,
                                      data->field[cat].data[i].lonname, data->field[cat].data[i].latname,
                                      data->field[cat].data[i].timename,
                                      data->conf->format, data->conf->compression_level,
                                      data->field[cat].nlon_ls, data->field[cat].nlat_ls, ntime_clim, TRUE);
          if (istat != 0) {
            /* In case of failure */
            (void) free(bufnoclim);
            (void) free(timein_ts);
            (void) free(timeclim);
            if (clim[cat+1] != NULL) (void) free(clim[cat+1]);
            return istat;
          }
        }

        /* Copy field with climatology removed to proper variable in data structure */
        for (ii=0; ii<(data->field[cat].nlon_ls * data->field[cat].nlat_ls * data->field[cat].ntime_ls); ii++)
          data->field[cat].data[i].field_ls[ii] = bufnoclim[ii];
      }
      /* Free memory */
      (void) free(bufnoclim);
      (void) free(timein_ts);
    }
  }

  /* Free memory */
  (void) free(timeclim);
  for (cat=0; cat<NCAT; cat++)
    if (clim[cat] != NULL) (void) free(clim[cat]);
  (void) free(clim);

  /* Success status */
  return 0;
}
