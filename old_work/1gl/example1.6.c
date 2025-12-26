#include <stdio.h>

int			main(void){
	int			c, nwhite = 0, nothers = 0;
	int			ndigits[10];

	for (int i = 0; i < sizeof(ndigits)/sizeof(int); i++)
		ndigits[i] = 0;	

	while ((c = getchar()) != EOF){
		if (c >= '0' && c <= '9')
			ndigits[c - '0']++;
		else if (c == ' ' || c == '\t' || c == '\n')
			nwhite++;
		else 
			nothers++;
	}

	printf("Digits =");
	for (int i = 0; i < sizeof(ndigits)/sizeof(int); i++)
		printf(" %d", ndigits[i]);
	printf(", white spaced = %d, others = %d\n", nwhite, nothers);

	return 0;
}



