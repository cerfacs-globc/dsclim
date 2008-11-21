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

/** Extract subdomain in a variable given latitudes and longitudes. */
void extract_subdomain(double **buf_sub, double **lon_sub, double **lat_sub, int *nlon_sub, int *nlat_sub, double *buf,
                       double *lon, double *lat, double minlon, double maxlon, double minlat, double maxlat,
                       int nlon, int nlat, int ndim) {
  /**
     @param[out] buf_sub         3D buffer spanning only subdomain
     @param[out] lon_sub         Longitude array spanning only subdomain
     @param[out] lat_sub         Latitude array spanning only subdomain
     @param[out] nlon_sub        Longitude dimension length spanning only subdomain
     @param[out] nlat_sub        Latitude dimension length spanning only subdomain
     @param[in]  buf             3D input buffer
     @param[in]  lon             Longitude array
     @param[in]  lat             Latitude array
     @param[in]  minlon          Subdomain bounds: minimum longitude
     @param[in]  maxlon          Subdomain bounds: maximum longitude
     @param[in]  minlat          Subdomain bounds: minimum latitude
     @param[in]  maxlat          Subdomain bounds: maximum latitude
     @param[in]  nlon            Longitude dimension length
     @param[in]  nlat            Latitude dimension length
     @param[in]  ndim            Third dimension length
   */

  /* Compute subdomain and apply to arrays */
  
  int i; /* Loop counter */
  int j; /* Loop counter */
  int t; /* Time loop counter */
  int ii; /* Subdomain loop counter */
  int jj; /* Subdomain loop counter */
  double curlon; /* Current longitude */
  double curlat; /* Current latitude */

  /* Initializing */
  *nlon_sub = *nlat_sub = 0;

  /* Count latitude dimension length */
  for (i=0; i<nlat; i++)
    if (lat[i*nlon] >= minlat && lat[i*nlon] <= maxlat)
      (*nlat_sub)++;

  /* Count longitude dimension length */
  /* Adjust to span -180 to +180 */
  for (i=0; i<nlon; i++) {
    if (lon[i] > 180.0)
      curlon = lon[i] - 360.0;
    else
      curlon = lon[i];
    if (curlon >= minlon && curlon <= maxlon)
      (*nlon_sub)++;
  }

  /* Allocate memory with dimension lengths */
  (*buf_sub) = (double *) malloc((*nlon_sub)*(*nlat_sub)*ndim * sizeof(double));
  if ((*buf_sub) == NULL) alloc_error(__FILE__, __LINE__);
  (*lon_sub) = (double *) malloc((*nlon_sub)*(*nlat_sub) * sizeof(double));
  if ((*lon_sub) == NULL) alloc_error(__FILE__, __LINE__);
  (*lat_sub) = (double *) malloc((*nlon_sub)*(*nlat_sub) * sizeof(double));
  if ((*lat_sub) == NULL) alloc_error(__FILE__, __LINE__);

  /* Loop over all gridpoints and construct new buffer array spanning only subdomain */
  ii = 0;
  jj = 0;
  /* Loop over latitudes */
  for (j=0; j<nlat; j++) {
    if (ii > 0)
      jj++;
    ii = 0;
    /* Loop over longitudes */
    for (i=0; i<nlon; i++) {
      /* Adjust longitude to span -180 to +180 */
      if (lon[i] > 180.0)
        curlon = lon[i] - 360.0;
      else
        curlon = lon[i+j*nlon];
      curlat = lat[i+j*nlon];
      /* Retrieve only gridpoints within bounds */
      if (curlon >= minlon && curlon <= maxlon && curlat >= minlat && curlat <= maxlat) {
        /* Loop over last dimension to assign all values for this gridpoint */
        for (t=0; t<ndim; t++)
          (*buf_sub)[ii+jj*(*nlon_sub)+t*(*nlon_sub)*(*nlat_sub)] = buf[i+j*nlon+t*nlon*nlat];
        /* Create also latitude and longitude arrays */
        (*lon_sub)[ii+jj*(*nlon_sub)] = lon[i+j*nlon];
        (*lat_sub)[ii+jj*(*nlon_sub)] = lat[i+j*nlon];
        ii++;
      }
    }
  }
}
