#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "log.h"
#include "common.h"
#include "checker.h"

// internal proc
static int             expand(char *restrict t, const char *restrict s, int sz);

const char *usage_str = "Usage: %s <str_to_expand\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/expand.3..log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR expand task 3.3\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(2, usage_str, *argv) ){
        return 1;
    }
    const char  *s = argv[1];
    int         sz = strlen(s) * 10 + 1;
    char        *t = malloc(sz);

    if (!inv(t != 0, "Unable to allocatd %d", sz) )
        return 2;

    expand(t, s, sz - 1);
    free(t);

    logclose("...");
    return 0;
}

static int             expand(char *restrict t, const char *restrict s, int sz){
    logenter("[%s] - [%d]", s, sz);
    int         pos = 0;
    /// TODO:

    return logret(pos, "expanded to %d  chars", pos);
}
