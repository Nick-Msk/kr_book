#include "stack.h"
#include "bool.h"

/********************************************************************
                 STACK MODULE IMPLEMENTATION
********************************************************************/

// static globals

static const int                STACK_MAXVAL = 100; // TODO: to be replaced
static int                      sp = 0;
static double                   st[STACK_MAXVAL];   // to be removed to normal stack

// internal type

// ---------- pseudo-header for utility procedures -----------------

// ------------------------- CONSTRUCTOTS/DESTRUCTORS --------------

void                    stack_clear(void){
    sp = 0;
}

// ------------------------------ Utilities ------------------------

double                  stack_pop(void){
    double val = 0.0;
    if (sp > 0)
        val = st[--sp];
    else
        fprintf(stderr, "Error: stack is empty\n");// TODO: userraisesig here!
    return val;
}

bool                    stack_push(double val){
    bool ret = true;

    if (sp < STACK_MAXVAL)
        st[sp++] = val;
    else {
        fprintf(stderr, "Stack is full (%d)\n", sp);
        ret = false;
    }
    return ret;
}

int                     stack_count(void){
    return sp;
}

// -------------------------- (Utility) printers -------------------

// --------------------------- API ---------------------------------

// -------------------------- (API) printers -----------------------

int                     stack_fprint(FILE *f){
    int cnt = 0;
    cnt = fprintf(f, "STACK:\n");
    for (int i = 0; i < sp; i++){
        cnt += fprintf(f, "\t%.8g ", st[i]);
        if ( (i + 1) % 10 == 0)
             cnt += fprintf(f, "\n");
    }
    return cnt;
}

// -------------------------------Testing --------------------------

#ifdef STACKTESTING

#include <signal.h>
#include "test.h"
#include "array.h"

//types for testing

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(void)
{
    logenter("Simple Push/pop test");

    double some_val = 444.333, res;
    stack_push(some_val);
    res = stack_pop();
    if (res != some_val){
        return logerr(TEST_FAILED, "Pop return %f but is must be %f", res, some_val);
    }
    stack_clear();
    if (sp != 0)
        return logret(TEST_FAILED, "Stack must be empty, but sp = %d", sp);
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf2(void)
{
    logenter("Multiple push/pop test");

    DArray arr = DArray_create(STACK_MAXVAL, ARRAY_RND);
    double res, *some_vals = arr.v;
    for (int i = 0; i < STACK_MAXVAL; i++)
        stack_push(some_vals[i]);
    for (int i = STACK_MAXVAL - 1; i >=0; i--){
        res = stack_pop();
        if (res != some_vals[i]){
            return logerr(TEST_FAILED, "i = %d, Pop return %f but is must be %f", 
                    i, res, some_vals[i]);
        }
    }
    stack_clear();
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}


// ------------------------- TEST 3 ---------------------------------

static TestStatus
tf3(void)
{
    logenter("Empty stack test");

    double res = stack_pop();
    if (res != 0.0){
        return logerr(TEST_FAILED, "Pop return %f but is must be %f", res, 0.0);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

int
main(int argc, char *argv[])
{
    const char *logfilename = "log/stack.log";

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false, 0, "Starting");

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Simple Push/pop test"       , .desc = "", .mandatory=true)
      , testnew(.f2 = tf2, .num = 2, .name = "Multiple push/pop test"     , .desc = "", .mandatory=true)
      , testnew(.f2 = tf3, .num = 3, .name = "Empty stack test"           , .desc = "", .mandatory=true)
    );

    logclose("end...");
    return 0;
}


#endif /* SKELETONTESTING */

