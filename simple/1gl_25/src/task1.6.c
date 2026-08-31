#include <stdio.h>

// gets if getchar() != EOF is 0 or 1

int 	main(void){
	int res;
	while( (res = getchar()) != EOF)
		printf("%d\n", res);
	printf("%d\n", res);
}
