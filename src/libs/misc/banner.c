/* ***************************************************** */
/* Show banners for execution of a program.              */
/* banner.c                                              */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file banner.c
    \brief Show banners for execution of a program.
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




#include <misc.h>

/** Output banner on terminal for BEGIN and END of programs. */
void
banner(char *pgm, char *verstat, char *type)
{
  /**
     @param[in]      pgm           Program name.
     @param[in]      verstat       Program version.
     @param[in]      type          Type of banner: BEGIN or END.
  */

  static clock_t clk; /* Store CPU clock time. */
  time_t tim; /* To compute clock time. */
  char buf[50]; /* Temporary buffer for reentrant subroutine access. */
  char *ctim; /* Date/time string. */

  /* Get current time */
  tim = time(NULL);

  /* Create date/time string */
  ctim = ctime_r(&tim, buf);
  ctim[strlen(ctim)-1] = '\0';

  /* Output banner */
  if ( !strcmp(type, "BEGIN") ) {
    /* Initialize CPU clock time */
    clk = clock();
    /*    (void) printf("1\n"); */
    (void) printf("   ********************************************************************************************\n");
    (void) printf("   *                                                                                          *\n");
    (void) printf("   *          %-50s                V%-10s   *\n", pgm, verstat);
    (void) printf("   *                                                                                          *\n");
    (void) printf("   *                                                                                          *\n");
    (void) printf("   *          %-24s                                                        *\n", ctim);
    (void) printf("   *                                                                                          *\n");
    (void) printf("   *          BEGIN EXECUTION                                                                 *\n");
    (void) printf("   *                                                                                          *\n");
    (void) printf("   ********************************************************************************************\n");
  }
  else if ( !strcmp(type, "END") ) {

    float clktime;

    /* Compute elapsed CPU clock time */
    clktime = (float) (clock() - clk) / (float) CLOCKS_PER_SEC;
    /* (void) printf("1\n"); */
    (void) printf("   ********************************************************************************************\n");
    (void) printf("   *                                                                                          *\n");
    (void) printf("   *          %-50s                 %-10s   *\n", pgm, verstat);
    (void) printf("   *                                                                                          *\n");
    (void) printf("   *          %-24s                                                        *\n", ctim);
    (void) printf("   *                                                                                          *\n");
    (void) printf("   *          END EXECUTION                                                                   *\n");
    (void) printf("   *                                                                                          *\n");
    (void) printf("   *          CP SECS = %-10.3f                                                            *\n",
		  clktime);
    (void) printf("   *                                                                                          *\n");
    (void) printf("   ********************************************************************************************\n");
  }
}
