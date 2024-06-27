#include <stdio.h>
#include <string.h>

#define             MAXLINE     1000

// checker.h  -- new version of init_check
#define             arg_check(argcmax, msg, ...)\
    ( argc < (argcmax) ? fprintf(stderr, msg, ##__VA_ARGS__), 0 : 1)

int                 get_line(char *line, int max);

int                 mystrrindex(const char *str, const char *pattern);

int                 main(int argc, const char *argv[]){
    // new version of init_check
    if (!arg_check(2, "Usage: %s rpattern\n", *argv))
        return 1;

    const char      *pattern = argv[1];
    char            line[MAXLINE];

    while (get_line(line, MAXLINE) > 0){
        int  pos;
        if ((pos = mystrrindex(line, pattern)) >= 0){
            printf("RIGHT POS %d: %s", pos, line); // w/o \n
        }
    }
    return 0;
}

int                 mystrrindex(const char *str, const char *pattern){
    for (int i = strlen(str) - 1; i>= 0; i--){
        int  k = 0;
        for (int j = i; pattern[k] != '\0' && str[j] == pattern[k]; j++, k++)
            ;
        if (k > 0 && pattern[k] == '\0')
            return i;
    }
    return -1;
}

int                 get_line(char *line, int max){
    int     i = 0;
    char    c = '\0';

    while (--max > 0 && (c = getchar()) != EOF && c != '\n')
        line[i++] = c;
    if (c == '\n')
        line[i++] = '\n';
    line[i] = '\0';
    return i;
}
