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

void apply_regression(double *buf, double *reg, double *cst, double *dist, double *sup_dist, int npts, int ntime, int nclust, int nreg) {

  int nt;
  int pts;
  int clust;

  for (pts=0; pts<npts; pts++) {
    for (nt=0; nt<ntime; nt++) {
      buf[pts+nt*npts] = 0.0;
      for (clust=0; clust<nclust; clust++) {
        buf[pts+nt*npts] += (dist[nt+clust*ntime] * reg[pts+clust*npts]);
        //        if (pts == 0 && nt == (ntime-5))
        //          printf("%d %lf %lf %lf\n",clust,buf[pts+nt*npts],(dist[nt+clust*ntime]),(reg[pts+clust*npts]));
      }
      if (nclust == (nreg-1)) {
        buf[pts+nt*npts] += (sup_dist[nt] * reg[pts+(nreg-1)*npts]);
      }      
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
