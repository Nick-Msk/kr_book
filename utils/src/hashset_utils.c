/********************************************************************
                    HASH-BASED SET MODULE IMPLEMENTATION
********************************************************************/

#include "hashset_utils.h"

// ------------------------------------ Utilities ------------------------------------------


// ------------------------------------- API -----------------------------------------------

// return new hset se1 - se2
hset             hset_init_minus(const hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);

    if (hset_getype(se1) != hset_getype(se2) )
         userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(hset_getype(se1)), value64_typename(hset_getype(se2) ) );

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
hset                        hset_init_intersect(const hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);

    if (hset_getype(se1) != hset_getype(se2) )
          userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(hset_getype(se1)), value64_typename(hset_getype(se2) ) );

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
hset                        hset_init_symmdiff(const hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    if (hset_getype(se1) != hset_getype(se2) )
         userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(hset_getype(se1)), value64_typename(hset_getype(se2) ) );
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

// union with construct
hset                        hset_init_union(const hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers");
    if (hset_getype(se1) != hset_getype(se2) )
         userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(hset_getype(se1)), value64_typename(hset_getype(se2) ) );
    hset res = hset_init(MAX(se1->sz, se2->sz) - 1, se1->flags);

    for (int i = 0; i < se1->sz; i++){
        const hset_elem *el = se1->table[i];
        while (el){
            hset_set(&res, el->v);
            el = el->next;
        }
    }
    for (int i = 0; i < se2->sz; i++){
        const hset_elem *el = se2->table[i];
        while (el){
            hset_set(&res, el->v);
            el = el->next;
        }
    }
    return logsimpleret(res, "Created union - total %d", res.count);
}

// check if all of se2 in se1 strictly or not
bool                        hset_subset_check(const hset *restrict se1, const hset *restrict se2, bool strict){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);

    if (hset_getype(se1) != hset_getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(hset_getype(se1) ), value64_typename(hset_getype(se2) ) );

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
// if not exists se2 in se1
bool                        hset_notexists(const hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    // refsctor tha to common
    if (hset_getype(se1) != hset_getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(hset_getype(se1) ), value64_typename(hset_getype(se2) ) );
    HSET_FOREACH(se2, val){
        if (hset_get(se1, val) ){
            value64_fprint(logfile, val, hset_getype(se2));
            return logsimpleret(false, "Found element in se1");
        }
    }
    return logsimpleret(true, "Not found - OK");
}
// if exists any of se2 in se1
bool                        hset_any(const hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    // refsctor tha to common
    if (hset_getype(se1) != hset_getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(hset_getype(se1) ), value64_typename(hset_getype(se2) ) );
    HSET_FOREACH(se2, val){
        if (hset_get(se1, val) ){
            value64_fprint(logfile, val, hset_getype(se2));
            return logsimpleret(true, "Element of se2 Found in se1");
        }
    }
    return logsimpleret(false, "All not found - OK");
}

// se1 -= se2 as SET, returns count of deleted element
hset                       *hset_minus(hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    // TODO: rework checkers!!! at least move that into check_it()
    if (hset_getype(se1) != hset_getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(hset_getype(se1) ), value64_typename(hset_getype(se2) ) );

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
    return logsimpleret(se1, "Removed %d elements", cnt);
}

// se1 insersect= se2 as SET
hset                       *hset_intersect(hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    // TODO: rework checkers!!! at least move that into check_it()
    if (hset_getype(se1) != hset_getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(hset_getype(se1) ), value64_typename(hset_getype(se2) ) );

    // foreach elements in se2 try to delete it from se1
    int     delcnt = 0;
    for (int i = 0; i < se1->sz; i++){
        const hset_elem *el = se1->table[i];
        while (el){
            hset_elem *next = el->next;
            if (!hset_get(se2, el->v) )
                delcnt += (int) hset_del(se1, el->v);    // +1 if reallt deleted
            el = next;
        }
    }
    return logsimpleret(se1, "%d elements remains, deleted %d", se1->count, delcnt);
}

// se1 symmdiff= se2 as SET
hset                       *hset_symmdiff(hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    // TODO: rework checkers!!! at least move that into check_it()
    if (hset_getype(se1) != hset_getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(hset_getype(se1) ), value64_typename(hset_getype(se2) ) );

    // done via constructor
    hset tmp = hset_init_symmdiff(se1, se2);
    hset_moveall(se1, &tmp);       // no need to free tmp!!!!
    return logsimpleret(se1, "Done, new cnt = %d", se1->count);
}
// union= as SET
hset            *hset_union(hset *restrict se1, const hset *restrict se2){ 
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    // TODO: rework checkers!!! at least move that into check_it()
    if (hset_getype(se1) != hset_getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(hset_getype(se1) ), value64_typename(hset_getype(se2) ) );

    int         addcnt = 0;
    if (se2->count > 0){
        for (int i = 0; i < se2->sz; i++){
            const hset_elem *el = se2->table[i];
            while (el){
                addcnt += hset_set(se1, el->v);
                el = el->next;
            }
        }
    }
    return  logsimpleret(se1, "%d elements remains, added from se2 %d", se1->count, addcnt);
}

// --------------------------------------- ITERATORS ---------------------------------------

void                        hset_const_foreach(const hset *se, hset_const_proc_t proc) {
    for (int i = 0; i < se->sz; i++) {
        const hset_elem *el = se->table[i];
        while (el) {
            proc(el->v);   // передаём копию
            el = el->next;
        }
    }
}
/*      NOT USED FOR NOW
void                        hset_foreach(hset *se, hset_proc_t proc) {
    for (int i = 0; i < se->sz; i++) {
        hset_elem *el = se->table[i];
        while (el) {
            proc(&el->v);
            el = el->next;
        }
    }
}
void                        hset_modify_foreach(hset *se, hset_modify_proc_t proc) {
    for (int i = 0; i < se->sz; i++) {
        hset_elem *el = se->table[i];
        while (el) {
            hset_elem *next = el->next;
            proc(se, el);   // можно вызывать hset_del внутри
            el = next;
        }
    }
} */

// ----------------------------------------- REDUCE -----------------------------------------

hset_accum                  hset_initreduce(const hset *se, hset_accum init, hset_reduce_func func) {
    hset_accum acc = init;
    for (int i = 0; i < se->sz; i++) {
        const hset_elem *el = se->table[i];
        while (el) {
            func(&acc, el->v);
            el = el->next;
        }
    }
    return acc;
}
// ------------------------------------- REDUCE IMPL -----------------------------------------
void                        hset_sum_int(hset_accum *acc, value64 v) {
    acc->value.ival += v.ival;
    acc->count++;
}

void                        hset_count_int(hset_accum *acc, value64 v) {
    (void) v;
    acc->count++;
}

void                        hset_max_int(hset_accum *acc, value64 v) {
    if (acc->count == 0 || v.ival > acc->value.ival) {
        acc->value.ival = v.ival;
        acc->count = 1;   // помечаем, что значение уже есть
    }
}

void                        hset_min_int(hset_accum *acc, value64 v) {
    if (acc->count == 0 || v.ival < acc->value.ival) {
        acc->value.ival = v.ival;
        acc->count = 1;
    }
}

/*void                        hset_avg_int(hset_accum *acc, value64 v) {
    acc->value.ival += v.ival;
    acc->count++;
}*/

// double
void                        hset_sum_dbl(hset_accum *acc, value64 v) {
    if (isfinite(v.dval)) {
        acc->value.dval += v.dval;
        acc->count++;
    }
}

void                        hset_count_dbl(hset_accum *acc, value64 v) {
    if (isfinite(v.dval))
        acc->count++;
}

void                        hset_max_dbl(hset_accum *acc, value64 v) {
    if (!isfinite(v.dval))
        return;
    if (acc->count == 0 || v.dval > acc->value.dval) {
        acc->value.dval = v.dval;
        acc->count = 1;   // помечаем, что значение уже есть
    }
}

void                        hset_min_dbl(hset_accum *acc, value64 v) {
    if (!isfinite(v.dval))
        return;
    if (acc->count == 0 || v.dval < acc->value.dval) {
        acc->value.dval = v.dval;
        acc->count = 1;
    }
}

/*void                        hset_avg_dbl(hset_accum *acc, value64 v) {
    if (isfinite(v.dval)) {
        acc->value.dval += v.dval;
        acc->count++;
    }
}*/

// ---------------------------------------- Testing ------------------------------------------
#ifdef HASHSET_UTILS_TESTING

#include "test.h"
#include "array.h"
#include <time.h>

