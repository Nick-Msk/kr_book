#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <strings.h>

#include "log.h"
#include "common.h"
#include "checker.h"

// internal proc
static const char             *itoa(int n, char *s);

const char *usage_str = "Usage: %s <val:int>\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/itoa.3.4.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR itoa 3.4\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(2, usage_str, *argv) ){
        return 1;
    }
    int     n;
    char    buf[100];

    if (strcmp(argv[1], "INT_MIN") == 0)
        n = INT_MIN;
    else if (strcmp(argv[1], "INT_MAX") == 0)
        n = INT_MAX;
    else
        n = atoi(argv[1]);

    printf("INT: %s\n", itoa(n, buf) );

    logclose("...");
    return 0;
}

static const char             *itoa(int n, char *s){
    int     i = 0, sign;
    sign = n;
      //  n = n; // TODO:
    do {
        logsimple("n %% 10 = %d, n / 10 = %d", n % 10, n / 10);
        s[i++] = itoc(abs(n % 10));;
    } while ( abs( (n /= 10) ) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s, i);
    return s;
}

