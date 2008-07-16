#include <utils.h>

void banner(char *pgm, char *verstat, char *type)
{
  static clock_t clk;
  time_t tim;
  char buf[50];
  char *ctim;

  tim = time(NULL);

  ctim = ctime_r(&tim, buf);
  ctim[strlen(ctim)-1] = '\0';

  if ( !strcmp(type, "BEGIN") ) {
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