//types, macro for testing
// TODO: not sure
#define HSET_HAS_FS(se, path) \
    ({ \
        value64 _v = value64_createfs_asstr(path); \
        bool _res = hset_get((se), _v); \
        value64_freefs(&_v); \
        _res; \
    })

// ------------------------- TEST 8 ---------------------------------
static TestStatus
tf8(const char *name)
{
    logenter("%s", name);
    int subnum = 0;
              
    test_sub("subtest %d: empty in empty", ++subnum);
    {         
        hset empty1 = hset_init(10, VALUE64_INT);
        hset empty2 = hset_init(100, VALUE64_INT);
              
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
        hset empty    = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals) );
              
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
        hset empty    = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals) );

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

        hset superset = hset_from_intarr(all_vals, COUNT(all_vals) );
        hset subset   = hset_from_intarr(all_vals, COUNT(all_vals) );

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

        hset superset = hset_from_intarr(all_vals, COUNT(all_vals) );
        hset subset   = hset_from_intarr(sub_vals, COUNT(sub_vals) );

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

        hset superset = hset_from_intarr(all_vals, COUNT(all_vals) );
        hset subset   = hset_from_intarr(sub_vals, COUNT(sub_vals) );

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
        hset int_set = hset_from_intarr(int_vals, 3);
        hset dbl_set = hset_init(10, VALUE64_DBL);
        hset_set(&dbl_set, LITERAL64_DBL(1.0));
        hset_set(&dbl_set, LITERAL64_DBL(2.0));
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
    /* // fs
    */
    test_sub("subtest %d: empty FS in empty FS", ++subnum);
    {
        hset empty1 = hset_init_fs(10);
        hset empty2 = hset_init_fs(10);

        test_validatefree(
            hset_validate(stdout, &empty1) && hset_validate(stdout, &empty2),
            (hset_free(&empty1), hset_free(&empty2)),
            "Validation failed empty FS"
        );
        test_validatefree(
            hset_in(&empty1, &empty2),
            (hset_free(&empty1), hset_free(&empty2)),
            "Empty FS set should be subset of empty FS set"
        );
        hset_free(&empty1);
        hset_free(&empty2);
    }
    fs_alloc_check(true);
    test_sub("subtest %d: empty FS in nonempty FS", ++subnum);
    {
        hset empty    = hset_init_fs(10);
        hset nonempty = HSET_CREATEFS_ASSTR("/tmp/a", "/tmp/b");

        test_validatefree(
            hset_validate(stdout, &empty) && hset_validate(stdout, &nonempty),
            (hset_free(&empty), hset_free(&nonempty)),
            "Validation failed empty FS vs nonempty FS"
        );
        test_validatefree(
            hset_in(&empty, &nonempty),
            (hset_free(&empty), hset_free(&nonempty)),
            "Empty FS set should be subset of any FS set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: nonempty FS not in empty FS", ++subnum);
    {
        hset empty    = hset_init_fs(10);
        hset nonempty = HSET_CREATEFS_ASSTR("/tmp/x");

        test_validatefree(
            !hset_in(&nonempty, &empty),
            (hset_free(&empty), hset_free(&nonempty)),
            "Non-empty FS set should NOT be subset of empty FS set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    fs_alloc_check(true);
    test_sub("subtest %d: equal FS sets", ++subnum);
    {
        hset superset = HSET_CREATEFS_ASSTR("/tmp/1", "/tmp/2");
        hset subset   = HSET_CREATEFS_ASSTR("/tmp/1", "/tmp/2");

        test_validatefree(
            hset_in(&subset, &superset),
            (hset_free(&superset), hset_free(&subset)),
            "Equal FS sets: subset should be in superset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS subset in FS superset", ++subnum);
    {
        hset superset = HSET_CREATEFS_ASSTR("/tmp/a", "/tmp/b", "/tmp/c");
        hset subset   = HSET_CREATEFS_ASSTR("/tmp/a", "/tmp/c");

        test_validatefree(
            hset_in(&subset, &superset),
            (hset_free(&superset), hset_free(&subset)),
            "FS subset should be in superset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS superset not in FS subset", ++subnum);
    {
        hset superset = HSET_CREATEFS_ASSTR("/tmp/a", "/tmp/b", "/tmp/c");
        hset subset   = HSET_CREATEFS_ASSTR("/tmp/a", "/tmp/c");

        test_validatefree(
            !hset_in(&superset, &subset),
            (hset_free(&superset), hset_free(&subset)),
            "FS superset should NOT be in subset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    fs_alloc_check(true);
    test_sub("subtest %d: FS vs INT type mismatch raise SIGINT", ++subnum);
    {
        hset fs_set  = HSET_CREATEFS_ASSTR("/tmp/z");
        hset int_set = HSET_CREATE_INT(42);

        if (!try()) {
            test_validatefree(
                !hset_in(&fs_set, &int_set),
                (hset_free(&fs_set), hset_free(&int_set)),
                "FS set should not be subset of INT set (type mismatch)"
            );
        } else {
            hset_free(&fs_set);
            hset_free(&int_set);
            return logret(TEST_PASSED, "done");
        }
        return logret(TEST_FAILED, "done");
    }
    fs_alloc_check(true);

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
        hset empty1    = hset_init(10, VALUE64_INT);
        hset empty2 = hset_init(100, VALUE64_INT);

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
        hset empty    = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals) );

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
        hset empty    = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals) );

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

        hset superset = hset_from_intarr(all_vals, COUNT(all_vals) );
        hset subset = hset_from_intarr(sub_vals, COUNT(sub_vals) );

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

        hset superset = hset_from_intarr(all_vals, COUNT(all_vals) );
        hset subset   = hset_from_intarr(all_vals, COUNT(all_vals) );

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

        hset superset = hset_from_intarr(all_vals, COUNT(all_vals) );
        hset subset = hset_from_intarr(sub_vals, COUNT(sub_vals) );

        test_validatefree(
            !hset_strictin(&superset, &subset), (hset_free(&superset), hset_free(&subset) ),
            "Superset should NOT be strict in subset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    // fs
    test_sub("subtest %d: empty strict in empty", ++subnum);
    {
        hset empty1 = hset_init_fs(10);
        hset empty2 = hset_init_fs(100);

        test_validatefree(
            hset_validate(stdout, &empty1) && hset_validate(stdout, &empty2),
            (hset_free(&empty1), hset_free(&empty2)),
            "Validation failed empty FS"
        );
        test_validatefree(
            hset_strictin(&empty1, &empty2) == false,
            (hset_free(&empty1), hset_free(&empty2)),
            "Empty FS set should NOT be strict subset of empty FS set"
        );
        hset_free(&empty1);
        hset_free(&empty2);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: empty strict in nonempty", ++subnum);
    {
        hset empty    = hset_init_fs(10);
        hset nonempty = HSET_CREATEFS_ASSTR("/tmp/a", "/tmp/b");

        test_validatefree(
            hset_validate(stdout, &empty) && hset_validate(stdout, &nonempty),
            (hset_free(&empty), hset_free(&nonempty)),
            "Validation failed empty FS vs nonempty FS"
        );
        test_validatefree(
            hset_strictin(&empty, &nonempty),
            (hset_free(&empty), hset_free(&nonempty)),
            "Empty FS set should be strict subset of any non‑empty FS set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: nonempty not strict in empty", ++subnum);
    {
        hset empty    = hset_init_fs(10);
        hset nonempty = HSET_CREATEFS_ASSTR("/tmp/x");

        test_validatefree(
            !hset_strictin(&nonempty, &empty),
            (hset_free(&empty), hset_free(&nonempty)),
            "Non‑empty FS set should NOT be strict subset of empty FS set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: strict subset in superset", ++subnum);
    {
        hset superset = HSET_CREATEFS_ASSTR("/tmp/a", "/tmp/b", "/tmp/c");
        hset subset   = HSET_CREATEFS_ASSTR("/tmp/a", "/tmp/c");

        test_validatefree(
            hset_strictin(&subset, &superset),
            (hset_free(&superset), hset_free(&subset)),
            "Proper subset should be strict in superset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: equal sets (not strict)", ++subnum);
    {
        hset set1 = HSET_CREATEFS_ASSTR("/tmp/1", "/tmp/2");
        hset set2 = HSET_CREATEFS_ASSTR("/tmp/1", "/tmp/2");

        test_validatefree(
            hset_strictin(&set1, &set2) == false,
            (hset_free(&set1), hset_free(&set2)),
            "Equal FS sets should NOT be strict subsets of each other"
        );
        hset_free(&set1);
        hset_free(&set2);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: superset not strict in subset", ++subnum);
    {
        hset superset = HSET_CREATEFS_ASSTR("/tmp/a", "/tmp/b", "/tmp/c");
        hset subset   = HSET_CREATEFS_ASSTR("/tmp/a", "/tmp/c");

        test_validatefree(
            !hset_strictin(&superset, &subset),
            (hset_free(&superset), hset_free(&subset)),
            "Superset should NOT be strict in its proper subset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS vs INT type mismatch raise SIGINT", ++subnum);
    {
        hset fs_set  = HSET_CREATEFS_ASSTR("/tmp/z");
        hset int_set = hset_init_int(10);
        hset_set(&int_set, LITERAL64_INT(42));

        if (!try()) {
            test_validatefree(
                !hset_strictin(&fs_set, &int_set),
                (hset_free(&fs_set), hset_free(&int_set)),
                "FS set should not be strict subset of INT set (type mismatch)"
            );
        } else {
            hset_free(&fs_set);
            hset_free(&int_set);
            return logret(TEST_PASSED, "done");
        }
        return logret(TEST_FAILED, "done");
    }
    fs_alloc_check(true);

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
        hset    se1 = hset_init(10, VALUE64_INT);
        hset    se2 = hset_init(10, VALUE64_INT);
        hset   *res = hset_minus(&se1, &se2);

        int     ok = (res == &se1) && (hset_cnt(&se1) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Empty minus empty: res=%p, cnt=%d (expected res=&se1, cnt=0)",
            (void*)res, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 2. Пустое минус непустое */
    test_sub("subtest %d: empty minus non-empty", ++subnum);
    {
        hset    empty = hset_init(10, VALUE64_INT);
        int     vals[] = {1, 2, 3};
        hset    nonempty = hset_from_intarr(vals, COUNT(vals));
        hset   *res = hset_minus(&empty, &nonempty);

        int     ok = (res == &empty) && (hset_cnt(&empty) == 0) &&
                     (hset_cnt(&nonempty) == COUNT(vals));
        test_validatefree(
            ok,
            (hset_free(&empty), hset_free(&nonempty)),
            "Empty minus non-empty: res=%p, cnt=%d (expected res=&empty, cnt=0)",
            (void*)res, hset_cnt(&empty)
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    /* 3. Непустое минус пустое */
    test_sub("subtest %d: non-empty minus empty", ++subnum);
    {
        int     vals[] = {10, 20, 30};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    empty = hset_init(10, VALUE64_INT);
        int     cnt_before = hset_cnt(&se1);
        hset   *res = hset_minus(&se1, &empty);

        int     ok = (res == &se1) && (hset_cnt(&se1) == cnt_before);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty)),
            "Non-empty minus empty: res=%p, cnt=%d (expected res=&se1, cnt=%d)",
            (void*)res, hset_cnt(&se1), cnt_before
        );
        hset_free(&se1);
        hset_free(&empty);
    }

    /* 4. Множество минус его копия (все элементы должны быть удалены) */
    test_sub("subtest %d: set minus its copy", ++subnum);
    {
        int     vals[] = {5, 10, 15, 20};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    se2 = hset_from_intarr(vals, COUNT(vals));
        /*hset    se2 = hset_init(10, VALUE64_INT);
        for (int i = 0; i < COUNT(vals); i++)
            hset_set(&se2, LITERAL64_INT(vals[i])); */

        int     cnt_before = hset_cnt(&se1);
        hset   *res = hset_minus(&se1, &se2);

        int     ok = (res == &se1) && (hset_cnt(&se1) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Set minus its copy: res=%p, cnt=%d (expected res=&se1, cnt=0 after %d deleted)",
            (void*)res, hset_cnt(&se1), cnt_before
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 5. Непустое минус строгое подмножество */
    test_sub("subtest %d: non-empty minus subset", ++subnum);
    {
        int     all_vals[] = {1, 2, 3, 4, 5, 6};
        int     sub_vals[] = {2, 5};
        hset    se1 = hset_from_intarr(all_vals, COUNT(all_vals));
        hset    se2 = hset_from_intarr(sub_vals, COUNT(sub_vals));
        /* hset    se2 = hset_init(10, VALUE64_INT);
        for (int i = 0; i < COUNT(sub_vals); i++)
            hset_set(&se2, LITERAL64_INT(sub_vals[i])); */

        hset   *res = hset_minus(&se1, &se2);

        int     expected_cnt = 4;   // {1,3,4,6}
        int     ok = (res == &se1) && (hset_cnt(&se1) == expected_cnt) &&
                     !hset_get(&se1, LITERAL64_INT(2)) &&
                     !hset_get(&se1, LITERAL64_INT(5)) &&
                     hset_get(&se1, LITERAL64_INT(1)) &&
                     hset_get(&se1, LITERAL64_INT(3)) &&
                     hset_get(&se1, LITERAL64_INT(4)) &&
                     hset_get(&se1, LITERAL64_INT(6));
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Non-empty minus subset: res=%p, cnt=%d (expected res=&se1, cnt=%d)",
            (void*)res, hset_cnt(&se1), expected_cnt
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 6. Непустое минус непересекающееся множество */
    test_sub("subtest %d: non-empty minus disjoint", ++subnum);
    {
        int     vals1[] = {7, 8, 9};
        int     vals2[] = {1, 2, 3};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));

        int     cnt_before = hset_cnt(&se1);
        hset   *res = hset_minus(&se1, &se2);

        int     ok = (res == &se1) && (hset_cnt(&se1) == cnt_before);
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Non-empty minus disjoint: res=%p, cnt=%d (expected res=&se1, cnt=%d)",
            (void*)res, hset_cnt(&se1), cnt_before
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 7. Удаление, когда se2 содержит лишние элементы */
    test_sub("subtest %d: se2 with extra elements", ++subnum);
    {
        int     vals1[] = {100, 200, 300};
        int     vals2[] = {200, 400, 500};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));

        hset   *res = hset_minus(&se1, &se2);

        // Ожидаем, что останутся только 100 и 300
        int     expected_remaining[] = {100, 300};
        int     ok = (res == &se1) && (hset_cnt(&se1) == COUNT(expected_remaining));
        for (int i = 0; ok && i < COUNT(expected_remaining); i++)
            ok = hset_get(&se1, LITERAL64_INT(expected_remaining[i]));
        // Убедимся, что 200 действительно удалён
        if (ok)
            ok = !hset_get(&se1, LITERAL64_INT(200));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "se2 with extra elements: res=%p, cnt=%d (expected res=&se1, cnt=2)",
            (void*)res, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 8. Несовпадение типов – ожидаем аварийный сигнал */
    test_sub("subtest %d: type mismatch raises SIGINT", ++subnum);
    {
        int int_vals[] = {1, 2, 3};
        hset int_set = hset_from_intarr(int_vals, COUNT(int_vals) );
        hset dbl_set = hset_init(10, VALUE64_DBL);
        hset_set(&dbl_set, LITERAL64_DBL(1.0));

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
    // fs
    test_sub("subtest %d: empty minus empty", ++subnum);
    {
        hset a = hset_init_fs(10);
        hset b = hset_init_fs(10);
        hset *res = hset_minus(&a, &b);   // res == &a, a изменён

        test_validatefree(
            res->count == 0,
            (hset_free(res), hset_free(&b)),
            "Empty minus empty should be empty"
        );
        hset_free(res);
        hset_free(&b);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: empty minus nonempty", ++subnum);
    {
        hset a = hset_init_fs(10);
        hset b = HSET_CREATEFS_ASSTR("/tmp/x", "/tmp/y");
        hset *res = hset_minus(&a, &b);

        test_validatefree(
            res->count == 0,
            (hset_free(res), hset_free(&b)),
            "Empty minus nonempty should be empty"
        );
        hset_free(res);
        hset_free(&b);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: nonempty minus empty", ++subnum);
    {
        hset a = HSET_CREATEFS_ASSTR("/tmp/a", "/tmp/b", "/tmp/c");
        hset b = hset_init_fs(10);
        hset *res = hset_minus(&a, &b);

        test_validatefree(
            res->count == 3 &&
            HSET_HAS_FS(res, "/tmp/a") &&
            HSET_HAS_FS(res, "/tmp/b") &&
            HSET_HAS_FS(res, "/tmp/c"),
            (hset_free(res), hset_free(&b)),
            "A minus empty should equal A"
        );
        hset_free(res);
        hset_free(&b);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: minus with common elements", ++subnum);
    {
        hset a = HSET_CREATEFS_ASSTR("/tmp/1", "/tmp/2", "/tmp/3", "/tmp/4");
        hset b = HSET_CREATEFS_ASSTR("/tmp/3", "/tmp/4", "/tmp/5");
        hset *res = hset_minus(&a, &b);

        test_validatefree(
            res->count == 2 &&
            HSET_HAS_FS(res, "/tmp/1") &&
            HSET_HAS_FS(res, "/tmp/2") &&
            !HSET_HAS_FS(res, "/tmp/3") &&
            !HSET_HAS_FS(res, "/tmp/4") &&
            !HSET_HAS_FS(res, "/tmp/5"),
            (hset_free(res), hset_free(&b)),
            "A \\ B should contain only /tmp/1 and /tmp/2"
        );
        hset_free(res);
        hset_free(&b);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: minus with no common elements", ++subnum);
    {
        hset a = HSET_CREATEFS_ASSTR("/tmp/1", "/tmp/2");
        hset b = HSET_CREATEFS_ASSTR("/tmp/3", "/tmp/4");
        hset *res = hset_minus(&a, &b);

        test_validatefree(
            res->count == 2 &&
            HSET_HAS_FS(res, "/tmp/1") &&
            HSET_HAS_FS(res, "/tmp/2"),
            (hset_free(res), hset_free(&b)),
            "A \\ B with no overlap should equal A"
        );
        hset_free(res);
        hset_free(&b);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: minus results in empty set", ++subnum);
    {
        hset a = HSET_CREATEFS_ASSTR("/tmp/x", "/tmp/y");
        hset b = HSET_CREATEFS_ASSTR("/tmp/x", "/tmp/y");
        hset *res = hset_minus(&a, &b);

        test_validatefree(
            res->count == 0,
            (hset_free(res), hset_free(&b)),
            "A \\ B when A == B should be empty"
        );
        hset_free(res);
        hset_free(&b);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS vs INT type mismatch raises SIGINT", ++subnum);
    {
        hset fs_set  = HSET_CREATEFS_ASSTR("/tmp/z");
        hset int_set = hset_init_int(10);
        hset_set(&int_set, LITERAL64_INT(42));

        if (!try()) {
            hset *res = hset_minus(&fs_set, &int_set);   // должно вызвать ошибку
            hset_free(res);
            hset_free(&int_set);
            test_validate(false, 
                "Type mismatch should have raised SIGINT"
            );
        } else {
            hset_free(&fs_set);
            hset_free(&int_set);
            logmsg("Exception correctly raised on type mismatch");
        }
    }
    fs_alloc_check(true);

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
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_init(10, VALUE64_INT);
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
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset empty = hset_init(10, VALUE64_INT);
        hset res = hset_init_minus(&se1, &empty);

        int ok = (hset_cnt(&res) == COUNT(vals));
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&res, LITERAL64_INT(vals[i]));

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
        hset empty = hset_init(10, VALUE64_INT);
        hset se2 = hset_from_intarr(vals, COUNT(vals));
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
        hset se1 = hset_from_intarr(vals, COUNT(vals));
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
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_init(10, VALUE64_INT);
        for (int i = 0; i < COUNT(vals2); i++)
            hset_set(&se2, LITERAL64_INT(vals2[i]));

        hset res = hset_init_minus(&se1, &se2);

        // Ожидаем, что останутся {1,3,4,6}
        int expected_vals[] = {1, 3, 4, 6};
        int ok = (hset_cnt(&res) == COUNT(expected_vals));
        for (int i = 0; ok && i < COUNT(expected_vals); i++)
            ok = hset_get(&res, LITERAL64_INT(expected_vals[i]));

        // Дополнительно убедимся, что удалённых нет
        if (ok) {
            ok = !hset_get(&res, LITERAL64_INT(2)) &&
                 !hset_get(&res, LITERAL64_INT(5));
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
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset res = hset_init_minus(&se1, &se2);

        int ok = (hset_cnt(&res) == COUNT(vals1));
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&res, LITERAL64_INT(vals1[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Disjoint sets: cnt=%d (expected %d)", hset_cnt(&res), COUNT(vals1)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }
    // fs
    test_sub("subtest %d: empty minus empty", ++subnum);
    {
        hset a = hset_init_fs(10);
        hset b = hset_init_fs(10);
        hset res = hset_init_minus(&a, &b);

        test_validatefree(
            res.count == 0,
            (hset_free(&a), hset_free(&b), hset_free(&res)),
            "Empty minus empty should be empty"
        );
        hset_free(&a);
        hset_free(&b);
        hset_free(&res);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: empty minus nonempty", ++subnum);
    {
        hset a = hset_init_fs(10);
        hset b = HSET_CREATEFS_ASSTR("/tmp/x", "/tmp/y");
        hset res = hset_init_minus(&a, &b);

        test_validatefree(
            res.count == 0,
            (hset_free(&a), hset_free(&b), hset_free(&res)),
            "Empty minus nonempty should be empty"
        );
        hset_free(&a);
        hset_free(&b);
        hset_free(&res);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: nonempty minus empty", ++subnum);
    {
        hset a = HSET_CREATEFS_ASSTR("/tmp/a", "/tmp/b", "/tmp/c");
        hset b = hset_init_fs(10);
        hset res = hset_init_minus(&a, &b);

        test_validatefree(
            res.count == 3 &&
            HSET_HAS_FS(&res, "/tmp/a") &&
            HSET_HAS_FS(&res, "/tmp/b") &&
            HSET_HAS_FS(&res, "/tmp/c"),
            (hset_free(&a), hset_free(&b), hset_free(&res)),
            "A minus empty should equal A"
        );
        hset_free(&a);
        hset_free(&b);
        hset_free(&res);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: minus with common elements", ++subnum);
    {
        hset a = HSET_CREATEFS_ASSTR("/tmp/1", "/tmp/2", "/tmp/3", "/tmp/4");
        hset b = HSET_CREATEFS_ASSTR("/tmp/3", "/tmp/4", "/tmp/5");
        hset res = hset_init_minus(&a, &b);

        test_validatefree(
            res.count == 2 &&
            HSET_HAS_FS(&res, "/tmp/1") &&
            HSET_HAS_FS(&res, "/tmp/2") &&
            !HSET_HAS_FS(&res, "/tmp/3") &&
            !HSET_HAS_FS(&res, "/tmp/4") &&
            !HSET_HAS_FS(&res, "/tmp/5"),
            (hset_free(&a), hset_free(&b), hset_free(&res)),
            "A \\ B should contain only /tmp/1 and /tmp/2"
        );
        hset_free(&a);
        hset_free(&b);
        hset_free(&res);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: minus with no common elements", ++subnum);
    {
        hset a = HSET_CREATEFS_ASSTR("/tmp/1", "/tmp/2");
        hset b = HSET_CREATEFS_ASSTR("/tmp/3", "/tmp/4");
        hset res = hset_init_minus(&a, &b);

        test_validatefree(
            res.count == 2 &&
            HSET_HAS_FS(&res, "/tmp/1") &&
            HSET_HAS_FS(&res, "/tmp/2"),
            (hset_free(&a), hset_free(&b), hset_free(&res)),
            "A \\ B with no overlap should equal A"
        );
        hset_free(&a);
        hset_free(&b);
        hset_free(&res);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: minus results in empty set", ++subnum);
    {
        hset a = HSET_CREATEFS_ASSTR("/tmp/x", "/tmp/y");
        hset b = HSET_CREATEFS_ASSTR("/tmp/x", "/tmp/y");
        hset res = hset_init_minus(&a, &b);

        test_validatefree(
            res.count == 0,
            (hset_free(&a), hset_free(&b), hset_free(&res)),
            "A \\ B when A == B should be empty"
        );
        hset_free(&a);
        hset_free(&b);
        hset_free(&res);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS vs INT type mismatch raises SIGINT", ++subnum);
    {
        hset fs_set  = HSET_CREATEFS_ASSTR("/tmp/z");
        hset int_set = hset_init_int(10);
        hset_set(&int_set, LITERAL64_INT(42));

        if (!try()) {
            hset res = hset_init_minus(&fs_set, &int_set);   // должно вызвать ошибку
            hset_free(&fs_set);
            hset_free(&int_set);
            hset_free(&res);
            test_validate(false, 
                "Type mismatch should have raised SIGINT"
            );
        } else {
            hset_free(&fs_set);
            hset_free(&int_set);
            logmsg("Exception correctly raised on type mismatch");
        }
    }
    fs_alloc_check(true);
    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 12 ---------------------------------

static TestStatus
tf12(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое пересекается с пустым – остаётся пустое */
    test_sub("subtest %d: empty intersect empty", ++subnum);
    {
        hset    se1 = hset_init(10, VALUE64_INT);
        hset    se2 = hset_init(10, VALUE64_INT);

        hset   *res = hset_intersect(&se1, &se2);

        int     ok = (res == &se1) && (hset_cnt(&se1) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Empty intersect empty: res=%p, cnt=%d (expected res=&se1, cnt=0)",
            (void*)res, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 2. Непустое пересекается с пустым – остаётся пустое */
    test_sub("subtest %d: non-empty intersect empty", ++subnum);
    {
        int     vals[] = {1, 2, 3};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    empty = hset_init(10, VALUE64_INT);
        hset   *res = hset_intersect(&se1, &empty);

        int     ok = (res == &se1) && (hset_cnt(&se1) == 0) &&
                     (hset_cnt(&empty) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty)),
            "Non-empty intersect empty: res=%p, cnt=%d (expected res=&se1, cnt=0)",
            (void*)res, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&empty);
    }
    /* 3. Пустое пересекается с непустым – остаётся пустое */
    test_sub("subtest %d: empty intersect non-empty", ++subnum);
    {
        int     vals[] = {10, 20};
        hset    empty = hset_init(10, VALUE64_INT);
        hset    se2 = hset_from_intarr(vals, COUNT(vals));
        hset   *res = hset_intersect(&empty, &se2);

        int     ok = (res == &empty) && (hset_cnt(&empty) == 0) &&
                     (hset_cnt(&se2) == COUNT(vals));
        test_validatefree(
            ok,
            (hset_free(&empty), hset_free(&se2)),
            "Empty intersect non-empty: res=%p, cnt=%d (expected res=&empty, cnt=0)",
            (void*)res, hset_cnt(&empty)
        );
        hset_free(&empty);
        hset_free(&se2);
    }
    /* 4. Одинаковые множества – остаются все элементы */
    test_sub("subtest %d: identical sets", ++subnum);
    {
        int     vals[] = {7, 8, 9};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    se2 = hset_from_intarr(vals, COUNT(vals));
        int     cnt_before = hset_cnt(&se1);

        hset   *res = hset_intersect(&se1, &se2);

        int     ok = (res == &se1) && (hset_cnt(&se1) == cnt_before);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Identical sets: res=%p, cnt=%d (expected res=&se1, cnt=%d)",
            (void*)res, hset_cnt(&se1), cnt_before
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 5. Частичное перекрытие – остаются только общие элементы */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int     vals1[] = {1, 2, 3, 4, 5};
        int     vals2[] = {2, 4, 6};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));

        hset   *res = hset_intersect(&se1, &se2);

        int     expected_vals[] = {2, 4};
        int     expected_cnt = COUNT(expected_vals);
        int     ok = (res == &se1) && (hset_cnt(&se1) == expected_cnt);
        for (int i = 0; ok && i < expected_cnt; i++)
            ok = hset_get(&se1, LITERAL64_INT(expected_vals[i]));
        // Убедимся, что остальные отсутствуют
        if (ok) {
            ok = !hset_get(&se1, LITERAL64_INT(1)) &&
                 !hset_get(&se1, LITERAL64_INT(3)) &&
                 !hset_get(&se1, LITERAL64_INT(5));
        }

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Partial overlap: res=%p, cnt=%d (expected res=&se1, cnt=%d)",
            (void*)res, hset_cnt(&se1), expected_cnt
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 6. Непересекающиеся множества – результат пуст */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int     vals1[] = {100, 200};
        int     vals2[] = {300, 400};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));

        hset   *res = hset_intersect(&se1, &se2);

        int     ok = (res == &se1) && (hset_cnt(&se1) == 0) &&
                     (hset_cnt(&se2) == COUNT(vals2));
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Disjoint sets: res=%p, cnt=%d (expected res=&se1, cnt=0)",
            (void*)res, hset_cnt(&se1)
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
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_init(10, VALUE64_INT);
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
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset empty = hset_init(10, VALUE64_INT);
        hset res = hset_init_intersect(&se1, &empty);

        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&se1) == COUNT(vals)) && (hset_cnt(&empty) == 0);
        // дополнительно убедимся, что элементы se1 на месте
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

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
        hset empty = hset_init(10, VALUE64_INT);
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        hset res = hset_init_intersect(&empty, &se2);

        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&empty) == 0) && (hset_cnt(&se2) == COUNT(vals));
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals[i]));

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
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        hset res = hset_init_intersect(&se1, &se2);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&res) == expected) && (hset_cnt(&se1) == expected) && (hset_cnt(&se2) == expected);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, LITERAL64_INT(vals[i]));

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
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset res = hset_init_intersect(&se1, &se2);

        int common_vals[] = {2, 4};
        int expected = COUNT(common_vals);
        int ok = (hset_cnt(&res) == expected) && (hset_cnt(&se1) == COUNT(vals1)) && (hset_cnt(&se2) == COUNT(vals2));
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, LITERAL64_INT(common_vals[i]));
        // убедимся, что не общих элементов нет
        if (ok) {
            ok = !hset_get(&res, LITERAL64_INT(1)) &&
                 !hset_get(&res, LITERAL64_INT(3)) &&
                 !hset_get(&res, LITERAL64_INT(5));
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
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset res = hset_init_intersect(&se1, &se2);

        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&se1) == COUNT(vals1)) && (hset_cnt(&se2) == COUNT(vals2));
        // se1 и se2 не должны потерять элементы
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals2[i]));

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
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_init(10, VALUE64_INT);
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
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset empty = hset_init(10, VALUE64_INT);
        hset res = hset_init_symmdiff(&se1, &empty);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&res) == expected) && (hset_cnt(&se1) == expected) && (hset_cnt(&empty) == 0);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, LITERAL64_INT(vals[i]));

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
        hset empty = hset_init(10, VALUE64_INT);
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        hset res = hset_init_symmdiff(&empty, &se2);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&res) == expected) && (hset_cnt(&empty) == 0) && (hset_cnt(&se2) == expected);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, LITERAL64_INT(vals[i]));

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
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        hset res = hset_init_symmdiff(&se1, &se2);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&se1) == expected) && (hset_cnt(&se2) == expected);
        // элементы исходных множеств на месте
        for (int i = 0; ok && i < expected; i++) {
            if (!hset_get(&se1, LITERAL64_INT(vals[i])) ||
                !hset_get(&se2, LITERAL64_INT(vals[i])))
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
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset res = hset_init_symmdiff(&se1, &se2);

        // Ожидаем {1,3,5,6}
        int expected_vals[] = {1, 3, 5, 6};
        int expected_cnt = COUNT(expected_vals);
        int ok = (hset_cnt(&res) == expected_cnt) &&
                 (hset_cnt(&se1) == COUNT(vals1)) &&
                 (hset_cnt(&se2) == COUNT(vals2));
        // Проверяем наличие ожидаемых
        for (int i = 0; ok && i < expected_cnt; i++)
            ok = hset_get(&res, LITERAL64_INT(expected_vals[i]));
        // Проверяем отсутствие общих (2 и 4)
        if (ok) {
            ok = !hset_get(&res, LITERAL64_INT(2)) &&
                 !hset_get(&res, LITERAL64_INT(4));
        }
        // Убедимся, что исходные множества не изменились
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals2[i]));

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
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset res = hset_init_symmdiff(&se1, &se2);

        int expected_total = COUNT(vals1) + COUNT(vals2);
        int ok = (hset_cnt(&res) == expected_total) &&
                 (hset_cnt(&se1) == COUNT(vals1)) &&
                 (hset_cnt(&se2) == COUNT(vals2));
        // все элементы vals1 и vals2 должны быть в res
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&res, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&res, LITERAL64_INT(vals2[i]));

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
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_init(10, VALUE64_INT);
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
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset empty = hset_init(10, VALUE64_INT);
        int cnt_before = hset_cnt(&se1);
        hset *res = hset_symmdiff(&se1, &empty);

        int ok = (hset_cnt(&se1) == cnt_before) && (hset_cnt(&empty) == 0) && (res == &se1);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

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
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        hset *res = hset_symmdiff(&se1, &se2);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&se1) == expected) && (hset_cnt(&se2) == expected) && (res == &se1);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

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
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        hset *res = hset_symmdiff(&se1, &se2);

        int expected_orig = COUNT(vals);
        int ok = (hset_cnt(&se1) == 0) && (hset_cnt(&se2) == expected_orig) && (res == &se1);
        // элементы se2 должны остаться на месте
        for (int i = 0; ok && i < expected_orig; i++)
            ok = hset_get(&se2, LITERAL64_INT(vals[i]));

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
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset *res = hset_symmdiff(&se1, &se2);

        // Ожидаем в se1: {1,3,5,6}
        int expected_vals[] = {1, 3, 5, 6};
        int expected_cnt = COUNT(expected_vals);
        int ok = (hset_cnt(&se1) == expected_cnt) && (hset_cnt(&se2) == COUNT(vals2)) && (res == &se1);
        // проверяем наличие ожидаемых
        for (int i = 0; ok && i < expected_cnt; i++)
            ok = hset_get(&se1, LITERAL64_INT(expected_vals[i]));
        // проверяем отсутствие общих (2 и 4)
        if (ok) {
            ok = !hset_get(&se1, LITERAL64_INT(2)) &&
                 !hset_get(&se1, LITERAL64_INT(4));
        }
        // se2 не изменился
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals2[i]));

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
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset *res = hset_symmdiff(&se1, &se2);

        int expected_total = COUNT(vals1) + COUNT(vals2);
        int ok = (hset_cnt(&se1) == expected_total) && (hset_cnt(&se2) == COUNT(vals2)) && (res == &se1);
        // все элементы vals1 и vals2 должны быть в se1
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals2[i]));
        // se2 не изменился
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals2[i]));

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


