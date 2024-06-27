#include "common.h"

// fill with 0.0 cnt elements 
void                cleaner_double(void *arr, int cnt)
{
	double *d = (double *) arr;
    for (int i = 0; i < cnt; i++)
        d[i] = 0.0;
}

// for now in common.c, then will be moved out
int			        get_line(char *line, int lim){
	int		c, i;
	for (i = 0; i < lim - 1 && (c = getchar()) != EOF && c != '\n'; i++)
		line[i] = c;
	if (c == '\n')
		line[i++] = c;
	line[i] = '\0';
	return i;
}

