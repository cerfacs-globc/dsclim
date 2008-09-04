/*! \file utCalendar_cal.h
    \brief Include file for calendar library.
*/
#ifndef UTCALENDAR_H
#define UTCALENDAR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/** GNU extensions */
#define _GNU_SOURCE

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <udunits.h>

/* Prototypes */
int utCalendar_cal(double val, utUnit *dataunits, int *year, int *month, int *day, int *hour, int *minute, float *second,
                   char *calendar);
int utInvCalendar_cal( int year, int month, int day, int hour, int minute, double second, utUnit *unit, double *value,
                       const char *calendar );
#endif
