/* ***************************************************** */
/* Filter subroutine.                                    */
/* filter.c                                              */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file filter.c
    \brief Filter subroutine. Uses hanning and wrap edges.
*/

#include <filter.h>

/** Filter subroutine. Uses hanning and wrap edges. */
void filter(double *bufferf, double *buffer, int width, int nx) {

  double *filter = NULL;
  double *tmpvec = NULL;

  int half_width;
  int i;
  int ii;

  double sum;

  /* Compute filter window vector */
  (void) filter_window(&filter, width);

  /* Half-width */
  half_width = ( width - 1 ) / 2;

  /* Expanded version of vector: wrapping edges. */
  tmpvec = (double *) calloc(nx*2, sizeof(double));
  if (tmpvec == NULL) alloc_error(__FILE__, __LINE__);

  for (i=0; i<half_width; i++) {
    tmpvec[i] = buffer[nx-half_width+i];
  }
  for (i=half_width; i<(half_width+nx); i++) {
    tmpvec[i] = buffer[i-half_width];
  }
  for (i=(half_width+nx); i<(nx*2); i++) {
    tmpvec[i] = buffer[i-(half_width+nx)];
  }

  /* Apply filter. */
  for (i=0; i<nx; i++) {
    sum = 0.0;
    for (ii=i; ii<(i+width-1); ii++)
      sum += (filter[ii-i] * tmpvec[ii]);
    bufferf[i] = sum;
  }

  (void) free(filter);
  (void) free(tmpvec);
}
