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

#include <../utils/utils.h>
#include <../filter/filter.h>

typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int min;
  float sec;
} tstruct;

void clim_daily_tserie_climyear(double *bufout, double *bufin, tstruct *buftime, double missing_val, int ntime);
void remove_seasonal_cycle(double *bufout, double *bufin, tstruct *buftime, double missing_val, int filter_width, char *type, int ntime);

#endif

