// version without get_line(), just read input via getchar()

#include <stdio.h>

#define 		BUFSIZE			1024
#define			BEGLINEFLAG		1
#define			INTHELINEFLAG	0

int				is_space(int c){
	return c == ' ' || c == '\t';		// something else? 
}

int				main(void){
	int		c, pos = 0, flags = BEGLINEFLAG;
	char	spaces[BUFSIZE];

	while ((c = getchar()) != EOF){
		if (is_space(c))
			if (pos < BUFSIZE - 1)
				spaces[pos++] = c;
			else { // out of buf, error exit
				fprintf(stderr, "Out of buffer (%d)!\n", pos);
				return 1;
			}
		else {
			if (c != '\n'){
				// print the spaces in buffer
              	if (pos > 0){   // pos + 1 < bufsize!!!
                  	spaces[pos] = '\0';
                  	printf("%s", spaces);
                  	pos = 0;        // reset buffer
              	}
				flags = INTHELINEFLAG;		// line with real symblols
			}
			if (flags != BEGLINEFLAG)		// flags == BEGLINEFLAG only when no any real sybmol in the line	
				putchar(c);
		}
		if (c == '\n')
			flags = BEGLINEFLAG;
	}
	return 0;
}
