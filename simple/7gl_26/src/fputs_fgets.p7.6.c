#include <stdio.h>
#include <string.h>

#include "log.h"
#include "common.h"
#include "error.h"

const char *usage_str = "Usage: %s <val1:TYPE1> <val2:TYPE2>\n";

static char            *myfgets(char *restrict s, int n, FILE *restrict in);
static int              myfputs(const  char *restrict s, FILE *restrict out);

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    FILE    *fp;
    const char *prog = *argv;

    

    if (ferror(stdout))
        userraiseint(ERR_STREAM_ERROR, "%s: error while writing to stdout\n", prog);
    return logret(0, "end...");  // as replace of logclose()
}

static char            *myfgets(char *restrict s, int n, FILE *restrict in){
    int     c;
    char   *cs = s;
    while (--n > 0 && (c = getc(in) ) != EOF)
        if ( (*cs++ = c) == '\n')
            break;
    *cs = '\0';
    return (c == EOF && s == cs) ? 0 : s;
}

static int              myfputs(const  char *restrict s, FILE *restrict out){
    int     c;
    while ( (c = *s) != '\0')
        putc(c, out);
    return ferror(out) ? EOF : 0;
}


