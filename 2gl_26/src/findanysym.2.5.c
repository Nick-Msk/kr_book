#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "metric.h"
#include "log.h"
#include "check.h"
#include "common.h"

enum FindType {
    FIND_ONEBYONE       = 0,
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

// to common.c!!! TODO:
char                    *read_from_file(FILE *f, int cnt /* not used for now*/, int *p_cnt){
    logenter("read from input (%d)", cnt);
    int      sz = 1024, len, sum = 0;
    char    *s = 0;        // string to store
    s = malloc(sz);
    if (!s){
        fprintf(stderr, "Unable to acclocate %d\n", sz);
        return 0;
    }
    while ((len = fread(s, 1, sz - 1, f)) > 0){
        sum += len;
        if (len == sz - 1){
            s = realloc(s, sz *= 2);
            if (!s){
                fprintf(stderr, "Unable to acclocate %d\n", sz);
                free(s);
                return 0;
            }
        }
    }
    s[sum] = '\0';      // to make a normal c-string
    if (p_cnt)
        *p_cnt = sum;
    return logret(s, "%d bytes were read", sum);
}

int                       main(int argc, const char *argv[]){

    static const char *logfilename = "log/findanysym.2.5.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    enum FindType ty = FIND_ONEBYONE;
    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s any string KR task 2.5\nUsage: %s <pattern> <method F/S>\n", __FILE__, *argv); // Usage here??
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
    const char  *pt = strdup(argv[1]);
    Metric *m = 0;
    int cnt = 0;
    char *s = read_from_file(stdin, cnt, &cnt);

    if (!s){
        fprintf(stderr, "Unable to read from input source\n");
        return 2;
    }

    if (ty == FIND_ONEBYONE)
        m = metric_get("FIND_ONEBYONE_cnt", true);

    printf("Found pos %d\n", findanysym(s, pt, ty));

    metric_print(m);
    free(s);

    logclose("...");
    return 0;
}

static char                      *uniq_srt(char *s, int *p_len){
    logenter("str [%s]", s);
    bool    hash[256] = {false};
    int     j = 0;
    char    c;
    while ( (c = s[j]) != '\0'){
        if (!hash[(int) c]){
            hash[(int) c] = true;
            s[j++] = c;
        }
    }
    s[j] = '\0';
    if (p_len)
        *p_len = j;
    return logret(s, "new len = %d, new str[%s]", j, s);
}

static char                     *sort_str(char *s, int len){
    logenter("[%s]:%d", s, len);
    qsort(s, len, 1, char_cmp);
    return logret(s, "[%s]", s);
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

// just a launcher
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
