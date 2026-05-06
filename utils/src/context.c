#include "common.h"
#include "log.h"
#include "error.h"
#include "fileutils.h"
#include "checker.h"
#include "guard.h"
#include "numeric_ops.h"


/********************************************************************
                    CONTEXT STRING MODULE IMPLEMENTATION
********************************************************************/

// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers -------------------


// ------------------------------ Utilities ------------------------

static void          free_elements(ContextSortedElem *els){
    // TODO via recursion or iter
}

// --------------------------- API ---------------------------------
// ------------------ API Constructs/Destrucor  --------------------

Context              ctxinit(int cnt){
    logenter("%d", cnt);
    unsigned    newcnt = next_prime(sz);
    Context     tmp = (Context) {.cnt = newcnt};
    newcnt *= sizeof(ContextSortedElem);
    tmp.ctx = malloc(newcnt);
    if (!tmp.ctx)
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to allocated %d", newcnt);
    memset(tmp.ctx, 0, newcnt); // not sure
    return logret(tmp, "Created");
}

void                  ctxfreed(Context *c){
    invraise(ctx != 0, "Null pointer");
    for (int i = 0; i < ctx->cnt; i++)  // TODO: think about foreach iter
        if (c->ctx[i])
            free_elements(c->ctx[i]), c->ctx[i] = 0;
    logsimple(c->ctx = 0, "Freed");
}

// ------------------ General functions ----------------------------

// -------------------------- (API) printers -----------------------

// -------------------------------Testing --------------------------

#ifdef CONTEXTTESTING

#include "test.h"
#include "checker.h"

//types for testing


// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    fs s = FS();
    const char shift[] = "----------";

    test_sub("subtest %d: getint simple", ++subnum);
    {

    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}



// ------------------------------------------------------------------------------------------------------------------------------
int
main( /* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 = tf1,  .num =  1, .name = "Simple init and validate test"              , .desc=""                , .mandatory=true)
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* CONTEXTTESTING */
