#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "common.h"
#include "checker.h"

static const char              *mystrcat(char * restrict t, const char * restrict s);

const char *usage_str = "Usage: %s <to:str> <from:str>\n";

int                             main(int argc, const char *argv[]){
    static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR mystrcat, task 5.3\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(3, usage_str, *argv) ){
        return 2;
    }

    // TODO: rework with fs!
    int     len = strlen(argv[1]) + strlen(argv[2]);
    char   *s = malloc(len + 1);
    if (!inv(s != 0, "Unable to allocate %d", len) )
        return logerr(1, " ");

    strcpy(s, argv[1]);
    printf("[%s]\n", mystrcat(s, argv[2]));

    free(s);
    logclose("...");
    return 0;
}

static const char              *mystrcat(char * restrict t, const char * restrict s){
    char    *res = t;
    while (*t != '\0')
        t++;
    while ( (*t++ = *s++) )
        ;
    return res;
}

