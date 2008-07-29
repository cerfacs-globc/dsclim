/* ***************************************************** */
/* Algorithm to find best clusters among many tries.     */
/* best_clusters.c                                       */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file best_clusters.c
    \brief Algorithm to generate best clusters among many tries.
*/

#include <classif.h>

/** Algorithm to generate best clusters among many tries. */
void best_clusters(double *best_clusters, double *pc_eof_days, char *type, int npart, int nclassif, int neof, int ncluster, int ndays) {
  /**
     @param[out]     best_clusters      Best clusters' positions.
     @param[in]      pc_eof_days        Principal Components of EOF (daily data).
     @param[in]      type               Type of distance used. Possible values: euclidian.
     @param[in]      npart              Number of classification partitions to try.
     @param[in]      nclassif           Maximum number of classifications to perform in the iterative algorithm.
     @param[in]      neof               Number of EOFs.
     @param[in]      ncluster           Number of clusters.
     @param[in]      ndays              Number of days in the pc_eof_days vector.
  */

  double min_meandistval; /* Minimum distance between a partition and all other partitions */
  double meandistval; /* Mean distance value between each corresponding clusters for the comparison of two partitions. */
  double maxdistval; /* Maximum distance over all clusters for the two partitions comparison. */
  double minval; /* Minimum distance to find a corresponding closest cluster in another partition. */
  double dist_bary; /* Distance summed over all EOFs between a cluster in one partition and other clusters in other partitions. */
  double val; /* Difference in positions between a cluster in one partition and other clusters in other partitions for a particular EOF. */

  double *tmpcluster = NULL; /* Temporary vector of clusters for one partition. */
  double *testclusters = NULL; /* Temporary vector of clusters for all partitions. */
  
  int min_cluster = -1; /* Cluster number used to find a corresponding cluster in another partition. */
  int min_partition = -1; /* Partition number used to find the partition which has the minimum distance to all other partitions. */

  int part; /* Loop counter for partitions */
  int part1; /* Loop counter for partitions inside loop */
  int part2; /* Loop counter for partitions inside loop */
  int clust; /* Loop counter for clusters */
  int clust1; /* Loop counter for clusters inside loop */
  int clust2; /* Loop counter for clusters inside loop */
  int eof; /* Loop counter for eofs */

  (void) fprintf(stdout, "%s:: BEGIN: Find the best partition of clusters.\n", __FILE__);

  /* Allocate memory */
  tmpcluster = (double *) calloc(neof*ncluster, sizeof(double));
  if (tmpcluster == NULL) alloc_error(__FILE__, __LINE__);
  testclusters = (double *) calloc(neof*ncluster*npart, sizeof(double));
  if (testclusters == NULL) alloc_error(__FILE__, __LINE__);

  /* Generate npart clusters (which will be used to find the best clustering). */
  (void) fprintf(stdout, "%s:: Generating %d partitions of clusters.\n", __FILE__, npart);
  for (part=0; part<npart; part++) {
#if DEBUG >= 1
    (void) fprintf(stdout, "%s:: Generating %d/%d partition of clusters.\n", __FILE__, part, npart);
#endif
    (void) generate_clusters(tmpcluster, pc_eof_days, type, nclassif, neof, ncluster, ndays);
    for (clust=0; clust<ncluster; clust++)
      for (eof=0; eof<neof; eof++)
        testclusters[part+eof*npart+clust*npart*neof] = tmpcluster[eof+clust*neof];
  }

  /** Try to find best partition (clustering) which is closest to all the other partitions (which corresponds to
      the partition closest to the barycenter of partitions. */
  min_meandistval = 9999999999.9;
  min_partition = -1;
  /* Loop over all partition and compute distance between each other partition. */
  (void) fprintf(stdout, "%s:: Computing distance between each partitions of clusters.\n", __FILE__);
  for (part1=0; part1<npart; part1++) {
#if DEBUG >= 1
    (void) fprintf(stdout, "%s:: Partition %d/%d.\n", __FILE__, part1, npart);
#endif
    meandistval = 0.0;
    for (part2=0; part2<npart; part2++) {

      /* Don't compute for the same partition number. */
      if (part1 != part2) {

        maxdistval = -9999999999.9;
        
        for (clust1=0; clust1<ncluster; clust1++) {
          
          /* Find closest cluster to current one (in terms of distance summed over all EOF). */
          minval = 9999999999.9;
          min_cluster = -1;
          for (clust2=0; clust2<ncluster; clust2++) {

            if ( !strcmp(type, "euclidian") ) {
              /* Sum distances over all EOF. */
              dist_bary = 0.0;
              for (eof=0; eof<neof; eof++) {
                val = testclusters[part2+eof*npart+clust1*npart*neof] - testclusters[part1+eof*npart+clust2*npart*neof];
                dist_bary += (val * val);
              }
              
              dist_bary = sqrt(dist_bary);
            }
            else {
              (void) fprintf(stderr, "best_clusters: ABORT: Unknown distance type=%s!!\n", type);
              (void) abort();
            }
            
            /* Check for minimum distance. We want to find the corresponding closest cluster in another partition. */
            if (dist_bary < minval) {
              minval = dist_bary;
              min_cluster = clust2;
            }
          }

          if (min_cluster == -1) {
            (void) fprintf(stderr, "best_clusters: ABORT: Error in algorithm. Cannot find best cluster!\n");
            (void) abort();
          }
          
          /* Save the maximum distance over all clusters for the two partitions comparison. */
          if (minval > maxdistval)
            maxdistval = minval;
        }
        /* Sum the maximum distance of the clusters between each corresponding one over all the partitions.
           We want to compute the mean afterward. */
        meandistval += maxdistval;
      }
    }
    /* Compute the mean of the distances between each corresponding clusters for the comparison of two partitions. */
    meandistval = meandistval / (double) (npart-1);
    /* We want to keep the partition which has the minimum distance to all other partitions. */
    if (meandistval < min_meandistval) {
      min_meandistval = meandistval;
      min_partition = part1;
    }
  }

  if (min_partition == -1) {
    /* Failing algorithm */
    (void) fprintf(stderr, "best_clusters: ABORT: Error in algorithm. Cannot find best partition!\n");
    (void) abort();
  }

  /* Save data for the best selected partition of clusters. */
  (void) fprintf(stdout, "%s:: Save best partition of clusters.\n", __FILE__);
  for (clust=0; clust<ncluster; clust++)
    for (eof=0; eof<neof; eof++)
      best_clusters[eof+clust*neof] = testclusters[min_partition+eof*npart+clust*npart*neof];  

  /* Free memory. */
  (void) free(tmpcluster);
  (void) free(testclusters);

  (void) fprintf(stdout, "%s:: END: Find the best partition of clusters. Partition %d selected.\n", __FILE__, min_partition);
}
