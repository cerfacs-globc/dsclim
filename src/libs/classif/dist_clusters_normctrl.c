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

#include <classif.h>

void dist_clusters_normctrl(double *dist_pc, double *pc, double *clusters, double *var_pc,
                            double *var_pc_norm_all, double *mean_ctrl, double *var_ctrl,
                            int neof, int nclust, int ntime) {
  double sum;
  double val;
  
  int eof;
  int nt;
  int clust;

  for (clust=0; clust<nclust; clust++) {
    for (nt=0; nt<ntime; nt++) {
      sum = 0.0;
      for (eof=0; eof<neof; eof++) {
        val = (pc[nt+eof*ntime] / sqrt(var_pc_norm_all[eof])) - (clusters[eof+clust*neof] / var_pc[eof]);
        sum += (val * val);
        //        if (nt == (ntime-5)) {
        //          printf("dist_val %d %d %d %lf %lf %lf %lf %lf\n",clust,nt,eof,val,pc[nt+eof*ntime],sqrt(var_pc_norm_all[eof]),clusters[eof+clust*neof],var_pc[eof]);
        //        }
      }
      dist_pc[nt+clust*ntime] = ( (sqrt(sum) - mean_ctrl[clust]) / sqrt(var_ctrl[clust]) );
      //      if (nt == (ntime -5))
      //        printf("dist_pc %d %d %lf %lf %lf %lf %lf\n",clust,nt,dist_pc[nt+clust*ntime],sqrt(sum),mean_ctrl[clust],var_ctrl[clust]);
      //    }
    }
  }
}
