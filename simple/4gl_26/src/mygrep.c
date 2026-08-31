#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "log.h"
#include "common.h"
#include "checker.h"


static int              mygetline(char *s, int max);

static int              strindex(const char *restrict str, const char *restrict pt);

const char *usage_str = "Usage: %s <pattert:str>\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/mygrep.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR grep p.83\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(2, usage_str, *argv) ){
        return 1;
    }

    static const int        MaxLine = 1000;
    char                    buf[MaxLine];
    const char             *pt = argv[1];
    int                     found = 0;

    while (mygetline(buf, MaxLine) > 0)
        if (strindex(buf, pt) >= 0){
            printf("%s\n", buf);
            found++;
        }
    printf("\nTotal %d\n", found);
    logclose("found %d", found);
    return 0;
}

static int              mygetline(char *s, int max){
    logenter("max %d", max);
    int     c, i = 0;

    while (--max > 0 && ( c = getchar()) != EOF && c != '\n')
        s[i++] = c;

    if (c == '\n')
        s[i++] = c;
    s[i] = '\0';

    return logret(i, "%d chars", i);
}

static int              strindex(const char *restrict str, const char *restrict pt){
    logenter("str [%s] pt [%s]", str, pt);
    int     i, j, k;
    for (i = 0; str[i] != '\0'; i++){
        for (j = i, k = 0; pt[k] != '\0' && str[j] == pt[k]; j++, k++)
            ;
        logmsg("i = %d, j = %d, k = %d", i, j, k);
        if (k > 0 && pt[k] == '\0')
            return logret(k, "pos %d", k);
    }
    return logret(-1, "Not found");
}


