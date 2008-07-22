/* ***************************************************** */
/* Classification subroutine.                            */
/* classif.c                                             */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file classif.c
    \brief Classification subroutine.
*/

#include <classif.h>

void generate_clusters(double *clusters, double *pc_eof_days, char *type, int nclassif, int neof, int ncluster, int ndays) {

  unsigned long int random_num = NULL;
  int i;

  const gsl_rng_type *T;
  gsl_rng *rng;

  /** Generate ncluster random days **/

  /* Initialize random number generator */
  T = gsl_rng_default;
  rng = gsl_rng_alloc(T);
  /** Warning: we are using time() as the seed. Don't run this subroutine twice in the same second in time. **/
  (void) gsl_rng_set(rng, time());
  
  /* Generate ncluster random days and initialize cluster PC array */
  random_num = (unsigned long int *) calloc(ncluster, sizeof(unsigned long int));
  if (random_num == NULL) alloc_error();
  for (clust=0; clust<ncluster; clust++)
    random_num[clust] = gsl_rng_uniform_int(rng, ndays);  
  
  /* Free random number generator */
  (void) gsl_rng_free(rng);
  
  /** Main algorithm **/
  
  /* Initialize cluster PC array */
  for (eof=0; eof<neof; eof++)
    for (clust=0; clust<ncluster; clust++)
      eof_days_cluster[eof+clust*neof] = pc_eof_days[eof+random_num[clust]*neof];

  /* Perform up to nclassif classifications. Stop if same cluster barycenter positions. */
  bary_coords = -9999.9;
  classif = 0;
  while (same_bary_coords != 0.0 && classif < nclassif) {
    /* Classify each day (pc_eof_days) in the current clusters (eof_days_cluster) = days_class_cluster */
    (void) class_days_pc_centroids(days_class_cluster, pc_eof_days, eof_days_cluster, type, neof, ncluster, ndays);
    /* For each cluster, perform a mean of all points falling in that cluster.
       Compare to the current clusters by calculating the 'coordinates' (PC-space) of the 'new' cluster center. */
    bary_coords = -9999.9;
    for (eof=0; eof<neof; eof++) {
      for (clust=0; clust<ncluster; clust++) {
        mean_days = 0.0;
        ndays_cluster = 0;
        /* Compute the mean (PC-space) of all the days of the current cluster */
        for (days=0; days<ndays; days++)
          if (days_class_cluster[days] == clust) {
            mean_days += pc_eof_days[eof+day*neof];
            ndays_cluster++;
          }
        mean_days = mean_days / (double) ndays_cluster;
        /* Try to find the maximum distance (PC-space) between the new cluster center and the previous value */
        if ( fabs(mean_days - eof_days_cluster[eof+clust*neof]) > bary_coords )
          bary_coords = mean_days;
        /* Store the new cluster center value */
        clusters[eof+clust*neof] = mean_days;
      }
    }
    classif++;
    /* Update the cluster center matrix with the new values */
    for (eof=0; eof<neof; eof++)
      for (clust=0; clust<ncluster; clust++)
        eof_days_cluster[eof+clust*neof] = clusters[eof+clust*neof];
  }
}
