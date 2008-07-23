/* ***************************************************** */
/* Classification subroutine: find the closest cluster   */
/* of the principal components for each given day        */
/* in EOF space.                                         */
/* class_days_pc_centroids.c                             */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file class_days_pc_centroids.c
    \brief Classification subroutine find the closest cluster of the principal components for each given day in EOF space.            
*/

#include <classif.h>

/** Output a vector (dimension days) containing the closer (Euclidian distance) cluster number.
    The distance is computed as the distance between a day's principal components for each EOF and
    the cluster principal components for each EOF.
    To evaluate the closest one, all the square of the distances for all EOFs are summed for each cluster,
    before the square root is applied. */
void class_days_pc_centroids(int *days_class_cluster, double *pc_eof_days, double *eof_days_cluster, char *type,
                             int neof, int ncluster, int ndays) {

  double dist_min;
  int clust_dist_min;
  double dist_sum;
  double val;
  double dist_clust;
  
  int day;
  int clust;
  int eof;

  if ( !strcmp(type, "euclidian") ) {

    /* Parse each day */
    for (day=0; day<ndays; day++) {
      /* Initialize */
      dist_min = 9999.0;
      clust_dist_min = 9999;
      /* Parse each cluster */
      for (clust=0; clust<ncluster; clust++) {
        dist_sum = 0.0;
        /* Sum all distances (over EOF) between the PC of the day and the PC of the cluster centroid for each EOF respectively */
        for (eof=0; eof<neof; eof++) {
          val = pc_eof_days[eof+day*neof] - eof_days_cluster[eof+clust*neof];
          /* Euclidian distance: square */
          dist_sum += (val * val);
        }
        /* Euclidian distance: square root of squares */
        dist_clust = sqrt(dist_sum);
        /* Is it a cluster which has less distance as the minimum found yet ? */
        if (dist_clust < dist_min) {
          /* Save cluster number */
          clust_dist_min = clust;
          dist_min = dist_clust;
        }
      }
      if (clust_dist_min == 9999) {
        (void) fprintf(stderr, "class_days_pc_centroids: ABORT: Impossible: no cluster was selected!! Problem in algorithm...\n");
        exit(1);
      }
      /* Assign cluster with minimum distance to all EOFs for this day */
      days_class_cluster[day] = clust_dist_min;
#ifdef DEBUG
      /*      (void) fprintf(stderr, "day %d cluster %d\n", day, clust_dist_min);*/
#endif
    }
  }
  else {
    (void) fprintf(stderr, "class_days_pc_centroids: ABORT: Unknown distance type=%s!!\n", type);
    exit(1);
  }
}
