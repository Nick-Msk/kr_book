#include <stdio.h>

#define			MAXLINE		4096

int			maxlen = 0;
char		line[MAXLINE];
char		longestline[MAXLINE];

int			get_line(void);
int			copy(void);


int			main(void){
	int		len;

	while ((len = get_line()) > 0)
		if (len > maxlen){
			maxlen = len;
			copy();
		}

	if (maxlen > 0)
		printf("%d:[%s]\n", maxlen, longestline);
	else 
		printf("Empty stream\n");
	return 0;
}

int         copy(void){
	int			i = 0;
	extern		char line[], longestline[];

	while ( i < maxlen - 1 && (longestline[i] = line[i]) != '\0')
		i++;
	if (longestline[i] != '\0')
		longestline[++i] = '\0';		// fill last
	return i;
}

int			get_line(){
	int				c, i;
	extern			char line[];

	for (i = 0; i < MAXLINE - 1 && (c = getchar()) != EOF && c != '\n'; i++)
		line[i] = c;
	if (c == '\n')
		line[i++] = c;	
	line[i] = '\0';

	return i;
}
