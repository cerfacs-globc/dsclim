/*! \file classif.h
    \brief Include file for classification library.
*/
#ifndef CLASSIF_H
#define CLASSIF_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

void class_days_pc_centroids(int *days_class_cluster, double *pc_eof_days, double *eof_days_cluster, char *type,
                             int neof, int ncluster, int ndays);
void generate_clusters(double *clusters, double *pc_eof_days, char *type, int nclassif, int neof, int ncluster, int ndays);

#endif

