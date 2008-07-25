/* ***************************************************** */
/* Filter window subroutine.                             */
/* filter_window.c                                       */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file filter_window.c
    \brief Filter window subroutine. Uses hanning.
*/

#include <filter.h>

/** Filter window subroutine. Uses hanning. */
void filter_window(double **filter_window, char *type, int width) {
  /**
     @param[out]     filter_window     Output filter window vector.
     @param[in]      type              Type of filter. Possible values: hanning.
     @param[in]      width             Width of filter.
  */

  double scale_factor; /* Hanning filter scale factor. */
  double alpha = 0.5; /* alpha value for hanning filter. */
  double sum; /* Sum for normalizing filter window. */
  int i; /* Loop counter. */

  /* Check if number is odd. If it is, make it even by adding one. */
  if (width % 2 != 0) width++;

  /* Allocate memory */
  (*filter_window) = (double *) calloc(width, sizeof(double));
  if ((*filter_window) == NULL) alloc_error(__FILE__, __LINE__);
  
  if ( !strcmp(type, "hanning") ) {
    /** We are using a hanning filter. **/
    
    /* Scale factor */
    scale_factor = 2.0 * M_PI / (double) width;
    
    /* Compute filter window. */
    sum = 0.0;
    for (i=0; i<width; i++) {
      /* Hanning definition */
      (*filter_window)[i] = (alpha - 1.0) * cos( ((double) i) * scale_factor) + alpha;
      sum += (*filter_window)[i];
    }
    
    /* Normalizing to 1.0 */
    for (i=0; i<width; i++)
      (*filter_window)[i] /= sum;
  }
  else {
    /* Unknown filter type */
    (void) fprintf(stderr, "%s: ABORT: Unknown filtering type: %s\n", __FILE__, type);
    (void) abort();
  }
}
