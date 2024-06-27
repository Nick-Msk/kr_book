#include <stdio.h>
#include <stdlib.h>

#define			MAXLINE			40

// TODO: rework normally!!! 

int			get_line(char *line, int maxline);
int			copy(char* to, const char* from, int maxline);


int			main(int argc, const char *argv[]){

	int		maxline = MAXLINE;
	int		len, maxlen = 0, currlen = 0;
	char	flag = 'n'; // n or c 
	char 	line[MAXLINE];

	if (argc > 1)
		maxline = atoi(argv[1]);

	if (maxline < 2){
		printf("Maxline = %d is too small, assuming %d\n", maxline, MAXLINE);
		maxline = MAXLINE;
	}
	
	while ((len = get_line(line, maxline)) > 0){
		
		if (line[len - 1] == '\n'){		// EOF OF LINE!!!
			if (flag == 'n')
				currlen = len;
			else  
				currlen += len, flag = 'n';

			if (currlen > maxlen){
				//printf("currlen = %d, maxlen = %d\n", currlen, maxlen);
				maxlen = currlen;
			}
		} else {
			if (flag == 'n')		// begin of the line
				currlen = len, flag = 'c';
			else {
				//printf("currlen = %d, len = %d, line=[%s]\n", currlen, len, line);
				currlen += len;
			}
		}
	}
	// check if EOF in the current line without \n
	if (line[len - 1] != '\n'){
		if (flag == 'n')
            currlen = len;
        else
            currlen += len;
		if (currlen > maxlen)
			maxlen = currlen;
	}

	if (maxlen > 0)
		printf("Maxlen = %d\n", maxlen);
	else 
		printf("Empty stream\n");
	return 0;
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
