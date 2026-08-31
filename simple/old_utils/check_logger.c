#include "log.h"
#include <stdio.h>

int			main(void){
	int		v = 1;
	printf("before v = %d\n", v);
	logauto(v++);
	printf("after v++ - %d\n", v);
	logauto(++v);
	printf("v = %d\n", v);

	v = 1;
	return logautoret(v += 6);
}

