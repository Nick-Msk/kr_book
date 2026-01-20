#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <strings.h>

#include "log.h"
#include "common.h"
#include "checker.h"

// internal proc
static const char             *itob(int n, char *s, int base);

const char *usage_str = "Usage: %s <val:int> <base:int 2-16>\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/itob.3.5.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR itoa 3.4\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(3, usage_str, *argv) ){
        return 1;
    }
    int     n;
    char    buf[100];
    int     base = 10;

    if (strcmp(argv[1], "INT_MIN") == 0)
        n = INT_MIN;
    else if (strcmp(argv[1], "INT_MAX") == 0)
        n = INT_MAX;
    else
        n = atoi(argv[1]);

    base = atoi(argv[2]);

    if (!inv(base >= 2 && base <= 16, "base should be between 2 and 16") )
        return 2;

    printf("base %d: %s\n", base, itob(n, buf, base) );

    logclose("...");
    return 0;
}

static const char             *itob(int n, char *s, int base){
    int     i = 0, sign;
    sign = n;
    do {
        //logsimple("n %% 10 = %d, n / 10 = %d", n % 10, n / 10);
        s[i++] = itoc(abs(n % base));;
    } while ( abs( (n /= base) ) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s, i);
    return s;
}

