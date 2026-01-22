#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>

#include "log.h"
#include "common.h"
#include "checker.h"

static double           myatof(const char *str);

const char *usage_str = "Usage: %s [val:str]\n";

int                     main(int argc, const char *argv[]){

    static const char *logfilename = "log/atof.4.2.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR atof task 4.2\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (argc > 1){
        const char  *str     = argv[1];
        double res = myatof(str);
        printf("Res %f\t%8.8g\t%e\n", res, res, res);
    } else {
        double                  sum = 0.0;
        char                    buf[1000];

        while (get_line(buf, sizeof(buf)) > 0){
            sum += myatof(buf);
            printf("\t%f\n", sum);
        }
    }
    logclose("...");
    return 0;
}

static double           myatof(const char *str){
    double  val = 0.0, power = 1.0;
    int     sign;

    while (isspace(*str))
        str++;
    sign = *str == '-' ? -1: 1;
    if (*str == '+' || *str == '-')
        str++;
    while ( isdigit(*str) ){
        val = val * 10.0 + ctoi(*str++);
    }
    if (*str == '.')
        str++;
    while (isdigit(*str) ){
        val = 10.0 * val + ctoi(*str++);
        power *= 10;
    }
    return sign * val / power;
}

