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

/* Local C includes */
#include <utils.h>
#include <clim.h>
#include <classif.h>
#include <io.h>

/** Data structure field_struct. */
typedef struct {
  short int n_ls; /* Number of large scale fields */
  double **field_ls; /* Large scale fields */
  double *field_ls_fillvalue; /* Large scale fields fillvalue (missing value) */
  char **fname_ls; /* Name of large scale field */
  char **filename_ls; /* Large scale field filename*/
  char **clim_filename_ls; /* Climatology of large-scale fields filename */
  double *lon_ls; /* Longitude of gridpoints of large scale fields */
  double *lat_ls; /* Latitude of gridpoints of large scale fields */
  double *time_ls; /* Time vector of large scale fields */
  int nlon_ls; /* X dimension of large scale fields */
  int nlat_ls; /* Y dimension of large scale fields */
  int ntime_ls; /* Time dimension of large scale fields */
} field_struct;

/** Configuration data structure conf_struct. */
typedef struct {
  short int debug; /* Debugging flag */
  int clim_filter_width; /* Climatology filter width */
  char *clim_filter_type; /* Climatology filter type */
  short int clim_provided; /* If climatology is already computed for all large scale fields and available in a file */
  char *cal_type; /* Calendar-type */
  char *time_units; /* Base time units */
  char *lonname; /* Longitude dimension name */
  char *latname; /* Latitude dimension name */
  char *timename; /* Time dimension name */
} conf_struct;

/** MASTER data structure data_struct. */
typedef struct {
  conf_struct *conf;
  field_struct *field;
} data_struct;

short int load_conf(data_struct *data, char *fileconf);
short int wt_downscaling(data_struct *data);
short int read_large_scale_fields(data_struct *data);
