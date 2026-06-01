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
                cnt += fprintf(out, "%d", elem->v.ival);
            break;
            case HSET_LONG:
                cnt += fprintf(out, "%ld", elem->v.lval);
            break;
            case HSET_DBL:
                cnt += fprintf(out, "%lf", elem->v.dval);
            break;
            case HSET_PTR:
                cnt += fprintf(out, "%p", elem->v.pval);
            break;
            case HSET_FS:
                cnt += fs_fprint(out, &elem->v.fsval, 0);
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

static inline int           compare_int(int v1, int v2){
    if (v1 > v2)
        return 1;
    else if (v1 < v2)
        return -1;
    else
        return 0;
}
static inline int           compare_long(long v1, long v2){
    if (v1 > v2)
        return 1;
    else if (v1 < v2)
        return -1;
    else
        return 0;
}
static inline int           compare_dbl(double v1, double v2){
    if (v1 > v2)
        return 1;
    else if (v1 < v2)
        return -1;
    else
        return 0;
}
static inline int           compare_ptr(const void *restrict v1, const void *restrict v2){
    uintptr_t a = (uintptr_t)v1;
    uintptr_t b = (uintptr_t)v2;
    if (a > b)
        return 1;
    else if (a < b)
        return -1;
    else
        return 0;
}

static inline int           compare(hset_value v1, hset_value v2, hset_type typ){
    switch (typ){
        case HSET_INT:
            return compare_int(v1.ival, v2.ival);
        break;
        case HSET_LONG:
            return compare_long(v1.lval, v2.lval);
        break;
        case HSET_DBL:
            return compare_dbl(v1.dval, v2.dval);
        break;
        case HSET_PTR:
            return compare_ptr(v1.pval, v2.pval);
        break;
        case HSET_FS:
            return fscmp(v1.fsval, v2.fsval);
        break;
    }
    userraiseint(ERR_UNSUPPORTED_TYPE, "%s: %d", hset_type_name(typ), typ);
}

static inline hset_type     getype(const hset *se){
    return se->flags & 0xFF;
}

static hset_elem           *getprevelem(const hset *restrict se, hset_value value, unsigned *restrict phash, hset_elem **restrict pnext, hset_elem **restrict pequal){
    logenter("Loopup...");

    unsigned hash = get_lhash(se, getype(se) );
    hset_elem *el = se->table[hash],
              *prevel = 0;
    logmsg("Hash %u", hash);
    while (el != 0 && compare(value, el->v, getype(se) ) < 0){
        logmsg("el %p, prevel %p", el, prevel);
        prevel = el;
        el = el->next;
    }
    logmsg("after: el %p", el);
    if (phash)
        *phash = hash;
    if (pnext)
        *pnext = el;
    if (pequal){
        if (el && compare(el->v, value, getype(se) ) == 0) //  found EXACLTY!
            *pequal = el;
        else
            *pequal = 0;        // NOT found exactly!
    }
    return logret(prevel, "Found prev %p, next %p", prevel, el);   // < or = in the list
}

static hset_elem           *elemalloc(void){
    return malloc(sizeof(hset_elem) );
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
    switch (getype(s) ){
        case HSET_INT: case HSET_LONG: case HSET_DBL: case HSET_PTR:
            // nothing for now
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

bool                        hset_iset(hset *se, int val){
    unsigned pos = get_lhash(se, val);
    hset_elem *equal = 0, *nextel = 0;
    
}

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

