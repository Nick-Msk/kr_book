#include <stdio.h>

#define			MAXLINE		4096
#define			NEWLINEFLAG 1

int			get_line(char *line, int maxline);

int			main(void){

	int			len, pos;
	char		buf[MAXLINE];
	int			flags = 0;

	while ((pos = len = get_line(buf, MAXLINE)) > 0){
		fprintf(stderr, "pos = %d, len = %d buf[%s]\n", pos, len, buf);
		flags = 0;
		// setup position to the end of line
		pos--;
		if (buf[pos] == '\n'){
			flags = NEWLINEFLAG;
			pos--;
			fprintf(stderr, "setup flag, pos = %d\n", pos);
		}

		while (pos >= 0 && (buf[pos] == ' ' || buf[pos] == '\t'))
			pos--;	
		fprintf(stderr, "pos after cutoff = %d\n", pos);

		pos++;
		if (pos > 0){ 
			fprintf(stderr, "pos + flags = %d\n", pos + flags);
			if (pos + flags < len){
				buf[pos + 1] = '\0';
				if (flags == NEWLINEFLAG)
					buf[pos] = '\n';
			} // if pos + flag == len => nothing to do, just print as is
			printf("%s", buf);
		}   // else if pos == 0 then empty line, do nothing
	}
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

