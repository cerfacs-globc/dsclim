/* ***************************************************** */
/* dsclim Downscaling climate scenarios                  */
/* dsclim.c                                              */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/* Date of creation: sep 2008                            */
/* Last date of modification: sep 2008                   */
/* ***************************************************** */
/* Original version: 1.0                                 */
/* Current revision:                                     */
/* ***************************************************** */
/* Revisions                                             */
/* ***************************************************** */
/*! \file dsclim.c
    \brief Downscaling climate scenarios program.
*/

/* LICENSE BEGIN

Copyright Cerfacs (Christian Page) (2010)

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

/** C prototypes. */
void show_usage(char *pgm);
short int show_license();

/** Main program. */
int main(int argc, char **argv)
{
  /**
     @param[in]  argc  Number of command-line arguments.
     @param[in]  argv  Vector of command-line argument strings.

     \return           Status.
   */

  int i; /* Loop counter */
  int istat = 0; /* Return status */
  data_struct *data = NULL; /* Main data structure */
  short int license_accept = 0; /* If user has accepted license or not */
  short int downscale = TRUE; /* If we want to downscale at least one period, excluding control */
  
  /* Command-line arguments variables */
  char fileconf[500]; /* Configuration filename */

  /* Show license so user can accept or deny it. */
  license_accept = show_license();
  if (license_accept == 1)
    (void) printf("\n\nYou accepted the license and its terms: the program can continue!\nYou will not have to input again the acceptance of the license\nand its terms for further use of the program in the current account.\n\n");
  else if (license_accept != 2) {
    (void) printf("\n\nYou did not accept the license and its terms: the program cannot continue. It must exit now.\n\n");
    (void) exit(0);
  }
  /* license_accept == 2 when user had already accepted the license in a previous execution of the program. */

  /* Print BEGIN banner */
  (void) banner(basename(argv[0]), "1.5.8", "BEGIN");

  /* Allocate memory */
  data = (data_struct *) malloc(sizeof(data_struct));
  if (data == NULL) alloc_error(__FILE__, __LINE__);

  /* Get command-line arguments and set appropriate variables */
  (void) printf("\n**** PROCESS COMMAND-LINE ARGUMENTS ****\n\n");
  if (argc <= 1) {
    (void) show_usage(basename(argv[0]));
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) exit(1);
  }
  else
    /* Decode command-line arguments */
    for (i=1; i<argc; i++) {
      if ( !strcmp(argv[i], "-conf") )
        (void) strcpy(fileconf, argv[++i]);
      else {
        (void) fprintf(stderr, "%s:: Wrong arg %s.\n\n", basename(argv[0]), argv[i]);
        (void) show_usage(basename(argv[0]));
        (void) banner(basename(argv[0]), "ABORT", "END");
        (void) exit(1);
      }
    }

  /* Read and store configuration file in memory */
  /* Allocate memory for main data structures */
  (void) printf("\n**** LOADING CONFIGURATION ****\n\n");
  istat = load_conf(data, fileconf);
  if (istat != 0) {
    (void) fprintf(stderr, "%s: Error in loading configuration file. Aborting.\n", __FILE__);
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }


  /* Generate analog data only if we are not reading it off disk */
  if (data->conf->output_only == FALSE) {
    
    /* Read regression points positions if needed */
    if (data->reg->filename != NULL) {
      (void) printf("\n**** READING REGRESSION POSITIONS ****\n\n");
      istat = read_regression_points(data->reg);
      if (istat != 0) {
        (void) fprintf(stderr, "%s: Error in reading regression points positions. Aborting.\n", __FILE__);
        (void) banner(basename(argv[0]), "ABORT", "END");
        (void) abort();
      }
    }

    /* Read mask for secondary large-scale fields if needed */
    if (data->secondary_mask->use_mask == TRUE) {
      (void) printf("\n**** SECONDARY LARGE-SCALE FIELDS MASK ****\n\n");
      istat = read_mask(data->secondary_mask);
      if (istat != 0) {
        (void) fprintf(stderr, "%s: Error in reading secondary large-scale mask file. Aborting.\n", __FILE__);
        (void) banner(basename(argv[0]), "ABORT", "END");
        (void) abort();
      }
    }
    
    /* Read mask for learning fields if needed */
    if (data->conf->learning_maskfile->use_mask == TRUE) {
      (void) printf("\n**** LEARNING FIELDS MASK ****\n\n");
      istat = read_mask(data->conf->learning_maskfile);
      if (istat != 0) {
        (void) fprintf(stderr, "%s: Error in reading learning mask file. Aborting.\n", __FILE__);
        (void) banner(basename(argv[0]), "ABORT", "END");
        (void) abort();
      }
    }

    /* If wanted, generate learning data */
    (void) printf("\n**** LEARNING ****\n\n");
    istat = wt_learning(data);
    if (istat != 0) {
      (void) fprintf(stderr, "%s: Error in computing or reading learning data needed for downscaling. Aborting.\n", __FILE__);
      (void) banner(basename(argv[0]), "ABORT", "END");
      (void) abort();
    }
  }
  
  /* Perform downscaling */
  downscale = FALSE;
  for (i=0; i<data->conf->nperiods; i++)
    if (data->conf->period[i].downscale == TRUE)
      downscale = TRUE;
  if (data->conf->period_ctrl->downscale == TRUE || downscale == TRUE) {
    (void) printf("\n**** DOWNSCALING ****\n\n");
    if (data->conf->output_only == TRUE)
      (void) printf("****WARNING: Configuration for reading analog dates and writing data ONLY!\n\n");
    istat = wt_downscaling(data);
    if (istat != 0) {
      (void) fprintf(stderr, "%s: Error in performing downscaling. Aborting.\n", __FILE__);
      (void) banner(basename(argv[0]), "ABORT", "END");
      (void) abort();
    }
  }
  
  /* Free main data structure */
  (void) printf("\n**** FREE MEMORY ****\n\n");
  (void) free_main_data(data);
  (void) free(data);

  /* Print END banner */
  (void) banner(basename(argv[0]), "OK", "END");
  
  return 0;
}


