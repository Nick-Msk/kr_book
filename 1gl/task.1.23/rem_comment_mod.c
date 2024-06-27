#include <stdio.h>

int			remove_comment(void);

int			main(void){

	int cnt = remove_comment();
	fprintf(stderr, "TOTAL %d comment ///detected!\n", cnt);
	return 0;
}


#define			TEXT		0
#define			STRING		1
#define			CHAR		2
#define			COMM1		3
#define			COMM2		4
#define			ESCAPE		'\\'

static inline void	putchars(int c, int c1){
	putchar(c);
	if (c1 != EOF)      
         putchar(c1);
}

int         		remove_comment(void){

	int			c, c1, comm_cnt = 0;
	int			state = TEXT;
	
	
	
	while ((c1 = EOF, c = getchar()) != EOF){
		if (state == TEXT){
			
			if (c == '"')
				state = STRING;
			if (c == '\'')
				state = CHAR;
			if (c == '/'){
				if ((c1 = getchar()) == '/'){
					state = COMM1;		
					comm_cnt++;
				} else if (c1 == '*'){
					state = COMM2;		
					comm_cnt++;
				}
			}
				
			
			if (state == STRING || state == CHAR || state == TEXT){
				putchars(c, c1);
			}
			if (state == COMM1 || state == COMM2)	
				;
		} else if (state == STRING){ 	
			if (c == '"')
				state = TEXT;
			if (c == ESCAPE){
				c1 = getchar();  
			}
			
			
			putchars(c, c1);			
		} else if (state == CHAR){
			if (c == '\'')
				state = TEXT;
			if (c == ESCAPE)
				c1 = getchar();
			
			putchars(c, c1);	
		} else if (state == COMM1){ 		
			if (c == '\n'){
				state = TEXT;
				putchar(c);		
			}
		} else if (state == COMM2){
			if (c == '*')
				if ((c1 = getchar()) == '/')
					state = TEXT;
			
			
		} else {
			fprintf(stderr, "Wrong state = %d\n", state);
			return 1;
		} 
	}
	return comm_cnt;  
}
