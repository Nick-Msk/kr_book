#include <stdio.h>

int             main(void){

    int     c, nwhite = 0, nother = 0, ndigit[10];
    for (int i = 0; i < sizeof(ndigit) / sizeof(int); i++)
        ndigit[i] = 0;

    while ((c = getchar()) != EOF){
        switch (c){
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                ndigit[c - '0']++;
            break;
            case  ' ': case '\t': case '\n':
                nwhite++;
            break;
            default:
                nother++;
            break;
        }
    }
    printf("Digits = ");
    for (int i = 0; i < sizeof(ndigit) / sizeof(int); i++)
        printf(" %d", ndigit[i]);
    printf(", white space = %d, others = %d\n", 
        nwhite, nother);

    return 0;
}
