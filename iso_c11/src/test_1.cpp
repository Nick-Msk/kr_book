#include <stdio.h>

int main(){
	int m = 1, n = 2;
	int *p = &m;
	int *q = &n;
	int **h = &p;
	*h = &n;
	**h = 5;
	printf("%d\n", n);
}
