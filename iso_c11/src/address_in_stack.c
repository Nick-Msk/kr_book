#include <stdio.h>

int		main(void){
	int		a1;		printf("a1 = %p\n", &a1);
	int		a2;		printf("a2 = %p\n", &a2);
	{
		int		b1; 	printf("b1 = %p\n", &b1);
	}
	int		c1; 	printf("c1 = %p\n", &c1);
	return 0;
}
