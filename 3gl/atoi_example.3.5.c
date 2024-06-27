#include <stdio.h>
#include <ctype.h>

int             myatoi(const char *s);

int             main(int argc, const char *argv[]){

    if (argc < 2){
        fprintf(stderr, "Usage: %s intval\n", *argv);
        return 1;
    }

    int     res = myatoi(argv[1]);

    printf("val = %d\n", res);

    return 0;
}


int             myatoi(const char *s){
    int     i, n, sign;

    for (i = 0; isspace(s[i]); i++)
        ;
    sign = (s[i] == '-') ? -1 : 1;
    if (s[i] == '+' || s[i] == '-')
        i++;
    for (n = 0; isdigit(s[i]); i++)
        n = 10 * n + (s[i] - '0');
    return n * sign;
}


