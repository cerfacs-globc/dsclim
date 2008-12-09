/* ***************************************************** */
/* read_large_scale_eof Read large scale EOFs.           */
/* read_large_scale_eof.c                                */
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
/*! \file read_large_scale_eof.c
    \brief Read Large-Scale EOFs from input files. Currently only NetCDF is implemented.
*/

#include <dsclim.h>

/** Read Large-Scale EOFs from input files. Currently only NetCDF is implemented. */
int read_large_scale_eof(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat; /* Diagnostic status */
  int i; /* Loop counter */
  int cat; /* Field category loop counter */
  double *buf = NULL; /* Temporary buffer for reading */
  double *lat = NULL; /* Temporary buffer for latitudes */
  double *lon = NULL; /* Temporary buffer for longitudes */
  int nlon; /* Longitude dimension */
  int nlat; /* Latitude dimension */
  int nlon_file; /* Longitude dimension for main large-scale fields in input file */
  int nlat_file; /* Latitude dimension for main large-scale fields in input file */
  int neof_file; /* EOF dimension for main large-scale fields in input file */
  proj_struct proj_eof; /* EOF spatial projection structure */

  proj_eof.eof_coords = (char *) NULL;
  proj_eof.name = (char *) NULL;

  /* Loop over large-scale field categories (control and model run) */
  for (cat=0; cat<2; cat++)
    /* Loop over large-scale fields */
    for (i=0; i<data->field[cat].n_ls; i++) {

      /* Verify that we need to project field onto EOF */
      if (data->field[cat].data[i].eof_info->eof_project == 1) {
      
        if (data->field[cat].lon_eof_ls == NULL) {
          /* Read dimensions for EOF */
          istat = read_netcdf_dims_eof(&lon, &lat, &nlon, &nlat, &(data->field[cat].data[i].eof_info->neof_ls),
                                       data->field[cat].data[i].eof_info->eof_coords,
                                       data->conf->lonname_eof, data->conf->latname_eof, data->conf->eofname,
                                       data->field[cat].data[i].eof_info->eof_filein_ls);
          if (istat != 0) {
            /* In case of failure */
            (void) free(lon);
            (void) free(lat);
            return istat;
          }
        
          /* Transfer data into proper data structure */
          (void) memcpy(&proj_eof, &(data->field[cat].proj[i]), sizeof(proj_struct));
          proj_eof.eof_coords = strdup(data->field[cat].data[i].eof_info->eof_coords);
        }
      
        /* Read EOF */
        istat = read_netcdf_var_3d(&buf, data->field[cat].data[i].eof_info->info, &proj_eof,
                                   data->field[cat].data[i].eof_info->eof_filein_ls,
                                   data->field[cat].data[i].eof_data->eof_nomvar_ls,
                                   data->conf->lonname_eof, data->conf->latname_eof, data->conf->eofname,
                                   &nlon_file, &nlat_file, &neof_file, TRUE);
        if (nlon != nlon_file || nlat != nlat_file || data->field[cat].data[i].eof_info->neof_ls != neof_file) {
          (void) fprintf(stderr, "%s: Problems in dimensions! nlat=%d nlat_file=%d nlon=%d nlon_file=%d neof=%d neof_file=%d\n",
                         __FILE__, nlat, nlat_file, nlon, nlon_file, data->field[cat].data[i].eof_info->neof_ls, neof_file);
          istat = -1;
        }
        if (istat != 0) {
          /* In case of failure */
          (void) free(buf);
          (void) free(lon);
          (void) free(lat);
          if (proj_eof.eof_coords != NULL)
            (void) free(proj_eof.eof_coords);
          if (proj_eof.name != NULL)
            (void) free(proj_eof.name);
          return istat;
        }
      
        /** Extract subdomain **/
        /* Free memory if needed because of loop. Set pointers to NULL for realloc. */
        if (data->field[cat].lon_eof_ls != NULL) {
          (void) free(data->field[cat].lon_eof_ls);
          data->field[cat].lon_eof_ls = NULL;
        }
        if (data->field[cat].lat_eof_ls != NULL) {
          (void) free(data->field[cat].lat_eof_ls);
          data->field[cat].lat_eof_ls = NULL;
        }
        if (data->field[cat].data[i].eof_data->eof_ls != NULL) {
          (void) free(data->field[cat].data[i].eof_data->eof_ls);
          data->field[cat].data[i].eof_data->eof_ls = NULL;
        }
        /* Extraction */
        (void) extract_subdomain(&(data->field[cat].data[i].eof_data->eof_ls), &(data->field[cat].lon_eof_ls),
                                 &(data->field[cat].lat_eof_ls),
                                 &(data->field[cat].nlon_eof_ls), &(data->field[cat].nlat_eof_ls), buf, lon, lat,
                                 data->conf->longitude_min, data->conf->longitude_max, data->conf->latitude_min, data->conf->latitude_max,
                                 nlon, nlat, data->field[cat].data[i].eof_info->neof_ls);
        (void) free(buf);
        
        /* Print missing value */
        printf("%s: EOF missing_value = %lf\n", __FILE__, (double) data->field[cat].data[i].eof_info->info->fillvalue);
      
        /* Read Singular Values */
        istat = read_netcdf_var_1d(&(data->field[cat].data[i].eof_data->sing_ls), (info_field_struct *) NULL,
                                   data->field[cat].data[i].eof_info->eof_filein_ls, data->field[cat].data[i].eof_data->sing_nomvar_ls,
                                   data->conf->eofname, &neof_file, TRUE);
        if (data->field[cat].data[i].eof_info->neof_ls != neof_file) {
          (void) fprintf(stderr, "%s: Problems in dimensions! neof=%d neof_file=%d\n",
                         __FILE__, data->field[cat].data[i].eof_info->neof_ls, neof_file);
          istat = -1;
        }
        if (istat != 0) {
          /* In case of failure */
          (void) free(lon);
          (void) free(lat);
          if (proj_eof.eof_coords != NULL)
            (void) free(proj_eof.eof_coords);
          if (proj_eof.name != NULL)
            (void) free(proj_eof.name);
          return istat;
        }
      }
      /* Free memory if needed */
      if (lon != NULL) (void) free(lon);
      lon = NULL;
      if (lat != NULL) (void) free(lat);
      lat = NULL;

      if (proj_eof.eof_coords != NULL) {
        (void) free(proj_eof.eof_coords);
        proj_eof.eof_coords = (char *) NULL;
      }
      if (proj_eof.name != NULL) {
        (void) free(proj_eof.name);
        proj_eof.name = (char *) NULL;
      }
    }
  
  /* Diagnostic status */
  return 0;
}