// ------------------------- TEST 17 ---------------------------------
static TestStatus
tf17(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое ∪ пустое = пустое */
    test_sub("subtest %d: empty union empty", ++subnum);
    {
        hset    se1 = hset_init(10, VALUE64_INT);
        hset    se2 = hset_init(10, VALUE64_INT);
        hset   *res = hset_union(&se1, &se2);

        int     ok = (hset_cnt(&se1) == 0) && (res == &se1);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Empty union empty: cnt=%d, res=%p (expected cnt=0, res=&se1)",
            hset_cnt(&se1), (void*)res
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 2. Непустое ∪ пустое = исходное непустое */
    test_sub("subtest %d: non-empty union empty", ++subnum);
    {
        int     vals[] = {3, 7, 11};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    empty = hset_init(10, VALUE64_INT);
        int     cnt_before = hset_cnt(&se1);
        hset   *res = hset_union(&se1, &empty);

        int     ok = (hset_cnt(&se1) == cnt_before) && (res == &se1);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty)),
            "Non-empty union empty: cnt=%d (expected %d), res=%p",
            hset_cnt(&se1), cnt_before, (void*)res
        );
        hset_free(&se1);
        hset_free(&empty);
    }

    /* 3. Пустое ∪ непустое = копия непустого в se1 */
    test_sub("subtest %d: empty union non-empty", ++subnum);
    {
        int     vals[] = {5, 9};
        hset    se1 = hset_init(10, VALUE64_INT);
        hset    se2 = hset_from_intarr(vals, COUNT(vals));
        hset   *res = hset_union(&se1, &se2);

        int     expected = COUNT(vals);
        int     ok = (hset_cnt(&se1) == expected) && (res == &se1);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Empty union non-empty: cnt=%d (expected %d), res=%p",
            hset_cnt(&se1), expected, (void*)res
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 4. Одинаковые множества: результат не меняется, количество не растёт */
    test_sub("subtest %d: identical sets", ++subnum);
    {
        int     vals[] = {1, 2, 3, 4};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    se2 = hset_from_intarr(vals, COUNT(vals));
        int     cnt_before = hset_cnt(&se1);
        hset   *res = hset_union(&se1, &se2);

        int     ok = (hset_cnt(&se1) == cnt_before) && (res == &se1);
        // все элементы должны остаться на месте
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Identical sets: cnt=%d (expected %d), res=%p",
            hset_cnt(&se1), cnt_before, (void*)res
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 5. Частичное перекрытие: добавляются только недостающие элементы */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int     vals1[] = {1, 2, 3, 4, 5};
        int     vals2[] = {3, 5, 6, 7};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset   *res = hset_union(&se1, &se2);

        // Ожидаем {1,2,3,4,5,6,7} – 7 элементов
        int     expected_vals[] = {1, 2, 3, 4, 5, 6, 7};
        int     expected_cnt = COUNT(expected_vals);
        int     ok = (hset_cnt(&se1) == expected_cnt) && (res == &se1);
        for (int i = 0; ok && i < expected_cnt; i++)
            ok = hset_get(&se1, LITERAL64_INT(expected_vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Partial overlap: cnt=%d (expected %d), res=%p",
            hset_cnt(&se1), expected_cnt, (void*)res
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 6. Непересекающиеся множества: все элементы se2 добавляются */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int     vals1[] = {100, 200};
        int     vals2[] = {300, 400};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset   *res = hset_union(&se1, &se2);

        int     expected_total = COUNT(vals1) + COUNT(vals2);
        int     ok = (hset_cnt(&se1) == expected_total) && (res == &se1);
        // все элементы должны быть доступны
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Disjoint sets: cnt=%d (expected %d), res=%p",
            hset_cnt(&se1), expected_total, (void*)res
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 18 ---------------------------------

static TestStatus
tf18(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое ∪ пустое = пустое */
    test_sub("subtest %d: empty union empty", ++subnum);
    {
        hset    se1 = hset_init(10, VALUE64_INT);
        hset    se2 = hset_init(10, VALUE64_INT);
        hset    res = hset_init_union(&se1, &se2);

        int     ok = (hset_cnt(&res) == 0) &&
                     (hset_cnt(&se1) == 0) &&
                     (hset_cnt(&se2) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Empty union empty: res=%d, se1=%d, se2=%d (expected all 0)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 2. Непустое ∪ пустое = копия se1, исходные не изменились */
    test_sub("subtest %d: non-empty union empty", ++subnum);
    {
        int     vals[] = {3, 7, 11};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    empty = hset_init(10, VALUE64_INT);

        int     cnt1_before = hset_cnt(&se1);
        hset    res = hset_init_union(&se1, &empty);
        int     expected = COUNT(vals);

        int     ok = (hset_cnt(&res) == expected) &&
                     (hset_cnt(&se1) == cnt1_before) &&
                     (hset_cnt(&empty) == 0);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, LITERAL64_INT(vals[i]));
        // se1 должен остаться без изменений
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty), hset_free(&res)),
            "Non-empty union empty: res=%d, se1=%d (expected %d, %d)",
            hset_cnt(&res), hset_cnt(&se1), expected, cnt1_before
        );
        hset_free(&se1);
        hset_free(&empty);
        hset_free(&res);
    }

    /* 3. Пустое ∪ непустое = копия se2, исходные не изменились */
    test_sub("subtest %d: empty union non-empty", ++subnum);
    {
        int     vals[] = {5, 9};
        hset    empty = hset_init(10, VALUE64_INT);
        hset    se2 = hset_from_intarr(vals, COUNT(vals));

        int     cnt2_before = hset_cnt(&se2);
        hset    res = hset_init_union(&empty, &se2);

        int     expected = COUNT(vals);
        int     ok = (hset_cnt(&res) == expected) &&
                     (hset_cnt(&empty) == 0) &&
                     (hset_cnt(&se2) == cnt2_before);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, LITERAL64_INT(vals[i]));
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&se2, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&empty), hset_free(&se2), hset_free(&res)),
            "Empty union non-empty: res=%d, se2=%d (expected %d, %d)",
            hset_cnt(&res), hset_cnt(&se2), expected, cnt2_before
        );
        hset_free(&empty);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 4. Одинаковые множества: результат совпадает с исходными */
    test_sub("subtest %d: identical sets", ++subnum);
    {
        int     vals[] = {1, 2, 3, 4};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    se2 = hset_from_intarr(vals, COUNT(vals));

        int     cnt_before = hset_cnt(&se1);
        hset    res = hset_init_union(&se1, &se2);

        int     expected = COUNT(vals);
        int     ok = (hset_cnt(&res) == expected) &&
                     (hset_cnt(&se1) == cnt_before) &&
                     (hset_cnt(&se2) == cnt_before);
        for (int i = 0; ok && i < expected; i++) {
            ok = hset_get(&res, LITERAL64_INT(vals[i])) &&
                 hset_get(&se1, LITERAL64_INT(vals[i])) &&
                 hset_get(&se2, LITERAL64_INT(vals[i]));
        }

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

    /* 5. Частичное перекрытие: объединение содержит все уникальные элементы */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int     vals1[] = {1, 2, 3, 4, 5};
        int     vals2[] = {3, 5, 6, 7};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));

        int     cnt1_before = hset_cnt(&se1);
        int     cnt2_before = hset_cnt(&se2);
        hset    res = hset_init_union(&se1, &se2);

        int     expected_vals[] = {1, 2, 3, 4, 5, 6, 7};
        int     expected_cnt = COUNT(expected_vals);
        int     ok = (hset_cnt(&res) == expected_cnt) &&
                     (hset_cnt(&se1) == cnt1_before) &&
                     (hset_cnt(&se2) == cnt2_before);
        for (int i = 0; ok && i < expected_cnt; i++)
            ok = hset_get(&res, LITERAL64_INT(expected_vals[i]));
        // se1 и se2 не тронуты
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals2[i]));

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

    /* 6. Непересекающиеся множества: все элементы обоих в результате */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int     vals1[] = {100, 200};
        int     vals2[] = {300, 400};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));

        int     cnt1_before = hset_cnt(&se1);
        int     cnt2_before = hset_cnt(&se2);
        hset    res = hset_init_union(&se1, &se2);

        int     expected_total = COUNT(vals1) + COUNT(vals2);
        int     ok = (hset_cnt(&res) == expected_total) &&
                     (hset_cnt(&se1) == cnt1_before) &&
                     (hset_cnt(&se2) == cnt2_before);
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&res, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&res, LITERAL64_INT(vals2[i]));
        // исходные целы
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals2[i]));

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

