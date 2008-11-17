/* ***************************************************** */
/* wt_learning Compute learning data needed for          */
/* downscaling climate scenarios using weather typing.   */
/* wt_learning.c                                         */
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
/*! \file wt_learning.c
    \brief Compute learning data needed for downscaling climate scenarios using weather typing.
*/

#include <dsclim.h>

int wt_learning(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat;

  if (data->learning->learning_provided == 1) {
    /** Read learning data **/
    istat = read_learning_fields(data);
    if (istat != 0) return istat;
  }
  //  else  {
    /** Compute learning data **/
  //  }
  
  return 0;
}
