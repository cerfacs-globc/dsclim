/* ***************************************************** */
/* Compute a field using regression coefficients         */
/* and the field values.                                 */
/* apply_regression.c                                    */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file apply_regression.c
    \brief Compute a field using regression coefficients and the field values.
*/

#include <regress.h>

/** Compute a field using regression coefficients and the field values. */
void apply_regression(double *buf, double *reg, double *cst, double *dist, double *sup_dist, int npts, int ntime,
                      int nclust, int nreg) {
  /**
     @param[out]  buf        2D field
     @param[in]   reg        Regression coefficients
     @param[in]   cst        Regression constant
     @param[in]   dist       Values of input vector
     @param[in]   sup_dist   If there is one supplemental regression coefficient, use a supplemental vector
     @param[in]   npts       Points dimension
     @param[in]   ntime      Time dimension
     @param[in]   nclust     Cluster dimension
     @param[in]   nreg       Regression dimension
   */

  int nt; /* Time loop counter */
  int pts; /* Points loop counter */
  int clust; /* Cluster loop counter */

  /* Loop over all regression points */
  for (pts=0; pts<npts; pts++) {
    /* Loop over all times */
    for (nt=0; nt<ntime; nt++) {
      /* Initialize value */
      buf[pts+nt*npts] = 0.0;
      /* Loop over all clusters and add each value after regression is applied */
      for (clust=0; clust<nclust; clust++) {
        buf[pts+nt*npts] += (dist[nt+clust*ntime] * reg[pts+clust*npts]);

        //        if (pts == 0 && nt == (ntime-5))
        //          printf("%d %lf %lf %lf\n",clust,buf[pts+nt*npts],(dist[nt+clust*ntime]),(reg[pts+clust*npts]));
      }
      /* Extra regression coefficient with a second supplemental vector */
      if (nclust == (nreg-1)) {
        buf[pts+nt*npts] += (sup_dist[nt] * reg[pts+(nreg-1)*npts]);
      }      
      
      /* Add regression constant */
      buf[pts+nt*npts] += cst[pts];

      //     if (pts == 0 && nt == (ntime-5))
      //       printf("%lf %lf\n",buf[pts+nt*npts],cst[pts]);
    }
  }
  //  nt = ntime-1;
  //  pts = 0;
  //  for (clust=0; clust<nclust; clust++)
  //    printf("REGRESS %d %lf %lf %lf %lf\n",clust,buf[pts+nt*npts],dist[nt+clust*ntime],reg[pts+clust*npts],cst[pts]);
}
