
/*! \file dsclim.h
    \brief Include file for main program of downscaling algorithm.
*/
#ifndef DSCLIM_H
#define DSCLIM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/** GNU extensions. */
#define _GNU_SOURCE

/* C standard includes */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

/* NetCDF-related includes */
#include <zlib.h>
#include <szlib.h>
#include <hdf5.h>
#include <netcdf.h>

#include <udunits.h>

/* GNU GSL includes */
#include <gsl/gsl_sort.h>

/** Pagesize value for the system for use with mmap. */
#define PAGESIZE sysconf(_SC_PAGESIZE)

/** Number of field type categories. */
#define NCAT 4
/** Large-scale fields category. */
#define FIELD_LS 0
/** Large-scale fields category for control-run. */
#define CTRL_FIELD_LS 1
/** Large-scale secondary fields category. */
#define SEC_FIELD_LS 2
/** Large-scale secondary fields category for control-run. */
#define CTRL_SEC_FIELD_LS 3

/** Kelvin/Celcius constant. */
#define K_TKELVIN 273.15

/* Local C includes. */
#include <utils.h>
#include <clim.h>
#include <classif.h>
#include <pceof.h>
#include <io.h>
#include <regress.h>

/** Data structure var_struct for observation database variables. */
typedef struct {
  int nobs_var; /**< Number of observation variables. */
  char *lonname; /**< Longitude variable name for observation files. */
  char *latname; /**< Latitude variable name for observation files. */
  char *timename; /**< Time dimension name for observation files. */
  proj_struct *proj; /**< Projection information of large scale fields. */
  char *dimxname; /**< X dimension name for observation files. */
  char *dimyname; /**< Y dimension name for observation files. */
  char *dimcoords; /**< Coordinates for X and Y dimensions (1D or 2D). */
  char *frequency; /**< Frequency of observation data. */
  char *path; /**< Directory where observation data is stored: the template is of the form path/acronym_YYYYYYYY.nc where YYYYYYYY are the beginning and ending years concatenated. */
  int month_begin; /**< The input year in the database begins at this month number (1-12). */
  int year_digits; /**< Number of digits to represent years in observations data filename. */
  char *template; /**< Observation datafiles template. */
  char **acronym; /**< Acronym for variable in filename and NetCDF file. */
  char **netcdfname; /**< Standard NetCDF variable acronym. */
  char **name; /**< Long name of observation variable. */
  double *delta; /**< Value to add to get SI units. */
  double *factor; /**< Value to multiply to get SI units. */
} var_struct;

/** Analog day structure analog_day_struct, season-dependent. */
typedef struct {
  int *tindex; /**< Time index of analog day. */
  int *tindex_all; /**< Time index of analog day in the season-merged index. */
  int *year; /**< Year of analog day. */
  int *month; /**< Month of analog day. */
  int *day; /**< Day of analog day. */
  int ntime; /**< Number of analog times. */
  int *tindex_s_all; /**< Time index of day being downscaled in the season-merged index. */
  int *year_s; /**< Years of dates being downscaled. */
  int *month_s; /**< Months of dates being downscaled. */
  int *day_s; /**< Days of dates being downscaled. */
} analog_day_struct;

/** EOF data field structure eof_data_struct. */
typedef struct {
/* The dimension should be for each independent field, for all categories */
/* Can be NULL for non-appropriate category. */
  double *eof_ls; /**< Large scale fields EOF. */
  double *sing_ls; /**< Large scale fields singular values for EOF. */
  char *eof_nomvar_ls; /**< Name of EOF of large scale field. */
  char *sing_nomvar_ls; /**< Name of singular values for EOF of large scale field. */
} eof_data_struct;

/** EOF information field structure eof_info_struct. */
typedef struct {
/* The dimension should be for each independent field, for all categories */
/* Can be NULL for non-appropriate category. */
  int eof_project; /**< If we want to project a large scale field onto its EOF. */
  int eof_provided; /**< If EOF is already computed for all large scale fields and available in a file. */
  double eof_scale; /**< Large scale field scaling for projection on EOF. */
  int eof_save; /**< If we want to save the EOF and singular values in a file. */
  char *eof_filein_ls; /**< EOF and singular values of large-scale fields input filename. */
  char *eof_fileout_ls; /**< EOF and singular values of large-scale fields input filename. */
  info_field_struct *info; /**< Information (field attributes) about large scale fields EOF. */
  int neof_ls; /**< EOF dimension of large scale fields. */
  char *eof_coords; /**< EOF coordinates NetCDF (1D or 2D). */
} eof_info_struct;

