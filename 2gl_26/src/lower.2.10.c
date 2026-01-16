#include <stdio.h>

#include "log.h"
#include "common.h"
#include "checker.h"

static int              cust_lower(char c);


int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/lower.2.10.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s lower KR task 2.10 (input:  stdin)\nUsage: %s\n", __FILE__, *argv); // Usage here??
            return 0;
        }
    }

    int c;
    while ( (c = getchar() ) != EOF)
        putchar(cust_lower(c));

    logclose("...");
    return 0;
}

static int              cust_lower(char c){
    return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a': c;
}

