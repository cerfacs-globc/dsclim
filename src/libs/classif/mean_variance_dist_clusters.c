/* ***************************************************** */
/* Compute mean and variance of distances to clusters.   */
/* mean_variance_dist_clusters.c                         */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file mean_variance_dist_clusters.c
    \brief Compute mean and variance of distances to clusters.
*/

#include <classif.h>

/** Compute mean and variance of distances to clusters. */
void mean_variance_dist_clusters(double *mean_dist, double *var_dist, double *pc, double *clusters, double *var_pc,
                                 double *var_pc_norm_all, int neof, int nclust, int ntime) {
  /**
     @param[out]  mean_dist       Mean of distances to clusters.
     @param[out]  var_dist        Variance of distances to clusters.
     @param[in]   pc              EOF-projected large-scale field.
     @param[in]   clusters        Cluster centroids for each EOF in EOF-projected space of the large-scale field.
     @param[in]   var_pc          Norm of the variance of the first EOF of the EOF-projected large-scale field of the learning period.
     @param[in]   var_pc_norm_all Norm of the variance of the first EOF of the EOF-projected large-scale field of the control run.
     @param[in]   neof            EOF dimension
     @param[in]   nclust          Clusters dimension
     @param[in]   ntime           Time dimension
  */

  double *dist_pc = NULL; /* Sum of the normalized distances to clusters over all EOFs */

  double sum; /* Sum of normalized distances tp clusters */
  double val; /* Normalized distance to cluster */
  
  int eof; /* EOF loop counter */
  int nt; /* Time loop counter */
  int clust; /* Cluster loop counter */

  /* Allocate memory */
  dist_pc = (double *) malloc(ntime * sizeof(double));
  if (dist_pc == NULL) alloc_error(__FILE__, __LINE__);

  /* Loop over all clusters */
  for (clust=0; clust<nclust; clust++) {
    /* Loop over time */
    for (nt=0; nt<ntime; nt++) {
      /* Initialize the sum */
      sum = 0.0;
      /* Loop over all EOFs */
      for (eof=0; eof<neof; eof++) {
        /* Calculate normalized distance to clusters */
        val = (pc[nt+eof*ntime] / sqrt(var_pc_norm_all[eof])) - (clusters[eof+clust*neof] / var_pc[eof]);
        /* Calculate squared sum */
        sum += (val * val);
      }
      /* Save for this timestep the square root of the sum of the squares of the distance to clusters */
      dist_pc[nt] = sqrt(sum);
    }
    /* Calculate mean over time */
    mean_dist[clust] = gsl_stats_mean(dist_pc, 1, ntime);
    /* Calculate variance over time */
    var_dist[clust] = gsl_stats_variance(dist_pc, 1, ntime);
    //    printf("ctrl dist pre... %d %lf %lf %lf %lf %lf\n",clust,sqrt(sum),pc[ntime-1+clust*ntime],sqrt(var_pc_norm_all[clust]),clusters[clust+clust*neof],var_pc[clust]);
    //    printf("ctrl dist.... %d %lf %lf %lf %lf\n",clust,dist_pc[ntime-1],sum,mean_dist[clust],var_dist[clust]);
  }

  /* Free memory */
  (void) free(dist_pc);
}
