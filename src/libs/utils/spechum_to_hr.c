/* ***************************************************** */
/* Compute relative humidity from specific humidity.     */
/* spechum_to_hr.c                                       */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file spechum_to_hr.c
    \brief Compute relative humidity from specific humidity.
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







#include <utils.h>

/** Compute relative humidity from specific humidity. */
void
spechum_to_hr(double *hur, double *tas, double *hus, double *pmsl, double fillvalue, int ni, int nj) {

  /** 
      @param[out]    hur           Relative humidity (%)
      @param[in]     tas           Input 2D temperature (K)
      @param[in]     hus           Input 2D specific humidity (kg/kg)
      @param[in]     pmsl          Input 2D mean sea-level pressure (hPa)
      @param[in,out] fillvalue     Missing Value for temperature and relative humidity
      @param[in]     ni            First dimension
      @param[in]     nj            Second dimension
   */

  int i; /* Loop counter */

  double curtas; /* Current temperature value */
  double curhus; /* Current specific humidity value */
  double mixr; /* Mixing ratio */
  double es; /* Saturation vapor pressure */
  double fact; /* Factor */

  for (i=0; i<(ni*nj); i++) {

    curtas = tas[i];
    if (curtas != fillvalue) {
      curhus = hus[i] * 1000.0;
      
      /* Begin by calculating the mixing ratio Q/(1.-Q/1000.) */
      mixr = curhus / (1.0 - (curhus / 1000.0));
      /* Compute relative humidity from the mixing ratio */
      /*                    ;                                     Mw*e              e
                            ;  W (mixing ratio) = m_h2o/m_dry = -------- = Mw/Md * ---
                            ;                                   Md*(p-e)           p-e
                            ;
                            ;  RH (rel. hum.)    = e/esat(T)*100.
      */
      /* Compute saturation vapor pressure in hPa */
      /* ; Formula with T = temperature in K
         ;    esat = exp( -6763.6/(T+T0) - 4.9283*alog((T+T0)) + 54.2190 )
         
         ; Formula close to that of Magnus, 1844 with temperature TC in Celsius
         ;    ESAT = 6.1078 * EXP( 17.2693882 * TC / (TC + 237.3) ) ; TC in Celsius
         
         ; or Emanuel's formula (also approximation in form of Magnus' formula,
         ; 1844), which was taken from Bolton, Mon. Wea. Rev. 108, 1046-1053, 1980.
         ; This formula is very close to Goff and Gratch with differences of
         ; less than 0.25% between -50 and 0 deg C (and only 0.4% at -60degC)    
         ;    esat=6.112*EXP(17.67*TC/(243.5+TC))
         
         ; WMO reference formula is that of Goff and Gratch (1946), slightly
         ; modified by Goff in 1965:
      */
      es = 1013.250 * pow( 10.0, ( 10.79586* (1.0-K_TKELVIN/curtas) -
                                   5.02808 * log10(curtas/K_TKELVIN) +
                                   1.50474 * 0.0001 *
                                   (1.0 - pow(10.0, (-8.29692*((curtas/K_TKELVIN)-1.0))) ) +
                                   0.42873 * 0.001 *
                                   (pow(10.0, (4.76955*(1.0-K_TKELVIN/curtas)))-1.0) - 2.2195983) );
      fact = mixr / 1000.0 * (K_MD/K_MW);
      /* Use Standard Pressure for now, given altitude.
         For more precise values we should use instead a pressure field close to the weather type... */
      hur[i] = pmsl[i] / es * fact / (1.0 + fact) * 100.0;
      if (hur[i] > 100.0) hur[i] = 100.0;
      if (hur[i] < 0.0) hur[i] = 0.0;
    }
    else
      hur[i] = fillvalue;
  }
}
