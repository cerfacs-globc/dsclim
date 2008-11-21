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

#include <io.h>

/** Get NetCDF string attribute. */
int get_attribute_str(char **var, int ncinid, int varid, char *attrname)
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
