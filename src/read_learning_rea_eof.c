/* ***************************************************** */
/* read_learning_rea_eof Read reanalysis data EOFs for   */
/* learning period.                                      */
/* read_learning_rea_eof.c                               */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: dec 2008                            */
/* Last date of modification: dec 2008                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file read_learning_rea_eof.c
    \brief Read reanalysis data EOFs for learning period. Currently only NetCDF is implemented.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2012)

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

/** Read reanalysis data EOFs for learning period. Currently only NetCDF is implemented. */
int
read_learning_rea_eof(data_struct *data) {
  /**
     @param[in]  data  MASTER data structure.
     
     \return           Status.
  */

  int istat; /* Diagnostic status */
  int neof; /* EOF dimension for main large-scale fields in input file */
  int ntime; /* Time dimension for main large-scale fields in input file */
  double *time;
  char *cal_type = NULL; /* Calendar type (udunits) */
  char *time_units = NULL; /* Time units (udunits) */
  double *buf = NULL;
  int t;
  int eof;

  /* Read EOF principal components */
  istat = read_netcdf_var_2d(&buf, (info_field_struct *) NULL, (proj_struct *) NULL,
                             data->learning->rea->filename_eof, data->learning->rea->nomvar_eof,
                             data->conf->eofname, data->learning->rea_timename, &neof, &ntime, TRUE);
  if (istat != 0) {
    /* In case of failure */
    return istat;
  }
  if (data->learning->rea_neof != neof) {
    (void) fprintf(stderr, "%s: ERROR: Number of EOFs (%d) for reanalysis %s field from EOF file (%s) is not equal to number of EOFs specified in XML configuration file for reanalysis fields (%d)!\n", __FILE__, neof,
                   data->learning->rea->nomvar_eof, data->learning->rea->filename_eof, data->learning->rea_neof);
    return -1;
  }
  /* Re-order array with time as fastest varying dimension */
  data->learning->rea->eof = malloc(neof*ntime * sizeof(double));
  if (data->learning->rea->eof == NULL) alloc_error(__FILE__, __LINE__);
  for (eof=0; eof<neof; eof++)
    for (t=0; t<ntime; t++)
      data->learning->rea->eof[t+eof*ntime] = buf[eof+t*neof];
  (void) free(buf);

  /* Get time information */
  istat = get_time_info(data->learning->rea->time_s, &time, &time_units, &cal_type,
                        &ntime, data->learning->rea->filename_eof, data->learning->nomvar_time, TRUE);
  (void) free(cal_type);
  (void) free(time_units);
  (void) free(time);
  if (istat != 0) {
    /* In case of failure */
    (void) free(data->learning->rea->time_s);
    return istat;
  }

  data->learning->rea->ntime = ntime;

  /* Read Singular Values */
  istat = read_netcdf_var_1d(&(data->learning->rea->sing), (info_field_struct *) NULL,
                             data->learning->rea->filename_eof, data->learning->rea->nomvar_sing,
                             data->conf->eofname, &neof, TRUE);
  if (istat != 0) {
    /* In case of failure */
    (void) free(data->learning->rea->time_s);
    return istat;
  }
  if (data->learning->rea_neof != neof) {
    (void) fprintf(stderr, "%s: ERROR: Number of EOFs (%d) for reanalysis %s field from EOF file (%s) is not equal to number of EOFs specified in XML configuration file for reanalysis fields (%d)!\n", __FILE__, neof,
                   data->learning->rea->nomvar_sing, data->learning->rea->filename_eof, data->learning->rea_neof);
    (void) free(data->learning->rea->time_s);
    return -1;
  }

  /* Diagnostic status */
  return 0;
}
