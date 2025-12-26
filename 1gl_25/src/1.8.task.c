#include <stdio.h>


int         main(void){
    int  space = 0, tab = 0, eol = 0;
    int c;
    while ( (c = getchar()) != EOF){
        switch(c){
            case ' ':
                space++;
            break;
            case '\t':
                tab++;
            break;
            case '\n':
                eol++;
            break;
        }
    }
    printf("Total: tabs=%4d, spaces=%6d, eols=%4d\n", tab, space, eol);
    return 0;
}
