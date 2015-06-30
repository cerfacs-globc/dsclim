/* ***************************************************** */
/* read_mask Read a mask file.                           */
/* read_mask.c                                           */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: feb 2009                            */
/* Last date of modification: feb 2009                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file read_mask.c
    \brief Read a mask file.
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







#include <dsclim.h>
#include <shapefil.h>

/** Read a mask file. */
int
read_mask(mask_struct *mask) {
  /**
     @param[in]  mask  Mask structure.
     
     \return           Status.
  */

  int istat;

  if ( !strcmp(get_filename_ext(mask->filename), "nc")) {

    /* Read latitudes and longitudes */
    istat = read_netcdf_latlon(&(mask->lon), &(mask->lat), &(mask->nlon), &(mask->nlat), mask->dimcoords, mask->coords, mask->proj,
                               mask->lonname, mask->latname, mask->dimxname, mask->dimyname, mask->filename);
    if (istat < 0)  {
      (void) fprintf(stdout, "%s: ERROR reading mask file.\n", __FILE__);
      return istat;
    }

    /* Read mask */
    istat = read_netcdf_var_2d(&(mask->field), (info_field_struct *) NULL, (proj_struct *) NULL, mask->filename, mask->maskname,
                               mask->dimxname, mask->dimyname, &(mask->nlon), &(mask->nlat), FALSE);
    if (istat < 0)  {
      (void) fprintf(stdout, "%s: ERROR reading mask file.\n", __FILE__);
      return istat;
    }

  }
  else if ( !strcmp(get_filename_ext(mask->filename), ".shp")) {

    
    
  }
    
    (void) fprintf(stdout, "%s: mask file read successfully.\n", __FILE__);

  /* Return status */
  return 0;
}
