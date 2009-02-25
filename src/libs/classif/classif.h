/*! \file classif.h
    \brief Include file for classification library.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2009)

christian.page@cerfacs.fr

This software is a computer program whose purpose is to downscale climate
scenarios using a statistical methodology based on weather regimes.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software. You can use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty and the software's author, the holder of the
economic rights, and the successive licensors have only limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading, using, modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean that it is complicated to manipulate, and that also
therefore means that it is reserved for developers and experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and, more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.

LICENSE END */
#ifndef CLASSIF_H
#define CLASSIF_H

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

/* GNU GSL includes */
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_statistics.h>

/* Local dependent includes */
#include <misc.h>

/* Prototypes */
void class_days_pc_clusters(int *days_class_cluster, double *pc_eof_days, double *eof_days_cluster, char *type,
                            int neof, int ncluster, int ndays);
void generate_clusters(double *clusters, double *pc_eof_days, char *type, int nclassif, int neof, int ncluster, int ndays);
void best_clusters(double *best_clusters, double *pc_eof_days, char *type, int npart, int nclassif, int neof, int ncluster, int ndays);
void mean_variance_dist_clusters(double *mean_dist, double *var_dist, double *pc, double *clusters, double *var_pc,
                                 double *var_pc_norm_all, int neof, int nclust, int ntime);
void dist_clusters_normctrl(double *dist_pc, double *pc, double *clusters, double *var_pc,
                            double *var_pc_norm_all, double *mean_ctrl, double *var_ctrl,
                            int neof, int nclust, int ntime);
#endif

