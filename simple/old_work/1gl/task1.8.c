#include <stdio.h>
   
int		main(void){
	int		c;
	int		cnt = 0;
	while((c = getchar()) != EOF)
		if (c == ' ' || c == '\t' || c == '\n')
			cnt ++;

	printf("%d\n", cnt);
	return 0;
}
