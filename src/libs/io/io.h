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

#include <zlib.h>
#include <szlib.h>
#include <hdf5.h>
#include <netcdf.h>

#include <udunits.h>

#include <utils.h>

short int read_netcdf_dims_3d(double **lon, double **lat, double **timeval, char **cal_type, char **time_units,
                              int *nlon, int *nlat, int *ntime, char *lonname, char *latname, char *timename,
                              char *filename);
short int read_netcdf_var_3d(double **buf, double *fillvalue, char *filename,
                             char *varname, char *lonname, char *latname, char *timename,
                             int nlon, int nlat, int ntime);
void handle_netcdf_error(int status, int lineno);

#endif
