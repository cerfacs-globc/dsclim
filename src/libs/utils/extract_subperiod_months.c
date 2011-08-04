/* ***************************************************** */
/* Extract a sub period of a vector of selected months.  */
/* extract_subperiod.c                                   */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file extract_subperiod_months.c
    \brief Extract a sub period of a vector of selected months.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2011)

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



#include <utils.h>

/** Extract a sub period of a vector of selected months. */
void
extract_subperiod_months(double **buf_sub, int *ntime_sub, double *bufin, int *year, int *month, int *day,
                         int *smonths, int timedim, int ndima, int ndimb, int ntime, int nmonths) {
  /**
     @param[out] buf_sub       3D buffer spanning only time subperiod
     @param[out] ntime_sub     Number of times in subperiod
     @param[in]  bufin         3D input buffer
     @param[in]  year          Year vector
     @param[in]  month         Month vector
     @param[in]  day           Day vector
     @param[in]  smonths       Selected months vector (values 1-12)
     @param[in]  timedim       Time dimension position (1 or 3)
     @param[in]  ndima         First dimension length
     @param[in]  ndimb         Second dimension length
     @param[in]  ntime         Time dimension length
     @param[in]  nmonths       Number of months in smonths vector
   */
  
  int *buf_sub_i = NULL; /* Temporary buffer */

  int i; /* Loop counter */
  int j; /* Loop counter */
  int t; /* Time loop counter */
  int tt; /* Time subperiod loop counter */

  /* Initializing */
  *ntime_sub = 0;
  
  /* Retrieve time index spanning selected months */
  for (t=0; t<ntime; t++)
    for (tt=0; tt<nmonths; tt++)
      if (month[t] == smonths[tt]) {
        buf_sub_i = (int *) realloc(buf_sub_i, ((*ntime_sub)+1) * sizeof(int));
        if (buf_sub_i == NULL) alloc_error(__FILE__, __LINE__);
        buf_sub_i[(*ntime_sub)++] = t;
      }
  
  /* Allocate memory */
  (*buf_sub) = (double *) malloc((*ntime_sub)*ndima*ndimb * sizeof(double));
  if ((*buf_sub) == NULL) alloc_error(__FILE__, __LINE__);

  /* Construct new 3D buffer */
  if (timedim == 3)
    /* Time dimension is the last one */
    for (t=0; t<(*ntime_sub); t++)
      for (j=0; j<ndimb; j++)
        for (i=0; i<ndima; i++)
          (*buf_sub)[i+j*ndima+t*ndima*ndimb] = bufin[i+j*ndima+buf_sub_i[t]*ndima*ndimb];
  else
    /* Time dimension is the first one */
    for (t=0; t<(*ntime_sub); t++)
      for (j=0; j<ndimb; j++)
        for (i=0; i<ndima; i++)
          (*buf_sub)[t+i*(*ntime_sub)+j*(*ntime_sub)*ndima] = bufin[buf_sub_i[t]+i*ntime+j*ntime*ndima];
  
  /* Free memory */
  (void) free(buf_sub_i);
}
