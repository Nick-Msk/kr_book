
#include "context.h"
#include "common.h"
#include "log.h"
#include "error.h"
#include "checker.h"
#include "guard.h"
#include "numeric_ops.h"
#include "iterator.h"


/********************************************************************
                    CONTEXT STRING MODULE IMPLEMENTATION
********************************************************************/

// ------------------------------ Utilities ------------------------

static unsigned      get_hash(const Context *c, const char *str){
    invraise(c != 0, "Null pointer");
    return hash_djb2(str) % c->cnt;
}

static void          free_elements(ContextSortedElem *els){
    int  cnt = 0;
    for (ContextSortedElem *p = els, *n; p != 0; p = n->next){
        n = p->next;
        free(p); cnt++;
    }
    logsimple("freed list of %d", cnt);
}

// --------------------------- API ---------------------------------
// ------------------ API Constructs/Destrucor  --------------------

Context              ctxinit(int cnt){
    invraise(cnt <= 0, "Wrong init size");
    logenter("%d", cnt);
    unsigned    newcnt = next_prime(cnt);
    Context     tmp = (Context) {.cnt = newcnt};
    unsigned    newsz = (newcnt + 1) * sizeof(ContextSortedElem *);  // 1 for last 0
    tmp.ctx = malloc(newsz);
    if (!tmp.ctx)
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to allocated %d", newsz);
    memset(tmp.ctx, 0, newsz); // not sure
    return logret(tmp, "Created with %d", tmp.cnt);
}

void                  ctxfreed(Context *c){
    invraise(c != 0 && c->ctx != 0, "Null pointer");
    /* for (int i = 0; i < c->cnt; i++)  // TODO: think about foreach iter
        if (c->ctx[i])
            free_elements(c->ctx[i]), c->ctx[i] = 0;*/
    // foreach_
    // pforeach iterate via pointer to array elements
    foreach_arr(item, c->ctx, c->cnt){
        if (item){
            free_elements(item);
            item = 0;
        }
    }
    c->ctx = 0;
    logsimple("Freed");
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
    const char shift[] = "----------";

    test_sub("subtest %d: init + free", ++subnum);
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
