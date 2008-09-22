/* ***************************************************** */
/* Extract subdomain in a variable given latitudes and   */
/* longitudes.                                           */
/* extract_subdomain.c                                   */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file extract_subdomain.c
    \brief Extract subdomain in a variable given latitudes and longitudes. 
*/

#include <utils.h>

void extract_subdomain(double **buf_sub, int *nlon_sub, int *nlat_sub, double *buf,
                       double *lon, double *lat, double minlon, double maxlon, double minlat, double maxlat,
                       int nlon, int nlat, int ndim) {
  
  /* Compute subdomain and apply to arrays */
  
  int i;
  int j;
  int t;
  int ii;
  int jj;
  double curlon;
  double curlat;

  *nlon_sub = *nlat_sub = 0;

  for (i=0; i<nlat; i++)
    if (lat[i] >= minlat && lat[i] <= maxlat)
      (*nlat_sub)++;
  for (i=0; i<nlon; i++) {
    if (lon[i] > 180.0)
      curlon = lon[i] - 360.0;
    else
      curlon = lon[i];
    if (curlon >= minlon && curlon <= maxlon)
      (*nlon_sub)++;
  }

  (*buf_sub) = (double *) malloc((*nlon_sub)*(*nlat_sub)*ndim * sizeof(double));
  if ((*buf_sub) == NULL) alloc_error(__FILE__, __LINE__);

  ii = 0;
  jj = 0;
  for (j=0; j<nlat; j++) {
    if (ii > 0)
      jj++;
    ii = 0;
    for (i=0; i<nlon; i++) {
      if (lon[i] > 180.0)
        curlon = lon[i] - 360.0;
      else
        curlon = lon[i];
      curlat = lat[j];
      if (curlon >= minlon && curlon <= maxlon && curlat >= minlat && curlat <= maxlat) {
        for (t=0; t<ndim; t++)
          (*buf_sub)[ii+jj*(*nlon_sub)+t*(*nlon_sub)*(*nlat_sub)] = buf[i+j*nlon+t*nlon*nlat];
        ii++;
      }
    }
  }
}
