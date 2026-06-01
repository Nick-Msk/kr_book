/********************************************************************
                    HASH-BASED SET MODULE IMPLEMENTATION
********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "hashset.h"
#include "numeric_ops.h"
#include "log.h"
#include "bool.h"
#include "common.h"
#include "error.h"
#include "checker.h"
#include "guard.h"

// ---------- pseudo-header for utility procedures -----------------

// ---------------------------- (Utility) printers -----------------------------

int                         sortedlist_fprint(FILE *restrict out, const hset_elem *restrict elem, hset_type typ){
    int     cnt = 0;
    while (elem){
        switch (typ){
            case HSET_INT:
                cnt += fprintf(out, "%d", elem->ival);
            break;
            case HSET_LONG:
                cnt += fprintf(out, "%ld", elem->lval);
            break;
            case HSET_DBL:
                cnt += fprintf(out, "%lf", elem->dval);
            break;
            case HSET_PTR:
                cnt += fprintf(out, "%p", elem->pval);
            break;
            case HSET_FS:
                cnt += fs_fprint(out, &elem->fsval, 0);
            break;
        }
        cnt += fprintf(out, "\t");
        elem = elem->next;
    }
    return cnt;
}

// ------------------------------------ Utilities ------------------------------------------

// for int & long
static unsigned             get_lhash(const hset *se, long val){
    invraise(se != 0, "Null pointer");
    return hash_long(val) % se->sz;
}

static inline hset_type     getype(const hset *se){
    return se->flags & 0xFF;
}

// ------------------------------------- API -----------------------------------------------

// ---------------------------------- API Constructs/Destrucor  ----------------------------

hset                        hset_init(int sz, hset_type typ){
    logenter("init sz %d", sz);

    if (int_notin(typ, HSET_INT, HSET_LONG, HSET_DBL, /* HSET_FS, */ HSET_PTR) )
        userraiseint(ERR_UNSUPPORTED_TYPE, "%d", typ);

    unsigned    newsz = next_prime(sz);
    hset res = HSET(newsz, typ);
    if ( (res.table = malloc(newsz * sizeof(hset_elem *) ) ) == 0)    // raise here - nothing to do
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to alloc newsz %u elems", newsz);
    clean_ptr( (void *) res.table,  newsz);

    return logret(res, "Crreated with %d", res.sz);
}

void                        hset_free(hset *s){
    invraise(s != 0, "Null pointer");
    switch (s->flags & 0xFF){
        case HSET_INT: case HSET_LONG: case HSET_DBL: case HSET_PTR:
        break;
        case HSET_FS:
            userraiseint(ERR_UNSUPPORTED_TYPE, "HSET_FS (%d) is'nt suppored for now", HSET_FS);
        break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "Unknown type %d", s->flags & 0xFF);
        break;
    }
    free(s->table);
    logsimple("freed");
    s->sz = s->flags = 0;
}

// -------
bool                        hset_validate(FILE *out, const hset *restrict se){
    logenter("...");
    if (!se){    // TODO: think about string creation via faststring here!!
        if (out)
            fprintf(out, "null pointer");
        return logerr(false, "null pointer");
    }
    if (se->sz <= 0){
        if (out)
            fprintf(out, "Incorrect sz (%d)", se->sz);
        return logerr(false, "Incorrect sz (%d)", se->sz);
    }
    if (int_notin(getype(se), HSET_INT, HSET_LONG, HSET_DBL, HSET_FS, HSET_PTR) ){
        if (out)
            fprintf(out, "Incorrect type %d", getype(se) );
        return logerr(false, "Incorrect type %d", getype(se) );
    }
    if (!se->table){
        if (out)
            fprintf(out, "null hashtable");
        return logerr(false, "null hashtable");
    }
    return logret(true, "Validated");
}

// ---------------------------------- General functions ------------------------------------


// ------------------------------------- (API) printers ------------------------------------

int                         hset_techfprint(FILE *restrict out, const hset *se, int sz){
    invraise(se != 0, "Null pointer");
    int     cnt = 0;
    if (out){
        sz = sz ? MIN(sz, se->sz) : se->sz;
        logauto(sz);
        cnt += fprintf(out, "HSET (%d:%s) [\n", se->sz, hset_type_name(getype(se) ) );
        for (int i = 0; i < sz; i++)
            if (se->table[i]){
                cnt += fprintf(out, "%4d: ", i);
                cnt += sortedlist_fprint(out, se->table[i], getype(se) );
                cnt += fprintf(out, "\n");
            }
        cnt += fprintf(out, "]\n");
    }
    return cnt;
}

// --------------------------------- SERIALIZATION -----------------------------------------

// -------------------------------Testing --------------------------
#ifdef HSETTESTING

#include "test.h"

//types for testing

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: init + free", ++subnum);
    {
        hset se1 = hset_init(100, HSET_INT);
        hset_techfprint(logfile, &se1, 0);
        test_validatefree(
            hset_validate(stdout, &se1), hset_free(&se1), "Validation failed"
        );
        hset_free(&se1);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main( /* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 = tf1,  .num =  1, .name = "Simple init and validate test"              , .desc="", .mandatory=true)
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HSETTESTING */

