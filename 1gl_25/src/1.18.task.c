#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "log.h"

void        space_remover(int);

int         main(int argc, const char *argv[]){
    
    int maxline = 400;
    if (argc > 1)
        maxline = atoi(argv[1]);

    space_remover(maxline);
    return 0;
}     

//                                                                  tytytyuty

void        space_remover(int maxline){     
    int         c;
    int         pos = 0, eolpos = 0;
    char        buf[maxline];
    while ( (c = getchar()) != EOF){
        if (c == '\n'){
            if (eolpos >= maxline - 2)
                eolpos = maxline - 2;           
            buf[eolpos] = '\0';
            if (eolpos > 0)
                printf("%s\n", buf);
            pos = eolpos = 0;
            continue;
        }
        else if (isspace(c))
            ;
        else    // letters, numbers
            eolpos = pos + 1;
        if (pos < maxline - 1) 
            buf[pos] = c;
        pos++;
    }
}