/** Climatology information structure clim_info_struct. */
typedef struct {
/* The should be for each independent field, for all categories */
/* Can be NULL for non-appropriate category.. */
  int clim_remove; /**< If we want to remove or not the climatology from a large scale field. */
  int clim_provided; /**< If climatology is already computed for all large scale fields and available in a file. */
  int clim_save; /**< If we want to save the climatology in a file. */
  char *clim_nomvar_ls; /**< Name of climatology of large scale field. */
  char *clim_filein_ls; /**< Climatology of large-scale fields input filename. */
  char *clim_fileout_ls; /**< Climatology of large-scale fields input filename. */
} clim_info_struct;

/** Downscaling data structure. */
typedef struct {
  double **mean_dist; /**< Mean of cluster distances. */
  double **var_dist; /**< Variance of cluster distances. */
  double **dist; /**< Cluster distances. */
  double **smean_norm; /**< Normalized spatial mean of secondary large-scale fields. */
  int **days_class_clusters; /**< Classification of days into clusters. */
  double *smean; /**< Spatial mean of secondary large-scale fields. */
  double *mean; /**< Seasonal mean of spatially-averaged secondary fields. */
  double *var; /**< Seasonal variance of spatially-averaged secondary fields. */
  double *var_pc_norm; /**< Normalization for EOF-projected large-scale fields. */
  double **delta; /**< Secondary large-scale field difference between value of learning field at analog date vs model field at downscaled date, seasonal-dependent. */
  double *delta_all; /**< Secondary large-scale field difference between value of learning field at analog date vs model field at downscaled date. */
} downscale_struct;

/** Field data structure field_data_struct. */
typedef struct {
/* The dimension should be for each independent field, for all categories. */
  char *nomvar_ls; /**< Name of large scale field. */
  double *field_ls; /**< Large scale fields. */
  char *filename_ls; /**< Large scale field filename. */
  double *field_eof_ls; /**< Large scale fields projected on EOF. */
  char *lonname; /**< Longitude variable name for large-scale fields. */
  char *latname; /**< Latitude variable name for large-scale fields. */
  char *timename; /**< Time variable name for large-scale fields. */
  info_field_struct *info; /**< Information (field attributes) about large scale fields. */
  clim_info_struct *clim_info; /**< Climatology info. */
  eof_info_struct *eof_info; /**< EOF info. */
  eof_data_struct *eof_data; /**< EOF data. */
  downscale_struct *down; /**< Downscaling fields for large scale fields. */
  double first_variance; /**< Variance of the first EOF. */
} field_data_struct;

/** Data structure to hold field information and data field_struct. */
typedef struct {
/* The dimension should be : 
 * Large-scale fields (0)
 * Large-scale Control Run fields (1)
 * Secondary large-scale fields (2)
 * Secondary large-scale Control Run fields (3)
 *
 * These variables are thus different for each of these categories
 */
  int n_ls; /**< Number of large scale fields. */
  int ntime_ls; /**< Time dimension of large scale fields. */
  double *time_ls; /**< Time vector of large scale fields. */
  time_struct *time_s; /**< Time structure of large scale fields. */  
  double *lon_ls; /**< Longitude of gridpoints of large scale fields. */
  double *lat_ls; /**< Latitude of gridpoints of large scale fields. */
  int nlon_ls; /**< X dimension of large scale fields. */
  int nlat_ls; /**< Y dimension of large scale fields. */
  double *lon_eof_ls; /**< Longitude of gridpoints of large scale fields EOF. */
  double *lat_eof_ls; /**< Latitude of gridpoints of large scale fields EOF. */
  int nlon_eof_ls; /**< X dimension of large scale fields EOF. */
  int nlat_eof_ls; /**< Y dimension of large scale fields EOF. */
  double **precip_index; /**< Precipitation index reconstructed from regression coefficients. */
  analog_day_struct *analog_days; /**< Analog days season-dependent. */
  analog_day_struct analog_days_year; /**< Analog days all seasons merged. */
  proj_struct *proj; /**< Projection information of large scale fields. */
  field_data_struct *data; /**< Fields data. */
} field_struct;

/** Data structure seasonal-dependent learning_data_struct. */
typedef struct {
  int ntime; /**< Number of times. */
  time_struct *time_s; /**< Time information in time structure. */
  double *time; /**< Time vector of learning data. */
  double *weight; /**< Weights of learning data. */
  int *class_clusters; /**< Classification clusters learning data. */
  double *precip_reg; /**< Precipitation regression values. */
  double *precip_reg_cst; /**< Precipitation regression constant values. */
  double *precip_index; /**< Precipitation index. */
  double *sup_index; /**< Secondary field index. */
  double sup_index_mean; /**< Mean of secondary field index. */
  double sup_index_var; /**< Variance of secondary field index. */
} learning_data_struct;

