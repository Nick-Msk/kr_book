#include <stdio.h>
#include <stdlib.h>

static const int        MAXLINE = 10000;

static int      get_line(char s[], int lim);
static int      copy(char to[], const char from[]);

int             main(int argc, const char *argv[]){
    int     len, max = 0;
    char    line[MAXLINE];
    char    longest[MAXLINE];

    while ( (len = get_line(line, MAXLINE)) > 0){
        if (len > max){
            max = len;
            copy(longest, line);
        }
    }
    if (max > 0)
        printf("%s", longest);

    return 0;
}

static int      get_line(char s[], int lim){
    int c, i;
    for (i = 0; i < lim - 1 && (c = getchar()) != EOF && c != '\n'; i++)
        s[i] = c;
    if (c == '\n'){
        s[i++] = c;
    }
    s[i] = '\0';
    return i;
}

static int      copy(char to[], const char from[]){
    int i = 0;
    while ((to[i] = from[i]) != '\0')
        i++;
    return i;
}
