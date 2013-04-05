/* ***************************************************** */
/* Compute distances to clusters normalized              */
/* by control run mean and variance.                     */
/* dist_clusters_normctrl.c                              */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file dist_clusters_normctrl.c
    \brief Compute distances to clusters normalized by control run mean and variance.
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

/** Compute distances to clusters normalized by control run mean and variance. */
void
dist_clusters_normctrl(double *dist_pc, double *pc, double *clusters, double *var_pc,
                       double *var_pc_norm_all, double *mean_ctrl, double *var_ctrl,
                       int neof, int nclust, int ntime) {
  /**
     @param[out]  dist_pc         Distances (normalized by control run mean and variance) of normalized EOF-projected large-scale field to clusters
     @param[in]   pc              EOF-projected large-scale field
     @param[in]   clusters        Cluster centroids for each EOF in EOF-projected space of the large-scale field
     @param[in]   var_pc          Variance of EOF-projected large-scale field of the learning period, for each EOF separately.
     @param[in]   var_pc_norm_all Norm of the variance of the first EOF of the EOF-projected large-scale field of the control run.
     @param[in]   mean_ctrl       Mean of the distances to clusters for the control run, for each cluster separately.
     @param[in]   var_ctrl        Variance of the distances to clusters for the control run, for each cluster separately.
     @param[in]   neof            EOF dimension
     @param[in]   nclust          Clusters dimension
     @param[in]   ntime           Time dimension
  */

  double sum;
  double val;
  
  int eof;
  int nt;
  int clust;

  for (clust=0; clust<nclust; clust++) {
    for (nt=0; nt<ntime; nt++) {
      sum = 0.0;
      for (eof=0; eof<neof; eof++) {
        val = (pc[nt+eof*ntime] / sqrt(var_pc_norm_all[eof])) - (clusters[eof+clust*neof] / sqrt(var_pc[eof]));
        sum += (val * val);
        //        if (nt == (ntime-5)) {
        //          printf("dist_val %d %d %d %lf %lf %lf %lf %lf\n",clust,nt,eof,val,pc[nt+eof*ntime],var_pc_norm_all[eof],clusters[eof+clust*neof],var_pc[eof]);
        //        }
      }
      dist_pc[nt+clust*ntime] = ( (sqrt(sum) - mean_ctrl[clust]) / sqrt(var_ctrl[clust]) );
      //      if (nt == (ntime -5))
      //        printf("dist_pc %d %d %lf %lf %lf %lf %lf\n",clust,nt,dist_pc[nt+clust*ntime],sqrt(sum),mean_ctrl[clust],var_ctrl[clust]);
      //    }
    }
  }
}
