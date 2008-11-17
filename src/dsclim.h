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

#include <zlib.h>
#include <szlib.h>
#include <hdf5.h>
#include <netcdf.h>

#include <udunits.h>

/** Pagesize value for the system for use with mmap. */
#define PAGESIZE sysconf(_SC_PAGESIZE)

/** Number of field type categories. */
#define NCAT 4
#define FIELD_LS 0
#define CTRL_FIELD_LS 1
#define SEC_FIELD_LS 2
#define CTRL_SEC_FIELD_LS 3

/* Local C includes. */
#include <utils.h>
#include <clim.h>
#include <classif.h>
#include <pceof.h>
#include <io.h>

/** EOF data field structure eof_data_struct. */
/* The dimension should be for each independent field, for all categories */
/* Can be NULL for non-appropriate category. */
typedef struct {
  double *eof_ls; /* Large scale fields EOF */
  double *sing_ls; /* Large scale fields singular values for EOF */
  char *eof_nomvar_ls; /* Name of EOF of large scale field */
  char *sing_nomvar_ls; /* Name of singular values for EOF of large scale field */
} eof_data_struct;

/** EOF info field structure eof_info_struct. */
/* The dimension should be for each independent field, for all categories */
/* Can be NULL for non-appropriate category. */
typedef struct {
  int eof_project; /* If we want to project a large scale field onto its EOF */
  int eof_provided; /* If EOF is already computed for all large scale fields and available in a file */
  double eof_scale; /* Large scale field scaling for projection on EOF */
  int eof_save; /* If we want to save the EOF and singular values in a file */
  char *eof_filein_ls; /* EOF and singular values of large-scale fields input filename */
  char *eof_fileout_ls; /* EOF and singular values of large-scale fields input filename */
  info_field_struct *info; /* Information (field attributes) about large scale fields EOF */
  int neof_ls; /* EOF dimension of large scale fields */
  char *eof_coords;
} eof_info_struct;

/** Climate info structure clim_info_struct. */
/* The should be for each independent field, for all categories */
/* Can be NULL for non-appropriate category. */
typedef struct {
  int clim_remove; /* If we want to remove or not the climatology from a large scale field */
  int clim_provided; /* If climatology is already computed for all large scale fields and available in a file */
  int clim_save; /* If we want to save the climatology in a file */
  char *clim_nomvar_ls; /* Name of climatology of large scale field */
  char *clim_filein_ls; /* Climatology of large-scale fields input filename */
  char *clim_fileout_ls; /* Climatology of large-scale fields input filename */
} clim_info_struct;

/** Downscaling data structures. */
typedef struct {
  double **mean_dist;
  double **var_dist;
  double **dist;
  double **smean_norm;
  int **days_class_clusters;
  double *smean;
  double *mean;
  double *var;
  double *var_pc_norm;
} downscale_struct;

/** Field data structure field_data_struct. */
/* The dimension should be for each independent field, for all categories */
typedef struct {
  char *nomvar_ls; /* Name of large scale field */
  double *field_ls; /* Large scale fields */
  char *filename_ls; /* Large scale field filename */
  double *field_eof_ls; /* Large scale fields projected on EOF */
  info_field_struct *info; /* Information (field attributes) about large scale fields */
  clim_info_struct *clim_info; /* Climatology info */
  eof_info_struct *eof_info; /* EOF info */
  eof_data_struct *eof_data; /* EOF data */
  downscale_struct *down; /* Downscaling fields for large scale fields */
  double first_variance; /* Variance of the first EOF */
} field_data_struct;

/** Data structure field_struct. */
/* The dimension should be : 
 * Large-scale fields (0)
 * Large-scale Control Run fields (1)
 * Secondary large-scale fields (2)
 * Secondary large-scale Control Run fields (3)
 *
 * These variables are thus different for each of these categories
 */
typedef struct {
  int n_ls; /* Number of large scale fields */
  int ntime_ls; /* Time dimension of large scale fields */
  double *time_ls; /* Time vector of large scale fields */
  time_struct *time_s; /* Time structure of large scale fields */
  double *lon_ls; /* Longitude of gridpoints of large scale fields */
  double *lat_ls; /* Latitude of gridpoints of large scale fields */
  int nlon_ls; /* X dimension of large scale fields */
  int nlat_ls; /* Y dimension of large scale fields */
  double *lon_eof_ls; /* Longitude of gridpoints of large scale fields EOF */
  double *lat_eof_ls; /* Latitude of gridpoints of large scale fields EOF */
  int nlon_eof_ls; /* X dimension of large scale fields EOF */
  int nlat_eof_ls; /* Y dimension of large scale fields EOF */
  double **precip_index; /* Precipitation index reconstructed from regression coefficients */
  int **analog_days; /* Analog days */
  proj_struct *proj; /* Projection information of large scale fields */
  field_data_struct *data; /* Fields data */
} field_struct;

