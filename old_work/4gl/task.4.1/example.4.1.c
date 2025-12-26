#include <stdio.h>

#define             MAXLINE     1000

// checker.h  -- new version of init_check
#define             arg_check(argcmax, msg, ...)\
    ( argc < (argcmax) ? fprintf(stderr, msg, ##__VA_ARGS__), 0 : 1)

int                 get_line(char *line, int max);

int                 mystrindex(const char *str, const char *pattern);

int                 main(int argc, const char *argv[]){
    // new version of init_check
    if (!arg_check(2, "Usage: %s pattern\n", *argv))
        return 1;

    int             found = 0, total = 0;
    const char      *pattern = argv[1];
    char            line[MAXLINE];

    while (get_line(line, MAXLINE) > 0){
        total++;
        if (mystrindex(line, pattern) >= 0){
            printf("%s", line); // w/o \n
            found++;
        }
    }
    printf("Total found %d/%d\n", found, total);
    return 0;
}

int                 mystrindex(const char *str, const char *pattern){
    for (int i = 0; str[i] != '\0'; i++){
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
