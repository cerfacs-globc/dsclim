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

void filter(double *bufferf, double *buffer, int width, int nx);
void filter_window(double **filter_window, int width);

#endif

