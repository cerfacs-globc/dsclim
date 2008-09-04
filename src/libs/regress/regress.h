/*! \file regress.h
    \brief Include file for regression library.
*/
#ifndef REGRESS_H
#define REGRESS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/** GNU extensions. */
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit.h>

/* Prototypes */
int regress(double *coef, double *x, double *y, double *cte, double *yreg, double *yerr, double *chisq, int nterm, int npts);

#endif
