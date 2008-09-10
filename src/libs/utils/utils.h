/*! \file utils.h
    \brief Include file for utilities library.
*/
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

#include "utCalendar_cal.h"

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

void alloc_error(char *filename, int line);
void banner(char *pgm, char *verstat, char *type);
int data_to_gregorian_cal_d(double **bufout, double **outtimeval, int *ntimeout, double *bufin,
                            double *intimeval, char *tunits_in, char *tunits_out, char *cal_type, int ni, int nj, int ntimein);
int data_to_gregorian_cal_f(float **bufout, double **outtimeval, int *ntimeout, float *bufin,
                            double *intimeval, char *tunits_in, char *tunits_out, char *cal_type, int ni, int nj, int ntimein);
int get_calendar(int *year, int *month, int *day, int *hour, int *minutes, float *seconds, char *tunits, double *timein, int ntime);
int get_calendar_ts(tstruct *timeout, char *tunits, double *timein, int ntime);
void project_field_eof(double *bufout, double *clim, double *bufin, double *bufeof, double *singular_value, tstruct *buftime,
                       double missing_value, double missing_value_eof,
                       int clim_filter_width, char *clim_filter_type, short int clim_provided,
                       int ni, int nj, int ntime, int neof);
void mean_variance_dist_clusters(double *mean_dist, double *var_dist, double *pc, double *clusters, double *var_pc,
                                 int neof, int nclust, int ntime);
void mean_variance_field_spatial(double *buf_smean, double *buf_mean, double *buf_var, double *buf, int ni, int nj, int ntime);

#endif
