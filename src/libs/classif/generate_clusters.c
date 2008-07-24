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

  double cluster_bary;
  double ndiff_cluster_bary;
  double mean_days;
  double *eof_days_cluster = NULL;
  int *days_class_cluster = NULL;
  
  (void) fprintf(stdout, "%s:: BEGIN: Find clusters among data points.\n", __FILE__);

  /***********************************/
  /** Generate ncluster random days **/
  /***********************************/

  (void) fprintf(stdout, "%s:: Choosing %d random points.\n", __FILE__, ncluster);

  /* Initialize random number generator */
  T = gsl_rng_default;
  rng = gsl_rng_alloc(T);
  /** Warning: we are using time() as the seed. Don't run this subroutine twice with the same time.
      If you do you will get the exact same sequence. **/
  (void) gsl_rng_set(rng, time(NULL));
  
  /* Generate ncluster random days and initialize cluster PC array */
  random_num = (unsigned long int *) calloc(ncluster, sizeof(unsigned long int));
  if (random_num == NULL) alloc_error(__FILE__, __LINE__);
  for (clust=0; clust<ncluster; clust++)
    random_num[clust] = gsl_rng_uniform_int(rng, ndays);  
  
  /* Free random number generator */
  (void) gsl_rng_free(rng);
  
  /********************/
  /** Main algorithm **/
  /********************/

  /* Allocate memory */
  eof_days_cluster = (double *) calloc(neof*ncluster, sizeof(double));
  if (eof_days_cluster == NULL) alloc_error(__FILE__, __LINE__);
  days_class_cluster = (int *) calloc(ndays, sizeof(int));
  if (days_class_cluster == NULL) alloc_error(__FILE__, __LINE__);
  
  /* Initialize cluster PC array */
  (void) fprintf(stdout, "%s:: Initializing cluster array.\n", __FILE__);
  for (clust=0; clust<ncluster; clust++)
    for (eof=0; eof<neof; eof++) {
      eof_days_cluster[eof+clust*neof] = pc_eof_days[eof+random_num[clust]*neof];

#if DEBUG >= 7
      /*      (void) fprintf(stderr, "eof=%d cluster=%d eof_days_cluster=%lf\n", eof, clust, eof_days_cluster[eof+clust*neof]);*/
#endif
    }

  (void) free(random_num);

  /* Iterate by performing up to nclassif classifications. Stop if same cluster center positions in two consecutive iterations. */
  cluster_bary = -9999.9;
  ndiff_cluster_bary = -9999.9;
  classif = 0;

  (void) fprintf(stdout, "%s:: Iterate up to %d classifications or when classification is stable.\n", __FILE__, nclassif);

  while (ndiff_cluster_bary != 0.0 && classif < nclassif) {

#if DEBUG >= 7
    (void) fprintf(stderr, "classif=%d cluster_bary=%lf\n", classif, cluster_bary);
#endif

    /* Classify each day (pc_eof_days) in the current clusters (eof_days_cluster) = days_class_cluster */
    (void) class_days_pc_centroids(days_class_cluster, pc_eof_days, eof_days_cluster, type, neof, ncluster, ndays);

    /* For each cluster, perform a mean of all points falling in that cluster.
       Compare to the current clusters by calculating the 'coordinates' (PC-space) of the 'new' cluster center. */
    cluster_bary = -9999.9;

    for (clust=0; clust<ncluster; clust++) {
      for (eof=0; eof<neof; eof++) {
        mean_days = 0.0;
        ndays_cluster = 0;

        /* Compute the mean (PC-space) of all the days of the current cluster */
        for (day=0; day<ndays; day++)
          if (days_class_cluster[day] == clust) {
            mean_days += pc_eof_days[eof+day*neof];
            ndays_cluster++;
          }

        if (ndays_cluster > 0) {

          mean_days = mean_days / (double) ndays_cluster;

          /** Try to find the maximum distance (PC-space) between the new cluster center and the previous value **/

#if DEBUG >= 7
          (void) fprintf(stderr, "eof=%d cluster=%d diff_cluster_bary=%lf mean_days=%lf eof_days_cluster=%lf\n",
                         eof, clust, fabs(mean_days - eof_days_cluster[eof+clust*neof]), mean_days, eof_days_cluster[eof+clust*neof]);
#endif

          /* Compute the difference between the new cluster center position and the previous one */
          ndiff_cluster_bary = fabs(mean_days - eof_days_cluster[eof+clust*neof]);

          /* If this new cluster center position is further away than the other EOF's ones, chosse this new cluster center */
          if ( ndiff_cluster_bary > cluster_bary )
            cluster_bary = mean_days;

          /* Store the new cluster center value */
          clusters[eof+clust*neof] = mean_days;

#if DEBUG >= 7
          if (classif == 0 || cluster_bary == 0.0)
            (void) fprintf(stderr, "eof=%d cluster=%d mean_pc_days=%lf ndays_cluster=%d cluster_bary=%lf\n",
                           eof, clust, mean_days, ndays_cluster, cluster_bary);
#endif
        }
        else
          clusters[eof+clust*neof] = 0;
      }
    }

    classif++;

    /* Update the cluster center matrix with the new values */
    if (cluster_bary != 0.0 && classif < nclassif)
      for (clust=0; clust<ncluster; clust++)
        for (eof=0; eof<neof; eof++)
          eof_days_cluster[eof+clust*neof] = clusters[eof+clust*neof];
  }

  /* Free memory */
  (void) free(eof_days_cluster);
  (void) free(days_class_cluster);

  (void) fprintf(stdout, "%s:: END: Find clusters among data points.\n", __FILE__);
}
