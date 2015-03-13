/* ***************************************************** */
/* testbestclassif_realdata Test best classification     */
/* algorithm using real NetCDF data.                     */
/* testbestclassif_realdata.c                            */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file testbestclassif_realdata.c
    \brief Test best classification algorithm using real NetCDF data.
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







#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/** GNU extensions */
#define _GNU_SOURCE

/* C standard includes */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#include <zlib.h>
#include <hdf5.h>
#include <netcdf.h>

#include <classif.h>

/** C prototypes. */
void show_usage(char *pgm);
void handle_netcdf_error(int status, int lineno);

/** Main program. */
int main(int argc, char **argv)
{
  /**
     @param[in]  argc  Number of command-line arguments.
     @param[in]  argv  Vector of command-line argument strings.

     \return           Status.
   */

  int i;
  int j;
  int neof;
  int ndays;
  int nclusters;
  int nclassif;
  int npart;

  size_t dimval;

  double *pc_eof_days = NULL;
  double *clusters = NULL;

  char *filein = NULL;
  char *fileoutclust = NULL;
  FILE *fileoutclust_ptr = NULL;
  char *fileoutpc = NULL;
  FILE *fileoutpc_ptr = NULL;

  int istat, ncid;
  int eofid, dayid, eofdimid, daydimid, varid;
  nc_type vartype;
  int varndims;
  int vardimids[NC_MAX_VAR_DIMS];    /* dimension ids */

  size_t start[2];
  size_t count[2];

  /* Print BEGIN banner */
  (void) banner(basename(argv[0]), "1.0", "BEGIN");

  /* Get command-line arguments and set appropriate variables */
  for (i=1; i<argc; i++) {
    if ( !strcmp(argv[i], "-h") ) {
      (void) show_usage(basename(argv[0]));
      (void) banner(basename(argv[0]), "OK", "END");
      return 0;
    }
    else if ( !strcmp(argv[i], "-i") ) {
      filein = (char *) malloc((strlen(argv[++i])+1) * sizeof(char));
      if (filein == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(filein, argv[i]);
    }
    else if ( !strcmp(argv[i], "-o_clust") ) {
      fileoutclust = (char *) malloc((strlen(argv[++i])+1) * sizeof(char));
      if (fileoutclust == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(fileoutclust, argv[i]);
      fileoutclust_ptr = fopen(fileoutclust, "w");
      if (fileoutclust_ptr == NULL) {
        (void) fprintf(stderr, "%s: Cannot open file %s for output!\n", __FILE__, fileoutclust);
        (void) banner(basename(argv[0]), "ABORT", "END");
        (void) abort();
      }
    }
    else if ( !strcmp(argv[i], "-o_pc") ) {
      fileoutpc = (char *) malloc((strlen(argv[++i])+1) * sizeof(char));
      if (fileoutpc == NULL) alloc_error(__FILE__, __LINE__);
      (void) strcpy(fileoutpc, argv[i]);
      fileoutpc_ptr = fopen(fileoutpc, "w");
      if (fileoutpc_ptr == NULL) {
        (void) fprintf(stderr, "%s: Cannot open file %s for output!\n", __FILE__, fileoutpc);
        (void) banner(basename(argv[0]), "ABORT", "END");
        (void) abort();
      }
    }
    else {
      (void) fprintf(stderr, "%s:: Wrong arg %s.\n\n", basename(argv[0]), argv[i]);
      (void) show_usage(basename(argv[0]));
      (void) banner(basename(argv[0]), "ABORT", "END");
      (void) abort();
    }
  }

  /* Try 1000 classifications */
  nclassif = 1000;
  /* Use 8 clusters */
  nclusters = 10;
  /* Try 100 partitions for best classification */
  npart = 30;

  /* Read data in NetCDF file */
  istat = nc_open(filein, NC_NOWRITE, &ncid);  /* open for reading */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  
  istat = nc_inq_dimid(ncid, "eof", &eofdimid);  /* get ID for eof dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncid, eofdimid, &dimval); /* get eof length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  neof = (int) dimval;

  istat = nc_inq_dimid(ncid, "day", &daydimid);  /* get ID for day dimension */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_dimlen(ncid, daydimid, &dimval); /* get day length */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  ndays = (int) dimval;
  
  istat = nc_inq_varid(ncid, "eof", &eofid);  /* get ID for eof variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);
  istat = nc_inq_varid(ncid, "day", &dayid);  /* get ID for day variable */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_varid(ncid, "pc_proj", &varid); /* get pc_proj variable ID */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  istat = nc_inq_var(ncid, varid, (char *) NULL, &vartype, &varndims, vardimids, (int *) NULL); /* get variable information */
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  if (vartype != NC_DOUBLE || varndims != 2) {
    (void) fprintf(stderr, "Error NetCDF type and/or dimensions.\n");
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }

  /** Read data variable **/
  /* Allocate memory and set start and count */
  start[0] = 0;
  start[1] = 0;
  count[0] = (size_t) ndays;
  count[1] = (size_t) neof;
  /* Allocate memory */
  pc_eof_days = (double *) calloc(neof*ndays, sizeof(double));
  if (pc_eof_days == NULL) alloc_error(__FILE__, __LINE__);
  clusters = (double *) calloc(neof*nclusters, sizeof(double));
  if (clusters == NULL) alloc_error(__FILE__, __LINE__);

  /* Read values from netCDF variable */
  istat = nc_get_vara_double(ncid, varid, start, count, pc_eof_days);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Close the netCDF file. */
  istat = ncclose(ncid);
  if (istat != NC_NOERR) handle_netcdf_error(istat, __LINE__);

  /* Find clusters: test best classification algorithm */
  (void) best_clusters(clusters, pc_eof_days, "euclidian", npart, nclassif, neof, nclusters, ndays);

  /* Output data */
  for (i=0; i<neof; i++)
    for (j=0; j<nclusters; j++)
      (void) fprintf(fileoutclust_ptr, "%d %d %lf\n", i, j, clusters[i+j*neof]);

  for (j=0; j<ndays; j++)
    for (i=0; i<neof; i++)
      (void) fprintf(fileoutpc_ptr, "%d %d %lf\n", i, j, pc_eof_days[i+j*neof]);

  (void) fclose(fileoutclust_ptr);
  (void) fclose(fileoutpc_ptr);

  /* Free memory */
  (void) free(pc_eof_days);
  (void) free(clusters);
  (void) free(filein);
  (void) free(fileoutclust);
  (void) free(fileoutpc);

  /* Print END banner */
  (void) banner(basename(argv[0]), "OK", "END");

  return 0;
}


/** Local Subroutines **/

/** Show usage for program command-line arguments. */
void show_usage(char *pgm) {
  /**
     @param[in]  pgm  Program name.
  */

  (void) fprintf(stderr, "%s: usage:\n", pgm);
  (void) fprintf(stderr, "-h: help\n");

}

/* Handle error */
void handle_netcdf_error(int status, int lineno)
{
  if (status != NC_NOERR) {
    fprintf(stderr, "Line: %d Error: %s\n", lineno, nc_strerror(status));
    exit(-1);
  }
}
