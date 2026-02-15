#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

static inline bool      isdoc(char c){ 
    return c == '\0' || c == ' ' || isalnum(c);
}

static int              strcmpasdoc(const char *s1, const char *s2, bool sens){
    //const char *p1 = s1, *p2 = s2;
    char    c1, c2;
    while ( (c1 = *s1) != '\0' && (c2 = *s2) != '\0' ){
        //printf("1: [%c] - [%c], %zu - %zu\t", c1, c2, s1 - p1, s2 - p2);
        while (!isdoc(c1) )
            c1 = *++s1;
        while (!isdoc(c2) ){
            c2 = *++s2;
        }
        //printf("2: [%c] - [%c] %zu - %zu\n", c1, c2, s1 - p1, s2 - p2);
        if (sens ? c1 == c2: tolower(c1) == tolower(c2) && c1 != '\0'){
            s1++, s2++;
            //printf("3: [%c] - [%c], %zu - %zu\t", c1, c2, s1 - p1, s2 - p2);
        }
        else {
            //printf("4: %d \n", c1 - c2);
            return sens ? c1 - c2 : tolower(c1) - tolower(c2);
        }
    }
    return 0;
}

int         main(int argc, const char *argv[]){
    if (argc < 3){
        fprintf(stderr, "Usage: %s str1 str2\n", *argv);
        return 1;
    }
    int cmp = strcmpasdoc(argv[1], argv[2], true);
    if (cmp > 0)
        printf("st1 > str2\n");
    else if (cmp < 0)
        printf("st1 < str2\n");
    else
        printf("st1 = str2\n");
    return 0;
}

