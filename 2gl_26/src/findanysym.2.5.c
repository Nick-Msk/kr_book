#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "metric.h"
#include "log.h"
#include "checker.h"
#include "common.h"

enum                            FindType {
    FIND_ONEBYONE       = 0,
    FIND_SORTED_PATTERN = 1     // ^
};

static inline const char        *getFindTypeName(enum FindType t){
    switch (t){
        CASE_RETURN(FIND_ONEBYONE);
        CASE_RETURN(FIND_SORTED_PATTERN);
        default: return "";
    }
}

static int                       print_str_n(const char *s, int n, int lastchar){
    int i;
    for (i = 0; i < n && s[i] != '\0'; i++)
        putchar(s[i]);
    if (lastchar != EOF)
        putchar(lastchar), i++;
    return i;
}

static int                      findanysym(const char *restrict str, char *restrict pt, enum FindType ty);

int                             main(int argc, const char *argv[]){

    static const char *logfilename = "log/findanysym.2.5.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    enum FindType ty = FIND_ONEBYONE;
    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s any string KR task 2.5 (input:  stdin)\nUsage: %s <pattern> <method F/S>\n", __FILE__, *argv); // Usage here??
            return 0;
        }
    }
    if (!check_arg(2, "Usage: %s <pattern> <method F/S>\n", *argv)){
        return 1;
        }

    if (toupper(*argv[2]) == 'F')
        ty = FIND_ONEBYONE;
    else if (toupper(*argv[2]) == 'S')
        ty = FIND_SORTED_PATTERN;
    else {
        fprintf(stderr, "Unsupported method %s\n", argv[2]);
        return 2;
    }
    char  *pt = strdup(argv[1]);
    // remove extra symbols
    uniq_str(pt, 0);
    Metric *m = 0;
    int cnt = 0;
    char *s = read_from_file(stdin, &cnt);

    if (!s){
        fprintf(stderr, "Unable to read from input source\n");
        return 2;
    }

    if (ty == FIND_ONEBYONE)
        m = metric_create("FIND_ONEBYONE_cnt");
    else
        m = metric_create("FIND_SORTED_PATTERN_cnt");

    cnt = findanysym(s, pt, ty);
    printf("Found pos %d\nPattern [%s] from\n", cnt, pt);
    //fputs(s, stdout);
    print_str_n(s, 100, '\n');    // TODO: is there any lib to do that?

    metric_print(m);
    free(s);

    logclose("...");
    return 0;
}

// TODO: think about complex testing
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

static int                       findanysym_bsearch(const char *restrict str, char *restrict pt){

    Metric *m = metric_get("FIND_SORTED_PATTERN_cnt", false);
    int len = strlen(pt);   // it can be optimized, but actually pattern isn't too big because of unique
    sort_str(pt, len);
    for (int i = 0; str[i] != '\0'; i++){
        metric_inc(m);  // +1
        if (bcharsearch(str[i], pt, len) != 0)
            return logsimpleret(i, "found %d", i);
    }

    return logsimpleret(-1, "Not found");
}

// just a launcher
static int                       findanysym(const char *restrict str, char *restrict pt, enum FindType ty){
    logenter("method %s, pattter [%s]", getFindTypeName(ty), pt);
    int  pos = -1;

    switch (ty){
        case FIND_ONEBYONE:
            pos = findanysym_onebyne(str, pt);
        break;
        case FIND_SORTED_PATTERN:
            pos = findanysym_bsearch(str, pt);
        break;
        default:
            fprintf(stderr, "Unknown method %c", ty);
    } 
    return logret(pos, "found %d", pos);
}

