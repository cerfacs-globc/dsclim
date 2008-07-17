/*! \file filter.h
    \brief Include file for filter library.
*/
#ifndef FILTER_H
#define FILTER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdlib.h>

void filter(double *bufferf, double *buffer, int width, int nx);
void filter_window(double **filter_window, int width);

#endif

