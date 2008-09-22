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

#include <utils.h>

#include <gsl/gsl_statistics.h>

void normalize_pc(double *norm_all, double *buf_renorm, double *bufin, int neof, int ntime) {

  double *buftmpt = NULL;
  double first_variance = 1.0;

  int eof;
  int nt;

  buftmpt = (double *) malloc(ntime * sizeof(double));
  if (buftmpt == NULL) alloc_error(__FILE__, __LINE__);

  for (eof=0; eof<neof; eof++) {

    for (nt=0; nt<ntime; nt++)
      buftmpt[nt] = bufin[eof+nt*neof];

    norm_all[eof] = sqrt(gsl_stats_variance(buftmpt, 1, ntime));

    if (eof == 0)
      first_variance = norm_all[0];

    for (nt=0; nt<ntime; nt++)
      buf_renorm[nt+eof*ntime] = bufin[eof+nt*neof] / first_variance;

    norm_all[eof] = sqrt(gsl_stats_variance(&(buf_renorm[eof*ntime]), 1, ntime));

  }

  (void) free(buftmpt);
}
