#ifndef _HASHSET_UTILS_H
#define _HASHSET_UTILS_H

// -------------------------------------------------------------------------------------------
// --------------------------- Public Hashset utils API --------------------------------------
// -------------------------------------------------------------------------------------------

// --------------- Includes -----------------------

#include "hashset.h"

// ----------- CONSTANTS AND GLOBALS ---------------

// ------------------- TYPES -----------------------

// generic code
// just intersect with construct
extern hset                 hset_init_intersect(const hset *restrict se1, const hset *restrict se2);
// minus with construct with construct
extern hset                 hset_init_minus(const hset *restrict se1, const hset *restrict se2);
// simm diff with construct
extern hset                 hset_init_symmdiff(const hset *restrict a, const hset *restrict b);
// union with construct
extern hset                 hset_init_union(const hset *restrict a, const hset *restrict b);

extern bool                 hset_eq(const hset *restrict se1, const hset *restrict se2);

extern bool                 hset_noteq(const hset *restrict se1, const hset *restrict se2);

// check if all of se2 in se1 strictly or not
extern bool                 hset_subset_check(const hset *restrict se1, const hset *restrict se2, bool strict);
// check if all of se2 in se1
static inline bool          hset_in(const hset *restrict se1, const hset *restrict se2){
    return hset_subset_check(se1, se2, false);
}
// check if all of se2 in se1  but se2 not equal se1
static inline bool          hset_strictin(const hset *restrict se1, const hset *restrict se2){
    return hset_subset_check(se1, se2, true);
}
// if not exists
extern bool                 hset_notexists(const hset *restrict se1, const hset *restrict se2);
// if exists any of se2 in se1
extern bool                 hset_any(const hset *restrict se1, const hset *restrict se2);
// se1 -= se2 as SET
extern hset                *hset_minus(hset *restrict se1, const hset *restrict se2);
// se1 insersect= se2 as SET
extern hset                *hset_intersect(hset *restrict se1, const hset *restrict se2);
// se1 symmdiff= se2 as SET
extern hset                *hset_symmdiff(hset *restrict a, const hset *restrict b);
// union= as SET
extern hset                *hset_union(hset *restrict a, const hset *restrict b);

// ----------------------------------------- iterators --------------------------------------
typedef                     void * pointer_to_void;

extern void                 hset_const_foreach(const hset *se, hset_const_proc_t proc);
//extern void               hset_foreach(hset *se, hset_proc_t proc);
//extern void               hset_modify_foreach(hset *se, hset_modify_proc_t proc);

// ----------------------------------------- REDUCE -----------------------------------------
typedef struct              hset_accum {
    value64     value;    // накопленное значение (сумма, максимум и т.п.)
    int         count;    // количество элементов, участвовавших в накоплении
} hset_accum;

// TODO: api for extracting from hset_accum
static inline value64       hset_accum_get(hset_accum c){
    return c.value;
}

#define                     HSET_ACCUM(...)     (hset_accum) { .value = LITERAL64_ZERO, .count = 0, __VA_ARGS__}
#define                     HSET_ACCUM_DBL_ZERO (hset_accum) { .value = LITERAL64_DBL(0.0), .count = 0 }
#define                     HSET_ACCUM_FS_ZERO  (hset_accum) { .value = value64_createfs(fs_create() ), .count = 0 }

typedef                     void (*hset_reduce_func)(hset_accum *acc, value64 v);
extern hset_accum           hset_initreduce(const hset *se, hset_accum init, hset_reduce_func func);

static inline hset_accum    hset_reduce(const hset *se, hset_reduce_func func){
    return hset_initreduce(se, HSET_ACCUM(), func);
}
static inline hset_accum    hset_reduce_dbl(const hset *se, hset_reduce_func func){
    return hset_initreduce(se, HSET_ACCUM_DBL_ZERO, func);
}
static inline hset_accum    hset_reduce_fs(const hset *se, hset_reduce_func func){
    return hset_initreduce(se, HSET_ACCUM_FS_ZERO, func);
}

// unified version! TODO:
typedef struct              hset_unified {
    value64  value[HSET_UNIFIED_CNT];    // unified values (int, double, fs etc...)
} hset_unified;


// ------------------------------------- REDUCE IMPL -----------------------------------------
extern void                 hset_sum_int    (hset_accum *acc, value64 v);
extern void                 hset_count_int  (hset_accum *acc, value64 v);
extern void                 hset_max_int    (hset_accum *acc, value64 v);
extern void                 hset_min_int    (hset_accum *acc, value64 v);
//extern void               hset_avg_int    (hset_accum *acc, value64 v);

extern void                 hset_sum_dbl    (hset_accum *acc, value64 v);
extern void                 hset_count_dbl  (hset_accum *acc, value64 v);
extern void                 hset_max_dbl    (hset_accum *acc, value64 v);
extern void                 hset_min_dbl    (hset_accum *acc, value64 v);
//extern void                 hset_avg_dbl    (hset_accum *acc, value64 v);

extern void                 hset_count_fs   (hset_accum *acc, value64 v);
extern void                 hset_max_fs     (hset_accum *acc, value64 v);
extern void                 hset_min_fs     (hset_accum *acc, value64 v);

#endif /* !_HASHSET_UTILS_H */

