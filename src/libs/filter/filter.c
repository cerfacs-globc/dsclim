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

/** Filter master subroutine. Uses wrap edges. */
void filter(double *bufferf, double *buffer, char *type, int width, int nx) {
  /**
     @param[out]     bufferf     Filtered version of buffer vector.
     @param[in]      buffer      Input vector data.
     @param[in]      type        Type of filter. Possible values: hanning.
     @param[in]      width       Width of filter.
     @param[in]      nx          Dimension of buffer input vector.
  */

  double *filter = NULL; /* Filter window vector */
  double *tmpvec = NULL; /* Temporary vector */

  int half_width; /* Half-width of filter window. */
  int i; /* Loop counter. */
  int ii; /* Loop counter. */

  double sum; /* To sum values over filter window width. */

  (void) fprintf(stdout, "%s: Filtering data with a %s filter.\n", __FILE__, type);

  if ( !strcmp(type, "hanning") ) {
    /* Hanning filter implementation */

    /* Compute filter window vector */
    (void) filter_window(&filter, type, width);
    
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

    /* Free memory */
    (void) free(filter);
    (void) free(tmpvec);
  }
  else {
    /* Unknown filter type */
    (void) fprintf(stderr, "%s: ABORT: Unknown filtering type: %s\n", __FILE__, type);
    (void) abort();
  }
}
