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
                                 double *var_pc_norm_all, int neof, int nclust, int ntime) {
  double *bufpc = NULL;
  double *dist_pc = NULL;

  double sum;
  double val;
  
  int eof;
  int nt;
  int clust;

  dist_pc = (double *) malloc(ntime * sizeof(double));
  if (dist_pc == NULL) alloc_error(__FILE__, __LINE__);

  for (clust=0; clust<nclust; clust++) {
    for (nt=0; nt<ntime; nt++) {
      sum = 0.0;
      for (eof=0; eof<neof; eof++) {
        val = (pc[nt+eof*ntime] / var_pc_norm_all[eof]) - (clusters[eof+clust*neof] / var_pc[eof]);
        sum += (val * val);
      }
      dist_pc[nt] = sqrt(sum);
    }
    mean_dist[clust] = gsl_stats_mean(dist_pc, 1, ntime);
    var_dist[clust] = gsl_stats_variance(dist_pc, 1, ntime);
  }

  (void) free(dist_pc);
}
