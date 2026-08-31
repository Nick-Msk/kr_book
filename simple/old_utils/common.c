#include "common.h"




// fill with 0.0 cnt elements 
void
cleaner_double(void *arr, int cnt)
{
	double *d = (double *) arr;
    for (int i = 0; i < cnt; i++)
        d[i] = 0.0;
}

