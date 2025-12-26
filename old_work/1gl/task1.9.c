#include <stdio.h>

void		skip_empty_lines(void){
	int     c;  
    int     space_cnt = 0;      // counter of spaces from the beginning of current string
    char    str_flag = 'b'; 	// b - begin of str, n - normal, non empry string    
    while ((c = getchar()) != EOF){
		if (str_flag == 'n') // normal  line - do nothing until '\n'
			putchar(c);
		else {
			if (c == ' '){
				space_cnt++;
			} else { // '       qwerty'
				if (space_cnt > 0)
					while (--space_cnt) 
						putchar(' ');
				str_flag = 'n';	// this is normal line, just print symbols
				putchar(c);
			}	
		}		
        if (c == '\n')
			str_flag = 'b', space_cnt = 0;
    }   
	// print other symbols
	if (str_flag == 'b' && space_cnt > 0)
		while (--space_cnt) 
        	putchar(' ');
}

int		main(void){
	skip_empty_lines();
	
	return 0;
}
