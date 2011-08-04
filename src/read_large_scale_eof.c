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

/** Read Large-Scale EOFs from input files. Currently only NetCDF is implemented. */
int
read_large_scale_eof(data_struct *data) {
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
  proj_eof.grid_mapping_name = (char *) NULL;

  /* Loop over large-scale field categories (control and model run) */
  for (cat=0; cat<2; cat++) {
    /* Loop over large-scale fields */
    for (i=0; i<data->field[cat].n_ls; i++) {

      /* Verify that we need to project field onto EOF */
      if (data->field[cat].data[i].eof_info->eof_project == TRUE) {
      
        if (data->field[cat].lon_eof_ls == NULL) {
          /* Read dimensions for EOF */
          istat = read_netcdf_dims_eof(&lon, &lat, &nlon, &nlat, &neof_file,
                                       data->field[cat].data[i].eof_info->eof_coords,
                                       data->conf->lonname_eof, data->conf->latname_eof,
                                       data->conf->dimxname_eof, data->conf->dimyname_eof,
                                       data->conf->eofname,
                                       data->field[cat].data[i].eof_info->eof_filein_ls);
          if (istat < 0) {
            /* In case of failure */
            (void) free(lon);
            (void) free(lat);
            return istat;
          }
        
          /* Transfer data into proper data structure */
          proj_eof.eof_coords = strdup(data->field[cat].proj[i].coords);
          proj_eof.name = strdup(data->field[cat].proj[i].name);
          proj_eof.grid_mapping_name = strdup(data->field[cat].proj[i].grid_mapping_name);
        }
      
        /* Read EOF */
        istat = read_netcdf_var_3d(&buf, data->field[cat].data[i].eof_info->info, &proj_eof,
                                   data->field[cat].data[i].eof_info->eof_filein_ls,
                                   data->field[cat].data[i].eof_data->eof_nomvar_ls,
                                   data->conf->dimxname_eof, data->conf->dimyname_eof,
                                   data->conf->eofname, &nlon_file, &nlat_file, &neof_file, TRUE);
        if (nlon != nlon_file || nlat != nlat_file) {
          (void) fprintf(stderr, "%s: Problems in dimensions! nlat=%d nlat_file=%d nlon=%d nlon_file=%d\n",
                         __FILE__, nlat, nlat_file, nlon, nlon_file);
          istat = -1;
        }
        if (data->field[cat].data[i].eof_info->neof_ls != neof_file) {
          (void) fprintf(stderr, "%s: ERROR: Number of EOFs (%d) for %s field from large-scale fields file (%s) is not equal to number of EOFs specified in XML configuration file for large-scale fields (%d)!\n", __FILE__, neof_file,
                         data->field[cat].data[i].eof_data->eof_nomvar_ls,
                         data->field[cat].data[i].eof_info->eof_filein_ls,
                         data->field[cat].data[i].eof_info->neof_ls);
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
          if (proj_eof.grid_mapping_name != NULL)
            (void) free(proj_eof.grid_mapping_name);
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
          (void) fprintf(stderr, "%s: ERROR: Number of EOFs (%d) for %s singular values from large-scale fields file (%s) is not equal to number of EOFs specified in XML configuration file for large-scale fields (%d)!\n", __FILE__, neof_file,
                          data->field[cat].data[i].eof_data->sing_nomvar_ls, data->field[cat].data[i].eof_info->eof_filein_ls,
                         data->field[cat].data[i].eof_info->neof_ls);
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
          if (proj_eof.grid_mapping_name != NULL)
            (void) free(proj_eof.grid_mapping_name);
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
      if (proj_eof.grid_mapping_name != NULL) {
        (void) free(proj_eof.grid_mapping_name);
        proj_eof.grid_mapping_name = NULL;
      }
    }
  }
  
  /* Diagnostic status */
  return 0;
}
