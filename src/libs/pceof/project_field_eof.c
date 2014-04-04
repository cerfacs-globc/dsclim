/* ***************************************************** */
/* Project physical 2D field on pre-computed EOF.        */
/* project_field_eof.c                                   */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file project_field_eof.c
    \brief Project physical field on pre-computed EOF.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2014)

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



#include <pceof.h>

/** Subroutine to project a 2D-time field on pre-calculated EOFs. */
int
project_field_eof(double *bufout, double *bufin, double *bufeof, double *singular_value,
                  double missing_value_eof, double *lon, double *lat, double scale, int ni, int nj, int ntime, int neof)
{
  /**
     @param[out]     bufout            Output 2D (neof x ntime) projected bufin field using input eof and singular_value
     @param[in]      bufin             Input field 3D (ni x nj x ntime)
     @param[in]      bufeof            EOF of input field 3D (ni x nj x neof)
     @param[in]      singular_value    Singular value for EOF
     @param[in]      missing_value_eof Missing value for bufeof
     @param[in]      lon               Longitude
     @param[in]      lat               Latitude
     @param[in]      scale             Scaling for units to apply before projecting onto EOF
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
  
  double variance_bufin; /* Variance of input buffer */
  double tot_variance_bufin = 0.0; /* Total Variance of input buffer */
  double variance_bufout; /* Variance of output buffer */
  double tot_variance_bufout = 0.0; /* Total Variance of output buffer */

  double sum_scal = 0.0; /* EOF scaling factor sum */
  double *scal = NULL; /* EOF Scaling factor */
  double e1n, e2n; /* Scaling factor components */

  int eof; /* Loop counter */
  int i; /* Loop counter */
  int j; /* Loop counter */
  int t; /* Loop counter */

  /*** Project field on EOFs ***/

  /* Allocate memory */
  true_val = (double *) malloc(ni*nj * sizeof(double));
  if (true_val == NULL) alloc_error(__FILE__, __LINE__);
  scal = (double *) malloc(ni*nj * sizeof(double));
  if (scal == NULL) alloc_error(__FILE__, __LINE__);

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

  /* Loop over all EOFs */
  for (eof=0; eof<neof; eof++) {

    /* Initializing */
    norm = 0.0;
    sum_verif_norm = 0.0;
    
    /* Loop over all gridpoints */
    /* Compute the sum of the squared values normalized by the singular value */
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
    (void) fprintf(stdout, "%s: Verifying the sqrt(norm)=%lf (should be equal to 1) for EOF #%d: %lf\n", __FILE__, sqrt(norm),
                   eof, sum_verif_norm);
    if (fabs(sum_verif_norm) < 0.01) {
      (void) fprintf(stderr, "%s: FATAL ERROR: Re-norming does not equal 1.0 : %lf.\nAborting\n", __FILE__, sum_verif_norm);
      /* Free memory */
      (void) free(true_val);
      (void) free(scal);
      return -1;
    }

    /* Compute EOF scale factor */
    sum_scal = 0.0;
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++) {
        if (j < (nj-1))
          e1n = ( 2.0*M_PI*EARTH_RADIUS/(DEGTORAD*lon[i+j*ni]) ) * fabs( cos( DEGTORAD*(lat[i+(j+1)*ni]-lat[i+j*ni]) ) );
        else
          e1n = ( 2.0*M_PI*EARTH_RADIUS/(DEGTORAD*lon[i+j*ni]) ) * fabs( cos( DEGTORAD*(lat[i+j*ni]-lat[i+(j-1)*ni]) ) );
        if (j < (nj-1))
          e2n = ( 2.0*EARTH_RADIUS ) * fabs( cos( DEGTORAD*(lat[i+(j+1)*ni]-lat[i+j*ni]) ) );
        else
          e2n = ( 2.0*EARTH_RADIUS ) * fabs( cos( DEGTORAD*(lat[i+j*ni]-lat[i+(j-1)*ni]) ) );
        //        printf("%lf %lf\n",e1n,e2n);
        scal[i+j*ni] = e1n * e2n;
        sum_scal += scal[i+j*ni];
      }
    for (j=0; j<nj; j++)
      for (i=0; i<ni; i++) {
        scal[i+j*ni] = sqrt( scal[i+j*ni] * (1.0/sum_scal) );
        //        if (eof == 0)
        //          printf("%d %d lon=%lf %lf %lf\n",i,j,lon[i+j*ni],lat[i+j*ni],scal[i+j*ni]);
      }

    /* Project field onto EOF */
    for (t=0; t<ntime; t++) {
      sum = 0.0;
      for (j=0; j<nj; j++)
        for (i=0; i<ni; i++)
          if (bufeof[i+j*ni+eof*ni*nj] != missing_value_eof)
            /*            sum += ( bufin[i+j*ni+t*ni*nj] * scale * scal[i+j*ni] / sqrt(norm) * true_val[i+j*ni] );*/
            sum += ( bufin[i+j*ni+t*ni*nj] * scale / sqrt(norm) * true_val[i+j*ni] );
      bufout[t+eof*ntime] = sum;
      //      printf("%d %d %lf\n",t,eof,sum);
    }

    variance_bufout = gsl_stats_variance(&(bufout[eof*ntime]), 1, ntime);
    tot_variance_bufout += variance_bufout;
    variance_bufin = gsl_stats_variance(&(bufin[eof*ntime]), 1, ntime);
    tot_variance_bufin += variance_bufin;

    /* Verify variance of field */
    /* Should be of the same order */
    (void) fprintf(stdout, "%s: Verifying square-root of variance (should be the same order): %lf %lf\n", __FILE__,
                   sqrt(variance_bufout), singular_value[eof]);
    (void) fprintf(stdout, "%s: %lf\n", __FILE__, sqrt(variance_bufout) / singular_value[eof]);
    if ( (sqrt(gsl_stats_variance(&(bufout[eof*ntime]), 1, ntime)) / singular_value[eof]) >= 10.0) {
      (void) fprintf(stderr, "%s: FATAL ERROR: Problem in scaling factor! Variance is not of the same order. Verify configuration file scaling factor.\nAborting\n", __FILE__);
      /* Free memory */
      (void) free(true_val);
      (void) free(scal);
      return -1;
    }
  }

  (void) fprintf(stdout, "%s: Comparing total variance of field before %lf and after %lf projection onto EOF: %% of variance remaining: %lf\n",
                 __FILE__, tot_variance_bufin, tot_variance_bufout, tot_variance_bufout / tot_variance_bufin * 100.0);

  /* Free memory */
  (void) free(true_val);
  (void) free(scal);

  /* Success status */
  return 0;
}
