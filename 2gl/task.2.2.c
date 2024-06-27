#include <stdio.h>

int				get_new_line(char *s, int sz);

int				main(void){
	
	char 		str[1024];
	while (get_new_line(str, sizeof(str)) > 0)
		printf("%s", str);

	return 0;
}

int             get_new_line(char *s, int sz){
	int		c, i;
	// cycle for similar is K&R
	for (i = 0; i < sz - 1; i++){
		c = getchar();
		if (c == '\n')
			break;
		if (c == EOF)
			break;
		s[i] = c;
	}
	if (c == '\n')
		s[i++] = c;
	s[i] = '\0';
	return i;
}
   