// ------------------------- TEST 21 ---------------------------------

static void             print_as_int(value64 v){
    printf("%4d\t", v.ival);
}

static void             dummy_const_proc(value64 v) {
    (void)v; /* ничего не делаем */
}
/*
static void             multiply_by_two(value64 *v) {
    v->ival *= 2;
}

static void             remove_if_even(hset *se, hset_elem *el) {
    if (el->v.ival % 2 == 0)
        hset_del(se, el->v);
}

static void             remove_all(hset *se, hset_elem *el) {
    hset_del(se, el->v);
} */

static TestStatus
tf21(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ---------- 1. hset_const_foreach: только чтение ---------- */
    test_sub("subtest %d: const_foreach read-only", ++subnum);
    {
        int     vals[] = {10, 20, 30, 40, 50};
        hset    se = hset_from_intarr(vals, COUNT(vals) );
        // just a printf
        hset_const_foreach(&se, print_as_int);
        hset_free(&se);
    }

    // Так как мы убрали ctx, для сбора данных в const_foreach нужен другой подход.
    // Пока оставим простую проверку: просто вызовем и убедимся, что множество не изменилось.
    test_sub("subtest %d: const_foreach does not modify", ++subnum);
    {
        int     vals[] = {1, 2, 3};
        hset    se = hset_from_intarr(vals, COUNT(vals));

        hset_const_foreach(&se, dummy_const_proc); // ничего не делает, но гарантирует проход

        // Проверяем, что все элементы на месте и их количество не изменилось
        test_validatefree(
            hset_cnt(&se) == COUNT(vals),
            hset_free(&se),
            "const_foreach: cnt=%d, expected %d", hset_cnt(&se), COUNT(vals)
        );
        for (int i = 0; i < COUNT(vals); i++) {
            test_validatefree(
                hset_get(&se, LITERAL64_INT(vals[i])),
                hset_free(&se),
                "const_foreach: missing value %d", vals[i]
            );
        }
        hset_free(&se);
    }

    return logret(TEST_PASSED, "done");
}
// ------------------------- TEST 22 ---------------------------------
static TestStatus
tf22(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Сумма int */
    test_sub("subtest %d: reduce sum int", ++subnum);
    {
        int     vals[] = {1, 2, 3, 4, 5};
        hset    se = hset_from_intarr(vals, COUNT(vals));
        hset_accum res = hset_reduce(&se, hset_sum_int);

        test_validatefree(
            res.value.ival == 15,
            hset_free(&se),
            "Sum int: value = %d, expected 15", res.value.ival
        );
        test_validatefree(
            res.count == 5,
            hset_free(&se),
            "Sum int: count = %d, expected 5", res.count
        );
        hset_free(&se);
    }

    /* 2. Счётчик int */
    test_sub("subtest %d: reduce count int", ++subnum);
    {
        int     vals[] = {10, 20, 30};
        hset    se = hset_from_intarr(vals, COUNT(vals));
        hset_accum res = hset_reduce(&se, hset_count_int);

        test_validatefree(
            res.count == COUNT(vals),
            hset_free(&se),
            "Count int: count = %d, expected %d", res.count, COUNT(vals)
        );
        // value не должно измениться (осталось нулевым)
        test_validatefree(
            res.value.ival == 0,
            hset_free(&se),
            "Count int: value = %d, expected 0 (unused)", res.value.ival
        );
        hset_free(&se);
    }

    /* 3. Максимум int */
    test_sub("subtest %d: reduce max int", ++subnum);
    {
        int     vals[] = {5, 2, 9, 1, 7};
        hset    se = hset_from_intarr(vals, COUNT(vals));
        hset_accum res = hset_reduce(&se, hset_max_int);

        test_validatefree(
            res.value.ival == 9,
            hset_free(&se),
            "Max int: value = %d, expected 9", res.value.ival
        );
        test_validatefree(
            res.count == 1,
            hset_free(&se),
            "Max int: count = %d, expected 1 (flag)", res.count
        );
        hset_free(&se);
    }

    /* 4. Минимум int */
    test_sub("subtest %d: reduce min int", ++subnum);
    {
        int     vals[] = {3, 8, 1, 6, 4};
        hset    se = hset_from_intarr(vals, COUNT(vals));
        hset_accum res = hset_reduce(&se, hset_min_int);

        test_validatefree(
            res.value.ival == 1,
            hset_free(&se),
            "Min int: value = %d, expected 1", res.value.ival
        );
        test_validatefree(
            res.count == 1,
            hset_free(&se),
            "Min int: count = %d, expected 1 (flag)", res.count
        );
        hset_free(&se);
    }

    /* 5. Пустое множество: сумма и счётчик */
    test_sub("subtest %d: reduce on empty set", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_INT);
        hset_accum res = hset_reduce(&se, hset_sum_int);

        test_validatefree(
            res.value.ival == 0,
            hset_free(&se),
            "Empty sum: value = %d, expected 0", res.value.ival
        );
        test_validatefree(
            res.count == 0,
            hset_free(&se),
            "Empty sum: count = %d, expected 0", res.count
        );
        hset_free(&se);
    }

    /* 6. Пустое множество: max (count остаётся 0) */
    test_sub("subtest %d: reduce max on empty set", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_INT);
        hset_accum res = hset_reduce(&se, hset_max_int);

        test_validatefree(
            res.count == 0,
            hset_free(&se),
            "Empty max: count = %d, expected 0 (no elements)", res.count
        );
        // value не важно, т.к. нет элементов
        hset_free(&se);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 23 ---------------------------------

