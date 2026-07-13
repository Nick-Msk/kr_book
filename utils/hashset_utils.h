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
// filter types
typedef bool                (*hset_predicate_t)(value64 v, value64 data);
typedef bool                (*hset_predicate2_t)(value64 v, value64 data1, value64 data2); // between etc...

// ------------------------------- engine ------------------------------------------
extern hset                 *hset_filter(hset *restrict se, hset_predicate_t pred, value64 data);
extern hset                  hset_init_filter(const hset *restrict src, hset_predicate_t pred, value64 data);

extern hset                 *hset_filter2(hset *restrict se, hset_predicate2_t pred2, value64 data1, value64 data2);
extern hset                  hset_init_filter2(const hset *restrict src, hset_predicate2_t pred2, value64 data1, value64 data2);

// ------------------------ simplifiers predicate engine ---------------------------
// fs - str
// sql-like create as select:fs where predicate:str
extern hset                  hset_create_fs_str_filter(const hset *restrict se, const char *restrict pattern, hset_predicate_t filter);
// sql-like delete :fs where NOT predicate:str
extern hset                 *hset_apply_fs_str_filter(hset *restrict se, const char *restrict pattern, hset_predicate_t filter);
// fs - int
// sql-like create as select:fs where predicate:int
extern hset                  hset_create_fs_int_filter(const hset *restrict se, int data, hset_predicate_t filter);
// sql-like delete :fs where NOT predicate:int
extern hset                 *hset_apply_fs_int_filter(hset *restrict se, int data, hset_predicate_t filter);

// int - int
// sql-like create as select:int where predicate:int
extern hset                  hset_create_int_int_filter(const hset *restrict se, int value, hset_predicate_t filter);
// sql-like delete :int where NOT predicate:int
extern hset                 *hset_apply_int_int_filter(hset *restrict se, int value, hset_predicate_t filter);
// int - (int, int)
// sql-like create as select:int where predicate:(int, int)
extern hset                  hset_create_int_int_filter2(const hset *restrict se, int value1, int value2, hset_predicate2_t filter2);
// sql-like apply:int where predicate:(int, int)
extern hset                 *hset_apply_int_int_filter2(hset *restrict se, int value1, int value2, hset_predicate2_t filter2);

// --------- simplifyers over filters ---------
// sql-like create :fs as select where length >=  :int
static inline hset           hset_create_fsminlen_int(const hset *restrict se, int len){
    return hset_create_fs_int_filter(se, len, value64_filter_fsminlen_int);
}
// sql-like delete :fs where NOT length >=  :int
static inline hset          *hset_apply_fsminlen_int(hset *restrict se, int len){
    return hset_apply_fs_int_filter(se, len, value64_filter_fsminlen_int);
}
// sql-like create :fs as select where length <=  :int
static inline hset           hset_create_fsmaxlen_int(const hset *restrict se, int len){
    return hset_create_fs_int_filter(se, len, value64_filter_fsmaxlen_int);
}
// sql-like delete :fs where NOT length <=  :int
static inline hset          *hset_apply_fsmaxlen_int(hset *restrict se, int len){
    return hset_apply_fs_int_filter(se, len, value64_filter_fsmaxlen_int);
}
// sql-like create :fs as select where length == :int
static inline hset           hset_create_fslen_int(const hset *restrict se, int len){
    return hset_create_fs_int_filter(se, len, value64_filter_fslen_int);
}
// sql-like delete :fs where NOT length == :int
static inline hset          *hset_apply_fslen_int(hset *restrict se, int len){
    return hset_apply_fs_int_filter(se, len, value64_filter_fslen_int);
}
// sql-like create :fs as select where prefix == :str
static inline hset           hset_create_fsprefix_str(const hset *restrict se, const char *restrict pattern){
    return hset_create_fs_str_filter(se, pattern, value64_filter_fsprefix_str);
}
// sql-like delete :fs where NOT prefix == :str
static inline hset          *hset_apply_fsprefix_str(hset *restrict se, const char *restrict pattern){
    return hset_apply_fs_str_filter(se, pattern, value64_filter_fsprefix_str);
}

// sql-like create :fs as select where like :str
static inline hset           hset_create_fslike_str(const hset *restrict se, const char *restrict pattern){
    return hset_create_fs_str_filter(se, pattern, value64_filter_fslike_str);
}
// sql-like create :fs as select where ulike :str
static inline hset           hset_create_fsulike_str(const hset *restrict se, const char *restrict pattern){
    return hset_create_fs_str_filter(se, pattern, value64_filter_fsulike_str);
}
// sql-like delete :fs where NOT like :str
static inline hset          *hset_apply_fslike_str(hset *restrict se, const char *restrict pattern){
    return hset_apply_fs_str_filter(se, pattern, value64_filter_fslike_str);
}
// sql-like delete :fs where NOT ulike :str
static inline hset          *hset_apply_fsulike_str(hset *restrict se, const char *restrict pattern){
    return hset_apply_fs_str_filter(se, pattern, value64_filter_fsulike_str);
}

// int - int
// ---------- less than ----------
static inline hset          hset_create_intlt_int(const hset *restrict se, int v) {
    return hset_create_int_int_filter(se, v, value64_filter_intlt_int);
}
static inline hset         *hset_apply_intlt_int(hset *restrict se, int v) {
    return hset_apply_int_int_filter(se, v, value64_filter_intlt_int);
}

// ---------- less or equal ----------
static inline hset          hset_create_intle_int(const hset *restrict se, int v) {
    return hset_create_int_int_filter(se, v, value64_filter_intle_int);
}
static inline hset         *hset_apply_intle_int(hset *restrict se, int v) {
    return hset_apply_int_int_filter(se, v, value64_filter_intle_int);
}

// ---------- greater than ----------
static inline hset          hset_create_intgt_int(const hset *restrict se, int v) {
    return hset_create_int_int_filter(se, v, value64_filter_intgt_int);
}
static inline hset         *hset_apply_intgt_int(hset *restrict se, int v) {
    return hset_apply_int_int_filter(se, v, value64_filter_intgt_int);
}

// ---------- greater or equal ----------
static inline hset          hset_create_intge_int(const hset *restrict se, int v) {
    return hset_create_int_int_filter(se, v, value64_filter_intge_int);
}
static inline hset         *hset_apply_intge_int(hset *restrict se, int v) {
    return hset_apply_int_int_filter(se, v, value64_filter_intge_int);
}

// ---------- equal ----------
static inline hset          hset_create_inteq_int(const hset *restrict se, int v) {
    return hset_create_int_int_filter(se, v, value64_filter_inteq_int);
}
static inline hset         *hset_apply_inteq_int(hset *restrict se, int v) {
    return hset_apply_int_int_filter(se, v, value64_filter_inteq_int);
}

// ---------- not equal ----------
static inline hset          hset_create_intne_int(const hset *restrict se, int v) {
    return hset_create_int_int_filter(se, v, value64_filter_intne_int);
}
static inline hset         *hset_apply_intne_int(hset *restrict se, int v) {
    return hset_apply_int_int_filter(se, v, value64_filter_intne_int);
}

// ------------- between int : int ---------------
static inline hset         hset_create_intbetween_int_int(const hset *restrict se, int v1, int v2) {
    return hset_create_int_int_filter2(se, v1, v2, value64_filter2_intbetween_int_int);
}
static inline hset *hset_apply_intbetween_int_int(hset *restrict se, int v1, int v2) {
    return hset_apply_int_int_filter2(se, v1, v2, value64_filter2_intbetween_int_int);
}

#endif /* !_HASHSET_UTILS_H */

