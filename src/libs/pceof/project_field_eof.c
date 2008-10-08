/* ***************************************************** */
/* Project physical 2D field on pre-computed EOF.        */
/* project_field_eof.c                                   */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file project_field_eof.c
    \brief Project physical field on pre-computed EOF.
*/

#include <pceof.h>

/** Subroutine to project a 2D-time field on pre-calculated EOFs. */
short int project_field_eof(double *bufout, double *bufin, double *bufeof, double *singular_value,
                            double missing_value_eof, int ni, int nj, int ntime, int neof)
{
  /**
     @param[out]     bufout            Output 2D (neof x ntime) projected bufin field using input eof and singular_value
     @param[in]      bufin             Input field 3D (ni x nj x ntime)
     @param[in]      eof               EOF of input field 3D (ni x nj x neof)
     @param[in]      singular_value    Singular value for EOF
     @param[in]      ni                Horizontal dimension
     @param[in]      nj                Horizontal dimension
     @param[in]      ntime             Temporal dimension
     @param[in]      neof              EOF dimension
  */

  double norm; /* Normalization factor. */
  double sum_verif_norm; /* Sum to verify normalization. */
  double val; /* Double temporary value */
  double sum; /* Temporary sum */

  double *true_val = NULL; /* 2D matrix of normalized value */

  int eof; /* Loop counter */
  int i; /* Loop counter */
  int j; /* Loop counter */
  int t; /* Loop counter */

  /*** Project field on EOFs ***/

  /* Allocate memory */
  true_val = (double *) malloc(ni*nj * sizeof(double));
  if (true_val == NULL) alloc_error(__FILE__, __LINE__);

  /* Compute norm */

  /* DEBUG */
  /*  sum = 0.0;
  for (j=0; j<nj; j++)
    for (i=0; i<ni; i++) {
      eof = 0;
      if (bufeof[i+j*ni+eof*ni*nj] != missing_value_eof)
        printf("%d %d %d %d %lf\n",(int) sum,i,j,eof,bufeof[i+j*ni+eof*ni*nj]);
      if (bufeof[i+j*ni] != missing_value_eof)
        sum = sum + 1.0;
        } */
  
  for (eof=0; eof<neof; eof++) {
    norm = 0.0;
    sum_verif_norm = 0.0;
    
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++) {
        if (bufeof[i+j*ni+eof*ni*nj] != missing_value_eof) {
          val = bufeof[i+j*ni+eof*ni*nj] / singular_value[eof];
          norm += (val * val);
        }
      }
    
    /* Compute true value */
    sum = 0.0;
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++) {
        if (bufeof[i+j*ni+eof*ni*nj] != missing_value_eof) {
          val = bufeof[i+j*ni+eof*ni*nj] / ( sqrt(norm) * singular_value[eof] );
          true_val[i+j*ni] = val;
          sum += val;
          sum_verif_norm += (val * val);
        }
      }

    /* Verify that the norm is equal to 1.0 */
    (void) fprintf(stdout, "%s: Verifying the norm (should be equal to 1) for EOF #%d: %lf\n", __FILE__, eof, sum_verif_norm);

    /* Project on EOF */
    for (t=0; t<ntime; t++) {
      sum = 0.0;
      for (j=0; j<nj; j++)
        for (i=0; i<ni; i++)
          if (bufeof[i+j*ni+eof*ni*nj] != missing_value_eof) {
            /*            if (t == 0)
                          printf("%d %d %lf\n",i,j,bufin[i+j*ni+t*ni*nj]);*/
            sum += ( bufin[i+j*ni+t*ni*nj] / sqrt(norm) * true_val[i+j*ni] );
          }
      bufout[t+eof*ntime] = sum;
    }

    /* Verify variance of field */
    /* Should be of the same order */
    (void) fprintf(stdout, "%s: Verifying variance (should be the same order): %lf %lf\n", __FILE__,
                   sqrt(gsl_stats_variance(&(bufout[eof*ntime]), 1, ntime)), singular_value[eof]);
    (void) fprintf(stdout, "%s: %lf\n", __FILE__,
                   sqrt(gsl_stats_variance(&(bufout[eof*ntime]), 1, ntime)) / singular_value[eof]);
  }
  
  (void) free(true_val);

  return 0;
}
