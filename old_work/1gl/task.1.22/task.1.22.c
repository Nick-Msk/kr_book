#include <stdio.h>
#include <stdlib.h>

#define			MAXLEN			40
#define			INWORD			0
#define			OUTWORD			1
#define			BUFSIZE			1024

int			process_cut(int mexlen);

//       gjgdfgdshfvdfdfghvfdghfvghfvdghfvghfvdsghfvdsghfvdsghfvdsghfvdsghfvdsfghdsvfdghsfvdsghfvdsfghdsvfghdsvfdshgfvdsghfdvsfdhsfgdshfvdgshfvdsghfvdsghfdsvh
int			main(int argc, const char **argv){
	
	int			maxlen = MAXLEN;
	if (argc > 1)
		maxlen = atoi(argv[1]);

	if (maxlen <= 0){
		fprintf(stderr, "Must be positive!\n");
		return 1;
	}
	process_cut(maxlen);

	return 0;
}

int			print_buf(const char *buf, int pos){

	int			linecount = 0;
	for (int i = 0; i < pos; i++){
		if (buf[i] == '\n')
			linecount++;
		putchar(buf[i]);
	}

	return linecount;
}

int         process_cut(int maxlen){
	
	int			c, pos = 0, linecount = 0; 
	int			spacecnt = 0, state = OUTWORD;
	char		spaces[BUFSIZE];
	spaces[0] = '\0';

	while ((c = getchar()) != EOF){
		if (c == ' ' || c == '\t' || c == '\n')
			state = OUTWORD;  
		else 
			state = INWORD;	
		
		if (state == INWORD){
			// if there were spaces - print then and reset spacecnt = 0
			if (spacecnt > 0){
				linecount += print_buf(spaces, spacecnt);
				spacecnt = 0;
			}
			putchar(c);	// c != '\n'
		} else 
			if (pos < maxlen){
				if (spacecnt < BUFSIZE - 1)
					spaces[spacecnt++] = c;	// add space symbol
				else {
					fprintf(stderr, "Out of buf %d\n", pos);
					return 2;
				}
			} else {// in this case state == OUTWORD 
				c = '\n';	// reset pos
				putchar(c);	// goto new line
				linecount++;
			}
		
		if (c == '\n'){
			if (spacecnt > 0){
				linecount += print_buf(spaces, spacecnt);
				spacecnt = 0;
			}
			pos = 0;		// newline, reset position
		}
		else 
			pos++;
	}
	return linecount;				
}

