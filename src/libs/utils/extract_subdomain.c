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

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2014)

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

/** Extract subdomain in a variable given latitudes and longitudes. */
void
extract_subdomain(double **buf_sub, double **lon_sub, double **lat_sub, int *nlon_sub, int *nlat_sub, double *buf,
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
