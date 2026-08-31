#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "bool.h"

static const int        MAXLINE = 10;

static int      get_line(char s[], int lim);
static int      copy(char to[], const char from[]);

int             main(int argc, const char *argv[]){
    static const char *logfilename = "log/1.16.task.log";
    loginit(logfilename, false, 0, "Start");
    int     len, max = 0;
    char    line[MAXLINE];
    char    longest[MAXLINE];

    while ( (len = get_line(line, MAXLINE)) > 0){
        //logmsg("len %d, max %d", len, max);
        if (len > max){
            max = len;
            copy(longest, line);
        }
    }
    if (max > 0)
        printf("Max: %s", longest);
    logclose("...");
    return 0;
}

// return a real string length
static int      get_line(char s[], int lim){
    int c, i = 0, j;
    for (j = 0; (c = getchar()) != EOF && c != '\n'; j++){
        //logsimple("i=%d, j=%d, c=%c", i, j, c);
        if (i < lim - 2)
            s[i++] = c;
    }
    if (c == '\n'){
        s[i++] = c;
        j++;
    }
    s[i] = '\0';
    return logsimpleret(j, "ret %d, i = %d [%s]", j, i, s);
}

static int      copy(char to[], const char from[]){
    int i = 0;
    while ((to[i] = from[i]) != '\0')
        i++;
    return i;
}
