/* ***************************************************** */
/* Filter subroutine.                                    */
/* filter.c                                              */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file filter.c
    \brief Filter subroutine. Uses hanning and wrap edges.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2015)

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







#include <filter.h>

/** Filter master subroutine. Uses wrap edges. */
void
filter(double *bufferf, double *buffer, char *type, int width, int ni, int nj, int nt) {
  /**
     @param[out]     bufferf     Filtered version of buffer input matrix.
     @param[in]      buffer      Input matrix.
     @param[in]      type        Type of filter. Possible values: hanning.
     @param[in]      width       Width of filter.
     @param[in]      ni          Horizontal dimension of buffer input matrix.
     @param[in]      nj          Horizontal dimension of buffer input matrix.
     @param[in]      nt          Temporal dimension of buffer input matrix.
  */

  double *filter = NULL; /* Filter window vector */
  double *tmpvec = NULL; /* Temporary vector */

  int half_width; /* Half-width of filter window. */
  int i; /* Loop counter for ni. */
  int j; /* Loop counter for nj. */
  int t; /* Loop counter. */
  int tt; /* Loop counter. */

  double sum; /* To sum values over filter window width. */

  /*  (void) fprintf(stdout, "%s: Filtering data with a %s filter.\n", __FILE__, type);*/

  if ( !strcmp(type, "hanning") ) {
    /* Hanning filter implementation */

    /* Compute filter window vector */
    (void) filter_window(&filter, type, width);
    
    /* Half-width */
    half_width = ( width - 1 ) / 2;
    
    /* Expanded version of vector: wrapping edges. */
    tmpvec = (double *) calloc(nt*2, sizeof(double));
    if (tmpvec == NULL) alloc_error(__FILE__, __LINE__);

    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++) {
        
        for (t=0; t<half_width; t++) {
          tmpvec[t] = buffer[i+j*ni+(nt-half_width+t)*ni*nj];
        }
        for (t=half_width; t<(half_width+nt); t++) {
          tmpvec[t] = buffer[i+j*ni+(t-half_width)*ni*nj];
        }
        for (t=(half_width+nt); t<(nt*2); t++) {
          tmpvec[t] = buffer[i+j*ni+(t-(half_width+nt))*ni*nj];
        }
        
        /* Apply filter. */
        for (t=0; t<nt; t++) {
          sum = 0.0;
          for (tt=t; tt<(t+width-1); tt++)
            sum += (filter[tt-t] * tmpvec[tt]);
          bufferf[i+j*ni+t*ni*nj] = sum;
        }
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
