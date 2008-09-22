/* ***************************************************** */
/* Extract a sub period of a vector of selected months.  */
/* extract_subperiod.c                                   */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file extract_subperiod.c
    \brief Extract a sub period of a vector of selected months.
*/

#include <utils.h>

void extract_subperiod(double **buf_sub, int *ntime_sub, double *bufin, int *year, int *month, int *day,
                       int *smonths, int ndima, int ndimb, int ntime, int nmonths) {
  
  int *buf_sub_i = NULL;

  int i;
  int j;
  int t;
  int tt;

  *ntime_sub = 0;
  
  for (t=0; t<ntime; t++)
    for (tt=0; tt<nmonths; tt++)
      if (month[t] == smonths[tt]) {
        buf_sub_i = (int *) realloc(buf_sub_i, ((*ntime_sub)+1) * sizeof(int));
        if (buf_sub_i == NULL) alloc_error(__FILE__, __LINE__);
        buf_sub_i[(*ntime_sub)++] = t;
      }
  
  (*buf_sub) = (double *) malloc((*ntime_sub)*ndima*ndimb * sizeof(double));
  if ((*buf_sub) == NULL) alloc_error(__FILE__, __LINE__);
  for (t=0; t<(*ntime_sub); t++)
    for (j=0; j<ndimb; j++)
      for (i=0; i<ndima; i++)
        (*buf_sub)[i+j*ndima+t*ndima*ndimb] = bufin[i+j*ndima+buf_sub_i[t]*ndima*ndimb];
  
  (void) free(buf_sub_i);
}
