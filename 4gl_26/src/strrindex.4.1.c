#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "log.h"
#include "common.h"
#include "checker.h"

static int              strrindex(const char *restrict str, const char *restrict pt);

const char *usage_str = "Usage: %s <:str> <pattern:str>\n";

int                     main(int argc, const char *argv[]){

    static const char *logfilename = "log/strrindex.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR strrindex task 4.1\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(3, usage_str, *argv) ){
        return 1;
    }

    const char             *str     = argv[1];
    const char             *pt      = argv[2];
    int                     found   = 0;

    found = strrindex(str, pt);
    if (found < 0)
        printf("Not found\n");
    else
        printf("Pos from right %d\n", found);

    logclose("found %d", found);
    return 0;
}

static int              strrindex(const char *restrict str, const char *restrict pt){

    logenter("str [%s], pt [%s]", str, pt); 
    int     lenpt = strlen(pt);
    int     lenstr = strlen(str);
    int     i, j;

    for (i = lenstr - lenpt;  i >= 0; i--){
        for (j = 0; pt[j] != '\0' && pt[j] == str[i + j]; j++)
            ;
        if (j > 0 && pt[j] == '\0')
            return logret(i, "rfound %d", i);
    }
    return logret(-1, "Not found");
}


