#include <stdio.h>

void		change_special(void){
	int		c;
	while ((c = getchar()) != EOF){
		if (c == '\t')
			putchar('\\'), putchar('t');
		else if (c == '\\')
			putchar('\\'), putchar('\\');
		else if (c == '\b')
			putchar('\\'), putchar('b');
		else
			putchar(c);
	}
}


int			main(void){
	change_special();

	return 0;
}

