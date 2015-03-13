/* ***************************************************** */
/* Compute Potential Evapotranspiration (ETP).           */
/* calc_etp_mf.c                                         */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file calc_etp_mf.c
    \brief Compute Potential Evapotranspiration (ETP) from Meteo-France formulation.
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

/** Compute Potential Evapotranspiration (ETP) from Meteo-France formulation. */
void
calc_etp_mf(double *etp, double *tas, double *hus, double *rsds, double *rlds, double *uvas, double *pmsl, double fillvalue, int ni, int nj) {

  /** 
      @param[out]    etp           Potential Evaportranspiration (mm)
      @param[in]     tas           Input 2D temperature (K)
      @param[in]     hus           Input 2D specific humidity (kg/kg)
      @param[in]     rsds          Input 2D shortwave incoming radiation
      @param[in]     rlds          Input 2D longwave incoming radiation
      @param[in]     uvas          Input 2D wind module
      @param[in]     pmsl          Input 2D mean sea-level standard atmosphere pressure
      @param[in,out] fillvalue     Missing Value for temperature and relative humidity
      @param[in]     ni            First dimension
      @param[in]     nj            Second dimension
   */

  /*
    ; Calculate Evapotranspiration
    ;!
    ;! Calculate ETP:
    ;!
    ;! Convert original specific humidity (kg/kg) into relative humidity (%)
    ;! ISBA F90 method:
    ;! (ZPP (Pa), ZQSAT (kg/kg), ZTA_FRC(K))
    ;! Method of Etchevers gene_forc_hydro.f
    ;!
    ;! Donnees d'entree journalieres en unites SI
    ;!
    ;! ETP = ETP1 + ETP2
    ;! ETP1 = desat * Rnet / (desat + gamma) / lambda
    ;! ETP2 = (gamma / (desat + gamma)) * 0.26 * (1 + 0.4*U(10m)) * (es - ea) / tau
    ;!
    ;! ETP en mm/s
    ;! Rn = rayonnement Net en W/m2 (albedo 0.2 et emissivite 0.95)
    ;! T = temperature a 2 m en K
    ;! U(10m) = vitesse du vent a 10 m en m/s
    ;! es = pression vapeur d'eau saturation en hPa
    ;! ea = pression vapeur d'eau a 2 m en hPa
    ;! gamma = constante psychrometrique = 65 Pa/k
    ;! lamba = chaleur latente de vaporisation de l'eau = 2.45E6 J/kg
    ;! tau = constante de temps = 86400 sec.
    ;!
    ;! EP1 >= 0 && EP2 >= 0 && EP <= 9 mm/jour
    ;!
  */

  int i; /* Loop counter */

  double albedo;
  double emissivity;
  double gamma;
  double stefan;
  double lvtt;
  double avogadro;
  double boltz;
  double md;
  double mv;
  double rd;
  double rv;
  
  double pp;
  double factd;
  double factm;

  double esat;
  double wmix;
  double epres;
  double desat;
  double rnet;

  double etp1;
  double etp2;
  double ea;

  /* Setup some constants */
  albedo = 0.20;
  emissivity = 0.95;
  gamma = 65.0; /*  Pa K^-1 */
  /* tau = 86400.0; */

  stefan = 5.6697 * pow(10.0, -8.0); /*  5 670 400.E-8 in  J K^-4 m^-2 s^-1 */
  lvtt = 2.5008 * pow(10.0, 6.0); /* units are in J kg^-1 */
  avogadro = 6.0221367 * pow(10.0, 23.0); /* units are in mol^-1 */
  boltz = 1.380658 * pow(10.0, -23.0); /* units are in J K^-1 */
  md = 28.9644 * pow(10.0,-3.0); /* Masse molaire d'air sec (Md = 28.96455E-3 kg mol-1 )  */
  mv = 18.0153 * pow(10.0,-3.0); /* Masse molaire de la vapeur d'eau (Mv = 18.01528E-3 kg mol-1 )  */
  rd = avogadro * boltz / md; /* Units J kg^-1 K^-1 */
  rv = avogadro * boltz / mv; /* Units J kg^-1 K^-1 */

  /*  if (keyword_set(hourly)) then begin
      factd = 24.0
      factm = 86400.0/factd
      endif else begin */

  factd = 86400.0;
  factm = 1.0;

  for (i=0; i<(ni*nj); i++) {

    if (tas[i] != fillvalue) {
      pp = pmsl[i] * 100.0; /* Pa */
      
      esat = 610.8 * exp( 17.27 * (tas[i] - K_TKELVIN) / (tas[i] - 35.86) );  /* Pa */
      wmix = hus[i] / (1.0 - hus[i]);                                       /* kg/kg */
      epres = pp * wmix / ( (rd/rv) + wmix );                         /* Pa */
      desat = esat * 4098.0 / pow((tas[i] - 35.86), 2.0);                    /* desat/dT : Pa/K */
      rnet = ((1.0 - albedo) * rsds[i]) + (emissivity * rlds[i]) - (emissivity * stefan * pow(tas[i],4.0)); /* W m^-2 */
      
      /*
        ; Pa K^-1 W m^-2 / J kg^-1 Pa K^-1 = W m^-2 J^-1 kg = J s^-1 m^-2 J^-1 kg = kg s^-1 m^-2
        ; kg s^-1 m^-2 = mm/s avec densite implicite de 1000 kg m^-3 qui fait la conversion m en mm
      */
      etp1 = (desat * rnet) / (lvtt * (desat + gamma)) * factm;
      
      if (etp1 < 0.0) etp1 = 0.0;
      
      /*
        ; We divide by 100.0 because ew must be in hPa for ea to be in mm/day
        ; Units kg m^-2 s^-1 = mm/s avec densite implicite de 1000 kg m^-3 qui fait la conversion m en mm
        ; si unites SI sont utilisees. Dans ce cas, esat est en Pa, pas en kPa
      */
      ea = 0.26 * (1.0 + 0.4 * uvas[i]) * (esat - epres) / 100.0;
      
      etp2 = (gamma * ea) / (desat + gamma) / factd;
      if (etp2 < 0.0) etp2 = 0.0;
      
      /*  etp in mm/s */
      etp[i] = etp1 + etp2;
      if (etp[i] > 9.0) etp[i] = 9.0;
    }
    else {
      etp[i] = fillvalue;
    }
  }
}
