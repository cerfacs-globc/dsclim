/* ***************************************************** */
/* Show banners for execution of a program.              */
/* banner.c                                              */
/* ***************************************************** */
/* Author: Christian Page, CERFACS, Toulouse, France.    */
/* ***************************************************** */
/*! \file banner.c
    \brief Show banners for execution of a program.
*/

#include <misc.h>

/** Output banner on terminal for BEGIN and END of programs. */
void banner(char *pgm, char *verstat, char *type)
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
    (void) printf("1\n");
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
    (void) printf("1\n");
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
