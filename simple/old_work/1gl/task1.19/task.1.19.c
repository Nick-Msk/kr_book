#include <stdio.h>

#define			MAXLINE		4096

int			get_line(char *line, int maxline);

int			reverse(char *s, int sz);

int			main(void){
	int			len;
	char 		buf[MAXLINE];

	while ((len = get_line(buf, MAXLINE)) > 0){
		reverse(buf, len);
		printf("%s", buf);
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

int			reverse(char *s, int sz){
	int			tmp;
	if (sz == 0)
		while(s[sz++])
		;
	// skip last \n if exists!
	if (sz > 0 && s[sz - 1] == '\n')
		sz--;
	// exchange till 1/2
	for (int i = 0; i < sz / 2; i++){
		tmp = s[i];
		s[i] = s[sz - 1 - i];
		s[sz - 1 - i] = tmp;
	}
	return sz;
}
