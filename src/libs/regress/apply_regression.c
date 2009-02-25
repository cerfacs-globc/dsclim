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

#include <regress.h>

/** Compute a field using regression coefficients and the field values. */
void
apply_regression(double *buf, double *reg, double *cst, double *dist, double *sup_dist, int npts, int ntime,
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
