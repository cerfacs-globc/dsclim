/* ***************************************************** */
/* Compute distance in meters for                        */
/* two latitude and longitude points.                    */
/* distance_point.c                                      */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file distance_point.c
    \brief Compute distance in meters for two latitude and longitude points.
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

/** Compute distance in meters for two latitude and longitude points. */
double
distance_point(double lon0, double lat0, double lon1, double lat1)
{
  /**
     @param[in]  lon0 First point longitude
     @param[in]  lat0 First point latitude
     @param[in]  lon1 Second point longitude
     @param[in]  lat1 Second point latitude

     \return     Distance in meters between 2 points.
   */

  /* Earth equatorial radius, meters, Clarke 1866 ellipsoid */
  const double r_earth = 6378206.4;
  const double degtorad = M_PI / 180.0;

  double coslt1;
  double coslt0;
  double sinlt1;
  double sinlt0;
  double cosl0l1;
  double cosc;
  
  coslt1 = cos(lat1 * degtorad);
  sinlt1 = sin(lat1 * degtorad);
  coslt0 = cos(lat0 * degtorad);
  sinlt0 = sin(lat0 * degtorad);
    
  cosl0l1 = cos((lon1-lon0) * degtorad);
  
  /* Cos of angle between points */
  cosc = sinlt0 * sinlt1 + coslt0 * coslt1 * cosl0l1;
  
  /* Restrict range between 1 and -1 */
  if (cosc > 1.0)
    cosc = 1.0;
  else if (cosc < -1.0)
    cosc = -1.0;

  return (acos(cosc) * r_earth);
}
