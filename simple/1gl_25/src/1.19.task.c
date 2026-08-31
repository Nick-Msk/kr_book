#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include "log.h"

static char                *reverse(char *s, int sz);

static void                 get_lines(int maxbuf);

int                 main(int argc, char *argv[]){
    int     maxline = 200;
    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0){
            printf("%s: Reserse of each input string. Usage %s <maxlen = 200>\n", __FILE__, argv[0]); 
            return 0;
        } else
            maxline = atoi(argv[1]);
    }

    get_lines(maxline);                                                                                     

    return 0;
}


static void                get_lines(int maxbuf){
    int c;
    char buf[maxbuf];
    int pos = 0;
    while ( (c = getchar()) != EOF){
        if (c == '\n'){
            buf[pos] = '\0';
            printf("%s\n", reverse(buf, pos));
            pos = 0;
        } else 
            if (pos < maxbuf - 1)
                buf[pos++] = c;
    }
}

static inline void          exch(char *a, char *b){
    char tmp = *a;
    *a = *b;
    *b = tmp;
}

static char                *reverse(char *s, int sz){
    for (int i = 0; i < sz/2; i++)
        exch(s + i, s + sz - i - 1);
    return s;
}
