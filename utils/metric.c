#include <stdio.h>
#include <strings.h>

#include "log.h"
#include "metric.h"
#include "bool.h"

/********************************************************************
                 METRIC MODULE IMPLEMENTATION
********************************************************************/

// static globals

Metric                      g_metric_array[MAX_METRIC];
int                         g_metricfreepos = 0;

// internal type

// ---------- pseudo-header for utility procedures -----------------

static Metric *metric_cr(int pos, const char *name);
static Metric *metric_search(const char *name);

// ------------------------------ Utilities ------------------------

static Metric *metric_search(const char *name){
    logsimple("name [%s]", name);
    for (int i = 0; i < g_metricfreepos; i++)
        if (strcmp(g_metric_array[i].name, name) == 0)
            return g_metric_array + i;
    return 0;
}

static Metric *metric_cr(int pos, const char *name){
    logsimple("create on %d position with [%s]", pos, name);
    g_metric_array[pos].value = 0;
    strncpy(g_metric_array[pos].name, name, MAX_METRIC_NAME - 1);
    g_metric_array[pos].name[MAX_METRIC_NAME - 1] = '\0';
    return g_metric_array + pos;
}

// -------------------------- (Utility) printers -------------------

// --------------------------- API ---------------------------------

// -------------------- CONSTRUCTOTS/DESTRUCTORS -------------------

// get or create
Metric      *metric_get(const char *name, bool create){
    logenter("name=[%s], create - %s", name, bool_str(create));
    Metric *m = metric_search(name);
    if (m)
        return logret(m, "found");
    // not found
    if (create && g_metricfreepos < MAX_METRIC)
        m = metric_cr(g_metricfreepos++, name);
    return logret(m, "created? %p", m);
}

// -------------------------- (API) printers -----------------------

int                       metric_fprint(FILE *f, Metric *m){
    if (m)
        return fprintf(f, "\nMETRIC:\n[%s] = [%d]\n", m->name, m->value);
    else
        return 0;
}

// -------------------------------Testing --------------------------


// SKIP FOR NOW
#ifdef METRICTESTING

#include "testing.h"
#include <signal.h>

// ------------------------- TEST 1 ---------------------------------

// ------------------------- TEST 2 ---------------------------------

// -------------------------------------------------------------------
int
main(int argc, char *argv[])
{
    const char *logfilename = "checker.log";

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false, 0, "Starting");

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Simple invariant text"       , .desc = "", .mandatory=true)
      , testnew(.f2 = tf2, .num = 2, .name = "Complex invariant test"      , .desc = "", .mandatory=true)
      //, testnew(.f2 = f3, .num = 3, .name = "Interrupt raising test"        , .desc = "Exception test."                                                             , .mandatory=true)
      //, testnew(.f2 = f4, .num = 4, .name = "System error test."            , .desc = "System error raising test (w/o exception)."  , .mandatory=true)
    );

        logclose("end...");
    return 0;
}


#endif /* METRICTESTING */

