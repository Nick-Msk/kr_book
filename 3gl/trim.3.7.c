#include <stdio.h>
#include <string.h>

// to common.h
// assuming argc as checked value!!!
#define         init_checker(retval, argcnt, msg, ...)\
    if (argc < (argcnt)){\
        fprintf(stderr, (msg), ##__VA_ARGS__);\
        return ((retval));\
    }

int             trim(char *s);

int             main(int argc, char *argv[]){

    init_checker(1, 2, "Usage: %s 'string'\n", *argv);
    char        *s = argv[1];
    int          len = trim(s);
    printf("Trimmed to [%s] with %d\n",
            s, len);

    return 0;
}

int             trim(char *s){
    int     n;
    for (n = strlen(s) - 1; n >= 0; n--)
        if (s[n] != ' '&& s[n] != '\t' && s[n] != '\n')
            break;
    s[n + 1] = '\0';
    return n + 1;
}


