#include <stdio.h>

#define			SPACES	1
#define			NORMAL	0

// just simpe skip the many spaces in one
void	skip_spaces(void){
	int     c;  
    int    	state = NORMAL;

    while ((c = getchar()) != EOF){
        if (c == ' ' || c == '\t')
            state = SPACES;     // just set flag, do notheing here
        else {
            if (state == SPACES)        // there were spaces before
                putchar(' ');
            //  
            putchar(c);
            state = NORMAL;
        }   
    }   
    // final char if were spaces 
    if (state == SPACES)
        putchar(' ');
}

int		main(void){
	skip_spaces();
	
	return 0;
}
