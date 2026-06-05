#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500   /* или 700 */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int		main(void){
	srand48(time(0));
	double val = drand48();
	printf("%lf\n", val);
	return 0;
}
