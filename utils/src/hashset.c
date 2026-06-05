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

static int                              HSET_ARRAY_CREATE_MULTIPLIER = 2;
static const size_t                     hset_elem_sizes[] = {
    [HSET_INT]  = sizeof(int),
    [HSET_LONG] = sizeof(long),
    [HSET_DBL]  = sizeof(double),
    [HSET_PTR]  = sizeof(void*),
    [HSET_FS]   = sizeof(fs *)
};

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
                cnt += fs_fprint(out, elem->v.fsval, 0);
            break;
        }
        cnt += fprintf(out, " -> ");
        elem = elem->next;
    }
    return cnt;
}

// ------------------------------------ Utilities ------------------------------------------

static unsigned long       get_lhash(unsigned cnt, hset_value value, hset_type typ){

    hset_value      tmp = (hset_value){.u64 = 0UL };
    switch (typ){
        case HSET_INT:
            tmp.u64 = (uint64_t) value.ival;
        break;
        case HSET_LONG:
            tmp.u64 = (uint64_t) value.lval;
        break;
        case HSET_DBL:
            //bits = (uint64_t) value.lval;   // OMG
            tmp.u64 = value.u64;
        break;
        case HSET_PTR:
            tmp.u64 = (uint64_t)(uintptr_t) value.pval;    // or just do nothing as for HSET_DBL
        break;
        case HSET_FS:
            return hash_djb2(fs_str(value.fsval) ) % cnt;
        break;
    }
    return tmp.u64 % cnt;
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
            return fs_cmp(v1.fsval, v2.fsval);
        break;
    }
    userraiseint(ERR_UNSUPPORTED_TYPE, "%s: %d", hset_type_name(typ), typ);
}

static inline hset_type     getype(const hset *se){
    return se->flags & 0xFF;
}

