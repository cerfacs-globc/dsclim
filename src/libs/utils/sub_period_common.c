/* ***************************************************** */
/* Select a sub period of a vector using a common period */
/* over two different time vectors.                      */
/* sub_period_common.c                                   */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file sub_period_common.c
    \brief Select a sub period of a vector using a common period over two different time vectors.
*/

#include <utils.h>

void sub_period_common(double **buf_sub, int *ntime_sub, double *bufin, int *year, int *month, int *day,
                       int *year_learn, int *month_learn, int *day_learn, int timedim, int ndima, int ndimb, int ntime, int ntime_learn) {
  
  int *buf_sub_i = NULL;

  int dima, dimb;
  int t;
  int tt;

  *ntime_sub = 0;
  
  for (t=0; t<ntime; t++)
    for (tt=0; tt<ntime_learn; tt++)
      if (year[t]  == year_learn[tt] &&
          month[t] == month_learn[tt] &&
          day[t]   == day_learn[tt]) {
        buf_sub_i = (int *) realloc(buf_sub_i, ((*ntime_sub)+1) * sizeof(int));
        if (buf_sub_i == NULL) alloc_error(__FILE__, __LINE__);
        buf_sub_i[(*ntime_sub)++] = t;
      }
  
  (void) printf("%s: Sub-period: %d %d %d %d %d %d. Indexes: %d %d\n",__FILE__, year[buf_sub_i[0]], month[buf_sub_i[0]],
                day[buf_sub_i[0]], year[buf_sub_i[(*ntime_sub)-1]],month[buf_sub_i[(*ntime_sub)-1]],
                day[buf_sub_i[(*ntime_sub)-1]], buf_sub_i[0], buf_sub_i[(*ntime_sub)-1]);

  (*buf_sub) = (double *) malloc((*ntime_sub)*ndima*ndimb * sizeof(double));
  if ((*buf_sub) == NULL) alloc_error(__FILE__, __LINE__);
  if (timedim == 3)
    for (t=0; t<(*ntime_sub); t++)
      for (dimb=0; dimb<ndimb; dimb++)
        for (dima=0; dima<ndima; dima++)
          (*buf_sub)[dima+(dimb*ndima)+t*ndima*ndimb] = bufin[dima+dimb*ndima+buf_sub_i[t]*ndima*ndimb];
  else if (timedim == 1)
    for (t=0; t<(*ntime_sub); t++)
      for (dimb=0; dimb<ndimb; dimb++)
        for (dima=0; dima<ndima; dima++)
          (*buf_sub)[t+dima*(*ntime_sub)+dimb*(ndima*(*ntime_sub))] = bufin[buf_sub_i[t]+dima*ntime+dimb*(ndima*ntime)];
  else
    (void) fprintf(stderr, "%s: Fatal error: timedim argument must be equal to 1 or 3.\n");

  (void) free(buf_sub_i);
}
