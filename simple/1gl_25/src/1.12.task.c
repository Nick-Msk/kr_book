#include <stdio.h>
#include <ctype.h>
#include "bool.h"

int     main(void){
    int c;
    bool space = true;
    while ( (c = getchar()) != EOF){
        if ( (c == '\n' ||  isspace(c)) && !space){
            putchar('\n');
            space = true;
        }
        else if (isalnum(c)){
            putchar(c);
            space = false;
        } 
    }
    return 0;
}
