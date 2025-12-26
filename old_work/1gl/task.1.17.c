#include <stdio.h>
#include <stdlib.h>

#define			MAXLINE		4096
#define			DEFLINE		80

int			get_line(char *line, int maxline);
int			copy(char* to, const char* from, int maxline);

int			main(int argc, const char **argv){
	int		linelim = DEFLINE;
	char 	line[MAXLINE];
	int		len;

	if (argc > 1) 
		linelim = atoi(argv[1]);
	
	// print line if length > linelim
	while ((len = get_line(line, MAXLINE)) > 0)
		if (len > linelim)
			printf("(%d)%s", len, line);

	return 0;
}

int         copy(char* to, const char* from, int maxline){
	int	i = 0;
	while ( i < maxline - 1 && (to[i] = from[i]) != '\0')
		i++;
	if (to[i] != '\0')
		to[++i] = '\0';		// fill last
	return i;
}

int			get_line(char *line, int lim){
	int		c, i;
	for (i = 0; i < lim - 1 && (c = getchar()) != EOF && c != '\n'; i++)
		line[i] = c;
	if (c == '\n')
		line[i++] = c;	
	line[i] = '\0';

	return i;
}


