/*! \file io.h
    \brief Include file for input/output library.
*/
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

/** TRUE value macro is 1. */
#define TRUE 1
/** FALSE value macro is 0. */
#define FALSE 0

/** Data structure for NetCDF metadata info_struct. */
typedef struct {
  char *title; /**< Title (english). */
  char *title_french; /**< Title (french). */
  char *summary; /**< Summary (english). */
  char *summary_french; /**< Summary *french). */
  char *keywords; /**< Keyword. */
  char *processor; /**< Program, processor which have generated the data. */
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

/** Easy time structure. **/
typedef struct {
  int *year; /**< 4-digit Year. */
  int *month; /**< Month 1-12. */
  int *day; /**< Day of the month 1-31. */
  int *hour; /**< Hour of the day 0-23. */
  int *minutes; /**< Minutes of the hour 0-59. */
  float *seconds; /**< Seconds of the minute 0-59. */
} time_struct;

/* NetCDF-related includes */
#include <zlib.h>
#include <szlib.h>
#include <hdf5.h>
#include <netcdf.h>

#include <udunits.h>

/* Dependent local includes. */
#include <utils.h>

/* Prototypes */
int read_netcdf_dims_3d(double **lon, double **lat, double **timeval, char **cal_type, char **time_units,
                              int *nlon, int *nlat, int *ntime, info_struct *info, char *coords, char *gridname,
                              char *lonname, char *latname, char *timename, char *filename);
int read_netcdf_latlon(double **lon, double **lat, int *nlon, int *nlat, char *dimcoords, char *coords, char *gridname,
                       char *lonname, char *latname, char *dimxname, char *dimyname, char *filename);
int read_netcdf_dims_eof(double **lon, double **lat, int *nlon, int *nlat, int *neof, char *coords,
                               char *lonname, char *latname, char *eofname, char *filename);
int read_netcdf_var_3d(double **buf, info_field_struct *info_field, proj_struct *proj, char *filename, char *varname,
                             char *lonname, char *latname, char *timename, int *nlon, int *nlat, int *ntime);
int read_netcdf_var_3d_2d(double **buf, info_field_struct *info_field, proj_struct *proj, char *filename, char *varname,
                          char *lonname, char *latname, char *timename, int t, int outinfo, int *nlon, int *nlat, int *ntime);
int read_netcdf_var_2d(double **buf, info_field_struct *info_field, proj_struct *proj, char *filename, char *varname,
                       char *lonname, char *latname, int *nlon, int *nlat);
int read_netcdf_var_1d(double **buf, info_field_struct *info_field, char *filename, char *varname,
                             char *dimname, int *ndim);
int read_netcdf_var_generic_val(double *buf, info_field_struct *info_field, char *filename, char *varname, int index);
int write_netcdf_var_3d(double *buf, double fillvalue, char *filename,
                              char *varname, char *gridname, char *lonname, char *latname, char *timename,
                              int nlon, int nlat, int ntime);
int write_netcdf_var_3d_2d(double *buf, double *timein, double fillvalue, char *filename,
                           char *varname, char *gridname, char *lonname, char *latname, char *timename,
                           int t, int newfile, int outinfo, int nlon, int nlat, int ntime);
int write_netcdf_dims_3d(double *lon, double *lat, double *timein, char *cal_type, char *time_units,
                         int nlon, int nlat, int ntime, char *timestep, char *gridname, char *coords,
                         char *grid_mapping_name, double latin1, double latin2,
                         double lonc, double lat0, double false_easting, double false_northing,
                         char *lonname, char *latname, char *timename,
                         char *filename, int outinfo);
int create_netcdf(char *title, char *title_french, char *summary, char *summary_french,
                        char *keywords, char *processor, char *description, char *institution,
                        char *creator_email, char *creator_url, char *creator_name,
                        char *version, char *scenario, char *scenario_co2, char *model,
                        char *institution_model, char *country, char *member, char *downscaling_forcing,
                        char *contact_email, char *contact_name, char *other_contact_email, char *other_contact_name,
                  char *filename, int outinfo);
int get_time_attributes(char **time_units, char **cal_type, char *filename, char *varname);
int get_attribute_str(char **var, int ncinid, int varid, char *attrname);
int get_time_info(time_struct *time_s, double **timeval, char **time_units, char **cal_type, int *ntime, char *filename, char *varname);
int compute_time_info(time_struct *time_s, double *timeval, char *time_units, char *cal_type, int ntime);
void handle_netcdf_error(int status, char *srcfilename, int lineno);

#endif
