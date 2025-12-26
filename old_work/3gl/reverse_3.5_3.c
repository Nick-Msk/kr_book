#include <stdio.h>
#include <string.h>

const char     *reverse(char *str);

int             main(int argc, char *argv[]){

    if (argc < 2){
        fprintf(stderr, "Usage: %s line\n", *argv);
        return -1;
    }
    char *str = argv[1];

    printf("%s\n", reverse(str));
    return 0;
}

const char     *reverse(char *str){
    int     i, j;
    char    c;
    for (i = 0, j = strlen(str) - 1; i < j; i++, j--){
        c = str[i];
        str[i] = str[j];
        str[j] = c;
    }
    return str;
}

