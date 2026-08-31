#include <stdlib.h>
#include <stdio.h>
#include "fileopers.h"
#include "common.h"

const int              MAXLEN = 8192;

// --------------------------- API ---------------------------------

int             readlines(char *ptr[], int maxline){
    int     nlines = 0, len;
    char   *p, line[MAXLEN];

    while ( (len = get_line(line, maxline) ) > 0)
        if (nlines >= maxline){
            fprintf(stderr, "maxline (%d) were execced\n", maxline);
            break;
        } else if ( (p = malloc(len + 1)) == 0){
            fprintf(stderr, "Unable to allocated %d\n", len + 1);
            break;
        } else {
            strcpy(p, line);
            ptr[nlines++] = p;
        }
    return nlines;
}

int             writelines(const char *ptr[], int maxline){
    int     cnt = 0;
    while (maxline-- > 0)
        cnt += printf("%s", *ptr++);
    return cnt;
}

void             freelines(const char *ptr[], int maxline){
    while (--maxline > 0)
        free( (void *) *ptr++);
}

