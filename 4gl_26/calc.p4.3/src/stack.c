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

#include "testing.h"
#include <signal.h>

// ------------------------- TEST 1 ---------------------------------

// ------------------------- TEST 2 ---------------------------------

// -------------------------------------------------------------------

// TODO:!!!!!!!!
int
main(int argc, char *argv[])
{
    const char *logfilename = "stack.log";

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


#endif /* SKELETONTESTING */

