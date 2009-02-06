/*! \file pceof.h
    \brief Include file for Principal Components and EOF related calculations library.
*/
#ifndef PCEOF_H
#define PCEOF_H

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

#include <gsl/gsl_statistics.h>

#include <misc.h>
#include <filter.h>
#include <clim.h>

/* Prototypes */
void normalize_pc(double *norm_all, double *first_variance, double *buf_renorm, double *bufin, int neof, int ntime);
int project_field_eof(double *bufout, double *bufin, double *bufeof, double *singular_value,
                            double missing_value_eof, double scale, int ni, int nj, int ntime, int neof);

#endif
