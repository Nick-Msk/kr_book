#include <stdio.h>
#include <stdlib.h>

#define		TOTAL	10
long		power(int base, int n);

int			main(int argc, const char **argv){
	int		total = TOTAL;

	if (argc > 1)
		total = atoi(argv[1]);		
	
	for (int i = 0; i < total; i++)
		printf("%d\t %ld\t %ld\n", i, power(2, i), power(-3, i));
	return 0;
}

long        power(int base, int n){
	long 	p = 1L;

	for (int i = 0; i < n; i++)
		p *= base;
	return p;
}
