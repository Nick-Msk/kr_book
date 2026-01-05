#include <stdio.h>
#include <strings.h>

#include "log.h"
#include "check.h"

// to metric.h
static const int            MAX_METRIC_NAME = 64;
static const int            MAX_METRIC = 100;

typedef struct {
    char    name[MAX_METRIC_NAME];
    union {
        int     value;
        double  dvalue;
    };
} Metric;

//extern int                  g_metricfreepos;
//extern Metric               g_metric_array[MAX_METRIC];

// metric.c
Metric                      g_metric_array[MAX_METRIC];
int                         g_metricfreepos = 0;


// metric.c
// internal
static Metric *metric_create(int pos, const char *name){
    logsimple("create on %d position with [%s]", pos, name);
    g_metric_array[pos].value = 0;
    strncpy(g_metric_array[pos].name, name, MAX_METRIC_NAME - 1);
    g_metric_array[pos].name[MAX_METRIC_NAME - 1] = '\0';
    return g_metric_array + pos;
}

// internal
static Metric *metric_search(const char *name){
    for (int i = 0; i < g_metricfreepos; i++)
        if (strcmp(g_metric_array[i].name, name) == 0)
            return g_metric_array + i;
    return 0;
}

// get or create
Metric      *metric_get(const char *name, bool create){
    logenter("name=[%s], create - %s", name, bool_str(create));
    Metric *m = metric_search(name);
    if (m)
        return logret(m, "found");
    // not found
    if (create && g_metricfreepos < MAX_METRIC)
        m = metric_create(g_metricfreepos++, name);
    return logret(m, "created? %p", m);
}

bool        metric_setval(Metric *m, int val){
    if (m){
        m->value = val;
        return true;
    } else
        return false;
}

int                       metric_fprint(FILE *f, Metric *m){
    if (m)
        return fprintf(f, "[%s] = [%d]\n", m->name, m->value);
    else
        return 0;
}

static inline Metric    *metric_inc(Metric *m){
    if (m)
        m->value++;
    return m;
}


static inline Metric    *metric_add(Metric *m, int val){
    if (m)
        m->value += val;
    return m;
}

static inline Metric    *metric_reset(Metric *m){
    if (m)
        m->value = 0;
    return m;
}

static inline int         metric_print(Metric *m){
    return metric_fprint(stdout, m);
}

static char               *squeezestr(char * restrict t, const char * restrict pt);

int                       main(int argc, const char *argv[]){

    static const char *logfilename = "log/squeeze.2.4.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s squeeze string KR task 2.4\nUsage: %s <str1> <str2>\n", __FILE__, *argv); // Usage here??
            return 0;
        }
    }
    if (!check_arg(2, "Usage: %s <str1> <str2>\n", *argv)){
    	return 1;
	}
    char        *str = strdup(argv[1]);
    const char  *pt = argv[2];
    Metric *m = metric_get("squeeze_cnt", true);

    printf("%s\n", squeezestr(str, pt));

    metric_print(m);

    logclose("...");
    return 0;
}


static char               *squeezechar(char *str, char c){
    int i, j;
    Metric *m = metric_get("squeeze_cnt", false);
    for (i = j = 0; str[i] !=0; i++){
        if (str[i] != c){    // TODO: add compl_alg here!
            str[j++] = str[i];
        }
        metric_inc(m);
    }
    str[j] = '\0';
    return str;
}

static char               *squeezestr(char *restrict str, const char *restrict pt){
    // very simple alg! TODO: try Virt alg for compare
    for (int k = 0; pt[k] != 0; k++){
        char c = pt[k];
        squeezechar(str, c);
    }
    return str;
}