/** Data structure for EOF-related learning data learning_eof_struct. */
typedef struct {
  double *eof;
  double *sing;
  time_struct *time_s;
  int ntime; /**< Number of times. */
  char *filename_eof;
  char *nomvar_eof;
  char *nomvar_sing;
} learning_eof_struct;

/** Data structure for learning data learning_struct. */
typedef struct {
  int learning_provided; /**< If learning data is already available and will be read from files. */
  int learning_save; /**< If learning data will be saved in files. */
  char *filename_open_weight; /**< Filename for reading weight data in NetCDF format. */
  char *filename_open_learn; /**< Filename for reading learning data in NetCDF format. */
  char *filename_open_clust_learn; /**< Filename for reading clusters learning data in NetCDF format. */
  char *filename_save_weight; /**< Filename for writing weight data in NetCDF format. */
  char *filename_save_learn; /**< Filename for writing learning data in NetCDF format. */
  char *filename_save_clust_learn; /**< Filename for writing clusters learning data in NetCDF format. */
  char *filename_rea_sup; /**< Filename for secondary large-scale field of reanalysis data. */
  char *nomvar_weight; /**< NetCDF variable name for weights. */
  char *nomvar_class_clusters; /**< NetCDF variable name clusters classification. */
  char *nomvar_precip_reg; /**< NetCDF variable name for precipitation regression coefficients. */
  char *nomvar_precip_reg_cst; /**< NetCDF variable name for precipitation regression constant. */
  char *nomvar_precip_index; /**< NetCDF variable name for precipitation index. */
  char *nomvar_sup_index; /**< NetCDF variable name for secondary large-scale field index. */
  char *nomvar_sup_index_mean; /**< NetCDF variable name for secondary large-scale field mean. */
  char *nomvar_sup_index_var; /**< NetCDF variable name for secondary large-scale field variance. */
  char *nomvar_pc_normalized_var; /**< NetCDF variable name for the variance of EOF-projected large-scale fields after normalization with the first EOF. */
  char *nomvar_time; /**< NetCDF variable name for time. */
  char *nomvar_rea_sup; /**< NetCDF variable name for secondary large-scale field in reanalysis data. */
  double *pc_normalized_var; /**< Variance of EOF-projected large-scale fields after normalization with the first one. */
  double *lon; /**< Longitudes. */
  double *lat; /**< Latitudes. */
  int nlon; /**< Number of longitudes. */
  int nlat; /**< Number of latitudes. */
  int ntime; /**< Number of times. */
  time_struct *time_s; /**< Time structure of the whole learning period. */
  int neof;
  char *rea_coords; /**< Coordinates for reanalysis data (1D or 2D). */
  char *rea_gridname; /**< Grid name for reanalysis data (1D or 2D). */
  char *rea_lonname; /**< Longitude variable name for reanalysis files. */
  char *rea_latname; /**< Latitude variable name for reanalysis files. */
  char *rea_timename; /**< Time dimension name for reanalysis files. */
  char *obs_lonname; /**< Longitude variable name for observations files. */
  char *obs_latname; /**< Latitude variable name for observations files. */
  char *obs_timename; /**< Time dimension name for observations files. */
  learning_eof_struct *obs;
  learning_eof_struct *rea;
  learning_data_struct *data; /**< Learning data, seasonal-dependent. */
} learning_struct;

/** Data structure for regression. */
typedef struct {
  char *filename; /**< Filename for regression points. */
  char *lonname; /**< Longitude field name. */
  char *latname; /**< Latitude field name. */
  char *ptsname; /**< Points dimension name. */
  double *lon; /**< Longitude of regression points. */
  double *lat; /**< Latitude of regression points. */
  int npts; /**< Number of regression points. */
  double dist; /**< Distance of spatial mean influence for regression points. */
} reg_struct;

/** Seasons definition season_struct */
typedef struct {
  int nmonths; /**< Number of months. */
  int *month; /**< Month numbers. */
  int nclusters; /**< Number of clusters. */
  int nreg; /**< Number of regression points. */
  int shuffle; /**< Shuffle (1) or sort (0) when choosing the analog day. */
  int secondary_choice; /**< Choose analog day in remaining days using the secondary large-scale field. */
  int secondary_main_choice; /**< Choose analog days using main and secondary large-scale fields. */
  int ndayschoices; /**< Number of days to choose before shuffling or sorting. */
  int ndays; /**< Number of +- days around current *day of year* being downscaled to search. Hard-bounded by seasons.. */
} season_struct;

