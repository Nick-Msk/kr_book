#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>

#include "log.h"
#include "common.h"
#include "checker.h"

// recursive one
static char *           itoa_recur(int val, char *str);

const char *usage_str = "Usage: %s [val:str]\n";

int                     main(int argc, const char *argv[]){

    static const char *logfilename = "log/itoa_rec.4.12.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR itoa recursive task 4.12\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(2, usage_str, *argv) ){
        return 2;
    }
    int val = atoi(argv[1]);
    char buf[100] = "dtfghjjkhjkhjkjhjghfrtdesersdredsrd";

    *itoa_recur(val, buf) = '\0';

    printf("%s\n", buf);
    logclose("...");
    return 0;
}

static char *          itoa_recur(int val, char *str){
    logenter("[%d]", val);
    if (val < 0){
        *str++ = '-';
        val = -val;
    }
    if (val / 10)
        str = itoa_recur(val / 10, str);
    *str = itoc(val % 10);
    return logret(str + 1, "[%c]", *str);
}