static TestStatus
tf23(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Сумма double (только конечные числа) */
    test_sub("subtest %d: sum double", ++subnum);
    {
        double  vals[] = {1.5, 2.5, 3.5};
        hset    se = hset_from_dblarr(vals, COUNT(vals));
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_sum_dbl);

        test_validatefree(
            fabs(res.value.dval - 7.5) < 0.0001,
            hset_free(&se),
            "Sum double: value = %f, expected 7.5", res.value.dval
        );
        test_validatefree(
            res.count == 3,
            hset_free(&se),
            "Sum double: count = %d, expected 3", res.count
        );
        hset_free(&se);
    }

    /* 2. Счётчик double (только конечные числа) */
    test_sub("subtest %d: count double", ++subnum);
    {
        double  vals[] = {10.0, 20.0, 30.0};
        hset    se = hset_from_dblarr(vals, COUNT(vals));
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_count_dbl);

        test_validatefree(
            res.count == 3,
            hset_free(&se),
            "Count double: count = %d, expected 3", res.count
        );
        test_validatefree(
            res.value.dval == 0.0,
            hset_free(&se),
            "Count double: value = %f, expected 0.0 (unused)", res.value.dval
        );
        hset_free(&se);
    }

    /* 3. Максимум double */
    test_sub("subtest %d: max double", ++subnum);
    {
        double  vals[] = {5.2, 2.3, 9.8, 1.0, 7.4};
        hset    se = hset_from_dblarr(vals, COUNT(vals));
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_max_dbl);

        test_validatefree(
            fabs(res.value.dval - 9.8) < 0.0001,
            hset_free(&se),
            "Max double: value = %f, expected 9.8", res.value.dval
        );
        test_validatefree(
            res.count == 1,
            hset_free(&se),
            "Max double: count = %d, expected 1 (flag)", res.count
        );
        hset_free(&se);
    }

    /* 4. Минимум double (все отрицательные) */
    test_sub("subtest %d: min double (all negative)", ++subnum);
    {
        double  vals[] = {-5.0, -2.5, -9.1, -1.0};
        hset    se = hset_from_dblarr(vals, COUNT(vals));
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_min_dbl);

        test_validatefree(
            fabs(res.value.dval - (-9.1)) < 0.0001,
            hset_free(&se),
            "Min double: value = %f, expected -9.1", res.value.dval
        );
        test_validatefree(
            res.count == 1,
            hset_free(&se),
            "Min double: count = %d, expected 1 (flag)", res.count
        );
        hset_free(&se);
    }

    /* 5. Пустое множество: сумма double */
    test_sub("subtest %d: sum double on empty set", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_DBL);
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_sum_dbl);

        test_validatefree(
            res.value.dval == 0.0,
            hset_free(&se),
            "Empty sum double: value = %f, expected 0.0", res.value.dval
        );
        test_validatefree(
            res.count == 0,
            hset_free(&se),
            "Empty sum double: count = %d, expected 0", res.count
        );
        hset_free(&se);
    }

    /* 6. Пустое множество: max double (count должен остаться 0) */
    test_sub("subtest %d: max double on empty set", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_DBL);
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_max_dbl);

        test_validatefree(
            res.count == 0,
            hset_free(&se),
            "Empty max double: count = %d, expected 0", res.count
        );
        hset_free(&se);
    }

    /* 7. Несколько значений, включая очень большое, для sum */
    test_sub("subtest %d: sum double with large value", ++subnum);
    {
        double  vals[] = {1e10, 2e10, 3e10};
        hset    se = hset_from_dblarr(vals, COUNT(vals));
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_sum_dbl);

        test_validatefree(
            fabs(res.value.dval - 6e10) < 1e5,
            hset_free(&se),
            "Sum double large: value = %f, expected 6e10", res.value.dval
        );
        test_validatefree(
            res.count == 3,
            hset_free(&se),
            "Sum double large: count = %d, expected 3", res.count
        );
        hset_free(&se);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 26 ---------------------------------
