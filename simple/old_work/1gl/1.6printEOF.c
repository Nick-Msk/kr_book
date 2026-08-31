#include <stdio.h>

int			main(void){
	int		i = EOF;
	char 	c = EOF;
	printf("[%d] [%c] char as int [%d]\n", i, c, (int) c);
	
	c = 20;	
	while (c--){
		i = (getchar() != EOF);
		printf("%d\n", i);
	}

	return 0;
}
