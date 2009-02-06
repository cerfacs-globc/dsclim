/* ***************************************************** */
/* Find string in vector and return index.               */
/* find_str_value.c                                      */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file find_str_value.c
    \brief Find string in vector and return index.
*/

#include <utils.h>

/** Find string in vector and return index */
int find_str_value(char *str, char **str_vect, int nelem) {
  /**
     @param[in]   str        String value to search
     @param[in]   str_vect   Vector of strings
     @param[in]   nelem      Number of elements in str_vect
     
     \return Vector index
  */
  
  int n; /* Loop counter */
  
  /* Loop over elements */
  for (n=0; n<nelem; n++)
    if ( !strcmp(str, str_vect[n]) )
      /* Found string */
      return n;

  /* String not found */
  return -1;
}
