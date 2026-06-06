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
    // probably it's better to calc hash by u64 attr (except fs for sure)
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

static hset_value           convert_value(hset_value v, hset_type from, hset_type to){
    if (from == to)
        return v;

    hset_value result = HSET_VALUE; // обнуляем u64

    switch (from) {
        case HSET_INT:
            if (to == HSET_LONG)
                result.lval = (long)v.ival;
            else if (to == HSET_DBL)
                result.dval = (double)v.ival;
        break;
    case HSET_LONG:
        if (to == HSET_INT)
            result.ival = (int)v.lval;
        else if (to == HSET_DBL)
            result.dval = (double)v.lval;
        break;
    case HSET_DBL:
        if (to == HSET_INT)
            result.ival = (int)v.dval;
        else if (to == HSET_LONG)
            result.lval = (long)v.dval;
        break;
    default:
        break; // неподдерживаемые типы — останется нулевое значение
    }
    return result;
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
    // // TODO: check ordering too!
    const hset_elem *prevel = 0;
    int              iternum = 0;
    while (el){
        unsigned hash = get_lhash(sz, el->v, typ);
        if (hash != pos)
            return logsimpleerr(false, "iter %d: %u, but must be %u", iternum, hash, pos);
        if (prevel)
            if (compare(el->v, prevel->v, typ) < 0)
                return logsimpleret(false, "iter %d: ordering violation", iternum);  // TODO: valprev %s valnext %s must be here via fs
        el = el->next;
    }
    return logsimpleerr(true, "%u Ok", pos);
}

