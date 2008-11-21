/* ***************************************************** */
/* Compare two float values to sort descending.          */
/* comparf.c                                             */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file comparf.c
    \brief Compare two float values to sort descending.
*/

#include <utils.h>

/** Compare two float values to sort descending. */
int comparf(const void *a, const void *b)
{
  /**
     @param[in]  a    First input value
     @param[in]  b    Second input value

     \return     Comparison result
   */

  /* Cast to float */
  float *n1 = (float *) a;
  float *n2 = (float *) b;

  /* Descending comparison */
  if (*n1 < *n2)
    return 1;
  else if (*n1 > *n2)
    return -1;
  else
    return 0;
}
