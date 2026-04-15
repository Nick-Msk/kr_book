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
    // TODO:
}

static int              myfputs(const  char *restrict s, FILE *restrict out){
    // TODO:
}
