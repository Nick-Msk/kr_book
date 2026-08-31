#include <stdio.h>
#include <ctype.h>

#include "bool.h"

int         main(int argc, const char *argv[]){
    printf("%s\n", argv[0]);
    bool    upper = false;
    if (isalpha(*argv[0]) ){
        upper = isupper(*argv[0]) ? true : false;
        int     c;
        while ( (c = getchar() ) != EOF)
            putchar(upper ? toupper(c) : tolower(c) );
    } else
        printf("1-st letter not a letter\n");
    return 0;
}

