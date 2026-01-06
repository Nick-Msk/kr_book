#include <stdio.h>
#include <strings.h>

#include "metric.h"
#include "log.h"
#include "check.h"
#include "common.h"

enum FindType {
    FIND_ONEBYONE = 0,
    FIND_SORTED_PATTERN = 1
};

static inline const char *getFindTypeName(enum FindType t){
    switch (t){
        CASE_RETURN(FIND_ONEBYONE);
        CASE_RETURN(FIND_SORTED_PATTERN);
        default: return 0;
    }
}

static int                       findanysym(const char *restrict str, const char *restrict pt, enum FindType ty);

int                       main(int argc, const char *argv[]){

    static const char *logfilename = "log/findanysym.2.5.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    enum FindType ty = FIND_ONEBYONE;
    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s any string KR task 2.5\nUsage: %s <str1> <str2>\n", __FILE__, *argv); // Usage here??
            return 0;
        }
    }
    if (!check_arg(2, "Usage: %s <str1> <str2>\n", *argv)){
        return 1;
        }
    char        *str = strdup(argv[1]);
    const char  *pt = argv[2];
    Metric *m = 0;
    
    if (ty == FIND_ONEBYONE)
        m = metric_get("FIND_ONEBYONE_cnt", true);

    printf("Found pos %d\n", findanysym(str, pt, ty));

    metric_print(m);

    logclose("...");
    return 0;
}

static int                       findanysym_onebyne(const char *restrict str, const char *restrict pt){

    Metric *m = metric_get("FIND_ONEBYONE_cnt", false); // if not found it's ok!
    for (int i = 0; str[i] != '\0'; i++)
        for (int j = 0; pt[j]; j++){   // stupid  search
            metric_inc(m);
            if (str[i] == pt[j])
                return logsimpleret(i, "found %d", i);
        }
    return -1;
}

static int                       findanysym(const char *restrict str, const char *restrict pt, enum FindType ty){
    logenter("method %s, pattter [%s]", getFindTypeName(ty), pt);
    int  pos = -1;

    switch (ty){
        case FIND_ONEBYONE:
            pos = findanysym_onebyne(str, pt);
        break;
        case FIND_SORTED_PATTERN:
            printf("TODO! %s", getFindTypeName(ty));
        break;
        default:
            fprintf(stderr, "Unknown method %c", ty);
    } 
    return logret(pos, "found %d", pos);
}
