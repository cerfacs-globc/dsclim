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

int read_large_scale_eof(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat;
  int i;
  int t;
  int ntime;
  double *buf = NULL;
  double *lat = NULL;
  double *lon = NULL;
  int nlon;
  int nlat;
  proj_struct proj_eof;
  info_field_struct info_field_eof;
  int cat;
  int j;
  int ii,jj;

  for (cat=0; cat<2; cat++)
    for (i=0; i<data->field[cat].n_ls; i++) {

      if (data->field[cat].data[i].eof_info->eof_project == 1) {
      
        if (data->field[cat].lon_eof_ls == NULL) {
          /* Read dimensions */
          istat = read_netcdf_dims_eof(&lon, &lat, &nlon, &nlat, &(data->field[cat].data[i].eof_info->neof_ls),
                                       data->field[cat].data[i].eof_info->eof_coords,
                                       data->conf->lonname_eof, data->conf->latname_eof, data->conf->eofname,
                                       data->field[cat].data[i].eof_info->eof_filein_ls);
          if (istat != 0) {
            (void) free(lon);
            (void) free(lat);
            return istat;
          }
        
          (void) memcpy(&proj_eof, &(data->field[cat].proj[i]), sizeof(proj_struct));
          proj_eof.eof_coords = strdup(data->field[cat].data[i].eof_info->eof_coords);
        }
      
        /* Read EOF */
        istat = read_netcdf_var_3d(&buf, data->field[cat].data[i].eof_info->info, &proj_eof,
                                   data->field[cat].data[i].eof_info->eof_filein_ls,
                                   data->field[cat].data[i].eof_data->eof_nomvar_ls,
                                   data->conf->lonname_eof, data->conf->latname_eof, data->conf->eofname,
                                   nlon, nlat, data->field[cat].data[i].eof_info->neof_ls);
        if (istat != 0) {
          (void) free(buf);
          (void) free(lon);
          (void) free(lat);
          return istat;
        }
      
        /* Extract subdomain of spatial fields */
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
        (void) extract_subdomain(&(data->field[cat].data[i].eof_data->eof_ls), &(data->field[cat].lon_eof_ls),
                                 &(data->field[cat].lat_eof_ls),
                                 &(data->field[cat].nlon_eof_ls), &(data->field[cat].nlat_eof_ls), buf, lon, lat,
                                 data->conf->longitude_min, data->conf->longitude_max, data->conf->latitude_min, data->conf->latitude_max,
                                 nlon, nlat, data->field[cat].data[i].eof_info->neof_ls);
        (void) free(buf);
        
        /* Print fillvalue */
        printf("%s: EOF missing_value = %lf\n", __FILE__, (double) data->field[cat].data[i].eof_info->info->fillvalue);
      
        /* Read Singular Values */
        istat = read_netcdf_var_1d(&(data->field[cat].data[i].eof_data->sing_ls), &info_field_eof,
                                   data->field[cat].data[i].eof_info->eof_filein_ls, data->field[cat].data[i].eof_data->sing_nomvar_ls,
                                   data->conf->eofname, data->field[cat].data[i].eof_info->neof_ls);
        if (istat != 0) {
          (void) free(lon);
          (void) free(lat);
          return istat;
        }
      }
    }
    
  if (lon != NULL) (void) free(lon);
  if (lat != NULL) (void) free(lat);

  return 0;
}
