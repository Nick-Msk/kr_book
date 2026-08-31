#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

static int          detab(int tabsize);

int                 main(int argc, const char *argv[]){
    int     tabsize = 8;    // default
    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s detab task 1.20 KR\nUsage: %s <tabsize=8>\n", __FILE__, *argv);
            return 0;   
        }
        tabsize = atoi(argv[1]);
    }
                    
    int tabcnt = detab(tabsize);
    printf("%d tabs were processed\n", tabcnt);
    return 0;
}

static inline void	printspace(int shift){
   	while (shift-- > 0)
	  	putchar(' ');
}

static int          detab(int tabsize){
    int tabcnt = 0, c, pos = 0;

    while ( (c = getchar()) != EOF){
        if  (c == '\t'){
			int shift = tabsize - (pos % tabsize); 
            printspace(shift);
            tabcnt++;
        }
        else { 
            if (c == '\n')
                pos = 0;
            else 
                pos++;
            putchar(c);
        }
    }
    return tabcnt;
}
