/* ***************************************************** */
/* Compute mean and variance of distances to clusters.   */
/* mean_variance_dist_clusters.c                         */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file mean_variance_dist_clusters.c
    \brief Compute mean and variance of distances to clusters.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2013)

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





#include <classif.h>

/** Compute mean and variance of distances to clusters. */
void
mean_variance_dist_clusters(double *mean_dist, double *var_dist, double *pc, double *clusters, double *var_pc,
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

  double sum = 0.0; /* Sum of normalized distances to clusters */
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
        val = (pc[nt+eof*ntime] / sqrt(var_pc_norm_all[eof])) - (clusters[eof+clust*neof] / sqrt(var_pc[eof]));
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

    //    printf("ctrl dist pre... %d %lf %lf %lf %lf %lf\n",clust,sqrt(sum),pc[ntime-1+clust*ntime],sqrt(var_pc_norm_all[clust]),clusters[clust+clust*neof],sqrt(var_pc[clust]));
    //    printf("ctrl dist.... %d %lf %lf %lf %lf\n",clust,dist_pc[ntime-1],sum,mean_dist[clust],sqrt(var_dist[clust]));
  }

  /* Free memory */
  (void) free(dist_pc);
}