/* Data structure season-dependent learning_data_struct. */
typedef struct {
  int ntime;
  time_struct *time_s;
  double *time; /* Time vector of learning data */
  double *weight; /* Weights of learning data */
  int *class_clusters; /* Classification clusters learning data */
  double *precip_reg; /* Precipitation regression values */
  double *precip_reg_cst; /* Precipitation regression constant values */
  double *precip_index; /* Precipitation index */
  double *sup_index; /* Secondary field index */
  double sup_index_mean; /* Mean of secondary field index */
  double sup_index_var; /* Variance of secondary field index */
} learning_data_struct;

/* Data structure learning_struct. */
typedef struct {
  int learning_provided; /* If learning data is already available */
  char *filename_weight;
  char *filename_learn;
  char *filename_clust_learn;
  char *nomvar_weight;
  char *nomvar_class_clusters;
  char *nomvar_precip_reg;
  char *nomvar_precip_reg_cst;
  char *nomvar_precip_index;
  char *nomvar_sup_index;
  char *nomvar_sup_index_mean;
  char *nomvar_sup_index_var;
  char *nomvar_pc_normalized_var;
  char *nomvar_time;
  double *pc_normalized_var; /* Variance of principal components after normalization with the first one */
  int ntime; 
  time_struct *time_s; /* Time structure of the whole learning period */
  learning_data_struct *data;
} learning_struct;

/* Data structure for regression. */
typedef struct {
  char *filename; /* Filename for regression points */
  char *lonname; /* Longitude field name */
  char *latname; /* Latitude field name */
  char *ptsname; /* Points dimension name */
  double *lon; /* Longitude of regression points */
  double *lat; /* Latitude of regression points */
  int npts; /* Number of regression points */
} reg_struct;

/** Season definition season_struct */
typedef struct {
  int nmonths;
  int *month;
  int nclusters;
  int nreg;
  int shuffle;
  int secondary_choice;
  int secondary_main_choice;
  int ndayschoices;
  int ndays;
} season_struct;

/** Period definition period_struct */
typedef struct {
  int year_begin;
  int month_begin;
  int day_begin;
  int year_end;
  int month_end;
  int day_end;
  int downscale;
} period_struct;

/** General configuration data structure conf_struct. */
typedef struct {
  int debug; /* Debugging flag */
  int clim_filter_width; /* Climatology filter width */
  char *clim_filter_type; /* Climatology filter type */
  char *cal_type; /* Calendar-type for downscaling */
  char *time_units; /* Base time units for downscaling */
  char *lonname; /* Longitude dimension name for downscaling */
  char *latname; /* Latitude dimension name for downscaling */
  char *timename; /* Time dimension name for downscaling */
  char *lonname_eof; /* Longitude dimension name (EOF file) for downscaling */
  char *latname_eof; /* Latitude dimension name (EOF file) for downscaling */
  char *eofname; /* EOF dimension name for downscaling */
  char *ptsname; /* Points dimension name for downscaling */
  char *clustname; /* Cluster dimension name */
  proj_struct *proj; /* Projection information for downscaling */
  double longitude_min;
  double longitude_max;
  double latitude_min;
  double latitude_max;
  int nseasons; /* Number of seasons */
  season_struct *season; /* Seasons definition */
  int nperiods; /* Number of periods */
  period_struct *period; /* Periods definition */
  period_struct *period_ctrl; /* Control run period definition */
  int downscale; /* Downscale or not control-run period */
  int npts_reg; /* Number of points for the regression */
  char *classif_type;
} conf_struct;

/** MASTER data structure data_struct. */
typedef struct {
  info_struct *info;
  conf_struct *conf;
  field_struct *field;
  learning_struct *learning;
  reg_struct *reg;
} data_struct;

int load_conf(data_struct *data, char *fileconf);
int wt_downscaling(data_struct *data);
int read_large_scale_fields(data_struct *data);
int read_large_scale_eof(data_struct *data);
int remove_clim(data_struct *data);
int read_regression_points(reg_struct *reg);
void find_the_days(int *analog_days, double *precip_index, double *precip_index_learn, double *sup_field, double *sup_index,
                   int *class_clusters, int *class_clusters_learn, int *year, int *month, int *day,
                   int *year_learn, int *month_learn, int *day_learn,
                   int ntime, int ntime_learn, int *months, int nmonths, int ndays, int ndayschoices, int npts,
                   int shuffle, int sup, int sup_choice);
