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
#include "fs.h"

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
        cnt += fprintf(out, " -> ");
        elem = elem->next;
    }
    return cnt;
}

// ------------------------------------ Utilities ------------------------------------------

static unsigned long       get_lhash(const hset *se, hset_value value, long val){
    invraise(se != 0, "Null pointer");

    unsigned long res;
    switch (typ){
        case HSET_INT: case HSET_LONG: case HSET_DBL: case HSET_PTR:
            res = hash_long(value.lval);    // that is IMPLICIT conversion to long via union {} !
        break;
        case HSET_FS:
            res = hash_djb2(fsstr(value.fsval) );
        break;
    }
    return res % se->sz;
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
    //logenter("Loopup...");

    unsigned hash = get_lhash(se, value, getype(se) );
    hset_elem *el = se->table[hash],
              *prevel = 0;
    logsimple("Hash %u", hash);
    while (el != 0 && compare(value, el->v, getype(se) ) < 0){
        //logmsg("el %p, prevel %p", el, prevel);
        prevel = el;
        el = el->next;
    }
    // logmsg("after: el %p", el);
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
    return prevel; //logsimpleret(prevel, "Found prev %p, next %p", prevel, el);   // < or = in the list
}

static hset_elem           *alloc_elem(void){
    return malloc(sizeof(hset_elem) );
}

static hset_elem           *create_ielem(int val){
    hset_elem *res = alloc_elem();
    if (!res)
        return logsimpleret( (hset_elem *) 0, "Unable to create elem");
    res->v.ival = val;
    res->next = 0;
    return res;
}

static void                 free_elem(hset_elem *el, hset_type typ){
    switch (typ){
        case HSET_FS:
            fsfree(el->v.fsval);        // to free space
        break;
        default:
            // nothing here
        break;
    }
    free(el);
}

static void                 free_elemlist(hset_elem *el, hset_type typ){
    hset_elem       *next = 0;
    while (el){
        next = el->next;
        free_elem(el, typ);
        el = next;
    }
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

void                        hset_free(hset *se){
    invraise(se != 0, "Null pointer");

    for (int i = 0; i < se->sz; i++)
        if (se->table[i] )
            free_elemlist(se->table[i], getype(se) );
    free(se->table);
    logsimple("freed");
    se->sz = se->flags = 0;
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
    invraise(se != 0 && getype(se) == HSET_INT, "Null pointer or non-int type");

    unsigned     hash = 0;
    bool         res = false;
    hset_elem   *equal = 0, *nextel = 0;
    hset_elem   *prevel = getprevelem(se, HSET_INTVALUE(val), &hash, &nextel, &equal);
    if (equal)
        res = true;
    else {
        hset_elem *newel = create_ielem(val);
        if (!newel)
            return userraise(false, ERR_UNABLE_ALLOCATE, "Can't create new element");
        if (prevel)
            prevel->next = newel;
        else  // mount to root
            se->table[hash] = newel;
        newel->next = nextel;
    }
    return logsimpleret(true, "Added %d, prev existing %s", val, bool_str(res) );
}

bool                        hset_iget(const hset *se, int val){
    invraise(se != 0 && getype(se) == HSET_INT, "Null pointer or non-int type");

    hset_elem   *equal = 0;
    getprevelem(se, HSET_INTVALUE(val), 0, 0, &equal);
    if (equal)
        return logsimpleret(true, "Exists %d", val);
    else
        return logsimpleret(false, "Not exists %d", val);
}

bool                        hset_idel(hset *se, int val){
    invraise(se != 0 && getype(se) == HSET_INT, "Null pointer or non-int type");

    unsigned            hash;
    hset_elem          *el = 0;
    hset_elem          *prevel = getprevelem(se, HSET_INTVALUE(val), &hash, 0, &el);
    if (!el)    // not found
        return logsimpleerr(false, "Not found");
    // umount el
    if (!prevel)
         se->table[hash] = el->next;
    else
        prevel->next = el->next;
    free_elem(el, getype(se) );
    return logsimpleret(true, "Deleted");
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

// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf2(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: HSET_INT init + get + free", ++subnum);
    {
        hset    se1 = hset_init(100, HSET_INT);
        int     num = 77;
        // mustb return false
        test_validatefree(
            hset_iset(&se1, num), hset_free(&se1), "Must be true"
        );
        hset_techfprint(logfile, &se1, 0);
        test_validatefree(
            hset_iset(&se1, num),  hset_free(&se1), "Must be true"
        );
        hset_techfprint(logfile, &se1, 0);
        test_validatefree(
            hset_validate(stdout, &se1), hset_free(&se1), "Validation failed"
        );
        test_validatefree(
            hset_iget(&se1, num), hset_free(&se1), "Must be true,  because already added %d", num
        );
        test_validatefree(
            hset_iget(&se1, num + 1) == false, hset_free(&se1), "Must be false %d", num + 1
        );
        test_validatefree(
            hset_idel(&se1, num), hset_free(&se1), "Must be true, because element %d exists", num
        );
        hset_techfprint(logfile, &se1, 0);
        test_validatefree(
            hset_idel(&se1, num) == false, hset_free(&se1), "Must be false, because element %d already deleted", num
        );
        hset_free(&se1);
    }
    test_sub("subtest %d: HSET_INT multiple add/del", ++subnum);
    {
        int     cnt = 100, mul = 3;
        hset    se1 = hset_init(cnt, HSET_INT);

        for (int i = 0; i < cnt * mul; i++)
            test_validatefree(
                hset_iset(&se1, i), hset_free(&se1), "Unable to add %d", i
            );
        hset_techfprint(logfile, &se1, 0);

        for (int i = 0; i < cnt * mul; i++)
            test_validatefree(
                hset_iget(&se1, i), hset_free(&se1), "Unable to get %d", i
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
      , testnew(.f2 = tf2,  .num =  2, .name = "Simple init and add test"                   , .desc="", .mandatory=true)
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HSETTESTING */

