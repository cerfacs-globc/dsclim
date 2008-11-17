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

/** Data structure info_struct. */
typedef struct {
  char *title;
  char *title_french;
  char *summary;
  char *summary_french;
  char *keywords;
  char *processor;
  char *description;
  char *institution;
  char *creator_email;
  char *creator_url;
  char *creator_name;
  char *version;
  char *scenario;
  char *scenario_co2;
  char *model;
  char *institution_model;
  char *country;
  char *member;
  char *downscaling_forcing;
  char *timestep;
  char *contact_email;
  char *contact_name;
  char *other_contact_email;
  char *other_contact_name;
} info_struct;

/** Data structure info_field_struct. */
typedef struct {
  double fillvalue;
  char *coordinates;
  char *grid_mapping;
  char *units;
  char *long_name;
} info_field_struct;

/** Data structure proj_struct. */
typedef struct {
  char *coords;
  char *eof_coords;
  char *name;
  char *grid_mapping_name;
  double latin1;
  double latin2;
  double lonc;
  double lat0;
  double false_easting;
  double false_northing;
} proj_struct;

/** Time structure. **/
typedef struct {
  int *year;
  int *month;
  int *day;
  int *hour;
  int *minutes;
  float *seconds;
} time_struct;

#include <zlib.h>
#include <szlib.h>
#include <hdf5.h>
#include <netcdf.h>

#include <udunits.h>

#include <utils.h>

int read_netcdf_dims_3d(double **lon, double **lat, double **timeval, char **cal_type, char **time_units,
                              int *nlon, int *nlat, int *ntime, info_struct *info, char *coords, char *gridname,
                              char *lonname, char *latname, char *timename, char *filename);
int read_netcdf_dims_eof(double **lon, double **lat, int *nlon, int *nlat, int *neof, char *coords,
                               char *lonname, char *latname, char *eofname, char *filename);
int read_netcdf_var_3d(double **buf, info_field_struct *info_field, proj_struct *proj, char *filename, char *varname,
                             char *lonname, char *latname, char *timename, int nlon, int nlat, int ntime);
int read_netcdf_var_2d(double **buf, info_field_struct *info_field, proj_struct *proj, char *filename, char *varname,
                       char *lonname, char *latname, int *nlon, int *nlat);
int read_netcdf_var_1d(double **buf, info_field_struct *info_field, char *filename, char *varname,
                             char *dimname, int ndim);
int read_netcdf_var_generic_1d(double **buf, info_field_struct *info_field, char *filename, char *varname, char *dimname, int *ndim);
int read_netcdf_var_generic_val(double *buf, info_field_struct *info_field, char *filename, char *varname, int index);
int write_netcdf_var_3d(double *buf, double fillvalue, char *filename,
                              char *varname, char *gridname, char *lonname, char *latname, char *timename,
                              int nlon, int nlat, int ntime);
int write_netcdf_dims_3d(double *lon, double *lat, double *timein, char *cal_type, char *time_units,
                               int nlon, int nlat, int ntime, char *timestep, char *gridname, char *coords,
                               char *grid_mapping_name, double latin1, double latin2,
                               double lonc, double lat0, double false_easting, double false_northing,
                               char *lonname, char *latname, char *timename,
                               char *filename);
int create_netcdf(char *title, char *title_french, char *summary, char *summary_french,
                        char *keywords, char *processor, char *description, char *institution,
                        char *creator_email, char *creator_url, char *creator_name,
                        char *version, char *scenario, char *scenario_co2, char *model,
                        char *institution_model, char *country, char *member, char *downscaling_forcing,
                        char *contact_email, char *contact_name, char *other_contact_email, char *other_contact_name,
                        char *filename);
int get_time_attributes(char **time_units, char **cal_type, char *filename, char *varname);
int get_attribute_str(char **var, int ncinid, int varid, char *attrname);
int get_time_info(time_struct *time_s, double **timeval, char **time_units, char **cal_type, int *ntime, char *filename, char *varname);
int compute_time_info(time_struct *time_s, double *timeval, char *time_units, char *cal_type, int ntime);
void handle_netcdf_error(int status, char *srcfilename, int lineno);

#endif
