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

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2012)

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

/** Mask region in a variable given latitude and longitude coordinates.*/
void
mask_region(double *buffer, double missing_value, double *lon, double *lat,
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
