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

        testnew(/*.f2 =  tf1,             .num =  1, .name = "Simple init and validate test"              , .desc="", .mandatory=true)
              , testnew(.f2 =  tf2,             .num =  2, .name = "Simple init and add test"                   , .desc="", .mandatory=true)
              , testnew(.f2 =  tf3,             .num =  3, .name = "Simple clone and create from array test"    , .desc="", .mandatory=true)
              , testnew(.f2 =  tf_loadfs_str,   .num =  4, .name = "Simple create fs from c-str array test"     , .desc="", .mandatory=true)
              , testnew(.f2 =  tf4,             .num =  5, .name = "Simple count test"                          , .desc="", .mandatory=true)
              , testnew(.f2 =  tf5,             .num =  6, .name = "Comparation simple test"                    , .desc="", .mandatory=true)
              , testnew(.f2 =  tf6,             .num =  7, .name = "Not equal simple test"                      , .desc="", .mandatory=true)
              , testnew(.f2 =  tf7,             .num =  8, .name = "Cloneas simple test"                        , .desc="", .mandatory=true)*/
              , testnew(.f2 =  tf8,             .num =  9, .name = "Hset_in simple test"                        , .desc="", .mandatory=true)
              //, testnew(.f2 =  tf9,             .num = 10, .name = "Hset_strictin simple test"                  , .desc="", .mandatory=true)
              //, testnew(.f2 = tf10,             .num = 11, .name = "Hset_minus simple test"                     , .desc="", .mandatory=true)
              //, testnew(.f2 = tf11,             .num = 12, .name = "Hset_init_minus simple test"                , .desc="", .mandatory=true)
              //, testnew(.f2 = tf12,             .num = 13, .name = "hset_intersect simple test"                 , .desc="", .mandatory=true)
              //, testnew(.f2 = tf13,             .num = 14, .name = "hset_init_intersect simple test"            , .desc="", .mandatory=true)
              //, testnew(.f2 = tf14,             .num = 15, .name = "hset_init_symmdiff simple test"             , .desc="", .mandatory=true)
              //, testnew(.f2 = tf15,             .num = 16, .name = "hset_symmdiff simple test"                  , .desc="", .mandatory=true)
              //, testnew(.f2 = tf16,             .num = 17, .name = "hset_init_resize simple test"               , .desc="", .mandatory=true)
              //, testnew(.f2 = tf17,             .num = 18, .name = "hset_union simple test"                     , .desc="", .mandatory=true)
              //, testnew(.f2 = tf18,             .num = 19, .name = "hset_init_union simple test"                , .desc="", .mandatory=true)
              //, testnew(.f2 = tf19,             .num = 20, .name = "hset_normalize simple test"                 , .desc="", .mandatory=true)
              //, testnew(.f2 = tf20,             .num = 21, .name = "hset_save/load simple test"                 , .desc="", .mandatory=true)
              //, testnew(.f2 = tf21,             .num = 22, .name = "hset_const_foreach simple test"             , .desc="", .mandatory=true)
              //, testnew(.f2 = tf22,             .num = 23, .name = "hset_initreduct int impl  simple test"      , .desc="", .mandatory=true)
              //, testnew(.f2 = tf23,             .num = 24, .name = "hset_initreduct double int simple test"     , .desc="", .mandatory=true)
              //, testnew(.f2 = tf24,             .num = 25, .name = "inf/nan double int simple test"             , .desc="", .mandatory=true)
              //, testnew(.f2 = tf25,             .num = 26, .name = "Macro-base iterator simple test"            , .desc="", .mandatory=true)
              //, testnew(.f2 = tf26,             .num = 27, .name = "hset_any(), hset_nonexists() simple test"   , .desc="", .mandatory=true)
              //, testnew(.f2 = tf27,             .num = 28, .name = "value64_movefs() simple test"               , .desc="", .mandatory=true)
            );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HASHSET_UTILS_TESTING */

