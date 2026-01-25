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

double                  stack_get(void){
    double val = 0.0;
    if (sp > 0)
        val = st[sp - 1];
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

double                  stack_exch(void){
    double val2 = 0.0;
    if (sp > 1){
        double val1 = stack_pop();
        val2 = stack_pop();
        stack_push(val1);
        stack_push(val2);
    } else
        fprintf(stderr, "Only %d elements in the stack\n", sp);
    return val2;
}

int                     stack_count(void){
    return sp - 1;
}

// -------------------------- (Utility) printers -------------------

// --------------------------- API ---------------------------------

// -------------------------- (API) printers -----------------------

int                     stack_fprint(FILE *f){
    int cnt = 0;
    cnt = fprintf(f, "STACK:\n");
    for (int i = 0; i < sp; i++){
        cnt += fprintf(f, "%d: \t%.8g ", i, st[i]);
        if ( (i + 1) % 10 == 0)
             cnt += fprintf(f, "\n");
    }
    cnt += fprintf(f, "\n-------------------------------------\n");
    return cnt;
}

// -------------------------------Testing --------------------------

#ifdef STACKTESTING

#include <signal.h>
#include "test.h"
#include "array.h"
#include "checker.h"

//types for testing

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(void)
{
    logenter("Simple Push/pop test");
    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);
        double some_val = 444.333, res;
        stack_push(some_val);
        res = stack_pop();
        if (res != some_val)
            return logerr(TEST_FAILED, "Pop return %f but is must be %f", res, some_val);
    }
    {
        test_sub("subtest %d", ++subnum);
        stack_clear();
        if (sp != 0)
            return logret(TEST_FAILED, "Stack must be empty, but sp = %d", sp);
    }
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
            return logacterr(DArray_free(&arr), TEST_FAILED, "i = %d, Pop return %f but is must be %f", 
                    i, res, some_vals[i]);
        }
    }
    DArray_free(&arr);
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

// ------------------------- TEST 4 ---------------------------------

static TestStatus
tf4(void)
{
    logenter("Get without pop() test");
    int     subnum = 0;

    {
        test_sub("subtest %d", ++subnum);
        double  val = 1.245678;      // anything
        stack_push(val);
        int     pos = sp;               // check stack pointer too
        double  res = stack_get();
        if (res != val)
            return logerr(TEST_FAILED, "Pop return %f but is must be %f", res, val);
        if (pos != sp)
             return logerr(TEST_FAILED, "Stack position became %d but is must be %d", sp, pos);
    }
    //
    {
        test_sub("subtest %d", ++subnum);
        stack_clear();
        double  val = 0.0;
        int     pos = sp;
        double  res = stack_get();
        if (res != val)
            return logerr(TEST_FAILED, "Pop return %f but is must be %f", res, val);
        if (pos != sp)
             return logerr(TEST_FAILED, "Stack position became %d but is must be %d", sp, pos);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 5 ---------------------------------

static TestStatus
tf5(void)
{
    logenter("Exch test");

    double      val1 = 1.234567;
    double      val2 = 6.890123;
    double      res1, res2;
    stack_push(val1);
    stack_push(val2);
    int         pos1 = sp;
    stack_exch();
    int         pos2 = sp;
    res1 = stack_pop();
    res2 = stack_pop();
    if (!inv(val1 == res1 && val2 == res2, "...") )
        return logerr(TEST_FAILED, "Pop return %f (%f) but is must be %f (%f)", res1, res2, val1, val2);
    if (!inv(pos1 == pos2, "...")){
        return logerr(TEST_FAILED, "Stack position became %d but is must be %d", pos2, pos1);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 6 ---------------------------------

static TestStatus
tf6(void)
{
    logenter("Exch test");

    double      val1 = 1.234567;
    double      res1;
    stack_push(val1);
    stack_pushsame();
    stack_pop(); // no need
    res1 = stack_pop();
    if (!inv(val1 == res1, "...") )
        return logerr(TEST_FAILED, "Pop return %f but is must be %f", res1, val1);
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
        testnew(.f2 = tf1, .num = 1, .name = "Simple Push/pop test"       , .desc = "", .mandatory=true)
      , testnew(.f2 = tf2, .num = 2, .name = "Multiple push/pop test"     , .desc = "", .mandatory=true)
      , testnew(.f2 = tf3, .num = 3, .name = "Empty stack test"           , .desc = "", .mandatory=true)
      , testnew(.f2 = tf4, .num = 4, .name = "Get without pop() test"     , .desc = "", .mandatory=true)
      , testnew(.f2 = tf5, .num = 5, .name = "Exch test"                  , .desc = "", .mandatory=true)
      , testnew(.f2 = tf6, .num = 6, .name = "Pushsame test"              , .desc = "", .mandatory=true)
    );

    logclose("end...");
    return 0;
}


#endif /* STACKNTESTING */

