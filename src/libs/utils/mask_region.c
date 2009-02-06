/* ***************************************************** */
/* Mask region in a variable given latitude and          */
/* longitude coordinates.                                */
/* mask_region_subdomain.c                               */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file mask_region.c
    \brief Mask region in a variable given latitude and longitude coordinates.
*/

#include <utils.h>

/** Mask region in a variable given latitude and longitude coordinates.*/
void mask_region(double *buffer, double missing_value, double *lon, double *lat,
                 double minlon, double maxlon, double minlat, double maxlat,
                 int nlon, int nlat, int ndim) {
  /**
     @param[out] buffer          3D buffer
     @param[in]  missing_value   Missing value
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

  int i; /* Loop counter */
  int j; /* Loop counter */
  int t; /* Time loop counter */
  double curlon; /* Current longitude */
  double curlat; /* Current latitude */

  (void) printf("%s: Masking domain. Longitudes: %lf %lf. Latitudes: %lf %lf.\n", __FILE__, minlon, maxlon, minlat, maxlat);

  /* Loop over all gridpoints */

  /* Loop over latitudes */
  for (j=0; j<nlat; j++) {
    /* Loop over longitudes */
    for (i=0; i<nlon; i++) {
      /* Adjust longitude to span -180 to +180 */
      if (lon[i] > 180.0)
        curlon = lon[i] - 360.0;
      else
        curlon = lon[i+j*nlon];
      curlat = lat[i+j*nlon];
      /* Mask only gridpoints within bounds */
      if (curlon >= minlon && curlon <= maxlon && curlat >= minlat && curlat <= maxlat) {
        /* Loop over last dimension to assign missing value for this gridpoint */
        for (t=0; t<ndim; t++)
          buffer[i+j*nlon+t*nlon*nlat] = missing_value;
      }
    }
  }
}
