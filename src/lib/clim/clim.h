/*! \file clim.h
    \brief Include file for climate tools library.
*/
#ifndef CLIM_H
#define CLIM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

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

