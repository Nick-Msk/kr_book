#include <stdio.h>
#include <stdlib.h>

#define			DEFTABSIZE 		4

int			main(int argc, const char **argv){
	int			tabsize = DEFTABSIZE;
	int			c;
	
	if (argc > 1)
		tabsize = atoi(argv[1]);

	while ((c = getchar()) != EOF){
		if (c == '\t')
			for (int i = 0; i < tabsize; i++)
				putchar(' ');
		else
			putchar(c);
	}

	return 0;
}
