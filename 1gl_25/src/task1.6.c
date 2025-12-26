#include <stdio.h>

// gets if getchar() != EOF is 0 or 1

int 	main(void){
	int res;
	while( (res = (getchar() != EOF)) )
		printf("res = %d\n", res);
	printf("res = %d\n", res);

	printf("EOF == %d\n", EOF);
}
