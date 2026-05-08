#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

static const int                CTX_MAX_NAME_LEN    = 8192;
static const int                CTX_MAX_VALUE_LEN   = 65536;

// ------------------------------ Utilities ------------------------

static unsigned             get_hash(const Context *c, const char *str){
    invraise(c != 0, "Null pointer");
    return hash_djb2(str) % c->cnt;
}

static void                 free_elements(ContextSortedElem *els){
    int  cnt = 0;
    for (ContextSortedElem *p = els, *n; p != 0; p = n->next){
        n = p->next;
        free(p); cnt++;
    }
    logsimple("freed list of %d", cnt);
}

static ContextSortedElem   *getprev(const Context *restrict c, const char *restrict name, unsigned *restrict phash){
    logenter("Loopup %s", name);

    ContextSortedElem *el = 0, *prevel = 0;
    unsigned hash;
    for (el =c->ctx[hash = get_hash(c, name)]; el != 0 && strcmp(el->name, name) < 0; el = el->next)
        prevel = el;
    if (phash)
        *phash = hash;
    return logret(prevel, "Found prev %p", prevel);   // < or = in the list
}

static ContextSortedElem   *elemalloc(void){
    return malloc(sizeof(ContextSortedElem) );
}

// construct a single element
static ContextSortedElem   *createelem(const char *restrict name, const char *restrict value){
    ContextSortedElem   *el = elemalloc();
    if (el == 0)
        userraiseint(ERR_UNABLE_ALLOCATE, "ContextSortedElem %lu", sizeof(ContextSortedElem) );
    el->flags = 0;
    el->next  = 0;
    el->name  = strndup(name, CTX_MAX_NAME_LEN);    // strndup is more preferable
    el->value = strndup(value, CTX_MAX_VALUE_LEN);
    return el;
}

static int                  printlist(FILE *restrict out, const ContextSortedElem *restrict elem){
    int     cnt = 0;
    if (out){
        while (elem){
            cnt += ctx_fprintelem(out, elem);
            elem = elem->next;
        }
    }
    return cnt;
}

// --------------------------- API ---------------------------------
// ------------------ API Constructs/Destrucor  --------------------

Context                     ctxinit(int cnt){
    invraise(cnt > 0, "Wrong init size");
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

void                        ctxfreed(Context *c){
    invraise(c != 0 && c->ctx != 0, "Null pointer");
    ctxreset(c);
    free(c->ctx);   // free array, but Context is in stack memory
    c->ctx = 0;
    c->cnt = 0;
    logsimple("Freed");
}

// ------------------ General functions ----------------------------

bool                  ctxreset(Context *c){
    invraise(c != 0 && c->ctx != 0, "Null pointer");
    /* for (int i = 0; i < c->cnt; i++)  // TODO: think about foreach iter
        if (c->ctx[i])
            free_elements(c->ctx[i]), c->ctx[i] = 0;*/
    // pforeach iterate via array elements
    foreach_arr(item, c->ctx, c->cnt){
        if (item){
            free_elements(item);
            item = 0;
        }
    }
    return true;
}

ContextSortedElem    *ctxget(const Context *restrict c, const char *restrict name){
    invraise(c != 0 && name != 0, "Null pointer");
    logenter("Loopup %s", name);

    ContextSortedElem *prevel = getprev(c, name, 0), *el = 0;
    logmsg("prev = %p, next = %p", prevel, prevel ? prevel->next : 0);

    if (prevel != 0 && (el = prevel->next) != 0 && strcmp(el->name, name) == 0)
         return logret(el, "Found %p", el);   // exact
    return logerr( (ContextSortedElem *) 0, "Not found");
}

bool                  ctxadd(Context *restrict c, const char *restrict name, const char *restrict value){
    invraise(c != 0 && name != 0 && value != 0, "Null pointer");
    logenter("Adding %.20s:%.40s", name, value);

    unsigned hash  = 0;
    ContextSortedElem *prevel = getprev(c, name, &hash), *el = 0;
    if (prevel != 0 && (el = prevel->next) != 0 && strcmp(el->name, name) == 0) {        // just update
        int newlen = strnlen(value, CTX_MAX_VALUE_LEN);
        logmsg("Revalue from %.40s", el->value);
        char *tmp = realloc(el->value, newlen);
        if (!tmp)
            userraiseint(ERR_UNABLE_ALLOCATE, "Realloc value for %d", newlen);
        el->value = tmp;
        strncpy(el->value, value, CTX_MAX_VALUE_LEN);
    } else {
        ContextSortedElem *newel = createelem(name, value);
        if (el){
            el->next = newel;
            logmsg("Mounted to elem %.20s", el->name);
        }
        else {
            c->ctx[hash] = el;
            logmsg("Mounter to ctx %u", hash);
        }
    }
    return logret(true, "Ok");
}

bool                         ctxdel(Context *restrict c, const char *restrict name){
    logenter("%.20s", name);
    ContextSortedElem *prevel = getprev(c, name, 0), *el = 0;
    // TODO:
    return logret(true, "Deleted");
}

// -------------------------- (API) printers -----------------------

int                          ctx_fprintelem(FILE *restrict out, const ContextSortedElem *restrict elem){
    invraise(elem != 0, "Null pointer");
    int         cnt = 0;
    if (out)
        cnt += fprintf(out, "%d[%.20s:%.100s]%s\n", elem->flags, elem->name, elem->value, elem->next ? "->": "");   // TODO: 20 and 100 can be configurable
    return cnt;
}

int                          ctx_techfprint(FILE *restrict out, const Context *restrict c, const char *restrict name){
    invraise(c != 0, "Null pointer");
    int cnt = 0;
    if (out){
        cnt += fprintf(out, "CONTEXT %s[%d - %p]:", name, c->cnt, c->ctx);
        foreach_arr(item, c->ctx, c->cnt){
            if (item)
                cnt += printlist(out, item);
        }
        cnt += fprintf(out, "\n");
    }
    return cnt;
}

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

    test_sub("subtest %d: init + free", ++subnum);
    {
        int     initcnt = 100;
        Context c = ctxinit(initcnt);
        test_validatefree(c.cnt > initcnt, ctxfree(c), "cnt %d  must be prime number and >= %d", c.cnt, initcnt);
        ctxtechfprint(stdout, c);
        ctxfree(c);
    }
    return logret(TEST_MANUAL, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
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
