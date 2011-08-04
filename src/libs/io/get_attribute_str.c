/* ***************************************************** */
/* get_attribute_str Get NetCDF string attribute.        */
/* get_attribute_str.c                                   */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: oct 2008                            */
/* Last date of modification: oct 2008                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file get_attribute_str.c
    \brief Get NetCDF string attribute.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2011)

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



#include <io.h>

/** Get NetCDF string attribute. */
int
get_attribute_str(char **var, int ncinid, int varid, char *attrname)
{
  /**
     @param[out] var       String attribute value
     @param[in]  ncinid    NetCDF input filename ID
     @param[in]  varid     NetCDF variable ID
     @param[in]  attrname  NetCDF attribute name
  */

  int istat; /* Diagnostic status */
  size_t t_len; /* Length of attribute value string */

  /* Get attribute length */
  istat = nc_inq_attlen(ncinid, varid, attrname, &t_len);
  if (istat == NC_NOERR) {
    /* Allocate required space before retrieving values */
    (*var) = (char *) malloc(t_len + 1);
    if ((*var) == NULL) alloc_error(__FILE__, __LINE__);
    /* Get attribute value */
    istat = nc_get_att_text(ncinid, varid, attrname, (*var));
    if (istat != NC_NOERR)
      (*var)[0] = '\0';
    else
      if ((*var)[t_len-1] != '\0')
        (*var)[t_len] = '\0'; /* null terminate if needed */
  }
  else {
    /* Allocate required space */
    (*var) = (char *) malloc(sizeof(char));
    if ((*var) == NULL) alloc_error(__FILE__, __LINE__);    
    (*var)[0] = '\0';
  }

  /* Diagnostic status */
  return ((int) istat);
}
