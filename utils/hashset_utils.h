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
    value64         value;    // накопленное значение (сумма, максимум и т.п.)
    int             count;    // количество элементов, участвовавших в накоплении
    value64_type    typ;      // type of value(s)
    const char     *sep;      // separator for fsagg
} hset_accum;

// TODO: api for extracting from hset_accum
static inline value64       hset_accum_get(hset_accum *c){
    return c->value;
}
#define                     hset_accum_int(ha) (*hset_accum_getint(ha) )
static inline int          *hset_accum_getint(hset_accum *c){
    return &c->value.ival;
}
#define                     hset_accum_long(ha) (*hset_accum_getlong(ha) )
static inline long         *hset_accum_getlong(hset_accum *c) {
    return &c->value.lval;
}
#define                     hset_accum_dbl(ha) (*hset_accum_getdbl(ha) )
static inline double       *hset_accum_getdbl(hset_accum *c) {
    return &c->value.dval;
}
#define                     hset_accum_fs(ha)  (*hset_accum_getfs(ha) )
static inline fs          **hset_accum_getfs(hset_accum *c) {
    return &c->value.fsval;
}

#define                     HSET_ACCUM(type, ...) \
(hset_accum) { \
    .value = LITERAL64_ZERO, \
    .count = 0, \
    .typ = (type), \
    __VA_ARGS__ \
}
#define                     HSET_ACCUM_DBL_ZERO \
(hset_accum) { \
    .value = LITERAL64_DBL(0.0), \
    .count = 0, \
    .typ = VALUE64_DBL \
}
#define                     HSET_ACCUM_FS_ZERO \
(hset_accum) { \
    .value = (value64) \
        { .u64 = 0, \
          .fsval = fs_create() \
        }, \
    .count = 0, \
    .typ = VALUE64_FS \
}
#define                     HSET_ACCUM_FS_AGG(sepa) \
(hset_accum) { \
    .value     = (value64) \
        { .u64 = 0, \
          .fsval = fs_create() \
        }, \
    .count     = 0, \
    .typ       = VALUE64_FS, \
    .sep       = (sepa) \
}

typedef                     void (*hset_reduce_func)(hset_accum *acc, value64 v);
extern hset_accum           hset_initreduce(const hset *se, hset_accum init, hset_reduce_func func);

static inline hset_accum    hset_reduce(const hset *se, hset_reduce_func func){
    return hset_initreduce(se, HSET_ACCUM(VALUE64_INT), func);  // VALUE64_INT is just mean do nothing when hset_accum_free()
}
static inline hset_accum    hset_reduce_int(const hset *se, hset_reduce_func func){
    return hset_initreduce(se, HSET_ACCUM(VALUE64_INT), func);
}
static inline hset_accum    hset_reduce_lng(const hset *se, hset_reduce_func func){
    return hset_initreduce(se, HSET_ACCUM(VALUE64_LNG), func);
}
static inline hset_accum    hset_reduce_dbl(const hset *se, hset_reduce_func func){
    return hset_initreduce(se, HSET_ACCUM_DBL_ZERO, func);
}
static inline hset_accum    hset_reduce_fs(const hset *se, hset_reduce_func func){
    return hset_initreduce(se, HSET_ACCUM_FS_ZERO, func);
}
static inline hset_accum    hset_reduce_fsagg(const hset *se, hset_reduce_func func, const char *sep){
    return hset_initreduce(se, HSET_ACCUM_FS_AGG(sep), func);
}

static inline void          hset_accum_free(hset_accum *ha){
    invraisecode(ERR_NULLABLE_PTR, ha != NULL, "Null pointer");
    switch (ha->typ) {
        case VALUE64_FS:
            fs_free(hset_accum_fs(ha) );
        break;
        case VALUE64_STR:
            userraiseint(ERR_UNSUPPORTED_TYPE, "VALUE64_STR isn't supported now");
        break;
        default:    // no action here
        break;
    }
    *ha = HSET_ACCUM(ha->typ);  // clear (hset_accum){ .typ = ha->typ }
}

/*
// unified version! TODO:
typedef struct              hset_unified {
    value64  value[HSET_UNIFIED_CNT];    // unified values (int, double, fs etc...)
} hset_unified;
*/

// ------------------------------------- REDUCE  -----------------------------------------
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
extern void                 hset_maxlen_fs  (hset_accum *acc, value64 v);
extern void                 hset_minlen_fs  (hset_accum *acc, value64 v);
extern void                 hset_sumlen_fs  (hset_accum *acc, value64 v);
//
extern void                 hset_agg_fs     (hset_accum *acc, value64 v);

// ------------------------------------- FILTER -----------------------------------------

typedef bool                (*hset_predicate_t)(value64 v, value64 data);
typedef bool                (*hset_predicate2_t)(value64 v, value64 data1, value64 data2); // between etc...
// engine
extern hset                 *hset_filter(hset *restrict se, hset_predicate_t pred, value64 data);
extern hset                  hset_init_filter(const hset *restrict src, hset_predicate_t pred, value64 data);
// common filters
extern bool                  hset_filter_true(value64 v, value64 data);
extern bool                  hset_filter_false(value64 v, value64 data);
// ---------------- fs filters -----------------
// fs filters (assuming v as fs*), data as int, check len >= data
extern bool                  hset_filter_fsminlen_int(value64 v, value64 data);
// fs filters (assuming v as fs*), data as int, check len <= data
extern bool                  hset_filter_fsmaxlen_int(value64 v, value64 data);
// fs filters (assuming v as fs*), data as int, check len == data
extern bool                  hset_filter_fslen_int(value64 v, value64 data);
// Проверка префикса (data.sval – строка-префикс)
extern bool                  hset_filter_fsprefix_str(value64 v, value64 data);
//
extern bool                  hset_filter_fslike_str(value64 v, value64 data);
extern bool                  hset_filter_fsulike_str(value64 v, value64 data);

// --------- simplifyers over filters ---------
extern hset                  hset_create_fslike_str_common(const hset *restrict se, const char *restrict pattern, hset_predicate_t filter);
// sql-like create as select where like
static inline hset           hset_create_fslike_str(const hset *restrict se, const char *restrict pattern){
    return hset_create_fslike_str_common(se, pattern, hset_filter_fslike_str);
}
// sql-like create as select where ulike
static inline hset           hset_create_fsulike_str(const hset *restrict se, const char *restrict pattern){
    return hset_create_fslike_str_common(se, pattern, hset_filter_fsulike_str);
}
extern hset                 *hset_delete_fs_notlike_str_common(hset *restrict se, const char *restrict pattern, hset_predicate_t filter);
// sql-like create as select where like
static inline hset          *hset_delete_fs_notlike_str(hset *restrict se, const char *restrict pattern){
    return hset_delete_fs_notlike_str_common(se, pattern, hset_filter_fslike_str);
}
// sql-like create as select where ulike
static inline hset          *hset_delete_fs_notulike_str(hset *restrict se, const char *restrict pattern){
    return hset_delete_fs_notlike_str_common(se, pattern, hset_filter_fsulike_str);
}


#endif /* !_HASHSET_UTILS_H */

