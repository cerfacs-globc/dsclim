/* ***************************************************** */
/* Compute standard atmosphere pressure given altitude.  */
/* alt_to_press.c                                        */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file alt_to_press.c
    \brief Compute standard atmosphere pressure given altitude.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2009)

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

/** Compute standard atmosphere pressure given altitude. */
void
alt_to_press(double *pres, double *alt, int ni, int nj) {

  /** 
      @param[out]  pres          Standard pressure in hPa
      @param[in]   alt           Input 2D altitude in meters
      @param[in]   ni            First dimension
      @param[in]   nj            Second dimension
   */

  /*
    ;	Convert an array of (pressure-) altitudes (m) into an array of 
    ;       pressures (hPa) using the ICAO standard atmosphere definition
    ;
    ;       See <A HREF="http://www.pdas.com/coesa.htm">exact definition
    ;       here<\A>
    ;       
    ;       The 7 layers of the US standard atmosphere are:
    ;
    ;        h1   h2     dT/dh    h1,h2 geopotential alt in km
    ;         0   11     -6.5     dT/dh in K/km
    ;        11   20      0.0
    ;        20   32      1.0
    ;        32   47      2.8
    ;        47   51      0.0
    ;        51   71     -2.8   
    ;        71   84.852 -2.0
  */

#define NLAYERS 7

  int i; /* Loop counter */
  int layr; /* Loop counter */

  double limits[NLAYERS+1];
  double lapse_rate[NLAYERS];
  int isoth[NLAYERS];
  double presb[NLAYERS];
  double tb[NLAYERS];

  int layer = 0;

  /* Layer boundaries in m */
  limits[0] = 0.0;
  limits[1] = 11.0 * 1000.0;
  limits[2] = 20.0 * 1000.0;
  limits[3] = 32.0 * 1000.0;
  limits[4] = 47.0 * 1000.0;
  limits[5] = 51.0 * 1000.0;
  limits[6] = 71.0 * 1000.0;
  limits[7] = 84.852 * 1000.0;

  /* Lapse rates in each layer (9 means 0) */
  lapse_rate[0] = -6.5 / 1000.0;
  lapse_rate[1] = 9.0;
  lapse_rate[2] = 1.0 / 1000.0;
  lapse_rate[3] = 2.8 / 1000.0;
  lapse_rate[4] = 9.0;
  lapse_rate[5] = -2.8 / 1000.0;
  lapse_rate[6] = -2.0 / 1000.0;
  
  /* Flag for isothermal layers */
  isoth[0] = 0;
  isoth[1] = 1;
  isoth[2] = 0;
  isoth[3] = 0;
  isoth[4] = 1;
  isoth[5] = 0;
  isoth[6] = 0;

  presb[0] = 1013.25;
  tb[0] = 288.15;

  /* Loop over layers and get pressures and temperatures at level tops */
  for (i=0; i<(NLAYERS-1); i++) {
    tb[i+1] = tb[i] + (1-isoth[i]) * lapse_rate[i] * (limits[i+1] - limits[i]);
    presb[i+1] = (1-isoth[i]) * presb[i] * exp(log(tb[i]/tb[i+1]) * K_GMR / lapse_rate[i]) +
      isoth[i] * presb[i] * exp(-K_GMR * (limits[i+1]-limits[i]) / tb[i]);
  }

  for (i=0; i<(ni*nj); i++) {
    /* Now calculate which layer each value belongs to */
    for (layr=0; layr<NLAYERS; layr++) {
      if ( (limits[layr] - alt[i]) > 0.0 ) {
        layer = layr - 1;
        layr = NLAYERS + 1;
      }
    }
    if (layer < 0) layer = 0;
    if (layer > (NLAYERS-1)) layer = NLAYERS - 1;
    
    /* Corresponding pressure */
    pres[i] = isoth[layer] * presb[layer] * exp(-K_GMR * (alt[i] - limits[layer]) / tb[layer]) +
      (1-isoth[layer]) * presb[layer] * pow( ( tb[layer] / ( tb[layer] + lapse_rate[layer] *
                                                             (alt[i] - limits[layer]) ) ), (K_GMR/lapse_rate[layer]) );
  }
  
}
