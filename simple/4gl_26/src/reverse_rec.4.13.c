#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>

#include "log.h"
#include "common.h"
#include "checker.h"

// recursive one
static char *           reverse_rec(char *str, int len);

const char *usage_str = "Usage: %s [val:str]\n";

int                     main(int argc, const char *argv[]){

    static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR reverse recursive task 4.13\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(2, usage_str, *argv) ){
        return 2;
    }
    char *buf  = strdup(argv[1]);

    printf("%s\n", reverse_rec(buf, strlen(buf)));

    free(buf);
    logclose("...");
    return 0;
}

static char *           reverse_rec(char *str, int len){
    if (len <= 1)
        return str;
    char_exch(str, str + len - 1);
    reverse(str + 1, len - 2);
    return str;
}

