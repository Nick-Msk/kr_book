#include <stdio.h>

int			remove_comment(void);

int			main(void){

	int cnt = remove_comment();
	fprintf(stderr, "TOTAL %d comment ///detected!\n", cnt);
	return 0;
}

//hfhgfggtyfhgfgfgfhfhfgfgfgfggfhfhhgffhhfhfghgffghffhhfg
#define			TEXT		0
#define			STRING		1
#define			CHAR		2
#define			COMM1		3
#define			COMM2		4
#define			ESCAPE		'\\'

static inline void	putchars(int c, int c1){
	putchar(c);
	if (c1 != EOF)      // print if required
         putchar(c1);
}

int         		remove_comment(void){

	int			c, c1, comm_cnt = 0;
	int			state = TEXT;
	
	// 1. remove only string comment
	// based on STATE (0  - 4)
	while ((c1 = EOF, c = getchar()) != EOF){
		if (state == TEXT){
			// process new status!
			if (c == '"')
				state = STRING;
			if (c == '\'')
				state = CHAR;
			if (c == '/'){
				if ((c1 = getchar()) == '/'){
					state = COMM1;		//  string type comment
					comm_cnt++;
				} else if (c1 == '*'){
					state = COMM2;		/* that type comment!!! */
					comm_cnt++;
				}
			}
				
			// print if required
			if (state == STRING || state == CHAR || state == TEXT){
				putchars(c, c1);
			}
			if (state == COMM1 || state == COMM2)	// nothing printed here!
				;
		} else if (state == STRING){ 	// "abcde\"fgh" 
			if (c == '"')
				state = TEXT;
			if (c == ESCAPE){
				c1 = getchar();  // just \" - STRING is continue	
			}
			
			// printing
			putchars(c, c1);			
		} else if (state == CHAR){
			if (c == '\'')
				state = TEXT;
			if (c == ESCAPE)
				c1 = getchar();
			// printing
			putchars(c, c1);	
		} else if (state == COMM1){ 		// comment // until \n
			if (c == '\n'){
				state = TEXT;
				putchar(c);		// put a newline
			}
		} else if (state == COMM2){
			if (c == '*')
				if ((c1 = getchar()) == '/')
					state = TEXT;
			
			/*
				*/
		} else {
			fprintf(stderr, "Wrong state = %d\n", state);
			return 1;
		} 
	}
	return comm_cnt;  
}
