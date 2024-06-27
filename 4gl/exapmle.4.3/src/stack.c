/**************************************** STACK API *********************************************/

#include "stack.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "log.h"

#define             MAXSTACKVAL         100

static              int     sp = 0;
static              double  st[MAXSTACKVAL];

bool                push(double f){
    logenter("%f", f);
    bool    res = false;
    if (sp < MAXSTACKVAL){
        st[sp++] = f;
        res = true;
    }
    else
        fprintf(stderr, "Error: stack if full, can't push %g\n", f);
    return logret(res, "%s", bool_str(res));
}

double              pop(void){
    logenter("");
    double  res = 0.0;
    if (sp > 0)
        res = st[--sp];
    else
        fprintf(stderr, "Error: stack is empty\n");
    return logret(res, "%f", res);
}

double              gettop(void){
    logenter("");
    double  res = 0.0;
    if (sp > 0)
        res = st[sp - 1];
    return logret(res, "res = %f", res);
}

int                 print_stack(int modif){
    for (int i = 0; i < sp; i++){
        printf("\t[%d]:\t%.8g", i, st[i]);
        if (i + 1 % modif == 0)
            putchar('\n');
    }
    if (sp % modif != 0)
        putchar('\n');
    return sp;
}

void                clear_stack(void){
    sp = 0; // just set up new pointer
}

// ------------------------------- Testing ----------------------------------
#ifdef STACKTESTING

#include "test.h"

// --------------------------------- TEST 1 ---------------------------------

// Bare  test set/check/unset
static TestStatus           tf1(void)
{
    logenter("%s: Bare test set/check/unset", __func__);

    double val = 1.2345, val2;
    push(val);
    if ((val2 = pop()) != val)
        return logret(TEST_FAILED, "SUB1: v = %f but must be = %f", val2, val);

    if (sp != 0)
        return logret(TEST_FAILED, "SUB1: sp %d but must be 0", sp);

    logmsg("SUB1 OK");

    int     i;
    for (i = 0; i < 50; i++)
        push((double) i);

    while (--i >= 0)
        if ( (val2 = pop()) != (double) i)
            return logret(TEST_FAILED, "SUB2: pop() returns %f but it must be %f", val2, (double) i);

    if (sp != 0)
        return logret(TEST_FAILED, "SUB2: sp %d but must be 0", sp);

    logmsg("SUB2 OK");

    return logret(TEST_PASSED, "done 2 subtests");
}

// ---------------------------------------------------------------------------
int                         main(int argc, char *argv[])
{
    LOG(const char *logfilename = "log/stack.log");   // TODO: rework that! It should be simple

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false,  0, "Starting");    // TODO: should be loginitsimple("Starting")

        testenginestd(
            testnew(.f2 = tf1, .num = 1, .name = "Base test", .desc = "Bare test set/check/unset"				, .mandatory=true)
        );

    logclose("end...");
    return 0;
}


#endif /* STACKTESTING */


