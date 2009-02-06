/*! \file filter.h
    \brief Include file for filter library.
*/
#ifndef FILTER_H
#define FILTER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#ifdef HAVE_MATH_H
#include <math.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

/* Local dependent includes */
#include <misc.h>

/* Prototypes */
void filter(double *bufferf, double *buffer, char *type, int width, int ni, int nj, int nt);
void filter_window(double **filter_window, char *type, int width);

#endif

