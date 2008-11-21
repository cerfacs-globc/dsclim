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

/** Select a sub period of a vector using a common period over two different time vectors. */
void sub_period_common(double **buf_sub, int *ntime_sub, double *bufin, int *year, int *month, int *day,
                       int *year_learn, int *month_learn, int *day_learn, int timedim, int ndima, int ndimb, int ntime, int ntime_learn) {
  /**
     @param[out]  buf_sub      Output 3D buffer spanning common time period
     @param[out]  ntime_sub    Number of times for the common time period (time dimension length)
     @param[in]   bufin        Input 3D buffer (ndima * ndimb * ntime)
     @param[in]   year         Year vector for the first time vector
     @param[in]   month        Month vector for the first time vector
     @param[in]   day          Day vector for the first time vector
     @param[in]   year_learn   Year vector for the second time vector
     @param[in]   month_learn  Month vector for the second time vector
     @param[in]   day_learn    Day vector for the second time vector
     @param[in]   timedim      Position of the time period dimension (1 or 3)
     @param[in]   ndima        First dimension
     @param[in]   ndimb        Second dimension
     @param[in]   ntime        Time dimension of the first time vector
     @param[in]   ntime_learn  Time dimension of the second time vector
   */
  
  int *buf_sub_i = NULL; /* Time indexes for common period */

  int dima; /* First dimension */
  int dimb; /* Second dimension */
  int t; /* Time loop counter */
  int tt; /* Time loop counter for second time vector */

  /* Initialize number of common times */
  *ntime_sub = 0;
  
  /* Loop over first time vector and find common day/month/year and store time indexes for these common times */
  for (t=0; t<ntime; t++)
    /* Search in all second time vector times for matching date */
    for (tt=0; tt<ntime_learn; tt++)
      if (year[t]  == year_learn[tt] &&
          month[t] == month_learn[tt] &&
          day[t]   == day_learn[tt]) {
        /* Found common date, store time index */
        buf_sub_i = (int *) realloc(buf_sub_i, ((*ntime_sub)+1) * sizeof(int));
        if (buf_sub_i == NULL) alloc_error(__FILE__, __LINE__);
        buf_sub_i[(*ntime_sub)++] = t;
      }
  
  (void) printf("%s: Sub-period: %d %d %d %d %d %d. Indexes: %d %d\n",__FILE__, year[buf_sub_i[0]], month[buf_sub_i[0]],
                day[buf_sub_i[0]], year[buf_sub_i[(*ntime_sub)-1]],month[buf_sub_i[(*ntime_sub)-1]],
                day[buf_sub_i[(*ntime_sub)-1]], buf_sub_i[0], buf_sub_i[(*ntime_sub)-1]);

  /* Allocate memory for output buffer */
  (*buf_sub) = (double *) malloc((*ntime_sub)*ndima*ndimb * sizeof(double));
  if ((*buf_sub) == NULL) alloc_error(__FILE__, __LINE__);
  /* Construct new 3D matrix with common times */
  if (timedim == 3)
    /* Time dimension is last */
    for (t=0; t<(*ntime_sub); t++)
      for (dimb=0; dimb<ndimb; dimb++)
        for (dima=0; dima<ndima; dima++)
          (*buf_sub)[dima+(dimb*ndima)+t*ndima*ndimb] = bufin[dima+dimb*ndima+buf_sub_i[t]*ndima*ndimb];
  else if (timedim == 1)
    /* Time dimension is first */
    for (t=0; t<(*ntime_sub); t++)
      for (dimb=0; dimb<ndimb; dimb++)
        for (dima=0; dima<ndima; dima++)
          (*buf_sub)[t+dima*(*ntime_sub)+dimb*(ndima*(*ntime_sub))] = bufin[buf_sub_i[t]+dima*ntime+dimb*(ndima*ntime)];
  else
    (void) fprintf(stderr, "%s: Fatal error: timedim argument must be equal to 1 or 3.\n", __FILE__);

  /* Free memory */
  (void) free(buf_sub_i);
}
