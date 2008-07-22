/* ***************************************************** */
/* Algorithm to generate clusters.                       */
/* generate_clusters.c                                   */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file generate_clusters.c
    \brief Algorithm to generate clusters.
*/

#include <classif.h>

void generate_clusters(double *clusters, double *pc_eof_days, char *type, int nclassif, int neof, int ncluster, int ndays) {

  unsigned long int *random_num = NULL;
  int eof;
  int clust;
  int day;
  int classif;
  int ndays_cluster;

  const gsl_rng_type *T;
  gsl_rng *rng;

  double bary_coords;
  double mean_days;
  double *eof_days_cluster = NULL;
  int *days_class_cluster = NULL;
  
  /***********************************/
  /** Generate ncluster random days **/
  /***********************************/

  /* Initialize random number generator */
  T = gsl_rng_default;
  rng = gsl_rng_alloc(T);
  /** Warning: we are using time() as the seed. Don't run this subroutine twice with the same time.
      If you do you will get the exact same sequence. **/
  (void) gsl_rng_set(rng, time());
  
  /* Generate ncluster random days and initialize cluster PC array */
  random_num = (unsigned long int *) calloc(ncluster, sizeof(unsigned long int));
  if (random_num == NULL) alloc_error();
  for (clust=0; clust<ncluster; clust++)
    random_num[clust] = gsl_rng_uniform_int(rng, ndays);  
  
  /* Free random number generator */
  (void) gsl_rng_free(rng);
  
  /********************/
  /** Main algorithm **/
  /********************/

  /* Allocate memory */
  eof_days_cluster = (double *) calloc(neof*ncluster, sizeof(double));
  if (eof_days_cluster == NULL) alloc_error();
  days_class_cluster = (int *) calloc(ndays, sizeof(int));
  if (days_class_cluster == NULL) alloc_error();
  
  /* Initialize cluster PC array */
  for (eof=0; eof<neof; eof++)
    for (clust=0; clust<ncluster; clust++)
      eof_days_cluster[eof+clust*neof] = pc_eof_days[eof+random_num[clust]*neof];

  (void) free(random_num);

  /* Perform up to nclassif classifications. Stop if same cluster barycenter positions. */
  bary_coords = -9999.9;
  classif = 0;
  while (bary_coords != 0.0 && classif < nclassif) {
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
        for (day=0; day<ndays; day++)
          if (days_class_cluster[day] == clust) {
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

  /* Free memory */
  (void) free(eof_days_cluster);
  (void) free(days_class_cluster);
}