/** Period definition period_struct */
typedef struct {
  int year_begin; /**< 4-digit year begin. */
  int month_begin; /**< Month of the year begin 1-12. */
  int day_begin; /**< Day of the month begin 1-31. */
  int year_end; /**< 4-digit year end. */
  int month_end; /**< Month of the year end 1-12. */
  int day_end; /**< Day of the month end 1-31. */
  int downscale; /**< Downscale period or not. */
} period_struct;

/** General configuration data structure conf_struct. */
typedef struct {
  int debug; /**< Debugging flag. */
  int clim_filter_width; /**< Climatology filter width. */
  char *clim_filter_type; /**< Climatology filter type. */
  char *cal_type; /**< Calendar-type for downscaling. */
  char *time_units; /**< Base time units for downscaling. */
  char *lonname_eof; /**< Longitude dimension name (EOF file) for downscaling. */
  char *latname_eof; /**< Latitude dimension name (EOF file) for downscaling. */
  char *eofname; /**< EOF dimension name for downscaling. */
  char *ptsname; /**< Points dimension name for downscaling. */
  char *clustname; /**< Cluster dimension name. */
  double longitude_min; /**< Domain minimum longitude. */
  double longitude_max; /**< Domain maximum longitude. */
  double latitude_min; /**< Domain minimum latitude. */
  double latitude_max; /**< Domain maximum latitude. */
  double learning_mask_longitude_min; /**< Learning Mask minimum longitude. */
  double learning_mask_longitude_max; /**< Learning Mask maximum longitude. */
  double learning_mask_latitude_min; /**< Learning Mask minimum latitude. */
  double learning_mask_latitude_max; /**< Learning Mask maximum latitude. */
  char *output_path; /**< Output path. */
  int output_month_begin; /**< Output month number (1-12) in where we begin the output year. */
  int nseasons; /**< Number of seasons. */
  season_struct *season; /**< Seasons definition. */
  int nperiods; /**< Number of periods. */
  period_struct *period; /**< Periods definition. */
  period_struct *period_ctrl; /**< Control run period definition. */
  int downscale; /**< Downscale or not control-run period. */
  char *classif_type; /**< Classification type (euclidian only for now). */
  int nclassifications; /**< Maximum number of classifications. */
  int npartitions; /**< Number of partitions. */
  var_struct *obs_var; /**< Structure for observation variables information. */
} conf_struct;

/** MASTER data structure data_struct. */
typedef struct {
  info_struct *info; /**< Information structure. */
  conf_struct *conf; /**< Configurable parameters structure. */
  field_struct *field; /**< Data fields structure. */
  learning_struct *learning; /**< Learning data structure. */
  reg_struct *reg; /**< Regression structure. */
} data_struct;

/* Prototypes */
int load_conf(data_struct *data, char *fileconf);
int wt_downscaling(data_struct *data);
int wt_learning(data_struct *data);
int read_large_scale_fields(data_struct *data);
int read_large_scale_eof(data_struct *data);
int read_learning_obs_eof(data_struct *data);
int read_learning_rea_eof(data_struct *data);
int read_learning_fields(data_struct *data);
int read_obs_period(double **buffer, double **lon, double **lat, double *missing_value, data_struct *data, char *varname,
                    int *year, int *month, int *day, int *nlon, int *nlat, int ntime);
int read_field_subdomain_period(double **buffer, double **lon, double **lat, double *missing_value, char *varname,
                                int *year, int *month, int *day, double lonmin, double lonmax, double latmin, double latmax,
                                char *coords, char *gridname, char *lonname, char *latname, char *timename, char *filename,
                                int *nlon, int *nlat, int ntime);
int remove_clim(data_struct *data);
int read_regression_points(reg_struct *reg);
void find_the_days(analog_day_struct analog_days, double *precip_index, double *precip_index_learn, double *sup_field_index,
                   double *sup_field_index_learn, int *class_clusters, int *class_clusters_learn, int *year, int *month, int *day,
                   int *year_learn, int *month_learn, int *day_learn,
                   int ntime, int ntime_learn, int *months, int nmonths, int ndays, int ndayschoices, int npts,
                   int shuffle, int sup, int sup_choice);
void compute_secondary_large_scale_diff(double *delta, analog_day_struct analog_days, double *sup_field_index,
                                        double *sup_field_index_learn, double sup_field_var, double sup_field_var_learn, int ntimes);
int merge_seasons(analog_day_struct analog_days_merged, analog_day_struct analog_days, int ntimes_merged, int ntimes);
int merge_seasonal_data(double *buf_merged, double *buf, analog_day_struct analog_days, int dimx, int dimy,
                        int ntimes_merged, int ntimes);
int output_downscaled_analog(analog_day_struct analog_days, double *delta, data_struct *data, double *time_ls, int ntime);
int write_learning_fields(data_struct *data);
void free_main_data(data_struct *data);

#endif
