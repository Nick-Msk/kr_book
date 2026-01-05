#include <stdio.h>
#include <strings.h>

#include "log.h"

char               *squeezestr(char * restrict t, const char * restrict pt);

int                 main(int argc, const char *argv[]){

    static const char *logfilename = "log/squeeze.2.4.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s squeeze string KR task 2.4\nUsage: %s <str1> <str2>\n", __FILE__, *argv);
            return 0;
        }
    }
    char        *str = strdup(argv[1]);
    const char  *pt = argv[2];

    printf("%s\n", squeezestr(str, pt));

    logclose("...");
    return 0;
}

char               *squeezestr(char * restrict str, const char * restrict pt){
    // TODO:
    return str;
}
