#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "log.h"
#include "common.h"
#include "checker.h"

const char *usage_str = "Usage: %s <val1:TYPE1> <val2:TYPE2>\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/skeleton.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s EXAMPLE SKELETON\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(3, usage_str, *argv) ){
        return 1;
    }

    // TODO: code from here! 

    logclose("...");
    return 0;
}

