#include <stdio.h>

char        getlower(char c);

int         main(int argc, const char *argv[]){

    if (argc < 2){
        fprintf(stderr, "Usage: %s line\n", *argv);
        return 1;
    }

    const char *s = argv[1];
    while (*s)
        putchar(getlower(*s++));
    putchar('\n');
    return 0;
}

char        getlower(char c){
    return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
}