static hset_elem           *getprevelem(const hset *restrict se, hset_value value, unsigned *restrict phash, hset_elem **restrict pnext, hset_elem **restrict pequal){
    //logenter("Loopup...");

    unsigned hash = get_lhash(se->sz, value, getype(se) );
    hset_elem *el = se->table[hash],
              *prevel = 0;
    // logsimple("Hash %u", hash);
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

static hset_elem           *create_elem(hset_value val){
    hset_elem *res = alloc_elem();
    if (!res)
        return logsimpleret( (hset_elem *) 0, "Unable to create elem");
    res->v = val;
    res->next = 0;

    return res;
}

static hset_elem           *clone_elemlist(const hset_elem *el, hset_type typ){

    hset_elem  *prev = 0, *newel = 0, *retel = 0;
    while (el){
        if (!(newel = create_elem(el->v) ) )
            return userraise((hset_elem *) 0, ERR_UNABLE_ALLOCATE, "Unable to create element");
        switch (typ){
            case HSET_FS:
                // NOT GOOD: TODO: refactor that!
                newel->v.fsval = malloc(sizeof(fs) );
                if (!newel->v.fsval)
                    return userraise((hset_elem *) 0, ERR_UNABLE_ALLOCATE, "Unable to create heap fs");
                *newel->v.fsval = fs_clone(el->v.fsval);
            break;
            /* case HSET_STR:
                newel->v.str = strdup(el->v.str);
            break;*/
            default:
            break;
        }
        if (!retel)
            retel = newel;  //  first elem
        if (prev)
            prev->next = newel;
        // next iter
        prev = newel;
        el = el->next;
    }
    return logsimpleret(retel, "Chain is created %p", retel);
}

static void                 free_elem(hset_elem *el, hset_type typ){
    switch (typ){
        case HSET_FS:
            fs_free(el->v.fsval);        // to free space
            free(el->v.fsval);
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

static bool                 validate_elemlist(const hset_elem *el, hset_type typ, unsigned pos, unsigned sz){
    while (el){
        unsigned hash = get_lhash(sz, el->v, typ);
        if (hash != pos)
            return logsimpleerr(false, "%u, but must be %u", hash, pos);
        el = el->next;
    }
    return logsimpleerr(true, "%u Ok", pos);
}

static inline hset_value    hset_createarrval(const void *arr, int idx, hset_type typ) {

    size_t elem_size = hset_elem_sizes[typ];

    if (elem_size == 0)
        userraiseint(ERR_UNSUPPORTED_TYPE, "type %d", typ);

    const char *base = (const char *)arr;
    return hset_createval(base + idx * elem_size, typ);
}

static inline int           count_elemlist(const hset_elem *el){
    int     cnt = 0;
    while (el){
        cnt++;
        el = el->next;
    }
    return cnt;
}

// ------------------------------------- API -----------------------------------------------

// ---------------------------------- API Constructs/Destrucor  ----------------------------

hset                        hset_init(int sz, hset_type typ){
    logenter("init sz %d - %s", sz, hset_type_name(typ) );

    if (int_notin(typ, HSET_INT, HSET_LONG, HSET_DBL, HSET_FS, HSET_PTR) )
        userraiseint(ERR_UNSUPPORTED_TYPE, "%d", typ);

    unsigned    newsz = next_prime(sz);
    hset res = HSET(newsz, typ);
    if ( (res.table = malloc(newsz * sizeof(hset_elem *) ) ) == 0)    // raise here - nothing to do
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to alloc newsz %u elems", newsz);
    clean_ptr( (void *) res.table,  newsz);

    return logret(res, "Created with %d", res.sz);
}

hset                        hset_clone(const hset *se){
    invraise(se != 0, "Null pointer");

    int     newsz = se->sz;
    hset    res = HSET(newsz, getype(se) );

    if ( (res.table = malloc(newsz * sizeof(hset_elem *) ) ) == 0)    // raise here - nothing to do
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to alloc newsz %u elems", newsz);

    for (int i = 0; i < newsz; i++){
        logauto(i);
        res.table[i] = clone_elemlist(se->table[i], getype(se) );
    }

    return logsimpleret(res, "Cloned");
}

hset                        hset_fromanyarr(const void *arr, int sz, hset_type typ){
    invraise(arr != 0 && sz > 0 && sz < INT_MAX / 4, "Incorrent input %p - %d", arr, sz);
    if (int_notin(typ, HSET_INT, HSET_LONG, HSET_DBL, HSET_PTR) )
        userraiseint(ERR_UNSUPPORTED_TYPE, "%d", typ);

    unsigned    newsz = next_prime(sz * HSET_ARRAY_CREATE_MULTIPLIER);
    int         i = 0;
    hset    res = hset_init(newsz, typ);
    while (i < sz){
        // not sure if that pretty good
        if (! hset_set(&res, hset_createarrval(arr, i, typ) ) )
            userraiseint(ERR_UNABLE_ALLOCATE, "Unable to add %d", i);
        i++;
    }
    return logsimpleret(res, "Created %u", newsz);
}

void                        hset_free(hset *se){
    invraise(se != 0, "Null pointer");

    hset_clean(se);
    free(se->table);
    logsimple("freed");
    se->sz = se->flags = 0;
}

// ---------------------------------------------------------------------------
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
    // cross validation value <=> hashkey
    for (int i = 0; i < se->sz; i++)
        if (se->table[i] )
            if (!validate_elemlist(se->table[i], getype(se), i, se->sz) ){
                if (out)
                    fprintf(out, "Hash Validation failed on %d", i);
                return logerr(false, "Hash Validation failed on %d", i);
            }

    return logret(true, "Validated");
}

// ---------------------------------- General functions ------------------------------------

bool                        hset_set(hset *se, hset_value val){
    invraise(se != 0, "Null pointer");

    unsigned     hash = 0;
    bool         res = false;
    hset_elem   *equal = 0, *nextel = 0;
    hset_elem   *prevel = getprevelem(se, val, &hash, &nextel, &equal);
    if (equal)
        res = true;
    else {
        hset_elem *newel = create_elem(val);
        if (!newel)
            return userraise(false, ERR_UNABLE_ALLOCATE, "Can't create new element");
        if (prevel)
            prevel->next = newel;
        else  // mount to root
            se->table[hash] = newel;
        newel->next = nextel;
    }
    hsetval_log(val, getype(se) );
    return logsimpleret(true, "Added the value prev existing %s", bool_str(res) );
}

bool                        hset_get(const hset *se, hset_value val){
    invraise(se != 0, "Null pointer");

    hset_elem   *equal = 0;
    getprevelem(se, val, 0, 0, &equal);
    if (equal){
        //hsetval_log(val, getype(se) );
        return true;
    } else {
        //hsetval_log(val, getype(se) );
        return false;
    }
}

bool                        hset_del(hset *se, hset_value val){
    invraise(se != 0, "Null pointer");

    unsigned            hash;
    hset_elem          *el = 0;
    hset_elem          *prevel = getprevelem(se, val, &hash, 0, &el);
    if (!el)    // not found
        return logsimpleerr(false, "Not found");
    // umount el
    if (!prevel)
         se->table[hash] = el->next;
    else
        prevel->next = el->next;
    free_elem(el, getype(se) );
    //hsetval_log(val, getype(se) );
    return logsimpleret(true, "Deleted");
}

int                         hset_cnt(const hset *se){
    invraise(se != 0, "Null pointer");

    int     cnt = 0;
    for (int i = 0; i < se->sz; i++)
        if (se->table[i]){  // actually this check is excess
            cnt += count_elemlist(se->table[i] );
        }
    return logsimpleret(cnt, "Total %d", cnt);
}

void                        hset_clean(hset *se){
    invraise(se != 0, "Null pointer");

    for (int i = 0; i < se->sz; i++)
        if (se->table[i] ){
            free_elemlist(se->table[i], getype(se) );
            se->table[i] = 0;   // clean that chain
        }
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
#include "array.h"
#include <time.h>

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
            hset_set(&se1, HSET_INTVALUE(num) ), hset_free(&se1), "Must be true"
        );
        hset_techfprint(logfile, &se1, 0);
        test_validatefree(
            hset_set(&se1, HSET_INTVALUE(num) ),  hset_free(&se1), "Must be true"
        );
        hset_techfprint(logfile, &se1, 0);
        test_validatefree(
            hset_validate(stdout, &se1), hset_free(&se1), "Validation failed"
        );
        test_validatefree(
            hset_get(&se1, HSET_INTVALUE(num) ), hset_free(&se1), "Must be true,  because already added %d", num
        );
        test_validatefree(
            hset_get(&se1, HSET_INTVALUE(num + 1) ) == false, hset_free(&se1), "Must be false %d", num + 1
        );
        test_validatefree(
            hset_del(&se1, HSET_INTVALUE(num) ), hset_free(&se1), "Must be true, because element %d exists", num
        );
        hset_techfprint(logfile, &se1, 0);
        test_validatefree(
            hset_del(&se1, HSET_INTVALUE(num) ) == false, hset_free(&se1), "Must be false, because element %d already deleted", num
        );
        test_validatefree(
            hset_validate(stdout, &se1), hset_free(&se1), "Validation failed"
        );
        hset_free(&se1);
    }
    test_sub("subtest %d: HSET_INT multiple add/del", ++subnum);
    {
        int     cnt = 100, mul = 3;
        hset    se1 = hset_init(cnt, HSET_INT);

        for (int i = 0; i < cnt * mul; i++)
            test_validatefree(
                hset_set(&se1, HSET_INTVALUE(i) ), hset_free(&se1), "Unable to add %d", i
            );
        hset_techfprint(logfile, &se1, 0);

        for (int i = 0; i < cnt * mul; i++)
            test_validatefree(
                hset_get(&se1, HSET_INTVALUE(i) ), hset_free(&se1), "Unable to get %d", i
            );

        int     delfrom = 40, delto = 50;
        for (int i = delfrom; i < delto; i++)
            test_validatefree(
                hset_del(&se1, HSET_INTVALUE(i) ), hset_free(&se1), "Unable to delete %d", i
            );

        for (int i = 0; i < cnt * mul; i++)
            if (i >= delfrom && i < delto){
                test_validatefree(
                    hset_get(&se1, HSET_INTVALUE(i) ) == false, hset_free(&se1), "Element %d was deleted, but return found somehow!", i
                );
            } else {
                test_validatefree(
                    hset_get(&se1, HSET_INTVALUE(i) ), hset_free(&se1), "Element %d not found", i
                );
            }
        test_validatefree(
            hset_validate(stdout, &se1), hset_free(&se1), "Validation failed"
        );

        hset_free(&se1);
    }
    test_sub("subtest %d: HSET_INT multiple reversed add/del", ++subnum);
    {
        int     cnt = 100, mul = 3;
        hset    se1 = hset_init(cnt, HSET_INT);

        for (int i = cnt * mul - 1; i >= 0; i--)
            test_validatefree(
                hset_set(&se1, HSET_INTVALUE(i) ), hset_free(&se1), "Unable to add %d", i
            );

        for (int i = 0; i < cnt * mul; i++)
            test_validatefree(
                hset_get(&se1, HSET_INTVALUE(i) ), hset_free(&se1), "Unable to get %d", i
            );
        int     delfrom = 40, delto = 50;
        for (int i = delfrom; i < delto ; i++)
            test_validatefree(
                hset_del(&se1, HSET_INTVALUE(i) ), hset_free(&se1), "Unable to delete %d", i
            );

        for (int i = 0; i < cnt * mul; i++)
            if (i >= delfrom && i < delto){
                test_validatefree(
                    hset_get(&se1, HSET_INTVALUE(i) ) == false, hset_free(&se1), "Element %d was deleted, but return found somehow!", i
                );
            } else {
                test_validatefree(
                    hset_get(&se1, HSET_INTVALUE(i) ), hset_free(&se1), "Element %d not found", i
                );
            }

        hset_free(&se1);
    }
    test_sub("subtest %d: random int add/del", ++subnum);
    {
        int     cnt = 100, mul = 3;
        hset    se1 = hset_init(cnt, HSET_INT);

        for (int i = cnt * mul - 1; i >= 0; i--)
            test_validatefree(
                hset_set(&se1, HSET_INTVALUE(i) ), hset_free(&se1), "Unable to add %d", i
            );
        // random del
        srand(time(NULL));
        for (int i = 0; i < cnt * mul; i += rndint(5) + 1 )
            //if (i < cnt * mul)
                test_validatefree(
                    hset_del(&se1, HSET_INTVALUE(i) ), hset_free(&se1), "Unable to delete [%d]", i
                );

        test_validatefree(
            hset_validate(stdout, &se1), hset_free(&se1), "Validation failed"
        );
        hset_free(&se1);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 3 ---------------------------------

static TestStatus
tf3(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: clone...", ++subnum);
    {
        hset    se1 = hset_init(100, HSET_LONG);
        int     cnt = 500;

        for (int i = 0; i < cnt; i++)
            test_validatefree(
                hset_set(&se1, HSET_LONGVALUE(i) ), hset_free(&se1), "Unable to add %d", i
            );
        hset    se2 = hset_clone(&se1);
        // then compare one by one
        bool        r1, r2;
        for (int i = 0; i < cnt; i++){
            r1 = hset_get(&se1, HSET_LONGVALUE(i) );
            r2 = hset_get(&se2, HSET_LONGVALUE(i) );
            test_validatefree(
                (r1 ^ r2) == false, (hset_free(&se1), hset_free(&se2) ),
                "%d: Must be true all, but origin %s, clone %s", i, bool_str(r1), bool_str(r2)
            );
        }

        hset_free(&se1);

        hset_techfprint(logfile, &se2, 0);
        test_validatefree(
            hset_validate(stdout, &se2), hset_free(&se2), "Validation failed"
        );

        hset_free(&se2);
    }
    test_sub("subtest %d: create from int array", ++subnum);
    {
        Array arr = IArray_create(200, ARRAY_RND);

        hset    se1 = hset_fromiarr(arr.iv, arr.len);

        hset_techfprint(logfile, &se1, 0);

        for (int i = 0; i < arr.len; i++)
            test_validatefree(
                hset_get(&se1, HSET_INTVALUE(arr.iv[i]) ), (Array_free(&arr), hset_free(&se1) ),
                "Element %d isn't found", arr.iv[i]
            );

        test_validatefree(
            hset_validate(stdout, &se1), hset_free(&se1), "Validation failed"
        );

        Array_free(&arr);
        hset_free(&se1);
    }
    test_sub("subtest %d: create from int array, contained 0 value", ++subnum);
    {
        Array arr = IArray_create(10, ARRAY_ZERO);
        hset    se1 = hset_fromiarr(arr.iv, arr.len);

        hset_techfprint(stdout, &se1, 0);

        for (int i = 0; i < arr.len; i++)
            test_validatefree(
                hset_get(&se1, HSET_INTVALUE(arr.iv[i]) ), (Array_free(&arr), hset_free(&se1) ),
                "Element %d isn't found", arr.iv[i]
            );

        Array_free(&arr);
        hset_free(&se1);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 4 ---------------------------------

static TestStatus
tf4(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: create from array and compare", ++subnum);
    {
        Array arr = DArray_create(200, ARRAY_RND);

        hset    se1 = hset_fromdarr(arr.dv, arr.len);

        hset    se2 = hset_clone(&se1);

        // TODO: compare

        hset_free(&se1);
        hset_free(&se2);
    }
    test_sub("subtest %d: clone and compare", ++subnum);
    {

    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 5 ---------------------------------

static TestStatus
tf5(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: empty count", ++subnum);
    {
        hset    se1 = hset_init(100, HSET_LONG);
        int     res;
        test_validatefree(
            (res = hset_cnt(&se1) ) == 0, hset_free(&se1), "Must be zero, but not %d", res
        );

        hset_free(&se1);
    }
    test_sub("subtest %d: loaded count", ++subnum);
    {
        rndinit();
        Array   arr = DArray_create(500, ARRAY_ASC);
        hset    se2 = hset_fromdarr(arr.dv, arr.len);
        int     res;
        // hset_techprint(&se2, 5);
        //Array_print(arr, 5);
        test_validatefree(
            (res = hset_cnt(&se2) ) == Arraylen(arr), (hset_free(&se2), Arrayfree(arr) ), "Must be == %d, but not %d", Arraylen(arr), res
        );
        Arrayfree(arr);
        hset_free(&se2);
    }
    test_sub("subtest %d: loaded count int", ++subnum);
    {
        Array   arr = IArray_create(500, ARRAY_ASC);
        hset    se2 = hset_fromiarr(arr.iv, arr.len);
        int     res;
        //hset_techprint(&se2, 0);
        test_validatefree(
            (res = hset_cnt(&se2) ) == Arraylen(arr), (hset_free(&se2), Arrayfree(arr) ), "Must be == %d, but not %d", Arraylen(arr), res
        );
    test_sub("subtest %d: count after clean", ++subnum);

        hset_clean(&se2);
        test_validatefree(
            (res = hset_cnt(&se2) ) == 0, (hset_free(&se2), Arrayfree(arr) ), "Must be zero after cleanbut not %d", res
        );
        Arrayfree(arr);
        hset_free(&se2);
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
      , testnew(.f2 = tf3,  .num =  3, .name = "Simple clone and create from array test"    , .desc="", .mandatory=true)
   //   , testnew(.f2 = tf4,  .num =  4, .name = "Comparation simple test"                    , .desc="", .mandatory=true)
      , testnew(.f2 = tf5,  .num =  5, .name = "Simple count test"                          , .desc="", .mandatory=true)
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HSETTESTING */

