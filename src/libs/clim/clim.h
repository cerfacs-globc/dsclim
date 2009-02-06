/*! \file clim.h
    \brief Include file for climate tools library.
*/
#ifndef CLIM_H
#define CLIM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#ifdef HAVE_STDIO_H
#include <stdio.h>
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
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Local dependent includes */
#include <misc.h>
#include <utils.h>
#include <filter.h>

/* Prototypes */
void clim_daily_tserie_climyear(double *bufout, double *bufin, tstruct *buftime, double missing_val, int ni, int nj, int ntime);
void remove_seasonal_cycle(double *bufout, double *clim, double *bufin, tstruct *buftime, double missing_val,
                           int filter_width, char *type, int clim_provided, int ni, int nj, int ntime);
int dayofclimyear(int day, int month);

#endif

