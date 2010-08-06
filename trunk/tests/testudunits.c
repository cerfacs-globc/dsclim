#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <udunits.h>

int main(int argc, char *argv[])
{
	utUnit  dataunits;
	double  newmonth=2.0;
	int     year, month, day, hour, minute;
	float   second;
	static  char *months[12] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	char *units = "months since 1975-11-01 00:00:00.0";

	if(argc==2)
		newmonth = (double)atof(argv[1]);

	utInit(NULL);
	if(utScan(units,&dataunits) != 0)
	{
		printf("internal error: udu_fmt_time can't parse data unit string!\n" );
		printf("problematic units: \"%s\"\n", units );
		exit( -1 );
	}

	if(utCalendar( newmonth, &dataunits, &year, &month, &day, &hour, &minute, &second ) != 0)
	{
		printf("internal error: udu_fmt_time can't convert to calendar value!\n");
		printf("units: >%s<\n", units );
		exit( -1 );
	}
	
	printf("month: %d, %s-%04d\n", month,months[month-1], year );
        
        (void) utTerm();

	return 0;
}
