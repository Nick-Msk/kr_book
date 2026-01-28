#include "var.h"
#include "bool.h"
#include "common.h"

/********************************************************************
                 VAR MODULE IMPLEMENTATION
********************************************************************/

// static globals

static double           vars[27];  // 26 + last result var :?

// internal type

// ---------- pseudo-header for utility procedures -----------------

// ------------------------- CONSTRUCTOTS/DESTRUCTORS --------------

void                    var_clear(void){
    cleaner_double(vars, COUNT(vars));
}

// ------------------------------ Utilities ------------------------

static int              getpos(char c){
    int     pos = -1;
    if (c == '?')
        pos =  0;
    else if (c >= 'a' && c <= 'z')
        pos =  c - 'a' + 1;
    return pos;
}


// -------------------------- (Utility) printers -------------------

// --------------------------- API ---------------------------------

double                  var_get(char c){
    int     pos = getpos(c);
    if (pos < 0)
        fprintf(stderr, "%s: Var not exists [%c]\n", __func__, c);
    return vars[pos];
}

double                  var_set(char c, double val){
    int     pos = getpos(c);
    if (pos < 0)
        fprintf(stderr, "%s: Var not exists [%c]\n", __func__, c);
    return vars[pos] = val;
}

// -------------------------- (API) printers -----------------------

int                     var_fprint(FILE *f){
    int cnt = 0;
    cnt = fprintf(f, "VARS:\n");
    for (int i = 0; i < COUNT(vars); i++){
        cnt += fprintf(f, "%d: \t%.8g ", i, vars[i]);
        if ( (i + 1) % 10 == 0)
             cnt += fprintf(f, "\n");
    }
    cnt += fprintf(f, "\n-------------------------------------\n");
    return cnt;
}

// -------------------------------Testing --------------------------

#ifdef VARTESTING

#include <signal.h>
#include "test.h"
#include "array.h"
#include "checker.h"

//types for testing

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);
        double  some_val = 444.333, res;
        char    var = 'd';
        var_set(var, some_val);
        res = var_get(var);
        if (res != some_val)
            return logerr(TEST_FAILED, "Variable %c =  %f but is must be %f", var, res, some_val);

        test_sub("subtest %d", ++subnum);
        var_clear();
        some_val = 0.0; // etalon
        res = var_get(var);
        if (res != some_val)
            return logerr(TEST_FAILED, "Variable %c =  %f but is must be %f", var, res, some_val);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf2(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);
        for (int i = 1; i < COUNT(vars); i++)
           var_set('a' + i - 1, (double) i);
        for (int i = 1; i < COUNT(vars); i++)
            if (vars[i] != (double) i)
                return logerr(TEST_FAILED, "Variable position %d =  %f but is must be %f", i, vars[i], (double) i);
        //test_sub("subtest %d", ++subnum);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// -------------------------------------------------------------------

int
main(int argc, char *argv[])
{
    const char *logfilename = "log/stack.log";

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false, 0, "Starting");

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Set/get test"              , .desc = "", .mandatory=true)
      , testnew(.f2 = tf2, .num = 2, .name = "Multiple set/get test"     , .desc = "", .mandatory=true)
    );

    logclose("end...");
    return 0;
}


#endif /* VARTESTING */

