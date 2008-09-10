/* ***************************************************** */
/* Classification subroutine: find the closest cluster   */
/* of the principal components for each given day        */
/* in EOF space.                                         */
/* class_days_pc_clusters.c                             */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file class_days_pc_clusters.c
    \brief Classification subroutine find the closest cluster of the principal components for each given day in EOF space.            
*/

#include <classif.h>

/** Output a vector (dimension days) containing the closer (Euclidian distance) cluster number.
    The distance is computed as the distance between a day's principal components for each EOF and
    the cluster principal components for each EOF.
    To evaluate the closest one, all the square of the distances for all EOFs are summed for each cluster,
    before the square root is applied. */
void class_days_pc_clusters(int *days_class_cluster, double *pc_eof_days, double *eof_days_cluster, char *type,
                            int neof, int ncluster, int ndays) {
  /**
     @param[out]     days_class_cluster      Cluster number associated for each day.
     @param[in]      pc_eof_days             Principal Components of EOF (daily data).
     @param[in]      eof_days_cluster        Clusters' centroid positions for each eof.
     @param[in]      type                    Type of distance used. Possible values: euclidian.
     @param[in]      neof                    Number of EOFs.
     @param[in]      ncluster                Number of clusters.
     @param[in]      ndays                   Number of days in the pc_eof_days vector.
  */

  double dist_min; /* Minimum distance found between a given day PC (summed over all EOF) and each cluster centroid. */
  int clust_dist_min; /* Cluster number which has the minimum distance dist_min */
  double dist_sum; /* Sum of distances (partial computation) over all EOFs */
  double val; /* Distance between a given day PC (for a particular EOF) and one cluster centroid. */
  double dist_clust; /* Distance (full computation of dist_sum). */
  
  int day; /* Loop counter for days */
  int clust; /* Loop counter for cluster */
  int eof; /* Loop counter for eofs */

  if ( !strcmp(type, "euclidian") ) {
    /* Euclidian distance type */

    /* Parse each day */
    for (day=0; day<ndays; day++) {

      /* Initialize */
      dist_min = 9999999999.0;
      clust_dist_min = 999;

#if DEBUG >= 7
      (void) fprintf(stderr, "day=%d\n", day);
#endif

      /* Parse each cluster */
      for (clust=0; clust<ncluster; clust++) {

#if DEBUG >= 7
      (void) fprintf(stderr, "clust=%d\n", clust);
#endif

        dist_sum = 0.0;
        /* Sum all distances (over EOF) between the PC of the day and the PC of the cluster centroid for each EOF respectively */
        for (eof=0; eof<neof; eof++) {
          val = pc_eof_days[eof+day*neof] - eof_days_cluster[eof+clust*neof];
          /* Euclidian distance: square */
          dist_sum += (val * val);
        }
        /* Euclidian distance: square root of squares */
        dist_clust = sqrt(dist_sum);

#if DEBUG >= 7
      (void) fprintf(stderr, "dist_clust=%lf\n", dist_clust);
#endif

        /* Is it a cluster which has less distance as the minimum found yet ? */
        if (dist_clust < dist_min) {
          /* Save cluster number */
          clust_dist_min = clust;
          dist_min = dist_clust;
        }
      }
      if (clust_dist_min == 999) {
        /* Failing algorithm */
        (void) fprintf(stderr, "%s: ABORT: Impossible: no cluster was selected!! Problem in algorithm...\n", __FILE__);
        (void) abort();
      }
      /* Assign cluster with minimum distance to all EOFs for this day */
      days_class_cluster[day] = clust_dist_min;
#if DEBUG >= 9
      (void) fprintf(stderr, "%s: day %d cluster %d\n", __FILE__, day, clust_dist_min);
#endif
    }
  }
  else {
    /* Unknown distance type */
    (void) fprintf(stderr, "%s: ABORT: Unknown distance type=%s!!\n", __FILE__, type);
    (void) abort();
  }
}