static TestStatus
tf26(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== hset_any ========== */

    /* 1. any: пустое с пустым -> false (нет элементов для поиска) */
    test_sub("subtest %d:hset_any  empty vs empty", ++subnum);
    {
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_init(10, VALUE64_INT);
        test_validatefree(
            !hset_any(&se1, &se2),   // false
            (hset_free(&se1), hset_free(&se2)),
            "hset_any empty vs empty must be false"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 2. any: пустое с непустым -> false */
    test_sub("subtest %d:hset_any  empty vs non-empty", ++subnum);
    {
        int vals[] = {1, 2, 3};
        hset empty = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals));
        test_validatefree(
            !hset_any(&empty, &nonempty),   // false
            (hset_free(&empty), hset_free(&nonempty)),
            "hset_any empty vs non-empty must be false"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    /* 3. any: непустое с пустым -> false (нет элементов в se2 для поиска) */
    test_sub("subtest %d: any non‑empty vs empty", ++subnum);
    {
        int vals[] = {1, 2, 3};
        hset empty = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals));
        test_validatefree(
            !hset_any(&nonempty, &empty),   // false
            (hset_free(&empty), hset_free(&nonempty)),
            "hset_any non‑empty vs empty must be false"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    /* 4. any: непустое с непустым, есть пересечение -> true */
    test_sub("subtest %d:hset_any  with overlap", ++subnum);
    {
        int vals1[] = {1, 3, 5, 7};
        int vals2[] = {5, 9, 11};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        test_validatefree(
            hset_any(&se1, &se2),   // true
            (hset_free(&se1), hset_free(&se2)),
            "hset_any with overlap must be true"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 5. any: непустое с непустым, нет пересечения -> false */
    test_sub("subtest %d:hset_any  disjoint", ++subnum);
    {
        int vals1[] = {100, 200};
        int vals2[] = {300, 400};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        test_validatefree(
            !hset_any(&se1, &se2),   // false
            (hset_free(&se1), hset_free(&se2)),
            "hset_any disjoint must be false"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 6. any: одинаковые множества -> true (все элементы se2 есть в se1) */
    test_sub("subtest %d:hset_any  identical sets", ++subnum);
    {
        int vals[] = {7, 8, 9};
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        test_validatefree(
            hset_any(&se1, &se2),   // true
            (hset_free(&se1), hset_free(&se2)),
            "hset_any identical sets must be true"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* ========== hset_notexists ========== */

    /* 7. notexists: пустое с пустым -> true (нет общих элементов) */
    test_sub("subtest %d: notexists empty vs empty", ++subnum);
    {
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_init(10, VALUE64_INT);
        test_validatefree(
            hset_notexists(&se1, &se2),   // true
            (hset_free(&se1), hset_free(&se2)),
            "hset_notexists empty vs empty must be true"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 8. notexists: пустое с непустым -> true */
    test_sub("subtest %d: notexists empty vs non-empty", ++subnum);
    {
        int vals[] = {1, 2, 3};
        hset empty = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals));
        test_validatefree(
            hset_notexists(&empty, &nonempty),   // true
            (hset_free(&empty), hset_free(&nonempty)),
            "hset_notexists empty vs non-empty must be true"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }

    /* 9. notexists: непустое с непустым, нет пересечения -> true */
    test_sub("subtest %d: notexists disjoint", ++subnum);
    {
        int vals1[] = {10, 20};
        int vals2[] = {30, 40};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        test_validatefree(
            hset_notexists(&se1, &se2),   // true
            (hset_free(&se1), hset_free(&se2)),
            "hset_notexists disjoint must be true"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 10. notexists: непустое с непустым, есть пересечение -> false */
    test_sub("subtest %d: notexists with overlap", ++subnum);
    {
        int vals1[] = {1, 2, 3};
        int vals2[] = {3, 4, 5};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        test_validatefree(
            !hset_notexists(&se1, &se2),   // false
            (hset_free(&se1), hset_free(&se2)),
            "hset_notexists with overlap must be false"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 11. notexists: одинаковые множества -> false */
    test_sub("subtest %d: notexists identical sets", ++subnum);
    {
        int vals[] = {5, 6, 7};
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        test_validatefree(
            !hset_notexists(&se1, &se2),   // false
            (hset_free(&se1), hset_free(&se2)),
            "hset_notexists identical sets must be false"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main(int argc, const char *argv[])
{
    logsimpleinit("Start");
    bool    runall = argc == 1;

    while (runall || *++argv){
        int     num = INT_MAX;    // INT_MAX for all test
        if (!runall){
            num = atoi(*argv);
            if (num < 0){
                fprintf(stderr,"Invalid test num %d\n", num);
                continue;
            }
        }
        printf("Num %d\n", num);
            testenginestd_run(num,
                testnew(.f2 =  tf8,             .num =  9, .name = "Hset_in simple test"                        , .desc="", .mandatory=true)
              , testnew(.f2 =  tf9,             .num = 10, .name = "Hset_strictin simple test"                  , .desc="", .mandatory=true)
              , testnew(.f2 = tf10,             .num = 11, .name = "Hset_minus simple test"                     , .desc="", .mandatory=true)
              , testnew(.f2 = tf11,             .num = 12, .name = "Hset_init_minus simple test"                , .desc="", .mandatory=true)
              , testnew(.f2 = tf12,             .num = 13, .name = "hset_intersect simple test"                 , .desc="", .mandatory=true)
              , testnew(.f2 = tf13,             .num = 14, .name = "hset_init_intersect simple test"            , .desc="", .mandatory=true)
              , testnew(.f2 = tf14,             .num = 15, .name = "hset_init_symmdiff simple test"             , .desc="", .mandatory=true)
              , testnew(.f2 = tf15,             .num = 16, .name = "hset_symmdiff simple test"                  , .desc="", .mandatory=true)
              , testnew(.f2 = tf17,             .num = 18, .name = "hset_union simple test"                     , .desc="", .mandatory=true)
              , testnew(.f2 = tf18,             .num = 19, .name = "hset_init_union simple test"                , .desc="", .mandatory=true)
              , testnew(.f2 = tf21,             .num = 22, .name = "hset_const_foreach simple test"             , .desc="", .mandatory=true)
              , testnew(.f2 = tf22,             .num = 23, .name = "hset_initreduct int impl  simple test"      , .desc="", .mandatory=true)
              , testnew(.f2 = tf23,             .num = 24, .name = "hset_initreduct double int simple test"     , .desc="", .mandatory=true)
              , testnew(.f2 = tf26,             .num = 27, .name = "hset_any(), hset_nonexists() simple test"   , .desc="", .mandatory=true)
            );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HASHSET_UTILS_TESTING */

