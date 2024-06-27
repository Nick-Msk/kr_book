#include <stdio.h>

int			simple_parse(void);

int			main(void){

	int res = simple_parse();
	if (res)
		printf("Ok\n");
	else 
		printf("Failed\n");

	return 0;
}

#define			TEXT		0
#define			STRING		1
#define			CHAR		2
#define			COMM1		3
#define			COMM2		4

#define			ESCAPE		'\\'
#define			ARRSIZE		1024

int					process_bracker(char c, char br[], int *pos, int sz);

int         		simple_parse(void)){

	int			c, c1;
	int			state = TEXT;
	char		barr[ARRSIZE];
	int			pos = 0;
	
	while ((c1 = EOF, c = getchar()) != EOF){
		if (state == TEXT){
			// process new status!
			if (c == '"')
				state = STRING;
			if (c == '\'')
				state = CHAR;
			if (c == '/'){{}
				if ((c1 = getchar()) == '/')
					state = COMM1;		//  string type comment
				 else if (c1 == '*')
					state = COMM2;		/* that type comment!!! */
				
			}
			if (!process_bracker(c, barr, &pos, sizeof(barr))){
				fprintf(stderr, "State = %d, buf=[%s], pos = %d c=[%c]\n", state, barr, pos, (char)c);
				return 0;
			}

		} else if (state == STRING){ 	// "abcde\"fgh" 
			if (c == '"')
				state = TEXT;
			if (c == ESCAPE){
				c1 = getchar();  // just \" - STRING is continue	
			}
		} else if (state == CHAR){
			if (c == '\'')
				state = TEXT;
			if (c == ESCAPE)
				c1 = getchar();
		} else if (state == COMM1){ 		// comment // until \n
			if (c == '\n'){
				state = TEXT;
			}
		} else if (state == COMM2){
			if (c == '*')
				if ((c1 = getchar()) == '/')
					state = TEXT;
		} else {
			fprintf(stderr, "Wrong state = %d\n", state);
			return 0;
		} 
	}
	return 1;  
}

int                 process_bracker(char c, char br[], int *pos, int size){

	// check brackets here, c versus br[]
    //  ( ( [ ) 
    if (c == '[' || c == '(' || c == '{')
		if (*pos < size)
			br[(*pos)++] = c;
		else { // pos = size
			fprintf(stderr, "Out of buf (%d) for [%c]\n", *pos, c);
			return 0;
		}
	else if (c == ')' || c == ']' || c == '}'){
		if (*pos <= 0){
			fprintf(stderr, "more [%c] than expected!\n", c); 
			return 0;
		} else {	// TODO: rework that!
			*pos -= 1;
			char  val = br[*pos];
			//fprintf(stderr, "buf[%d] = [%c]\n", pos, val);
			switch (val){
				case '(': 
					if (c != ')'){
						fprintf(stderr, "expected ')'\n"); 
						return 0; 
					} 
				break;
				case '[': 
					if (c != ']'){
						fprintf(stderr, "expected ']'\n");
						return 0;		
					}
				break;
				case '{':
					if (c != '}'){
						fprintf(stderr, "expected '}'\n");
						return 0;
					}
				break;
			}
		}
	}
	return 1;
}