/** Local Subroutines **/

/** Show usage for program command-line arguments. */
void
show_usage(char *pgm) {
  /**
     @param[in]  pgm  Program name.
  */

  (void) fprintf(stderr, "%s:: usage:\n", pgm);
  (void) fprintf(stderr, "-conf: configuration file\n");

}

/** Show license so user can accept or deny it. */
short int
show_license() {
  /**
     \return license_accept
  */

  short int license_accept = 0;
  int istat = -1;
  const char license[5000] = "\nCopyright Cerfacs (Christian Page) (2010)\n\nchristian.page@cerfacs.fr\n\nThis software is a computer program whose purpose is to downscale climate\nscenarios using a statistical methodology based on weather regimes.\n\nThis software is governed by the CeCILL license under French law and\nabiding by the rules of distribution of free software. You can use, \nmodify and/ or redistribute the software under the terms of the CeCILL\nlicense as circulated by CEA, CNRS and INRIA at the following URL\n\"http://www.cecill.info\". \n\nAs a counterpart to the access to the source code and rights to copy,\nmodify and redistribute granted by the license, users are provided only\nwith a limited warranty and the software's author, the holder of the\neconomic rights, and the successive licensors have only limited\nliability. \n\nIn this respect, the user's attention is drawn to the risks associated\nwith loading, using, modifying and/or developing or reproducing the\nsoftware by the user in light of its specific status of free software,\nthat may mean that it is complicated to manipulate, and that also\ntherefore means that it is reserved for developers and experienced\nprofessionals having in-depth computer knowledge. Users are therefore\nencouraged to load and test the software's suitability as regards their\nrequirements in conditions enabling the security of their systems and/or \ndata to be ensured and, more generally, to use and operate it in the \nsame conditions as regards security. \n\nTo further continue, you must accept the CeCILL license and its terms.\n\n";
  char *dsclim_lic_file = NULL; /* dsclim license check file */
  char *dsclim_lic = NULL; /* dsclim license check */
  FILE *ifile = NULL; /* File for license acceptance */

  /* Let user accepts license */
  dsclim_lic_file = (char *) malloc(MAXPATH * sizeof(char));
  if (dsclim_lic_file == NULL) alloc_error(__FILE__, __LINE__);
  (void) sprintf(dsclim_lic_file, "%s/%s", getenv("HOME"), ".dsclim_lic");

  if ( access(dsclim_lic_file, F_OK) != 0 ) {
    dsclim_lic = (char *) malloc(10 * sizeof(char));
    if (dsclim_lic == NULL) alloc_error(__FILE__, __LINE__);
    /* User never had the license acceptance */
    (void) fprintf(stdout, license);
    (void) fprintf(stdout, "Do you accept or deny the license? [Type accept or deny]: ");
    /* Read input from user */
    istat = scanf("%7s", dsclim_lic);
    if ( !strcmp(dsclim_lic, "accept") ) {
      license_accept = 1;
      ifile = fopen(dsclim_lic_file, "w");
      if (ifile == NULL)
        (void) fprintf(stderr, "%s: Failed to write license acceptance file.\n", __FILE__);
      (void) fprintf(ifile, license);
      (void) fprintf(ifile, "The user accepted the dsclim license and its terms.\n\n");
      (void) fclose(ifile);
    }
    (void) free(dsclim_lic);
  }
  else
    license_accept = 2;

  (void) free(dsclim_lic_file);
  
  return license_accept;
}