static hset_value           hset_createarrval(const void *arr, int idx, hset_type typ) {

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
// find every value in the list in se
static bool                 find_elems(const hset_elem *restrict el, const hset *restrict se){
    while (el){
        if (!hset_get(se, el->v) )
            return false;
        el = el->next;
    }
    return true;
}

// ------------------------------------- API -----------------------------------------------

// ---------------------------------- API Constructs/Destrucor  ----------------------------

hset                        hset_init(int sz, hset_type typ){
    logenter("init sz %d - %s", sz, hset_type_name(typ) );

    if (int_notin(typ, HSET_INT, HSET_LONG, HSET_DBL, HSET_FS, HSET_PTR) )
        userraiseint(ERR_UNSUPPORTED_TYPE, "%d", typ);

    if (sz <= 0)
        sz = 1;
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
    hset    res = hset_init(newsz - 1, getype(se) );

    if ( (res.table = malloc(newsz * sizeof(hset_elem *) ) ) == 0)    // raise here - nothing to do
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to alloc newsz %u elems", newsz);

    for (int i = 0; i < newsz; i++){
        res.table[i] = clone_elemlist(se->table[i], getype(se) );
    }

    return logsimpleret(res, "Cloned");
}
// created with new type only (INT, LONG, DBL) as allowed
hset                        hset_cloneas(const hset *se, hset_type typ){
    invraise(se != 0, "Null pointer");

    if (int_notin(typ, HSET_INT, HSET_LONG, HSET_DBL) && int_notin(getype(se), HSET_INT, HSET_LONG, HSET_DBL))
        userraiseint(ERR_UNSUPPORTED_TYPE, "From %d:%s - to %d:%s",
            getype(se), hset_type_name(getype(se) ), typ, hset_type_name(typ) );

    hset    res = hset_init(se->sz - 1, typ);
    for (int i = 0; i < se->sz; i++){
        const hset_elem *el = se->table[i];     // probably better to create separate function
        while (el){
            if (!hset_set(&res, convert_value(el->v, getype(se), typ) ) )
                userraiseint(ERR_UNABLE_ALLOCATE, "Unable to create element");
            el = el->next;
        }
    }
    return logsimpleret(res, "Cloned as %d:%s", typ, hset_type_name(typ) );
}

hset                        hset_fromanyarr(const void *arr, int sz, hset_type typ){
    invraise(arr != 0 && sz > 0 && sz < INT_MAX / 4, "Incorrent input %p - %d", arr, sz);
    if (int_notin(typ, HSET_INT, HSET_LONG, HSET_DBL, HSET_PTR) )
        userraiseint(ERR_UNSUPPORTED_TYPE, "%d", typ);

    if (sz <= 0)
        sz = 1;     // to avoid 0 initializing
    hset        res = hset_init(sz * HSET_ARRAY_CREATE_MULTIPLIER, typ);
    int         cnt = hset_loadanyarr(&res, arr, sz, typ);
    return logsimpleret(res, "Created hest with sz %u, loaded %d", res.sz, cnt);
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
    // hsetval_log(val, getype(se) );
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
// check the equality as SET!
bool                        hset_eq(const hset *restrict se1, const hset *restrict se2){
    invraise(se1 != 0 && se2 != 0, "Null pointers");

    if (getype(se1) != getype(se2) )
        return userraise(false, ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", hset_type_name(getype(se1)), hset_type_name(getype(se2)));

    int     cnt1 = hset_cnt(se1);
    int     cnt2 = hset_cnt(se2);
    if (cnt1 != cnt2 )
        return logsimpleret(false, "Count's not equal %d vs %d", cnt1, cnt2);
    // chech the elements
    bool        res = true;
    for (int i = 0; i < se1->sz; i++)
        if (!find_elems(se1->table[i], se2) ){
            res = false;
            break;
        }

    return logsimpleret(res, "Equal %s", bool_str(res) );
}

// check NOT equality as SET
bool                        hset_noteq(const hset *restrict se1, const hset *restrict se2){
    invraise(se1 != 0 && se2 != 0, "Null pointers");

    if (getype(se1) != getype(se2) )
        return userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", hset_type_name(getype(se1)), hset_type_name(getype(se2)));

    int     cnt1 = hset_cnt(se1);
    int     cnt2 = hset_cnt(se2);
    if (cnt1 != cnt2 )
        return logsimpleret(true, "Not equal counts %d vs %d", cnt1, cnt2);
    bool    res = false;
    for (int i = 0; i < se1->sz; i++)
        if (!find_elems(se1->table[i], se2) ){
            res = true;
            break;  // found at least 1 elem which not match to se2
        }
    return logsimpleret(res,  "Equal %s", bool_str(!res) );
}

int                         hset_loadanyarr(hset *restrict se, const void *arr, int sz, hset_type typ){ 
    invraise(arr != 0 && sz > 0 && sz < INT_MAX / 4, "Incorrent input %p - %d", arr, sz);
    if (int_notin(typ, HSET_INT, HSET_LONG, HSET_DBL, HSET_PTR) )
        userraiseint(ERR_UNSUPPORTED_TYPE, "%d - %s", typ, hset_type_name(typ) );

    int         cnt = 0;
    while (cnt < sz){
        // not sure if that pretty good
        if (! hset_set(se, hset_createarrval(arr, cnt, typ) ) )
            userraiseint(ERR_UNABLE_ALLOCATE, "Unable to add %d", cnt);
        cnt++;
    }
    return logsimpleret(cnt, "Loaded %d", cnt);
}

bool                        hset_in(const hset *restrict se1, const hset *restrict se2){
    invraise(se1 != 0 && se2 != 0, "Null pointers");

    if (getype(se1) != getype(se2) )
        return userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", hset_type_name(getype(se1)), hset_type_name(getype(se2)));
    // check that EVERY element in se1 exists in se2
    for (int i = 0; i < se1->sz; i++){
        const hset_elem *el = se1->table[i];
        while (el){
            if (!hset_get(se2, el->v) )
                return logsimpleret(false, "Not matched as in");
            el = el->next;
        }
    }
    return logsimpleret(true, "Matched as in");
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

// ------------------------- TEST 5 ---------------------------------
static TestStatus
tf5(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: clone and compare", ++subnum);
    {
        Array   arr = IArray_create(200, ARRAY_RND);

        hset    se1 = hset_fromiarr(arr.iv, arr.len);
        int     elem = arr.iv[0];   // save one
        Arrayfree(arr);

        hset    se2 = hset_clone(&se1);

        test_validatefree(
            hset_eq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be equal!"
        );
        // remove one elem and check again
        test_validatefree(
            hset_del(&se1, HSET_INTVALUE(elem) ), (hset_free(&se1), hset_free(&se2) ), "Element %d must exists in se1", elem
        );
        test_validatefree(
            !hset_eq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be NOT equal after deleting %d!", elem
        );
        test_validatefree(
            hset_del(&se2, HSET_INTVALUE(elem) ), (hset_free(&se1), hset_free(&se2) ), "Element %d must exists in se2", elem
        );
        test_validatefree(
            hset_eq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be equal again!"
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    test_sub("subtest %d: compare with different hash table size", ++subnum);
    {
        // TODO:
        Array   arr = IArray_create(200, ARRAY_RND);
        // create from array
        hset    se1 = hset_fromiarr(arr.iv, Arraylen(arr) );
        // manually creating, small
        hset    se2 = hset_init(100, HSET_INT);
        hset_loadiarr(&se2, arr.iv, Arraylen(arr) );
        test_validatefree(
            hset_eq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be equal!"
        );
        // reload se1
        hset_loadiarr(&se1, arr.iv, Arraylen(arr) );
        test_validatefree(
            hset_eq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be equal again!"
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 6 ---------------------------------
static TestStatus
tf6(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: clone and !=", ++subnum);
    {
        Array   arr = IArray_create(200, ARRAY_RND);

        hset    se1 = hset_fromiarr(arr.iv, arr.len);
        int     elem = arr.iv[0];   // save one
        Arrayfree(arr);

        hset    se2 = hset_clone(&se1);

        test_validatefree(
            hset_noteq(&se1, &se2) == false, (hset_free(&se1), hset_free(&se2) ), "Must be equal (not equalt must return false)!"
        );
        // remove one elem and check again
        test_validatefree(
            hset_del(&se1, HSET_INTVALUE(elem) ), (hset_free(&se1), hset_free(&se2) ), "Element %d must exists in se1", elem
        );
        test_validatefree(
            hset_noteq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be NOT equal (returns true) after deleting %d!", elem
        );
        test_validatefree(
            hset_del(&se2, HSET_INTVALUE(elem) ), (hset_free(&se1), hset_free(&se2) ), "Element %d must exists in se2", elem
        );
        test_validatefree(
            hset_noteq(&se1, &se2) == false, (hset_free(&se1), hset_free(&se2) ), "Must be equal again (returns false)!"
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    test_sub("subtest %d: != with different hash table size", ++subnum);
    {
        Array   arr = IArray_create(150, ARRAY_RND);
        int     elem = arr.iv[0];   // save one
        // create from array
        hset    se1 = hset_fromiarr(arr.iv, Arraylen(arr) );
        // manually creating, small
        hset    se2 = hset_init(50, HSET_INT);
        hset_loadiarr(&se2, arr.iv, Arraylen(arr) );
        test_validatefree(
            hset_noteq(&se1, &se2) == false, (hset_free(&se1), hset_free(&se2) ), "Must be equal (returns false)!"
        );
        // reload se1
        hset_loadiarr(&se1, arr.iv, Arraylen(arr) );
        test_validatefree(
            hset_noteq(&se1, &se2) == false, (hset_free(&se1), hset_free(&se2) ), "Must be equal again (return false)!"
        );
        test_validatefree(
            hset_del(&se1, HSET_INTVALUE(elem) ), (hset_free(&se1), hset_free(&se2) ), "Element %d must exists in se1", elem
        );
        test_validatefree(
            hset_noteq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be not equal after deleting %d (return tur)!", elem
        );
        test_validatefree(
            hset_del(&se2, HSET_INTVALUE(elem) ), (hset_free(&se1), hset_free(&se2) ), "Element %d must exists in se2", elem
        );
        test_validatefree(
            hset_noteq(&se1, &se2) == false, (hset_free(&se1), hset_free(&se2) ),
            "Must be equal again after deleting %d from se2 (return false)!", elem
        );

        hset_free(&se1);
        hset_free(&se2);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 7 ---------------------------------
static TestStatus
tf7(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: cloneas int -> long", ++subnum);
    {
        Array   arr = IArray_create(200, ARRAY_ASC);

        hset    se1 = hset_fromiarr(arr.iv, arr.len);
        Arrayfree(arr);

        hset    se2 = hset_cloneas(&se1, HSET_LONG);

        int     sz1 = se1.sz, sz2 = se2.sz;
        test_validatefree(
            se1.sz == se2.sz, (hset_free(&se1), hset_free(&se2) ), "Must have equal table size %d : %d", sz1, sz2
        );

        sz1 = hset_cnt(&se1);
        sz2 = hset_cnt(&se2);
        test_validatefree(
            se1.sz == se2.sz, (hset_free(&se1), hset_free(&se2) ), "Must have equal element count %d : %d", sz1, sz2
        );

        // compare directly!
        for (int i = 0; i < se1.sz; i++){
            const hset_elem *el = se1.table[i];
            while (el){
                int val = el->v.ival;
                test_validatefree(
                    hset_get(&se2, HSET_LONGVALUE(val) ), (hset_free(&se1), hset_free(&se2) ), "Unable to find elem %d in se2", val
                );
                el = el->next;
            }
        }
        hset_free(&se1);
        hset_free(&se2);
    }
    test_sub("subtest %d: cloneas int -> double", ++subnum);
    {
        Array   arr = IArray_create(110, ARRAY_ASC);

        hset    se1 = hset_fromiarr(arr.iv, arr.len);
        Arrayfree(arr);

        hset    se2 = hset_cloneas(&se1, HSET_DBL);

        int     sz1 = se1.sz, sz2 = se2.sz;
        test_validatefree(
            se1.sz == se2.sz, (hset_free(&se1), hset_free(&se2) ), "Must have equal table size %d : %d", sz1, sz2
        );

        sz1 = hset_cnt(&se1);
        sz2 = hset_cnt(&se2);
        test_validatefree(
            se1.sz == se2.sz, (hset_free(&se1), hset_free(&se2) ), "Must have equal element count %d : %d", sz1, sz2
        );

        // compare directly! is it correct to compare int vs double?
        for (int i = 0; i < se1.sz; i++){
            const hset_elem *el = se1.table[i];
            while (el){
                int val = el->v.ival;
                test_validatefree(
                    hset_get(&se2, HSET_DBLVALUE(val) ), (hset_free(&se1), hset_free(&se2) ), "Unable to find elem %d in se2", val
                );
                el = el->next;
            }
        }
        hset_free(&se1);
        hset_free(&se2);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 8 ---------------------------------
static TestStatus
tf8(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: empty in nonempty", ++subnum);
    {
        int vals[] = {1, 3, 5, 7, 9};
        hset empty    = hset_init(10, HSET_INT);
        hset nonempty = hset_fromiarr(vals, 5);

        test_validatefree(
            hset_in(&empty, &nonempty), (hset_free(&empty), hset_free(&nonempty) ),
            "Empty set should be subset of any set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }

    test_sub("subtest %d: nonempty not in empty", ++subnum);
    {
        int vals[] = {1, 3, 5, 7, 9};
        hset empty    = hset_init(10, HSET_INT);
        hset nonempty = hset_fromiarr(vals, 5);

        test_validatefree(
            !hset_in(&nonempty, &empty), (hset_free(&empty), hset_free(&nonempty) ),
            "Non-empty set should NOT be subset of empty set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }

    test_sub("subtest %d: subset in superset", ++subnum);
    {
        int all_vals[] = {1, 3, 5, 7, 9};
        int sub_vals[] = {1, 5, 9};

        hset superset = hset_fromiarr(all_vals, 5);
        hset subset   = hset_init(10, HSET_INT);
        for (int i = 0; i < 3; i++) {
            test_validatefree(
                hset_set(&subset, HSET_INTVALUE(sub_vals[i] ) ), ( hset_free(&superset), hset_free(&subset) ),
                "Failed to add element to subset %d", i
            );
        }

        test_validatefree(
            hset_in(&subset, &superset), (hset_free(&superset), hset_free(&subset) ),
            "Subset should be in superset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }

    test_sub("subtest %d: superset not in subset", ++subnum);
    {
        int all_vals[] = {1, 3, 5, 7, 9};
        int sub_vals[] = {1, 5, 9};

        hset superset = hset_fromiarr(all_vals, 5);
        hset subset   = hset_init(10, HSET_INT);
        for (int i = 0; i < 3; i++) {
            test_validatefree(
                hset_set(&subset, HSET_INTVALUE(sub_vals[i] ) ), ( hset_free(&superset), hset_free(&subset) ),
                "Failed to add element to subset %d", i
            );
        }

        test_validatefree(
            !hset_in(&superset, &subset), (hset_free(&superset), hset_free(&subset) ),
            "Superset should NOT be in subset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }

    test_sub("subtest %d: type mismatch raise SIGINT", ++subnum);
    {
        int int_vals[] = {1, 2, 3};
        hset int_set = hset_fromiarr(int_vals, 3);
        hset dbl_set = hset_init(10, HSET_DBL);
        hset_set(&dbl_set, HSET_DBLVALUE(1.0));
        hset_set(&dbl_set, HSET_DBLVALUE(2.0));
        if (!try () ){
            test_validatefree(
                !hset_in(&int_set, &dbl_set),
                (hset_free(&int_set), hset_free(&dbl_set)),
                "Different types should never be subset"
            );
        } else {
            //err_printstacktrace();
            hset_free(&int_set);
            hset_free(&dbl_set);
            return logret(TEST_PASSED, "done");
        }
        return logret(TEST_FAILED, "done");
    }

    return logret(TEST_PASSED, "done");
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
      , testnew(.f2 = tf4,  .num =  4, .name = "Simple count test"                          , .desc="", .mandatory=true)
      , testnew(.f2 = tf5,  .num =  5, .name = "Comparation simple test"                    , .desc="", .mandatory=true)
      , testnew(.f2 = tf6,  .num =  6, .name = "Not equal simple test"                      , .desc="", .mandatory=true)
      , testnew(.f2 = tf7,  .num =  7, .name = "Cloneas simple test"                        , .desc="", .mandatory=true)
      , testnew(.f2 = tf8,  .num =  8, .name = "Hset_in simple test"                        , .desc="", .mandatory=true)
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HSETTESTING */

