
#include <../clim/clim.h>
#include <utils.h>

#include <gsl/gsl_statistics.h>

/** Subroutine to project a 2D-time field on pre-calculated EOFs. */
void project_field_eof(double *bufout, double *clim, double *bufin, double *bufeof, double *singular_value, tstruct *buftime,
                       double missing_value, double missing_value_eof,
                       int clim_filter_width, char *clim_filter_type, short int clim_provided,
                       int ni, int nj, int ntime, int neof)
{
  /**
     @param[out]     bufout            Output 2D (neof x ntime) projected bufin field using input eof and singular_value
     @param[in]      clim              Climatology 3D (ni x nj x 366 days) of input field bufin
     @param[in]      bufin             Input field 3D (ni x nj x ntime)
     @param[in]      eof               EOF of input field 3D (ni x nj x neof)
     @param[in]      singular_value    Singular value for EOF
     @param[in]      buftime           1D time vector
     @param[in]      missing_value     Missing value for bufin
     @param[in]      clim_filter_width Width of filter for climatology
     @param[in]      clim_filter_type  Type of filtering for climatology
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
  double *bufnoclim = NULL; /* 3D temporary matrix for field with climatology removed */

  int eof; /* Loop counter */
  int i; /* Loop counter */
  int j; /* Loop counter */
  int t; /* Loop counter */

  /* Allocate memory */
  bufnoclim = (double *) malloc(ni*nj*ntime * sizeof(double));
  if (bufnoclim == NULL) alloc_error(__FILE__, __LINE__);

  /* Substract seasonal cycle */
  (void) remove_seasonal_cycle(bufnoclim, clim, bufin, buftime, missing_value, clim_filter_width, clim_filter_type, clim_provided,
                               ni, nj, ntime);
  
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
                          printf("%d %d %lf\n",i,j,bufnoclim[i+j*ni+t*ni*nj]);*/
            sum += ( bufnoclim[i+j*ni+t*ni*nj] / sqrt(norm) * true_val[i+j*ni] );
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
  (void) free(bufnoclim);
}
