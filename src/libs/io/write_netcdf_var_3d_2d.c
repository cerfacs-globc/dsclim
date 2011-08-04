/* ***************************************************** */
/* write_netcdf_var_3d_2d Write a 2D field in a          */
/* 3D NetCDF variable.                                   */
/* write_netcdf_var_3d_2d.c                              */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: nov 2008                            */
/* Last date of modification: jan 2010                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision: 1.1                                 */
/* ***************************************************** */
/* Revisions                                             */
/* 1.1 Added compression level                           */
/* ***************************************************** */
/*! \file write_netcdf_var_3d_2d.c
    \brief Write a 2D field in a 3D NetCDF variable.
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



#include <io.h>

/** Write a 2D field in a 3D NetCDF variable. */
int
write_netcdf_var_3d_2d(double *buf, double *timein, double fillvalue, char *filename,
                       char *varname, char *longname, char *units, char *height,
                       char *gridname, char *lonname, char *latname, char *timename,
                       int t, int newfile, int format, int compression_level,
                       int nlon, int nlat, int ntime, int outinfo) {
  /**
     @param[in]  buf               3D Field to write
     @param[in]  timein            Time dimension value
     @param[in]  fillvalue         Missing value
     @param[in]  filename          Output NetCDF filename
     @param[in]  varname           Variable name in the NetCDF file
     @param[in]  longname          Variable long name in the NetCDF file
     @param[in]  units             Variable units in the NetCDF file
     @param[in]  height            Variable height in the NetCDF file
     @param[in]  gridname          Grid type name in the NetCDF file
     @param[in]  lonname           Longitude name dimension in the NetCDF file
     @param[in]  latname           Latitude name dimension in the NetCDF file
     @param[in]  timename          Time name dimension in the NetCDF file
     @param[in]  t                 Time index of value to write
     @param[in]  newfile           TRUE is new NetCDF file, FALSE if not
     @param[in]  format            Format of NetCDF file
     @param[in]  compression_level Compression level of NetCDF file (only for NetCDF-4: format==4)
     @param[in]  outinfo           TRUE if we want information output, FALSE if not
     @param[in]  nlon              Longitude dimension
     @param[in]  nlat              Latitude dimension
     @param[in]  ntime             Time dimension
     
     \return                       Status.
  */

  int istat; /* Diagnostic status */

  size_t dimval; /* Temporary variable used to get values from dimension lengths */

  int ncoutid; /* NetCDF output file handle ID */
  int varoutid; /* NetCDF variable output ID */
  int timedimoutid; /* NetCDF time dimension output ID */
  int timeid; /* NetCDF time variable ID */
  int londimoutid; /* NetCDF longitude dimension output ID */
  int latdimoutid; /* NetCDF latitude dimension output ID */
  int vardimids[NC_MAX_VAR_DIMS]; /* NetCDF dimension IDs */
  //  size_t chunksize[NC_MAX_VAR_DIMS]; /* Chunksize */
  //  size_t cachesize; /* HDF5 cache size */
  //  size_t cache_nelems = 2000; /* HDF5 cache number of elements */
  //  float cache_preemp;

  int ntime_file; /* Time dimension in NetCDF output file */
  int nlat_file; /* Latitude dimension in NetCDF output file */
  int nlon_file; /* Longitude dimension in NetCDF output file */

  size_t start[3]; /* Start element when writing */
  size_t count[3]; /* Count of elements to write */

  char *attname = NULL; /* Attribute name */
  char *tmpstr = NULL; /* Temporary string */

  /* Allocate memory */
  attname = (char *) malloc(MAXPATH * sizeof(char));
  if (attname == NULL) alloc_error(__FILE__, __LINE__);

  /* Change directory to output directory for autofs notification */
  tmpstr = strdup(filename);
  istat = chdir(dirname(tmpstr));
  (void) free(tmpstr);

  /*  if (format == 4 && compression_level > 0) {
    if ( !strcmp(gridname, "list") )
      cachesize = (size_t) nlon*sizeof(float)*cache_nelems;
    else
      cachesize = (size_t) nlat*nlon*sizeof(float)*cache_nelems;
    istat = nc_get_chunk_cache(&cachesize, &cache_nelems, &cache_preemp);
    cache_preemp = 0.75;
    cachesize=128000000;
    printf("%d %d %f\n",(int)cachesize,(int)cache_nelems,cache_preemp);
    istat = nc_set_chunk_cache(cachesize, cache_nelems, cache_preemp);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    } */

  /** Open already existing output file **/
  istat = nc_open(filename, NC_WRITE, &ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);  

  /* Get dimension lengths */
  istat = nc_inq_dimid(ncoutid, timename, &timedimoutid);  /* get ID for time dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncoutid, timedimoutid, &dimval); /* get time length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  ntime_file = (int) dimval;

  istat = nc_inq_varid(ncoutid, timename, &timeid);  /* get ID for time variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  istat = nc_inq_dimid(ncoutid, latname, &latdimoutid);  /* get ID for lat dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncoutid, latdimoutid, &dimval); /* get lat length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  nlat_file = (int) dimval;

  istat = nc_inq_dimid(ncoutid, lonname, &londimoutid);  /* get ID for lon dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  istat = nc_inq_dimlen(ncoutid, londimoutid, &dimval); /* get lon length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  nlon_file = (int) dimval;

  /* Verify that they match the provided ones in parameters */
  if ( !strcmp(gridname, "list") ) {
    if ( ((nlat_file != nlon) || (nlon_file != nlon) )) {
      (void) fprintf(stderr, "%s: Error NetCDF type and/or dimensions.\n", __FILE__);
      return -1;
    }
  }
  else {
    if ( ((nlat_file != nlat) || (nlon_file != nlon) )) {
      (void) fprintf(stderr, "%s: Error NetCDF type and/or dimensions.\n", __FILE__);
      return -1;
    }
  }

  /* Go into NetCDF define mode only if first element */
  if (newfile == TRUE) {
    istat = nc_redef(ncoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    
    /* Define main output variable */
    vardimids[0] = timedimoutid;
    if ( !strcmp(gridname, "list") ) {
      vardimids[1] = londimoutid;
      istat = nc_def_var(ncoutid, varname, NC_FLOAT, 2, vardimids, &varoutid);  
    }
    else {
      vardimids[1] = latdimoutid;
      vardimids[2] = londimoutid;
      istat = nc_def_var(ncoutid, varname, NC_FLOAT, 3, vardimids, &varoutid);  
    }
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

#ifdef NC_NETCDF4
    if (format == 4 && compression_level > 0) {
      /* Set up compression level */
      istat = nc_def_var_deflate(ncoutid, varoutid, 0, 1, compression_level);
      if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
      /* Set up chunking */
      /*      if ( !strcmp(gridname, "list") ) {
        chunksize[0] = (size_t) 1;
        chunksize[1] = (size_t) nlon;
        chunksize[2] = (size_t) 0;
      }
      else {
        chunksize[0] = (size_t) 1;
        chunksize[1] = (size_t) nlat;
        chunksize[2] = (size_t) nlon;
      }
      istat = nc_def_var_chunking(ncoutid, varoutid, NC_CHUNKED, chunksize);
      if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);*/
    }
#endif

    /* Set main variable attributes */
    (void) strcpy(attname, "_FillValue");
    istat = nc_put_att_double(ncoutid, varoutid, attname, NC_FLOAT, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    
    (void) strcpy(attname, "missing_value");
    istat = nc_put_att_double(ncoutid, varoutid, attname, NC_FLOAT, 1, &fillvalue);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
    
    tmpstr = (char *) malloc(100 * sizeof(char));
    if (tmpstr == NULL) alloc_error(__FILE__, __LINE__);
    istat = nc_put_att_text(ncoutid, varoutid, "long_name", strlen(longname), longname);
    istat = nc_put_att_text(ncoutid, varoutid, "grid_mapping", strlen(gridname), gridname);
    istat = nc_put_att_text(ncoutid, varoutid, "units", strlen(units), units);
    istat = nc_put_att_text(ncoutid, varoutid, "height", strlen(height), height);
    istat = sprintf(tmpstr, "lon lat");
    istat = nc_put_att_text(ncoutid, varoutid, "coordinates", strlen(tmpstr), tmpstr);
    (void) free(tmpstr);
    
    /* End definition mode */
    istat = nc_enddef(ncoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }
  else {
    istat = nc_inq_varid(ncoutid, varname, &varoutid);
    if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);
  }

  /* Write time dimension variable to NetCDF output file */
  start[0] = ntime_file;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) 1;
  count[1] = 0;
  count[2] = 0;
  istat = nc_put_vara_double(ncoutid, timeid, start, count, &(timein[t]));
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Write variable to NetCDF output file */
  start[0] = ntime_file;
  start[1] = 0;
  start[2] = 0;
  count[0] = (size_t) 1;
  if ( !strcmp(gridname, "list") ) {
    count[1] = (size_t) nlon;
    count[2] = 0;
  }
  else {
    count[1] = (size_t) nlat;
    count[2] = (size_t) nlon;
  }
  if (outinfo == TRUE)
    printf("%s: WRITE %s %s\n", __FILE__, varname, filename);
  istat = nc_put_vara_double(ncoutid, varoutid, start, count, buf);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Close the output netCDF file. */
  istat = ncclose(ncoutid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __FILE__, __LINE__);

  /* Free memory */
  (void) free(attname);

  /* Diagnostic status */
  return 0;
}
