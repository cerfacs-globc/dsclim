/*! \file utils.h
    \brief Include file for utilities library.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2009)

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
#ifndef UTILS_H
#define UTILS_H

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
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include <gsl/gsl_statistics.h>

#include "utCalendar_cal.h"

/* Local dependent includes */
#include <misc.h>
#include <constants.h>

/** TRUE value macro is 1. */
#define TRUE 1
/** FALSE value macro is 0. */
#define FALSE 0

/** Easy time structure. */
typedef struct {
  int year;  /**< Year (4-digits). */
  int month; /**< Month (1-12). */
  int day;   /**< Day (1-31). */
  int hour;  /**< Hour (0-24). */
  int min;   /**< Minute (0-59). */
  float sec; /**< Second (0-59). */
} tstruct;

void alloc_mmap_shortint(short int **map, int *fd, size_t *byte_size, char *filename, size_t page_size, int size);
void alloc_mmap_longint(long int **map, int *fd, size_t *byte_size, char *filename, size_t page_size, int size);
void alloc_mmap_int(int **map, int *fd, size_t *byte_size, char *filename, size_t page_size, int size);
void alloc_mmap_float(float **map, int *fd, size_t *byte_size, char *filename, size_t page_size, int size);
void alloc_mmap_double(double **map, int *fd, size_t *byte_size, char *filename, size_t page_size, int size);
int data_to_gregorian_cal_d(double **bufout, double **outtimeval, int *ntimeout, double *bufin,
                            double *intimeval, char *tunits_in, char *tunits_out, char *cal_type, int ni, int nj, int ntimein);
int data_to_gregorian_cal_f(float **bufout, double **outtimeval, int *ntimeout, float *bufin,
                            double *intimeval, char *tunits_in, char *tunits_out, char *cal_type, int ni, int nj, int ntimein);
int get_calendar(int *year, int *month, int *day, int *hour, int *minutes, float *seconds, char *tunits, double *timein, int ntime);
int get_calendar_ts(tstruct *timeout, char *tunits, double *timein, int ntime);
void change_date_origin(double *timeout, char *tunits_out, double *timein, char *tunits_in, int ntime);
void mean_variance_field_spatial(double *buf_mean, double *buf_var, double *buf, short int *mask, int ni, int nj, int ntime);
void mean_field_spatial(double *buf_mean, double *buf, short int *mask, int ni, int nj, int ntime);
int sub_period_common(double **buf_sub, int *ntime_sub, double *bufin, int *year, int *month, int *day,
                      int *year_learn, int *month_learn, int *day_learn, int timedim, int ndima, int ndimb, int ntime, int ntime_learn);
void extract_subdomain(double **buf_sub, double **lon_sub, double **lat_sub, int *nlon_sub, int *nlat_sub, double *buf,
                       double *lon, double *lat, double minlon, double maxlon, double minlat, double maxlat,
                       int nlon, int nlat, int ndim);
void extract_subperiod_months(double **buf_sub, int *ntime_sub, double *bufin, int *year, int *month, int *day,
                              int *smonths, int timedim, int ndima, int ndimb, int ntime, int nmonths);
void mask_region(double *buffer, double missing_value, double *lon, double *lat,
                 double minlon, double maxlon, double minlat, double maxlat,
                 int nlon, int nlat, int ndim);
void mask_points(double *buffer, double missing_value, short int *mask, int nlon, int nlat, int ndim);
void normalize_field(double *nbuf, double *buf, double mean, double var, int ndima, int ndimb, int ntime);
int comparf(const void *a, const void *b);
double distance_point(double lon1, double lat1, double lon2, double lat2);
int find_str_value(char *str, char **str_vect, int nelem);
void alt_to_press(double *pres, double *alt, int ni, int nj);
void spechum_to_hr(double *hr, double *tas, double *hus, double *pmsl, double fillvalue, int ni, int nj);
void calc_etp_mf(double *etp, double *tas, double *hus, double *rsds, double *rlds, double *uvas, double *pmsl, double fillvalue, int ni, int nj);

#endif
