/*! \file io.h
    \brief Include file for input/output library.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2012)

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



#ifndef IO_H
#define IO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/** GNU extensions */
#define _GNU_SOURCE

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

/** TRUE value macro is 1. */
#define TRUE 1
/** FALSE value macro is 0. */
#define FALSE 0

/** Maximum length of paths/filenames strings. */
#define MAXPATH 5000

/** Data structure for NetCDF metadata info_struct. */
typedef struct {
  char *title; /**< Title (english). */
  char *title_french; /**< Title (french). */
  char *summary; /**< Summary (english). */
  char *summary_french; /**< Summary *french). */
  char *keywords; /**< Keyword. */
  char *processor; /**< Program, processor which have generated the data. */
  char *software; /**< Program and version which have generated the data. */
  char *description; /**< Main description of the data. */
  char *institution; /**< Institution which generated the data. */
  char *creator_email; /**< Contact email of the creator of the data. */
  char *creator_url; /**< Website creator of the data. */
  char *creator_name; /**< Name of the creator of the data. */
  char *version; /**< Version of the data. */
  char *scenario; /**< Climate scenario. */
  char *scenario_co2; /**< CO2 scenario. */
  char *model; /**< Numerical model used. */
  char *institution_model; /**< Institution who developed the numerical model used. */
  char *country; /**< Country of the institution who developed the numerical model used. */
  char *member; /**< Member in the case of multi-member model configurations. */
  char *downscaling_forcing; /**< Observations database used when downscaling. */
  char *timestep; /**< timestep used in the file. */
  char *contact_email; /**< Contact email. */
  char *contact_name; /**< Contact name. */
  char *other_contact_email; /**< Other contact email. */
  char *other_contact_name; /**< Other contact name. */
} info_struct;

/** Data structure for field information info_field_struct. */
typedef struct {
  double fillvalue; /**< Missing value. */
  char *coordinates; /**< Coordinate system. */
  char *grid_mapping; /**< Mapping projection type. */
  char *units; /**< Data units. */
  char *height; /**< Data height. */
  char *long_name; /**< Long name for the variable. */
} info_field_struct;

/** Data structure for mapping projection proj_struct. */
typedef struct {
  char *coords; /**< Coordinate system. */
  char *eof_coords; /**< EOF-data coordinate system. */
  char *name; /**< Name of the variable. */
  char *grid_mapping_name; /**< Mapping projection type. */
  double latin1; /**< Latin1 projection parameter. */
  double latin2; /**< Latin2 projection parameter. */
  double lonc; /**< Lonc (longitude center) projection parameter. */
  double lat0; /**< Lat0 (first latitude point) projection parameter. */
  double false_easting; /**< False_easting projection parameter. */
  double false_northing; /**< False_northing projection parameter. */
} proj_struct;

/** Easy time structure of vectors. **/
typedef struct {
  int *year; /**< 4-digit Year. */
  int *month; /**< Month 1-12. */
  int *day; /**< Day of the month 1-31. */
  int *hour; /**< Hour of the day 0-23. */
  int *minutes; /**< Minutes of the hour 0-59. */
  double *seconds; /**< Seconds of the minute 0-59. */
} time_vect_struct;

/* NetCDF-related includes */
#include <zlib.h>
#include <szlib.h>
#include <hdf5.h>
#include <netcdf.h>

#ifdef HAVE_UDUNITS2
#include <udunits.h>
#endif

/* Dependent local includes. */
#include <misc.h>
#include <utils.h>

/* Prototypes */
int read_netcdf_dims_3d(double **lon, double **lat, double **timeval, char **cal_type, char **time_units,
                        int *nlon, int *nlat, int *ntime, info_struct *info, char *coords, char *gridname,
                        char *lonname, char *latname, char *dimxname, char *dimyname, char *timename, char *filename);
int read_netcdf_latlon(double **lon, double **lat, int *nlon, int *nlat, char *dimcoords, char *coords, char *gridname,
                       char *lonname, char *latname, char *dimxname, char *dimyname, char *filename);
int read_netcdf_xy(double **y, double **x, int *nx, int *ny, char *xname, char *yname, char *dimxname, char *dimyname, char *filename);
int read_netcdf_dims_eof(double **lon, double **lat, int *nlon, int *nlat, int *neof, char *coords,
                         char *lonname, char *latname, char *dimxname, char *dimyname, char *eofname, char *filename);
int read_netcdf_var_3d(double **buf, info_field_struct *info_field, proj_struct *proj, char *filename, char *varname,
                       char *dimxname, char *dimyname, char *timename, int *nlon, int *nlat, int *ntime, int outinfo);
int read_netcdf_var_3d_2d(double **buf, info_field_struct *info_field, proj_struct *proj, char *filename, char *varname,
                          char *dimxname, char *dimyname, char *timename, int t, int *nlon, int *nlat, int *ntime, int outinfo);
int read_netcdf_var_2d(double **buf, info_field_struct *info_field, proj_struct *proj, char *filename, char *varname,
                       char *dimxname, char *dimyname, int *nlon, int *nlat, int outinfo);
int read_netcdf_var_1d(double **buf, info_field_struct *info_field, char *filename, char *varname,
                       char *dimname, int *ndim, int outinfo);
int read_netcdf_var_generic_val(double *buf, info_field_struct *info_field, char *filename, char *varname, int index);
int write_netcdf_var_3d(double *buf, double fillvalue, char *filename,
                        char *varname, char *gridname, char *lonname, char *latname, char *timename,
                        int format, int compression_level, int nlon, int nlat, int ntime, int outinfo);
int write_netcdf_var_3d_2d(double *buf, double *timein, double fillvalue, char *filename,
                           char *varname, char *longname, char *units, char *height,
                           char *gridname, char *lonname, char *latname, char *timename,
                           int t, int newfile, int format, int compression_level, int nlon, int nlat, int ntime, int outinfo);
int write_netcdf_dims_3d(double *lon, double *lat, double *x, double *y, double *alt, double *timein, char *cal_type, char *time_units,
                         int nlon, int nlat, int ntime, char *timestep, char *gridname, char *coords,
                         char *grid_mapping_name, double latin1, double latin2,
                         double lonc, double lat0, double false_easting, double false_northing,
                         char *lonname, char *latname, char *timename,
                         char *filename, int outinfo);
int create_netcdf(char *title, char *title_french, char *summary, char *summary_french,
                  char *keywords, char *processor, char *software, char *description, char *institution,
                  char *creator_email, char *creator_url, char *creator_name,
                  char *version, char *scenario, char *scenario_co2, char *model,
                  char *institution_model, char *country, char *member, char *downscaling_forcing,
                  char *contact_email, char *contact_name, char *other_contact_email, char *other_contact_name,
                  char *filename, int outinfo, int format, int compression);
int get_time_attributes(char **time_units, char **cal_type, char *filename, char *varname);
int get_attribute_str(char **var, int ncinid, int varid, char *attrname);
int get_time_info(time_vect_struct *time_s, double **timeval, char **time_units, char **cal_type, int *ntime, char *filename,
                  char *varname, int outinfo);
int compute_time_info(time_vect_struct *time_s, double *timeval, char *time_units, char *cal_type, int ntime);
void handle_netcdf_error(int status, char *srcfilename, int lineno);

#endif
