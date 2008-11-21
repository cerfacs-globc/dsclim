/* ***************************************************** */
/* Normalize a 2D variable by the norm of the first      */
/* column of the first dimension and recompute the new   */
/* norm.                                                 */
/* normalize_pc.c                                        */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file normalize_pc.c
    \brief Normalize a 2D variable by the norm of the first column of the first dimension and recompute the new norm.
*/

#include <pceof.h>

/** Normalize a 2D variable by the norm of the first column of the first dimension and recompute the new norm. */
void normalize_pc(double *norm_all, double *first_variance, double *buf_renorm, double *bufin, int neof, int ntime) {

  /**
     @param[out]     norm_all       New norm vector
     @param[in,out]  first_variance Variance of the first column of the first dimension. Only calculate if not set to -9999.9999
     @param[out]     buf_renorm     Input 2D field normalized
     @param[in]      bufin          Input 2D field
     @param[in]      neof           EOF dimension
     @param[in]      ntime          Time dimension
  */

  int eof; /* EOF loop counter */
  int nt; /* Time loop counter */

  /* Check if we need to calculate the variance of the first column of the first dimension */
  if (*first_variance == -9999.9999) {
    eof = 0;
    *first_variance = gsl_stats_variance(&(bufin[eof*ntime]), 1, ntime);
    printf("first variance set=%lf %lf!\n",*first_variance,sqrt(*first_variance));
  }
  else
    printf("first variance=%lf %lf!\n",*first_variance,sqrt(*first_variance));

  /* Loop over all EOFs */
  for (eof=0; eof<neof; eof++) {

    //    for (nt=0; nt<ntime; nt++) {
    //      printf("norm_1 %d %d %lf\n",eof,nt,bufin[eof+nt*neof]);
    //    }

    /* Loop over time */
    for (nt=0; nt<ntime; nt++)
      /* Normalize with the first variance */
      buf_renorm[nt+eof*ntime] = bufin[nt+eof*ntime] / sqrt(*first_variance);

    //    for (nt=(ntime-5); nt<ntime; nt++)
    //      printf("bufin %d eof %d %lf\n",nt,eof,bufin[nt+eof*ntime]);

    //    printf("before renorm %d %d %lf %lf %lf ",eof,ntime,bufin[ntime-5+eof*ntime], sqrt(gsl_stats_variance(&(bufin[eof*ntime]), 1, ntime)));
    //    printf("renorm %lf\n",buf_renorm[ntime-5+eof*ntime]);

    /* Recompute the norm for each EOF */
    norm_all[eof] = gsl_stats_variance(&(buf_renorm[eof*ntime]), 1, ntime);

    //    printf("norm_all %d %lf\n",eof,norm_all[eof]);

  }
}
