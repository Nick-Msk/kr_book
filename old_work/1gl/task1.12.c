#include <stdio.h>

#define 		IN	0
#define			OUT	1

int			main(void){
	int			c, state = OUT;
	while ((c = getchar()) != EOF){
		if (c == ' ' || c == '\t' || c == '\n'){
			if (state == IN)
				putchar('\n');
			state = OUT;
		}
		else {
			putchar(c);
			if (state == OUT)
				state = IN;
		}
	}
	if (state == IN)
		putchar('\n');
	
	return 0;
}
