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

short int read_large_scale_eof(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  short int istat;
  int i;
  int t;
  int ntime;
  double *buf = NULL;
  double *lat = NULL;
  double *lon = NULL;
  int nlon;
  int nlat;

  for (i=0; i<data->field->n_ls; i++) {

    if (data->field->lon_eof_ls == NULL) {
      /* Read dimensions */
      istat = read_netcdf_dims_eof(&lon, &lat, &nlon, &nlat, &(data->field->neof_ls),
                                   data->field->proj[i].eof_coords, data->conf->lonname_eof, data->conf->latname_eof, data->conf->eofname,
                                   data->field->eof_filein_ls[i]);
      if (istat != 0) {
        (void) free(lon);
        (void) free(lat);
        return istat;
      }
    }

    /* Read EOF */
    istat = read_netcdf_var_3d(&buf, &(data->field->info_field[i]), &(data->field->proj[i]),
                               data->field->eof_filein_ls[i],
                               data->field->eof_nomvar_ls[i], data->conf->lonname_eof, data->conf->latname_eof, data->conf->eofname,
                               nlon, nlat, data->field->neof_ls);
    if (istat != 0) {
      (void) free(buf);
      (void) free(lon);
      (void) free(lat);
      return istat;
    }

    /* Extract subdomain of spatial fields */
    if (data->field->lon_eof_ls != NULL) {
      (void) free(data->field->lon_eof_ls);
      data->field->lon_eof_ls = NULL;
    }
    if (data->field->lat_eof_ls != NULL) {
      (void) free(data->field->lat_eof_ls);
      data->field->lat_eof_ls = NULL;
    }
    if (data->field->eof_ls[i] != NULL) {
      (void) free(data->field->eof_ls[i]);
      data->field->eof_ls[i] = NULL;
    }
    (void) extract_subdomain(&(data->field->eof_ls[i]), &(data->field->lon_eof_ls), &(data->field->lat_eof_ls),
                             &(data->field->nlon_ls), &(data->field->nlat_ls), buf, lon, lat,
                             data->conf->longitude_min, data->conf->longitude_max, data->conf->latitude_min, data->conf->latitude_max,
                             nlon, nlat, data->field->neof_ls);
    (void) free(buf);

    /* Read Singular Values */
    istat = read_netcdf_var_1d(&(data->field->sing_ls[i]), &(data->field->info_field[i]),
                               data->field->eof_filein_ls[i], data->field->sing_nomvar_ls[i],
                               data->conf->eofname, data->field->neof_ls);
    if (istat != 0) {
      (void) free(lon);
      (void) free(lat);
      return istat;
    }
    
  }
  
  (void) free(lon);
  (void) free(lat);

  return 0;
}
