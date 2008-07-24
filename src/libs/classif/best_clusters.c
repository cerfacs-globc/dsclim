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

void best_clusters(double *best_clusters, double *pc_eof_days, char *type, int npart, int nclassif, int neof, int ncluster, int ndays) {

  double min_meandistval;
  double meandistval;
  double maxdistval;
  double minval;
  double dist_bary;
  double val;

  double *tmpcluster = NULL;
  double *testclusters = NULL;
  
  int min_cluster = -1;
  int min_partition = -1;

  int part;
  int part1;
  int part2;
  int clust;
  int clust1;
  int clust2;
  int eof;

  /* Allocate memory */
  tmpcluster = (double *) calloc(neof*ncluster, sizeof(double));
  if (tmpcluster == NULL) alloc_error(__FILE__, __LINE__);
  testclusters = (double *) calloc(neof*ncluster*npart, sizeof(double));
  if (testclusters == NULL) alloc_error(__FILE__, __LINE__);

  /* Generate npart clusters, in the attempt to find the best clustering. */
  for (part=0; part<npart; part++) {
    (void) generate_clusters(tmpcluster, pc_eof_days, type, nclassif, neof, ncluster, ndays);
    for (clust=0; clust<ncluster; clust++)
      for (eof=0; eof<neof; eof++)
        testclusters[part+eof*npart+clust*npart*neof] = tmpcluster[eof+clust*neof];
  }

  /** Try to find best partition (clustering) which is closest to all the other partitions (which corresponds to
      the partition closest to the barycenter of partitions. */
  min_meandistval = 999999.9;
  min_partition = -1;
  /* Loop over all partition and compute distance between each other partition. */
  for (part1=0; part1<npart; part1++) {
    meandistval = 0.0;
    for (part2=0; part2<npart; part2++) {

      /* Don't compute for the same partition number. */
      if (part1 != part2) {

        maxdistval = -999999.9;
        
        for (clust1=0; clust1<ncluster; clust1++) {
          
          /* Find closest cluster to current one (in terms of distance summed over all EOF). */
          minval = 999999.9;
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
    (void) fprintf(stderr, "best_clusters: ABORT: Error in algorithm. Cannot find best partition!\n");
    (void) abort();
  }

  /* Save data for the best selected partition of clusters. */
  for (clust=0; clust<ncluster; clust++)
    for (eof=0; eof<neof; eof++)
      best_clusters[eof+clust*neof] = testclusters[min_partition+eof*npart+clust*npart*neof];  

  /* Free memory. */
  (void) free(tmpcluster);
  (void) free(testclusters);
}
