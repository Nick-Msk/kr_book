#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "common.h"
#include "checker.h"

static int                  mystrend(const char *restrict t, const char *restrict s);

const char *usage_str = "Usage: %s <str:str> <pt:str>\n";

int                             main(int argc, const char *argv[]){
    static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR mystrend, task 5.\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(3, usage_str, *argv) ){
        return 2;
    }

    // TODO: rework with fs!
    int     pos = mystrend(argv[1], argv[2]);
    if (pos >= 0)
        printf("Found on %d\n", pos);
    else
        printf("Not found\n");

    logclose("...");
    return 0;
}

static int                  mystrend(const char *restrict t, const char *restrict s){
    int           lent = 0, lens = 0;

    while (t[lent++])
        ;
    while (s[lens++])
        ;
    if (lens > lent)
        return -1;
    logsimple("t %d s %d", lent, lens);

    // just strcmp from t+(lent-lens) and s
    t += lent - lens;
    while (*t == *s && *t != '\0'){
        t++, s++;
    }
    logsimple("final [%c]", *t);
    if (*t == '\0') // end of line
        return lent - lens;
    else
        return -1;
}

