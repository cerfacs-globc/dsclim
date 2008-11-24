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

#include <dsclim.h>

/** C prototypes. */
void show_usage(char *pgm);

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

  /* Command-line arguments variables */
  char fileconf[500]; /* Configuration filename */

  /* Print BEGIN banner */
  (void) banner(basename(argv[0]), "1.0", "BEGIN");

  /* Allocate memory */
  data = (data_struct *) malloc(sizeof(data_struct));
  if (data == NULL) alloc_error(__FILE__, __LINE__);

  /* Get command-line arguments and set appropriate variables */
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
  istat = load_conf(data, fileconf);
  if (istat != 0) {
    (void) fprintf(stderr, "%s: Error in loading configuration file. Aborting.\n", __FILE__);
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }

  /* Read regression points positions */
  istat = read_regression_points(data->reg);
  if (istat != 0) {
    (void) fprintf(stderr, "%s: Error in reading regression points positions. Aborting.\n", __FILE__);
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }

  /* If wanted, generate learning data */
  istat = wt_learning(data);
  if (istat != 0) {
    (void) fprintf(stderr, "%s: Error in computing or reading learning data needed for downscaling. Aborting.\n", __FILE__);
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }

  /* Perform downscaling */
  istat = wt_downscaling(data);
  if (istat != 0) {
    (void) fprintf(stderr, "%s: Error in performing downscaling. Aborting.\n", __FILE__);
    (void) banner(basename(argv[0]), "ABORT", "END");
    (void) abort();
  }

  /* Free main data structure */
  (void) free_main_data(data);
  
  /* Free memory */
  (void) free(data);

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

  (void) fprintf(stderr, "%s:: usage:\n", pgm);
  (void) fprintf(stderr, "-conf: configuration file\n");

}
