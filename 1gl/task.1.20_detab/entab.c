#include <stdio.h>
#include <stdlib.h>

#define			DEFTABSIZE 		4

int			printtab(int spacescnt, int tabsize);

int			main(int argc, const char **argv){

	int			tabsize = DEFTABSIZE;
	int			c, cnt = 0;
	
	if (argc > 1)
		tabsize = atoi(argv[1]);

	if (tabsize <= 1){
		fprintf(stderr, "tabsize %d must be > 1\n", tabsize); 
		return 1;
	}

	while ((c = getchar()) != EOF){
		if (c == ' ')
			cnt++;
		else if (c == '\t')
			cnt += tabsize;
		else {
			// print all bunch of spaces
			printtab(cnt, tabsize);
			cnt = 0;	// reset
			putchar(c);	
		}
	}
	// if EOF just after the spaces
	printtab(cnt, tabsize);
	return 0;
}

int         printtab(int spacescnt, int tabsize){

	int			i, tabcnt = 0;
	for (i = 0; i < spacescnt / tabsize; i++)
		putchar('\t'), tabcnt++;

	// print the rest of spaces
	for (i = 0; i < spacescnt % tabsize; i++)
		putchar(' ');
	return tabcnt;
}
