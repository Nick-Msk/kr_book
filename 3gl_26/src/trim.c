#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <strings.h>

#include "log.h"
#include "common.h"
#include "checker.h"

// internal proc
static const char             *trim(char *s);

const char *usage_str = "Usage: %s <val:'str'>\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/trim.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR trim p.78\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(2, usage_str, *argv) ){
        return 1;
    }

    char *s = strdup(argv[1]);
    if (!inv(s != 0, "Unable to duplicate") )
        return 2;

    printf("[%s]\n", trim(s));

    logclose("...");
    return 0;
}

static const char             *trim(char *s){
   int  n;
   for (n = strlen(s) - 1;n >= 0; n--)
    if (!isspace(s[n] ) )
        break;
    s[n] = '\0';
    logsimple("truncated to %d", n);
    return s; 
}
