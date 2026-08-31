#include <stdio.h>

#define			MAXLINE		4096

int			get_line(char *line, int maxline);
int			copy(char* to, const char* from, int maxline);


int			main(void){
	int		len, maxlen = 0;
	char 	line[MAXLINE];
	char	longestline[MAXLINE];

	while ((len = get_line(line, MAXLINE)) > 0)
		if (len > maxlen){
			maxlen = len;
			copy(longestline, line, MAXLINE);
		}

	if (maxlen > 0)
		printf("%d:[%s]\n", maxlen, longestline);
	else 
		printf("Empty stream\n");
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
