/* ***************************************************** */
/* Algorithm to generate clusters.                       */
/* generate_clusters.c                                   */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file generate_clusters.c
    \brief Algorithm to generate clusters.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2009)

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

/** Algorithm to generate clusters based on the Michelangeli et al (1995) methodology. */
int
generate_clusters(double *clusters, double *pc_eof_days, char *type, int nclassif,
                  int neof, int ncluster, int ndays) {
  /**
     @param[out]     clusters      Clusters' positions.
     @param[in]      pc_eof_days   Principal Components of EOF (daily data).
     @param[in]      type          Type of distance used. Possible values: euclidian.
     @param[in]      nclassif      Maximum number of classifications to perform in the iterative algorithm.
     @param[in]      neof          Number of EOFs.
     @param[in]      ncluster      Number of clusters.
     @param[in]      ndays         Number of days in the pc_eof_days vector.

     \return         Number of iterations.
  */

  unsigned long int *random_num = NULL; /* Vector of random numbers for random choice of initial points. */
  int eof; /* Loop counter for EOF. */
  int clust; /* Loop counter for clusters. */
  int day; /* Loop counter for days. */
  int classif; /* Loop counter for classifications. */
  int ndays_cluster; /* Number of days in the current cluster. */

  const gsl_rng_type *T; /* For random number generation type. */
  gsl_rng *rng; /* For random number generation. */

  double cluster_bary; /* Cluster barycentre. */
  double ndiff_cluster_bary; /* Distance between current cluster barycenter and previous value in the iteration. */
  double mean_days; /* Mean of the days (PC-space) for a cluster. */
  double *eof_days_cluster = NULL; /* Vector of clusters' barycenter positions (PC-space). */
  int *days_class_cluster = NULL; /* Vector of classification of days into each cluster. */

  static unsigned long int seed = 0;
  
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
  if (seed == 0) seed = time(NULL);
  (void) gsl_rng_set(rng, seed++);
  
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
  
  /* Initialize cluster PC array randomly */
  (void) fprintf(stdout, "%s:: Initializing cluster array.\n", __FILE__);
  for (eof=0; eof<neof; eof++)
    for (clust=0; clust<ncluster; clust++) {
      eof_days_cluster[eof+clust*neof] = pc_eof_days[random_num[clust]+eof*ndays];

#if DEBUG >= 7
      (void) fprintf(stderr, "eof=%d cluster=%d eof_days_cluster=%lf\n", eof, clust, eof_days_cluster[eof+clust*neof]);
#endif
    }

  (void) free(random_num);

  /* Iterate by performing up to nclassif classifications. Stop if same cluster center positions in two consecutive iterations. */
  cluster_bary = -9999999999.9;
  ndiff_cluster_bary = -9999999999.9;
  classif = 0;

  (void) fprintf(stdout, "%s:: Iterate up to %d classifications or when classification is stable.\n", __FILE__, nclassif);

  while (ndiff_cluster_bary != 0.0 && classif < nclassif) {

#if DEBUG >= 7
    (void) fprintf(stderr, "classif=%d cluster_bary=%lf\n", classif, cluster_bary);
#endif

    /* Classify each day (pc_eof_days) in the current clusters (eof_days_cluster) = days_class_cluster */
    (void) class_days_pc_clusters(days_class_cluster, pc_eof_days, eof_days_cluster, type, neof, ncluster, ndays);

    /* For each cluster, perform a mean of all points falling in that cluster.
       Compare to the current clusters by calculating the 'coordinates' (PC-space) of the 'new' cluster center. */
    cluster_bary = -9999999999.9;

    /* Loop over clusters and EOFs */
    for (clust=0; clust<ncluster; clust++) {
      for (eof=0; eof<neof; eof++) {
        mean_days = 0.0;
        ndays_cluster = 0;

        /* Compute the mean (PC-space) of all the days of the current cluster */
        for (day=0; day<ndays; day++)
          if (days_class_cluster[day] == clust) {
            mean_days += pc_eof_days[day+eof*ndays];
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

          /* If this new cluster center position is further away than the other EOF's ones, choose this new cluster center */
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

  (void) fprintf(stdout, "%s:: END: Find clusters among data points. %d iterations needed.\n", __FILE__, classif);

  return classif;
}
