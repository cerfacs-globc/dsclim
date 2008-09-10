/* ***************************************************** */
/* Compute mean and variance of distances to clusters.   */
/* mean_variance_dist_clusters.c                         */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file mean_variance_dist_clusters.c
    \brief Compute mean and variance of distances to clusters.
*/

#include <utils.h>

#include <gsl/gsl_statistics.h>

void mean_variance_dist_clusters(double *mean_dist, double *var_dist, double *pc, double *clusters, double *var_pc,
                                 int neof, int nclust, int ntime) {
  double *bufpc = NULL;
  double *dist_pc = NULL;
  double *var_pc_norm = NULL;

  double sum;
  double val;
  
  int eof;
  int nt;
  int clust;

  bufpc = (double *) malloc(ntime*neof * sizeof(double));
  if (bufpc == NULL) alloc_error(__FILE__, __LINE__);
  dist_pc = (double *) malloc(ntime * sizeof(double));
  if (dist_pc == NULL) alloc_error(__FILE__, __LINE__);

  var_pc_norm = (double *) malloc(neof * sizeof(double));
  if (var_pc_norm == NULL) alloc_error(__FILE__, __LINE__);

  /* Normalisation of the principal component by the square root of the variance of the first one */
  for (eof=0; eof<neof; eof++) {
    var_pc_norm[eof] = sqrt(gsl_stats_variance(&(pc[eof*ntime]), 1, ntime));
    for (nt=0; nt<ntime; nt++)
      bufpc[nt+eof*ntime] = pc[nt+eof*ntime] / var_pc_norm[0];
    var_pc_norm[eof] = sqrt(gsl_stats_variance(&(bufpc[eof*ntime]), 1, ntime));
  }

  for (clust=0; clust<nclust; clust++) {
    for (nt=0; nt<ntime; nt++) {
      sum = 0.0;
      for (eof=0; eof<neof; eof++) {
        val = (bufpc[nt+eof*ntime] / var_pc_norm[eof]) - (clusters[clust+eof*nclust] / var_pc[eof]);
        sum += (val * val);
      }
      dist_pc[nt] = sqrt(sum);
    }
    mean_dist[clust] = gsl_stats_mean(dist_pc, 1, ntime);
    var_dist[clust] = gsl_stats_variance(dist_pc, 1, ntime);
  }

  (void) free(bufpc);
  (void) free(dist_pc);

  (void) free(var_pc_norm);
  
  return 0;
}
