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
                //newel->v = el->v;
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

static inline int           count_elemlist(const hset_elem *el){
    int     cnt = 0;
    while (el){
        cnt++;
        el = el->next;
    }
    return cnt;
}
static int                   hset_calc_cnt(const hset *se){
    invraise(se != 0, "Null pointer");

    int     cnt = 0;
    for (int i = 0; i < se->sz; i++)
        if (se->table[i]){  // actually this check is excess
            cnt += count_elemlist(se->table[i] );
        }
    return logsimpleret(cnt, "Total %d", cnt);
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
// return count of deleted elems
static int                  free_elemlist(hset_elem *el, hset_type typ){
    hset_elem       *next = 0;
    int              cnt = 0;
    while (el){
        next = el->next;
        free_elem(el, typ);
        el = next;
        cnt++;
    }
    return cnt;
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
// construct via new hash table size
hset             hset_init_resize(hset *se, int newsz){
    invraise(se != 0, "Null pointer");

    if (next_prime(newsz) == (unsigned) se->sz)
        return logsimpleret(*se, "No change");
    hset    res = hset_init(newsz, getype(se) );
    // hset_foreach(se)
    // simple version via hset_set()
    for (int i = 0; i < se->sz; i++){
        hset_elem *el = se->table[i];
        while (el){
            hset_set(&res, el->v);  // but that creates new hset_elem, probably it's better to use origin one
            el = el->next;
        }
    }
    hset_free(se);  // cleanup
    *se = res;  // and fill with newly created
    return logsimpleret(res, "Resized to %d", res.sz);
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
    res.count = se->count;

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
// return new hset se1 - se2
hset             hset_init_minus(const hset *restrict se1, const hset *restrict se2){
    invraise(se1 != 0 && se2 != 0, "Null pointers");

    if (getype(se1) != getype(se2) )
         userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", hset_type_name(getype(se1)), hset_type_name(getype(se2) ) );

    // simple implementation
    hset res = hset_init(se1->sz - 1, se1->flags);
    for (int i = 0; i < se1->sz; i++){
        const hset_elem *el = se1->table[i];
        while (el){
            if (!hset_get(se2, el->v) )
                hset_set(&res, el->v);
            el = el->next;
        }
    }
    return logsimpleret(res, "Created minus - total %d", se1->count);
}
// just intersect with construct
hset             hset_init_intersect(const hset *restrict se1, const hset *restrict se2){
    invraise(se1 != 0 && se2 != 0, "Null pointers");

    if (getype(se1) != getype(se2) )
          userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", hset_type_name(getype(se1)), hset_type_name(getype(se2) ) );

    // simple implementation
    hset res = hset_init(se1->sz - 1, se1->flags);
    if (se1->count > 0 && se2->count > 0)
        for (int i = 0; i < se1->sz; i++){
            const hset_elem *el = se1->table[i];
            while (el){
                if (hset_get(se2, el->v) )  // omg
                    hset_set(&res, el->v);
                el = el->next;
            }
        }
    return logsimpleret(res, "Created intersect - total %d", res.count);
}
// simm diff with construct
hset             hset_init_symmdiff(const hset *restrict se1, const hset *restrict se2){
    invraise(se1 != 0 && se2 != 0, "Null pointers");
    if (getype(se1) != getype(se2) )
         userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", hset_type_name(getype(se1)), hset_type_name(getype(se2) ) );
    hset res = hset_init(MAX(se1->sz, se2->sz) - 1, se1->flags);

        for (int i = 0; i < se1->sz; i++){
            const hset_elem *el = se1->table[i];
            while (el){
                if (!hset_get(se2, el->v))
                    hset_set(&res, el->v);
                el = el->next;
            }
        }
        for (int i = 0; i < se2->sz; i++){
            const hset_elem *el = se2->table[i];
            while (el){
                if (!hset_get(se1, el->v))
                    hset_set(&res, el->v);
                el = el->next;
            }
        }

    return logsimpleret(res, "Created symm diff - total %d", res.count);
}

// ---------------------------------------------------------------------------
bool                        hset_validate(FILE *out, const hset *restrict se){
    logenter("...");
    if (!se){    // TODO: think about string creation via faststring here!!
        if (out)
            fprintf(out, "null pointer");
        return logerr(false, "null pointer");
    }
    if (se->sz <= 1){
        if (out)
            fprintf(out, "Incorrect sz (%d)", se->sz);
        return logerr(false, "Incorrect sz (%d)", se->sz);
    }
    // check count agaist hset_cnt()
    int cnt = hset_calc_cnt(se);
    if (se->count != cnt){
        if (out)
            fprintf(out, "Cnt %d not matched to calculated cnt %d", se->count, cnt);
        return logerr(false, "Cnt %d not matched to calculated cnt %d", se->count, cnt);
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
    bool         already_existed = false;
    hset_elem   *equal = 0, *nextel = 0;
    hset_elem   *prevel = getprevelem(se, val, &hash, &nextel, &equal);
    if (equal)
        already_existed = true;
    else {
        hset_elem *newel = create_elem(val);
        if (!newel)
            userraiseint(ERR_UNABLE_ALLOCATE, "Can't create new element");
        if (prevel)
            prevel->next = newel;
        else  // mount to root
            se->table[hash] = newel;
        newel->next = nextel;
        se->count++;        // not thread-safe ((
    }
    // hsetval_log(val, getype(se) );
    return logsimpleret(!already_existed, "Added the value prev existing %s", bool_str(already_existed) );
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
// try to delete elemenet, true if deleted, false if not found
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
    se->count--;
    //hsetval_log(val, getype(se) );
    return logsimpleret(true, "Deleted");
}

void                        hset_clean(hset *se){
    invraise(se != 0, "Null pointer");

    for (int i = 0; i < se->sz; i++)
        if (se->table[i] ){
            se->count -= free_elemlist(se->table[i], getype(se) );
            se->table[i] = 0;   // clean that chain
        }
}
// check the equality as SET!
bool                        hset_eq(const hset *restrict se1, const hset *restrict se2){
    invraise(se1 != 0 && se2 != 0, "Null pointers");

    if (getype(se1) != getype(se2) )
        return userraise(false, ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", hset_type_name(getype(se1)), hset_type_name(getype(se2) ) );

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
        hset_set(se, hset_createarrval(arr, cnt, typ) );
        cnt++;
    }
    return logsimpleret(cnt, "Loaded %d", cnt);
}
// check if all of se2 in se1 strictly or not
bool                        hset_subset_check(const hset *restrict se1, const hset *restrict se2, bool strict){
    invraise(se1 != 0 && se2 != 0, "Null pointers");

    if (getype(se1) != getype(se2) )
        return userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", hset_type_name(getype(se1) ), hset_type_name(getype(se2) ) );

    if ( (!strict && hset_cnt(se1) > hset_cnt(se2) ) ||
        (strict && hset_cnt(se1) >= hset_cnt(se2) ) )
        return logsimpleret(false, "Count of se1 %d more (%s) that count of se2 %d", hset_cnt(se1), bool_str(strict), hset_cnt(se2) );
    // check that EVERY element in se1 exists in se2
    for (int i = 0; i < se1->sz; i++){
        const hset_elem *el = se1->table[i];
        while (el){
            if (!hset_get(se2, el->v) )
                return logsimpleret(false, "Not matched as in");
            el = el->next;
        }
    }
    return logsimpleret(true, "Matched %s as in", strict ? "strict": "");
}

// se1 -= se2 as SET, returns count of deleted element
int                         hset_minus(hset *restrict se1, const hset *restrict se2){
    invraise(se1 != 0 && se2 != 0, "Null pointers");
    // TODO: rework checkers!!! at least move that into check_it()
    if (getype(se1) != getype(se2) )
        return userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", hset_type_name(getype(se1) ), hset_type_name(getype(se2) ) );

    int     cnt = 0;
    if ( !(hset_cnt(se1) == 0 || hset_cnt(se2) == 0) ){
        // foreach elements in se2 try to delete it from se1
        for (int i = 0; i < se2->sz; i++){
            const hset_elem *el = se2->table[i];
            while (el){
                cnt += (int) hset_del(se1, el->v);    // +1 if reallt deleted
                el = el->next;
            }
        }
    }
    return logsimpleret(cnt, "Removed %d elements", cnt);
}
// se1 insersect= se2 as SET
int                         hset_intersect(hset *restrict se1, const hset *restrict se2){
    invraise(se1 != 0 && se2 != 0, "Null pointers");
    // TODO: rework checkers!!! at least move that into check_it()
    if (getype(se1) != getype(se2) )
        return userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", hset_type_name(getype(se1) ), hset_type_name(getype(se2) ) );

    // foreach elements in se2 try to delete it from se1
    if (se1->count > 0)
        for (int i = 0; i < se1->sz; i++){
            const hset_elem *el = se1->table[i];
            while (el){
                hset_elem *next = el->next;
                if (!hset_get(se2, el->v) )
                    hset_del(se1, el->v);    // +1 if reallt deleted
                el = next;
            }
        }
    return logsimpleret(se1->count, "%d elements remains", se1->count);
}
// se1 symmdiff= se2 as SET
hset                       *hset_symmdiff(hset *restrict se1, const hset *restrict se2){
    invraise(se1 != 0 && se2 != 0, "Null pointers");
    // TODO: rework checkers!!! at least move that into check_it()
    if (getype(se1) != getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", hset_type_name(getype(se1) ), hset_type_name(getype(se2) ) );

    hset tmp = hset_init_symmdiff(se1, se2);
    hset_move(se1, &tmp);       // no need to free tmp!!!!
    return logsimpleret(se1, "Done, new cnt = %d", se1->count);
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
            hset_set(&se1, HSET_INTVALUE(num) ),
            hset_free(&se1), "Must be true"
        );
        hset_techfprint(logfile, &se1, 0);
        test_validatefree(
            hset_set(&se1, HSET_INTVALUE(num) ) == false,
            hset_free(&se1), "Must be false because elem %d aleady in the set", num
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
                hset_set(&se1, HSET_INTVALUE(i) ), hset_free(&se1), "Already exists %d", i
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
                hset_set(&se1, HSET_INTVALUE(i) ), hset_free(&se1), "Already exists %d", i
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
                hset_set(&se1, HSET_INTVALUE(i) ), hset_free(&se1), "Already exists %d", i
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
        hset    se1 = hset_init(10, HSET_LONG);
        int     cnt = 50;

        for (int i = 0; i < cnt; i++)
            test_validatefree(
                hset_set(&se1, HSET_LONGVALUE(i) ), hset_free(&se1), "Already exists %d", i
            );
        hset    se2 = hset_clone(&se1);
        hset_techprint(&se2, 0);
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

        test_validatefree(
            hset_validate(stdout, &se1) && hset_validate(stdout, &se2),
            (hset_free(&se1), hset_free(&se2) ), "Validation failed"
        );
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

        test_validatefree(
            hset_validate(stdout, &se1) && hset_validate(stdout, &se2),
            (hset_free(&se1), hset_free(&se2)), "Validation failed se1"
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

    test_sub("subtest %d: empty in empty", ++subnum);
    {
        hset empty1 = hset_init(10, HSET_INT);
        hset empty2 = hset_init(100, HSET_INT);

        test_validatefree(
            hset_validate(stdout, &empty1) && hset_validate(stdout, &empty1),
            (hset_free(&empty1), hset_free(&empty2) ), "Validation failed empty"
        );
        test_validatefree(
            hset_in(&empty1, &empty2), (hset_free(&empty1), hset_free(&empty2) ),
            "Empty set should be subset of empty set"
        );
        hset_free(&empty1);
        hset_free(&empty2);
    }
    test_sub("subtest %d: empty in nonempty", ++subnum);
    {
        int vals[] = {1, 3, 5, 7, 9};
        hset empty    = hset_init(10, HSET_INT);
        hset nonempty = hset_fromiarr(vals, COUNT(vals) );

        test_validatefree(
            hset_validate(stdout, &empty) && hset_validate(stdout, &nonempty),
            (hset_free(&empty), hset_free(&nonempty) ), "Validation failed empty"
        );
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
        hset nonempty = hset_fromiarr(vals, COUNT(vals) );

        test_validatefree(
            !hset_in(&nonempty, &empty), (hset_free(&empty), hset_free(&nonempty) ),
            "Non-empty set should NOT be subset of empty set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    test_sub("subtest %d: equal sets", ++subnum);
    {
        int all_vals[] = {1, 3, 5, 7, 9};

        hset superset = hset_fromiarr(all_vals, COUNT(all_vals) );
        hset subset   = hset_fromiarr(all_vals, COUNT(all_vals) );

        test_validatefree(
            hset_in(&subset, &superset), (hset_free(&superset), hset_free(&subset) ),
            "Subset should be strict in equal set"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    test_sub("subtest %d: subset in superset", ++subnum);
    {
        int all_vals[] = {1, 3, 5, 7, 9};
        int sub_vals[] = {1, 5, 9};

        hset superset = hset_fromiarr(all_vals, COUNT(all_vals) );
        hset subset   = hset_fromiarr(sub_vals, COUNT(sub_vals) );

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

        hset superset = hset_fromiarr(all_vals, COUNT(all_vals) );
        hset subset   = hset_fromiarr(sub_vals, COUNT(sub_vals) );

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

// ------------------------- TEST 9 ---------------------------------
static TestStatus
tf9(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: empty strict in empty", ++subnum);
    {
        hset empty1    = hset_init(10, HSET_INT);
        hset empty2 = hset_init(100, HSET_INT);

        test_validatefree(
            hset_validate(stdout, &empty1) && hset_validate(stdout, &empty1),
            (hset_free(&empty1), hset_free(&empty2) ), "Validation failed empty"
        );
        test_validatefree(
            hset_strictin(&empty1, &empty2) == false, (hset_free(&empty1), hset_free(&empty2) ),
            "Empty set should NOT be subset of empty set"
        );
        hset_free(&empty1);
        hset_free(&empty2);
    }
    test_sub("subtest %d: empty strict in nonempty", ++subnum);
    {
        int vals[] = {1, 3, 5, 7, 9};
        hset empty    = hset_init(10, HSET_INT);
        hset nonempty = hset_fromiarr(vals, COUNT(vals) );

        test_validatefree(
            hset_validate(stdout, &empty) && hset_validate(stdout, &nonempty),
            (hset_free(&empty), hset_free(&nonempty) ), "Validation failed empty"
        );
        test_validatefree(
            hset_strictin(&empty, &nonempty), (hset_free(&empty), hset_free(&nonempty) ),
            "Empty set should be strict subset of any set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    test_sub("subtest %d: nonempty not in empty", ++subnum);
    {
        int vals[] = {1, 3, 5, 7, 9};
        hset empty    = hset_init(10, HSET_INT);
        hset nonempty = hset_fromiarr(vals, COUNT(vals) );

        test_validatefree(
            !hset_strictin(&nonempty, &empty), (hset_free(&empty), hset_free(&nonempty) ),
            "Non-empty set should NOT be strict subset of empty set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }

    test_sub("subtest %d: subset in superset", ++subnum);
    {
        int all_vals[] = {1, 3, 5, 7, 9};
        int sub_vals[] = {1, 5, 9};

        hset superset = hset_fromiarr(all_vals, COUNT(all_vals) );
        hset subset = hset_fromiarr(sub_vals, COUNT(sub_vals) );

        test_validatefree(
            hset_strictin(&subset, &superset), (hset_free(&superset), hset_free(&subset) ),
            "Subset should be strict in superset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    test_sub("subtest %d: equal sets", ++subnum);
    {
        int all_vals[] = {1, 3, 5, 7, 9};

        hset superset = hset_fromiarr(all_vals, COUNT(all_vals) );
        hset subset   = hset_fromiarr(all_vals, COUNT(all_vals) );

        test_validatefree(
            hset_strictin(&subset, &superset) == false, (hset_free(&superset), hset_free(&subset) ),
            "Subset should NOT be strict in equal set"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    test_sub("subtest %d: superset not in subset", ++subnum);
    {
        int all_vals[] = {1, 3, 5, 7, 9};
        int sub_vals[] = {1, 5, 9};

        hset superset = hset_fromiarr(all_vals, COUNT(all_vals) );
        hset subset = hset_fromiarr(sub_vals, COUNT(sub_vals) );

        test_validatefree(
            !hset_strictin(&superset, &subset), (hset_free(&superset), hset_free(&subset) ),
            "Superset should NOT be strict in subset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 10 ---------------------------------
static TestStatus
tf10(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое минус пустое */
    test_sub("subtest %d: empty minus empty", ++subnum);
    {
        hset se1 = hset_init(10, HSET_INT);
        hset se2 = hset_init(10, HSET_INT);
        int removed = hset_minus(&se1, &se2);
        test_validatefree(
            removed == 0 && hset_cnt(&se1) == 0,
            (hset_free(&se1), hset_free(&se2)),
            "Empty minus empty: removed=%d, cnt=%d (expected 0,0)",
            removed, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 2. Пустое минус непустое */
    test_sub("subtest %d: empty minus non-empty", ++subnum);
    {
        hset empty = hset_init(10, HSET_INT);
        int vals[] = {1, 2, 3};
        hset nonempty = hset_fromiarr(vals, COUNT(vals) );
        int removed = hset_minus(&empty, &nonempty);
        test_validatefree(
            removed == 0 && hset_cnt(&empty) == 0,
            (hset_free(&empty), hset_free(&nonempty)),
            "Empty minus non-empty: removed=%d, cnt=%d (expected 0,0)",
            removed, hset_cnt(&empty)
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }

    /* 3. Непустое минус пустое */
    test_sub("subtest %d: non-empty minus empty", ++subnum);
    {
        int vals[] = {10, 20, 30};
        hset se1 = hset_fromiarr(vals, COUNT(vals) );
        hset empty = hset_init(10, HSET_INT);

        int cnt_before = hset_cnt(&se1);
        int removed = hset_minus(&se1, &empty);
        test_validatefree(
            removed == 0 && hset_cnt(&se1) == cnt_before &&
            hset_get(&se1, HSET_INTVALUE(10)) &&
            hset_get(&se1, HSET_INTVALUE(20)) &&
            hset_get(&se1, HSET_INTVALUE(30)),
            (hset_free(&se1), hset_free(&empty)),
            "Non-empty minus empty: removed=%d, cnt=%d (expected 0,%d)",
            removed, hset_cnt(&se1), cnt_before
        );
        hset_free(&se1);
        hset_free(&empty);
    }

    /* 4. Множество минус его копия (все элементы должны быть удалены) */
    test_sub("subtest %d: set minus its copy", ++subnum);
    {
        int vals[] = {5, 10, 15, 20};
        hset se1 = hset_fromiarr(vals, COUNT(vals) );
        hset se2 = hset_clone(&se1);

        //hset_techprint(&se1, 0);
        //hset_techprint(&se2, 0);
        int cnt_before = hset_cnt(&se1);
        int removed = hset_minus(&se1, &se2);
        test_validatefree(
            removed == cnt_before && hset_cnt(&se1) == 0,
            (hset_free(&se1), hset_free(&se2)),
            "Set minus its copy: removed=%d, cnt=%d (expected %d, 0)",
            removed, hset_cnt(&se1), cnt_before
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 5. Непустое минус строгое подмножество */
    test_sub("subtest %d: non-empty minus subset", ++subnum);
    {
        int all_vals[] = {1, 2, 3, 4, 5, 6};
        int sub_vals[] = {2, 5};
        hset se1 = hset_fromiarr(all_vals, COUNT(all_vals) );
        hset se2 = hset_fromiarr(sub_vals, COUNT(sub_vals) );

        int removed = hset_minus(&se1, &se2);
        int ok = (removed == 2) && (hset_cnt(&se1) == 4) &&
                 !hset_get(&se1, HSET_INTVALUE(2)) &&
                 !hset_get(&se1, HSET_INTVALUE(5)) &&
                  hset_get(&se1, HSET_INTVALUE(1)) &&
                  hset_get(&se1, HSET_INTVALUE(3)) &&
                  hset_get(&se1, HSET_INTVALUE(4)) &&
                  hset_get(&se1, HSET_INTVALUE(6));
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Non-empty minus subset: removed=%d, cnt=%d (expected 2,4); element check failed",
            removed, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 6. Непустое минус непересекающееся множество */
    test_sub("subtest %d: non-empty minus disjoint", ++subnum);
    {
        int vals1[] = {7, 8, 9};
        int vals2[] = {1, 2, 3};
        hset se1 = hset_fromiarr(vals1, COUNT(vals1) );
        hset se2 = hset_fromiarr(vals2, COUNT(vals2) );

        int cnt_before = hset_cnt(&se1);
        int removed = hset_minus(&se1, &se2);
        int ok = (removed == 0) && (hset_cnt(&se1) == cnt_before) &&
                 hset_get(&se1, HSET_INTVALUE(7)) &&
                 hset_get(&se1, HSET_INTVALUE(8)) &&
                 hset_get(&se1, HSET_INTVALUE(9));
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Non-empty minus disjoint: removed=%d, cnt=%d (expected 0,%d)",
            removed, hset_cnt(&se1), cnt_before
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 7. Удаление, когда se2 содержит лишние элементы */
    test_sub("subtest %d: se2 with extra elements", ++subnum);
    {
        int vals1[] = {100, 200, 300};
        int vals2[] = {200, 400, 500};
        hset se1 = hset_fromiarr(vals1, COUNT(vals1) );
        hset se2 = hset_fromiarr(vals2, COUNT(vals2));

        int removed = hset_minus(&se1, &se2);
        int ok = (removed == 1) && (hset_cnt(&se1) == 2) &&
                 hset_get(&se1, HSET_INTVALUE(100)) &&
                 hset_get(&se1, HSET_INTVALUE(300)) &&
                 !hset_get(&se1, HSET_INTVALUE(200));
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "se2 with extra elements: removed=%d, cnt=%d (expected 1,2)",
            removed, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 8. Несовпадение типов – ожидаем аварийный сигнал */
    test_sub("subtest %d: type mismatch raises SIGINT", ++subnum);
    {
        int int_vals[] = {1, 2, 3};
        hset int_set = hset_fromiarr(int_vals, COUNT(int_vals) );
        hset dbl_set = hset_init(10, HSET_DBL);
        hset_set(&dbl_set, HSET_DBLVALUE(1.0));

        bool signal_caught = try();
        if (signal_caught) {
            hset_free(&int_set);
            hset_free(&dbl_set);
            return logret(TEST_PASSED, "Signal caught as expected");
        } else {
            hset_free(&int_set);
            hset_free(&dbl_set);
            return logerr(TEST_FAILED, "Expected signal for type mismatch, but none was raised");
        }
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 11 ---------------------------------
static TestStatus
tf11(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое минус пустое = пустое */
    test_sub("subtest %d: empty minus empty", ++subnum);
    {
        hset se1 = hset_init(10, HSET_INT);
        hset se2 = hset_init(10, HSET_INT);
        hset res = hset_init_minus(&se1, &se2);

        int ok = (hset_cnt(&res) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Empty minus empty: cnt=%d (expected 0)", hset_cnt(&res)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 2. Непустое минус пустое = копия исходного (все элементы) */
    test_sub("subtest %d: non-empty minus empty", ++subnum);
    {
        int vals[] = {3, 7, 11, 42};
        hset se1 = hset_fromiarr(vals, COUNT(vals));
        hset empty = hset_init(10, HSET_INT);
        hset res = hset_init_minus(&se1, &empty);

        int ok = (hset_cnt(&res) == COUNT(vals));
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&res, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty), hset_free(&res)),
            "Non-empty minus empty: cnt=%d (expected %d)", hset_cnt(&res), COUNT(vals)
        );
        hset_free(&se1);
        hset_free(&empty);
        hset_free(&res);
    }

    /* 3. Пустое минус непустое = пустое */
    test_sub("subtest %d: empty minus non-empty", ++subnum);
    {
        int vals[] = {100, 200};
        hset empty = hset_init(10, HSET_INT);
        hset se2 = hset_fromiarr(vals, COUNT(vals));
        hset res = hset_init_minus(&empty, &se2);

        int ok = (hset_cnt(&res) == 0);
        test_validatefree(
            ok,
            (hset_free(&empty), hset_free(&se2), hset_free(&res)),
            "Empty minus non-empty: cnt=%d (expected 0)", hset_cnt(&res)
        );
        hset_free(&empty);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 4. Множество минус его копия = пустое (все элементы удаляются) */
    test_sub("subtest %d: set minus its copy", ++subnum);
    {
        int vals[] = {5, 10, 15, 20};
        hset se1 = hset_fromiarr(vals, COUNT(vals));
        hset se2 = hset_clone(&se1);  // или ручное заполнение, но clone уже проверен
        hset res = hset_init_minus(&se1, &se2);

        int ok = (hset_cnt(&res) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Set minus its copy: cnt=%d (expected 0)", hset_cnt(&res)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 5. Частичное перекрытие: элементы, отсутствующие в se2, сохраняются */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int vals1[] = {1, 2, 3, 4, 5, 6};
        int vals2[] = {2, 5};
        hset se1 = hset_fromiarr(vals1, COUNT(vals1));
        hset se2 = hset_init(10, HSET_INT);
        for (int i = 0; i < COUNT(vals2); i++)
            hset_set(&se2, HSET_INTVALUE(vals2[i]));

        hset res = hset_init_minus(&se1, &se2);

        // Ожидаем, что останутся {1,3,4,6}
        int expected_vals[] = {1, 3, 4, 6};
        int ok = (hset_cnt(&res) == COUNT(expected_vals));
        for (int i = 0; ok && i < COUNT(expected_vals); i++)
            ok = hset_get(&res, HSET_INTVALUE(expected_vals[i]));

        // Дополнительно убедимся, что удалённых нет
        if (ok) {
            ok = !hset_get(&res, HSET_INTVALUE(2)) &&
                 !hset_get(&res, HSET_INTVALUE(5));
        }

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Partial overlap: cnt=%d (expected %d)", hset_cnt(&res), COUNT(expected_vals)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 6. Непересекающиеся множества: все элементы se1 должны остаться */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int vals1[] = {7, 8, 9};
        int vals2[] = {1, 2, 3};
        hset se1 = hset_fromiarr(vals1, COUNT(vals1));
        hset se2 = hset_fromiarr(vals2, COUNT(vals2));
        hset res = hset_init_minus(&se1, &se2);

        int ok = (hset_cnt(&res) == COUNT(vals1));
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&res, HSET_INTVALUE(vals1[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Disjoint sets: cnt=%d (expected %d)", hset_cnt(&res), COUNT(vals1)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }
    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 12 ---------------------------------

static TestStatus
tf12(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    // subtest 1: empty intersect empty => empty
    test_sub("subtest %d: empty intersect empty", ++subnum);
    {
        hset se1 = hset_init(10, HSET_INT);
        hset se2 = hset_init(100, HSET_INT);

        int remaining = hset_intersect(&se1, &se2);
        test_validatefree(
            remaining == 0 && hset_cnt(&se1) == 0,
            (hset_free(&se1), hset_free(&se2)),
            "Empty intersect empty: remaining=%d, cnt=%d (expected 0,0)",
            remaining, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    // subtest 2: non-empty intersect empty => empty (в se1 должно быть пусто)
    test_sub("subtest %d: non-empty intersect empty", ++subnum);
    {
        int vals[] = {1, 2, 3};
        hset se1 = hset_fromiarr(vals, COUNT(vals));
        hset empty = hset_init(10, HSET_INT);

        int remaining = hset_intersect(&se1, &empty);
        test_validatefree(
            remaining == 0 && hset_cnt(&se1) == 0,
            (hset_free(&se1), hset_free(&empty)),
            "Non-empty intersect empty: remaining=%d, cnt=%d (expected 0,0)",
            remaining, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&empty);
    }

    // subtest 3: empty intersect non-empty => empty (se1 пустое, результат пустой)
    test_sub("subtest %d: empty intersect non-empty", ++subnum);
    {
        int vals[] = {10, 20};
        hset empty = hset_init(10, HSET_INT);
        hset se2 = hset_fromiarr(vals, COUNT(vals));

        int remaining = hset_intersect(&empty, &se2);
        test_validatefree(
            remaining == 0 && hset_cnt(&empty) == 0,
            (hset_free(&empty), hset_free(&se2)),
            "Empty intersect non-empty: remaining=%d, cnt=%d (expected 0,0)",
            remaining, hset_cnt(&empty)
        );
        hset_free(&empty);
        hset_free(&se2);
    }

    // subtest 4: одинаковые множества => после пересечения должны остаться все элементы
    test_sub("subtest %d: identical sets", ++subnum);
    {
        int vals[] = {7, 8, 9};
        hset se1 = hset_fromiarr(vals, COUNT(vals));
        hset se2 = hset_clone(&se1);  // используем проверенный clone

        int expected_cnt = COUNT(vals);
        int remaining = hset_intersect(&se1, &se2);
        int ok = (remaining == expected_cnt) && (hset_cnt(&se1) == expected_cnt);
        for (int i = 0; ok && i < expected_cnt; i++)
            ok = hset_get(&se1, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Identical sets: remaining=%d, cnt=%d (expected %d)",
            remaining, hset_cnt(&se1), expected_cnt
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    // subtest 5: частичное перекрытие
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int vals1[] = {1, 2, 3, 4, 5};
        int vals2[] = {2, 4, 6};
        hset se1 = hset_fromiarr(vals1, COUNT(vals1));
        hset se2 = hset_fromiarr(vals2, COUNT(vals2));

        int remaining = hset_intersect(&se1, &se2);
        int expected_remaining = 2; // 2 и 4
        int ok = (remaining == expected_remaining) && (hset_cnt(&se1) == expected_remaining);
        if (ok) {
            ok = hset_get(&se1, HSET_INTVALUE(2)) &&
                 hset_get(&se1, HSET_INTVALUE(4)) &&
                 !hset_get(&se1, HSET_INTVALUE(1)) &&
                 !hset_get(&se1, HSET_INTVALUE(3)) &&
                 !hset_get(&se1, HSET_INTVALUE(5));
        }
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Partial overlap: remaining=%d, cnt=%d (expected %d)",
            remaining, hset_cnt(&se1), expected_remaining
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    // subtest 6: непересекающиеся множества => результат должен быть пустым
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int vals1[] = {100, 200};
        int vals2[] = {300, 400};
        hset se1 = hset_fromiarr(vals1, COUNT(vals1));
        hset se2 = hset_fromiarr(vals2, COUNT(vals2));

        int remaining = hset_intersect(&se1, &se2);
        test_validatefree(
            remaining == 0 && hset_cnt(&se1) == 0,
            (hset_free(&se1), hset_free(&se2)),
            "Disjoint sets: remaining=%d, cnt=%d (expected 0,0)",
            remaining, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 13 ---------------------------------

static TestStatus
tf13(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое с пустым -> пустое */
    test_sub("subtest %d: empty intersect empty", ++subnum);
    {
        hset se1 = hset_init(10, HSET_INT);
        hset se2 = hset_init(10, HSET_INT);
        hset res = hset_init_intersect(&se1, &se2);

        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&se1) == 0) && (hset_cnt(&se2) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Empty intersect empty: res=%d, se1=%d, se2=%d (expected all 0)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 2. Непустое с пустым -> пустое, se1 не изменилось */
    test_sub("subtest %d: non-empty intersect empty", ++subnum);
    {
        int vals[] = {10, 20, 30};
        hset se1 = hset_fromiarr(vals, COUNT(vals));
        hset empty = hset_init(10, HSET_INT);
        hset res = hset_init_intersect(&se1, &empty);

        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&se1) == COUNT(vals)) && (hset_cnt(&empty) == 0);
        // дополнительно убедимся, что элементы se1 на месте
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty), hset_free(&res)),
            "Non-empty intersect empty: res=%d, se1=%d, empty=%d (expected res=0, se1=%d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&empty), COUNT(vals)
        );
        hset_free(&se1);
        hset_free(&empty);
        hset_free(&res);
    }

    /* 3. Пустое с непустым -> пустое, se2 не изменилось */
    test_sub("subtest %d: empty intersect non-empty", ++subnum);
    {
        int vals[] = {5, 7};
        hset empty = hset_init(10, HSET_INT);
        hset se2 = hset_fromiarr(vals, COUNT(vals));
        hset res = hset_init_intersect(&empty, &se2);

        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&empty) == 0) && (hset_cnt(&se2) == COUNT(vals));
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se2, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&empty), hset_free(&se2), hset_free(&res)),
            "Empty intersect non-empty: res=%d, empty=%d, se2=%d (expected res=0, se2=%d)",
            hset_cnt(&res), hset_cnt(&empty), hset_cnt(&se2), COUNT(vals)
        );
        hset_free(&empty);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 4. Одинаковые множества -> все элементы */
    test_sub("subtest %d: identical sets", ++subnum);
    {
        int vals[] = {1, 2, 3, 4};
        hset se1 = hset_fromiarr(vals, COUNT(vals));
        hset se2 = hset_fromiarr(vals, COUNT(vals));
        hset res = hset_init_intersect(&se1, &se2);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&res) == expected) && (hset_cnt(&se1) == expected) && (hset_cnt(&se2) == expected);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Identical sets: res=%d, se1=%d, se2=%d (expected all %d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2), expected
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 5. Частичное перекрытие -> только общие элементы */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int vals1[] = {1, 2, 3, 4, 5};
        int vals2[] = {2, 4, 6};
        hset se1 = hset_fromiarr(vals1, COUNT(vals1));
        hset se2 = hset_fromiarr(vals2, COUNT(vals2));
        hset res = hset_init_intersect(&se1, &se2);

        int common_vals[] = {2, 4};
        int expected = COUNT(common_vals);
        int ok = (hset_cnt(&res) == expected) && (hset_cnt(&se1) == COUNT(vals1)) && (hset_cnt(&se2) == COUNT(vals2));
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, HSET_INTVALUE(common_vals[i]));
        // убедимся, что не общих элементов нет
        if (ok) {
            ok = !hset_get(&res, HSET_INTVALUE(1)) &&
                 !hset_get(&res, HSET_INTVALUE(3)) &&
                 !hset_get(&res, HSET_INTVALUE(5));
        }

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Partial overlap: res=%d, se1=%d, se2=%d (expected res=%d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2), expected
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 6. Непересекающиеся множества -> пустое */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int vals1[] = {100, 200};
        int vals2[] = {300, 400};
        hset se1 = hset_fromiarr(vals1, COUNT(vals1));
        hset se2 = hset_fromiarr(vals2, COUNT(vals2));
        hset res = hset_init_intersect(&se1, &se2);

        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&se1) == COUNT(vals1)) && (hset_cnt(&se2) == COUNT(vals2));
        // se1 и se2 не должны потерять элементы
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, HSET_INTVALUE(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, HSET_INTVALUE(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Disjoint sets: res=%d, se1=%d, se2=%d (expected res=0)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 14 ---------------------------------
static TestStatus
tf14(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое с пустым -> пустое */
    test_sub("subtest %d: empty symm diff empty", ++subnum);
    {
        hset se1 = hset_init(10, HSET_INT);
        hset se2 = hset_init(10, HSET_INT);
        hset res = hset_init_symmdiff(&se1, &se2);

        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&se1) == 0) && (hset_cnt(&se2) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Empty symm diff empty: res=%d, se1=%d, se2=%d (expected all 0)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 2. Непустое с пустым -> все элементы непустого */
    test_sub("subtest %d: non-empty symm diff empty", ++subnum);
    {
        int vals[] = {3, 7, 11};
        hset se1 = hset_fromiarr(vals, COUNT(vals));
        hset empty = hset_init(10, HSET_INT);
        hset res = hset_init_symmdiff(&se1, &empty);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&res) == expected) && (hset_cnt(&se1) == expected) && (hset_cnt(&empty) == 0);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty), hset_free(&res)),
            "Non-empty symm diff empty: res=%d, se1=%d, empty=%d (expected res=%d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&empty), expected
        );
        hset_free(&se1);
        hset_free(&empty);
        hset_free(&res);
    }

    /* 3. Пустое с непустым -> все элементы непустого */
    test_sub("subtest %d: empty symm diff non-empty", ++subnum);
    {
        int vals[] = {5, 9};
        hset empty = hset_init(10, HSET_INT);
        hset se2 = hset_fromiarr(vals, COUNT(vals));
        hset res = hset_init_symmdiff(&empty, &se2);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&res) == expected) && (hset_cnt(&empty) == 0) && (hset_cnt(&se2) == expected);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&empty), hset_free(&se2), hset_free(&res)),
            "Empty symm diff non-empty: res=%d, empty=%d, se2=%d (expected res=%d)",
            hset_cnt(&res), hset_cnt(&empty), hset_cnt(&se2), expected
        );
        hset_free(&empty);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 4. Одинаковые множества -> пустое */
    test_sub("subtest %d: identical sets", ++subnum);
    {
        int vals[] = {1, 2, 3, 4};
        hset se1 = hset_fromiarr(vals, COUNT(vals));
        hset se2 = hset_fromiarr(vals, COUNT(vals));
        hset res = hset_init_symmdiff(&se1, &se2);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&se1) == expected) && (hset_cnt(&se2) == expected);
        // элементы исходных множеств на месте
        for (int i = 0; ok && i < expected; i++) {
            if (!hset_get(&se1, HSET_INTVALUE(vals[i])) ||
                !hset_get(&se2, HSET_INTVALUE(vals[i])))
                ok = 0;
        }

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Identical sets: res=%d, se1=%d, se2=%d (expected res=0)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 5. Частичное перекрытие -> только элементы, принадлежащие ровно одному множеству */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int vals1[] = {1, 2, 3, 4, 5};
        int vals2[] = {2, 4, 6};
        hset se1 = hset_fromiarr(vals1, COUNT(vals1));
        hset se2 = hset_fromiarr(vals2, COUNT(vals2));
        hset res = hset_init_symmdiff(&se1, &se2);

        // Ожидаем {1,3,5,6}
        int expected_vals[] = {1, 3, 5, 6};
        int expected_cnt = COUNT(expected_vals);
        int ok = (hset_cnt(&res) == expected_cnt) &&
                 (hset_cnt(&se1) == COUNT(vals1)) &&
                 (hset_cnt(&se2) == COUNT(vals2));
        // Проверяем наличие ожидаемых
        for (int i = 0; ok && i < expected_cnt; i++)
            ok = hset_get(&res, HSET_INTVALUE(expected_vals[i]));
        // Проверяем отсутствие общих (2 и 4)
        if (ok) {
            ok = !hset_get(&res, HSET_INTVALUE(2)) &&
                 !hset_get(&res, HSET_INTVALUE(4));
        }
        // Убедимся, что исходные множества не изменились
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, HSET_INTVALUE(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, HSET_INTVALUE(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Partial overlap: res=%d, se1=%d, se2=%d (expected res=%d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2), expected_cnt
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 6. Непересекающиеся множества -> все элементы обоих */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int vals1[] = {100, 200};
        int vals2[] = {300, 400};
        hset se1 = hset_fromiarr(vals1, COUNT(vals1));
        hset se2 = hset_fromiarr(vals2, COUNT(vals2));
        hset res = hset_init_symmdiff(&se1, &se2);

        int expected_total = COUNT(vals1) + COUNT(vals2);
        int ok = (hset_cnt(&res) == expected_total) &&
                 (hset_cnt(&se1) == COUNT(vals1)) &&
                 (hset_cnt(&se2) == COUNT(vals2));
        // все элементы vals1 и vals2 должны быть в res
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&res, HSET_INTVALUE(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&res, HSET_INTVALUE(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Disjoint sets: res=%d, se1=%d, se2=%d (expected res=%d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2), expected_total
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 15 ---------------------------------
static TestStatus
tf15(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое с пустым -> пустое */
    test_sub("subtest %d: empty symmdiff empty", ++subnum);
    {
        hset se1 = hset_init(10, HSET_INT);
        hset se2 = hset_init(10, HSET_INT);
        hset *res = hset_symmdiff(&se1, &se2);

        int ok = (hset_cnt(&se1) == 0) && (hset_cnt(&se2) == 0) && (res == &se1);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Empty symmdiff empty: se1=%d, se2=%d, res=%p (expected se1=0, se2=0, res==&se1)",
            hset_cnt(&se1), hset_cnt(&se2), (void*)res
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 2. Непустое с пустым -> se1 без изменений */
    test_sub("subtest %d: non-empty symmdiff empty", ++subnum);
    {
        int vals[] = {3, 7, 11};
        hset se1 = hset_fromiarr(vals, COUNT(vals));
        hset empty = hset_init(10, HSET_INT);
        int cnt_before = hset_cnt(&se1);
        hset *res = hset_symmdiff(&se1, &empty);

        int ok = (hset_cnt(&se1) == cnt_before) && (hset_cnt(&empty) == 0) && (res == &se1);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty)),
            "Non-empty symmdiff empty: se1=%d, empty=%d, res=%p (expected se1=%d, empty=0)",
            hset_cnt(&se1), hset_cnt(&empty), (void*)res, cnt_before
        );
        hset_free(&se1);
        hset_free(&empty);
    }

    /* 3. Пустое с непустым -> se1 получает все элементы se2 */
    test_sub("subtest %d: empty symmdiff non-empty", ++subnum);
    {
        int vals[] = {5, 9};
        hset se1 = hset_init(10, HSET_INT);
        hset se2 = hset_fromiarr(vals, COUNT(vals));
        hset *res = hset_symmdiff(&se1, &se2);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&se1) == expected) && (hset_cnt(&se2) == expected) && (res == &se1);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&se1, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Empty symmdiff non-empty: se1=%d, se2=%d, res=%p (expected se1=%d, se2=%d)",
            hset_cnt(&se1), hset_cnt(&se2), (void*)res, expected, expected
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 4. Одинаковые множества -> se1 становится пустым */
    test_sub("subtest %d: identical sets", ++subnum);
    {
        int vals[] = {1, 2, 3, 4};
        hset se1 = hset_fromiarr(vals, COUNT(vals));
        hset se2 = hset_fromiarr(vals, COUNT(vals));
        hset *res = hset_symmdiff(&se1, &se2);

        int expected_orig = COUNT(vals);
        int ok = (hset_cnt(&se1) == 0) && (hset_cnt(&se2) == expected_orig) && (res == &se1);
        // элементы se2 должны остаться на месте
        for (int i = 0; ok && i < expected_orig; i++)
            ok = hset_get(&se2, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Identical sets: se1=%d, se2=%d, res=%p (expected se1=0, se2=%d)",
            hset_cnt(&se1), hset_cnt(&se2), (void*)res, expected_orig
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 5. Частичное перекрытие -> se1 содержит только элементы, принадлежащие ровно одному множеству */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int vals1[] = {1, 2, 3, 4, 5};
        int vals2[] = {2, 4, 6};
        hset se1 = hset_fromiarr(vals1, COUNT(vals1));
        hset se2 = hset_fromiarr(vals2, COUNT(vals2));
        hset *res = hset_symmdiff(&se1, &se2);

        // Ожидаем в se1: {1,3,5,6}
        int expected_vals[] = {1, 3, 5, 6};
        int expected_cnt = COUNT(expected_vals);
        int ok = (hset_cnt(&se1) == expected_cnt) && (hset_cnt(&se2) == COUNT(vals2)) && (res == &se1);
        // проверяем наличие ожидаемых
        for (int i = 0; ok && i < expected_cnt; i++)
            ok = hset_get(&se1, HSET_INTVALUE(expected_vals[i]));
        // проверяем отсутствие общих (2 и 4)
        if (ok) {
            ok = !hset_get(&se1, HSET_INTVALUE(2)) &&
                 !hset_get(&se1, HSET_INTVALUE(4));
        }
        // se2 не изменился
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, HSET_INTVALUE(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Partial overlap: se1=%d, se2=%d, res=%p (expected se1=%d, se2=%d)",
            hset_cnt(&se1), hset_cnt(&se2), (void*)res, expected_cnt, COUNT(vals2)
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 6. Непересекающиеся множества -> se1 содержит все элементы обоих */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int vals1[] = {100, 200};
        int vals2[] = {300, 400};
        hset se1 = hset_fromiarr(vals1, COUNT(vals1));
        hset se2 = hset_fromiarr(vals2, COUNT(vals2));
        hset *res = hset_symmdiff(&se1, &se2);

        int expected_total = COUNT(vals1) + COUNT(vals2);
        int ok = (hset_cnt(&se1) == expected_total) && (hset_cnt(&se2) == COUNT(vals2)) && (res == &se1);
        // все элементы vals1 и vals2 должны быть в se1
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, HSET_INTVALUE(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se1, HSET_INTVALUE(vals2[i]));
        // se2 не изменился
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, HSET_INTVALUE(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Disjoint sets: se1=%d, se2=%d, res=%p (expected se1=%d, se2=%d)",
            hset_cnt(&se1), hset_cnt(&se2), (void*)res, expected_total, COUNT(vals2)
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    return logret(TEST_PASSED, "done");
}
// ------------------------- TEST 16 ---------------------------------
static TestStatus
tf16(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое множество увеличиваем – остаётся пустым */
    test_sub("subtest %d: empty resize larger", ++subnum);
    {
        hset se = hset_init(10, HSET_INT);          // sz станет 11
        int old_sz = se.sz;
        hset new_se = hset_init_resize(&se, 30);    // запросим 30

        int ok = (se.sz > old_sz) && (se.sz >= 30) &&
                 (hset_cnt(&se) == 0) && (new_se.sz == se.sz);
        test_validatefree(
            ok,
            hset_free(&se),
            "Empty resize: old_sz=%d -> new_sz=%d, cnt=%d (expected cnt=0)",
            old_sz, se.sz, hset_cnt(&se)
        );
        hset_free(&se);
    }

    /* 2. Непустое множество увеличиваем – все элементы сохраняются */
    test_sub("subtest %d: non‑empty resize larger", ++subnum);
    {
        int vals[] = {1, 2, 3, 4, 5};
        hset se = hset_fromiarr(vals, COUNT(vals));
        int old_cnt = hset_cnt(&se);
        int old_sz = se.sz;
        hset_init_resize(&se, 100);

        int ok = (se.sz > old_sz) && (hset_cnt(&se) == old_cnt);
        // проверяем, что все значения доступны
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "Non‑empty resize larger: old_sz=%d -> new_sz=%d, cnt=%d (expected %d)",
            old_sz, se.sz, hset_cnt(&se), old_cnt
        );
        hset_free(&se);
    }

    /* 3. Непустое множество уменьшаем (но не меньше количества элементов) */
    test_sub("subtest %d: non‑empty resize smaller", ++subnum);
    {
        int vals[] = {10, 20, 30};
        hset se = hset_fromiarr(vals, COUNT(vals));
        int old_cnt = hset_cnt(&se);
        int old_sz = se.sz;
        hset_init_resize(&se, 3);   // 3 элемента, next_prime(3) = 3

        // Размер может стать меньше, но элементы должны остаться
        int ok = (hset_cnt(&se) == old_cnt);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "Non‑empty resize smaller: old_sz=%d -> new_sz=%d, cnt=%d (expected %d)",
            old_sz, se.sz, hset_cnt(&se), old_cnt
        );
        hset_free(&se);
    }

    /* 4. Попытка переразмерить на тот же размер – ничего не меняется */
    test_sub("subtest %d: resize to same size", ++subnum);
    {
        int vals[] = {7, 8, 9};
        hset se = hset_fromiarr(vals, COUNT(vals));
        int old_sz = se.sz;
        int old_cnt = hset_cnt(&se);
        hset_init_resize(&se, old_sz - 1); // передаём sz-1, чтобы next_prime вернуло old_sz

        int ok = (se.sz == old_sz) && (hset_cnt(&se) == old_cnt);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "Resize to same size: sz should stay %d, got %d, cnt=%d",
            old_sz, se.sz, hset_cnt(&se)
        );
        hset_free(&se);
    }

    /* 5. Последовательные ресайзы */
    test_sub("subtest %d: multiple resizes", ++subnum);
    {
        int vals[] = {100, 200, 300, 400};
        hset se = hset_fromiarr(vals, COUNT(vals));
        int cnt = hset_cnt(&se);

        hset_init_resize(&se, 50);
        hset_init_resize(&se, 10);
        hset_init_resize(&se, 200);

        int ok = (hset_cnt(&se) == cnt);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se, HSET_INTVALUE(vals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "Multiple resizes: final sz=%d, cnt=%d (expected %d)",
            se.sz, hset_cnt(&se), cnt
        );
        hset_free(&se);
    }

    /* 6. Ресайз с типом double (проверка, что тип не теряется) */
    test_sub("subtest %d: resize with double", ++subnum);
    {
        double dvals[] = {1.5, 2.5, 3.5};
        hset se = hset_fromdarr(dvals, COUNT(dvals));
        int old_cnt = hset_cnt(&se);
        hset_init_resize(&se, 50);

        int ok = (hset_cnt(&se) == old_cnt) && (se.flags == HSET_DBL);
        for (int i = 0; ok && i < COUNT(dvals); i++)
            ok = hset_get(&se, HSET_DBLVALUE(dvals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "Resize double: sz=%d, cnt=%d (expected %d)",
            se.sz, hset_cnt(&se), old_cnt
        );
        hset_free(&se);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main( /* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 =  tf1,  .num =  1, .name = "Simple init and validate test"              , .desc="", .mandatory=true)
      , testnew(.f2 =  tf2,  .num =  2, .name = "Simple init and add test"                   , .desc="", .mandatory=true)
      , testnew(.f2 =  tf3,  .num =  3, .name = "Simple clone and create from array test"    , .desc="", .mandatory=true)
      , testnew(.f2 =  tf4,  .num =  4, .name = "Simple count test"                          , .desc="", .mandatory=true)
      , testnew(.f2 =  tf5,  .num =  5, .name = "Comparation simple test"                    , .desc="", .mandatory=true)
      , testnew(.f2 =  tf6,  .num =  6, .name = "Not equal simple test"                      , .desc="", .mandatory=true)
      , testnew(.f2 =  tf7,  .num =  7, .name = "Cloneas simple test"                        , .desc="", .mandatory=true)
      , testnew(.f2 =  tf8,  .num =  8, .name = "Hset_in simple test"                        , .desc="", .mandatory=true)
      , testnew(.f2 =  tf9,  .num =  9, .name = "Hset_strictin simple test"                  , .desc="", .mandatory=true)
      , testnew(.f2 = tf10,  .num = 10, .name = "Hset_minus simple test"                     , .desc="", .mandatory=true)
      , testnew(.f2 = tf11,  .num = 11, .name = "Hset_init_minus simple test"                , .desc="", .mandatory=true)
      , testnew(.f2 = tf12,  .num = 12, .name = "hset_intersect simple test"                 , .desc="", .mandatory=true)
      , testnew(.f2 = tf13,  .num = 13, .name = "hset_init_intersect simple test"            , .desc="", .mandatory=true)
      , testnew(.f2 = tf14,  .num = 14, .name = "hset_init_symmdiff simple test"             , .desc="", .mandatory=true)
      , testnew(.f2 = tf15,  .num = 15, .name = "hset_symmdiff simple test"                  , .desc="", .mandatory=true)
      , testnew(.f2 = tf16,  .num = 16, .name = "hset_init_resize simple test"               , .desc="", .mandatory=true)
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HSETTESTING */

