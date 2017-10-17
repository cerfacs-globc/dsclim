/* ***************************************************** */
/* read_netcdf_var_generic_val Read a 1D NetCDF variable.*/
/* read_netcdf_var_generic_val.c                         */
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
/*! \file read_netcdf_var_generic_val.c
    \brief Read a NetCDF variable.
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







#include <io.h>

/** Read a NetCDF variable scalar double at the index position, and return information in info_field_struct structure. */
int
read_netcdf_var_generic_val(double *buf, info_field_struct *info_field, char *filename, char *varname, int index) {
  /**
     @param[out]  buf        Scalar double value
     @param[out]  info_field Information about the output variable
     @param[in]   filename   NetCDF input filename
     @param[in]   varname    NetCDF variable name
     @param[in]   index      Index position in the variable to retrieve
     
     \return           Status.
  */

  int istat; /* Diagnostic status */

  int ncinid; /* NetCDF input file handle ID */
  int varinid; /* NetCDF variable ID */
  nc_type vartype_main; /* Type of the variable (NC_FLOAT, NC_DOUBLE, etc.) */
  int varndims; /* Number of dimensions of variable */
  int vardimids[NC_MAX_VAR_DIMS]; /* Variable dimension ids */

  float valf; /* Variable used to retrieve fillvalue */
  char *tmpstr = NULL; /* Temporary string */
  size_t t_len; /* Length of string attribute */
  size_t *idx; /* Index position */

  /* Allocate memory and set index to input parameter value */
  idx = (size_t *) malloc(sizeof(size_t));
  if (idx == NULL) alloc_error(__FILE__, __LINE__);
  idx[0] = (size_t) index;

  /* Allocate memory */
  tmpstr = (char *) malloc(MAXPATH * sizeof(char));
  if (tmpstr == NULL) alloc_error(__FILE__, __LINE__);

  /* Read data in NetCDF file */

  /* Open NetCDF file for reading */
  printf("%s: Opening for reading NetCDF input file %s\n", __FILE__, filename);
  istat = nc_open(filename, NC_NOWRITE, &ncinid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  printf("%s: READ %s %s\n", __FILE__, varname, filename);

  /* Get main variable ID */
  istat = nc_inq_varid(ncinid, varname, &varinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Get variable information */
  istat = nc_inq_var(ncinid, varinid, (char *) NULL, &vartype_main, &varndims, vardimids, (int *) NULL);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* If info_field si not NULL, get some information about the read variable */
  if (info_field != NULL) {
    /* Get missing value */
    if (vartype_main == NC_FLOAT) {
      istat = nc_get_att_float(ncinid, varinid, "missing_value", &valf);
      if (istat != NC_NOERR) {
        istat = nc_get_att_float(ncinid, varinid, "_FillValue", &valf);
        if (istat != NC_NOERR)
          info_field->fillvalue = -9999.0;
      }
      else
        info_field->fillvalue = (double) valf;
    }
    else if (vartype_main == NC_DOUBLE) {
      istat = nc_get_att_double(ncinid, varinid, "missing_value", &(info_field->fillvalue));
      if (istat != NC_NOERR) {
        istat = nc_get_att_double(ncinid, varinid, "_FillValue", &(info_field->fillvalue));
        if (istat != NC_NOERR)
          info_field->fillvalue = -9999.0;
      }
    }

    /* Get units */
    istat = nc_inq_attlen(ncinid, varinid, "units", &t_len);
    if (istat == NC_NOERR) {
      handle_netcdf_error(istat, __FILE__, __LINE__);
      istat = nc_get_att_text(ncinid, varinid, "units", tmpstr);
      if (istat == NC_NOERR) {
        if (tmpstr[t_len-1] != '\0')
          tmpstr[t_len] = '\0';
        info_field->units = strdup(tmpstr);
      }
      else
        info_field->units = strdup("unknown");
    }
    else
      info_field->units = strdup("unknown");

    /* Get height */
    istat = nc_inq_attlen(ncinid, varinid, "height", &t_len);
    if (istat == NC_NOERR) {
      handle_netcdf_error(istat, __FILE__, __LINE__);
      istat = nc_get_att_text(ncinid, varinid, "height", tmpstr);
      if (istat == NC_NOERR) {
        if (tmpstr[t_len-1] != '\0')
          tmpstr[t_len] = '\0';
        info_field->height = strdup(tmpstr);
      }
      else
        info_field->height = strdup("unknown");
    }
    else
      info_field->height = strdup("unknown");

    /* Get long name */
    istat = nc_inq_attlen(ncinid, varinid, "long_name", &t_len);
    if (istat == NC_NOERR) {
      handle_netcdf_error(istat, __FILE__, __LINE__);
      istat = nc_get_att_text(ncinid, varinid, "long_name", tmpstr);
      if (istat == NC_NOERR) {
        if (tmpstr[t_len-1] != '\0')
          tmpstr[t_len] = '\0';
        info_field->long_name = strdup(tmpstr);
      }
      else
        info_field->long_name = strdup(varname);
    }
    else
      info_field->long_name = strdup(varname);
  }

  /* Read values from netCDF variable */
  istat = nc_get_var1_double(ncinid, varinid, idx, buf);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Close the input netCDF file. */
  istat = ncclose(ncinid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Free memory */
  (void) free(tmpstr);
  (void) free(idx);

  /* Success status */
  return 0;
}
