/********************************************************************
                    VALUE64(128) SET MODULE IMPLEMENTATION
********************************************************************/

// common include
#include "value64.h"

// -------------------------- TYPE SUPPORT API ------------------------

// Вся информация о типах в одном месте!
static const value64_typeinfo           value64_info[] = {
    [VALUE64_UNKNOWN]    = {"INVALID",     0,              false    , "VALUE64_UNKNOWN"},
    [VALUE64_INT]        = {"INT",         sizeof(int),    true     , "VALUE64_INT"},
    [VALUE64_LNG]        = {"LNG",         sizeof(long),   true     , "VALUE64_LNG"},
    [VALUE64_DBL]        = {"DBL",         sizeof(double), true     , "VALUE64_DBL"},
    [VALUE64_FS]         = {"FS",          sizeof(fs *),   true     , "VALUE64_FS"},
    [VALUE64_PTR]        = {"PTR",         sizeof(void *), true     , "VALUE64_PTR"},
    [VALUE64_STR]        = {"STR",         sizeof(char *), true     , "VALUE64_STR"},
    [VALUE64_TYPE_COUNT] = {"",            0,              false    , ""}
};

_Static_assert(COUNT(value64_info) == VALUE64_TYPE_COUNT + 1,
               "Размер массива value65_info не совпадает с количеством типов!");

const                   value64_typeinfo* value64_info_get(value64_type typ) {
    // Проверка границ массива
    if (typ < 0 || typ >= COUNT(value64_info) || !value64_info[typ].is_valid)
        return NULL;
    return &value64_info[typ];
}

static inline bool      is_long_int_range(long v) {
    return v >= INT_MIN && v <= INT_MAX;
}
static inline bool      is_dbl_long_range(double v) {
    return v >= (double) LONG_MIN && v <= (double) LONG_MAX;
}
static inline bool      is_dbl_int_range(double v) {
    return v >= (double) INT_MIN && v <= (double) INT_MAX;
}

// the part of mass creation API, probably'll be changed
// create value from pointer, value64 constructor ANY type, MOVE semantic
value64                   value64_pcopy_move(void *p, value64_type typ, bool move){
    invraisecode(p != NULL, ERR_NULLABLE_PTR, "Null pointer");
    value64     tmp = VALUE64_ZERO;  // init
    switch (typ){
        case VALUE64_INT:
            tmp.ival = *(const int *) p;
        break;
        case VALUE64_LNG:
            tmp.lval = *(const long *) p;
        break;
        case VALUE64_DBL:
            tmp.dval = *(const double *) p;
        break;
        case VALUE64_PTR:
            if (move)
                userraiseint(ERR_UNSUPPORTED_TYPE, "VALUE64_PTR can't be moved");
            tmp.pval = *(void * const *) p;
        break;
        case VALUE64_STR:
            if (move)
                tmp.sval = (char *) p;  //MOVE POINTER
            else
                tmp = value64_createstr(p);
        break;
        // create fs body in head with FS_FLAG_BODYALLOC
        case VALUE64_FS:
            if (move)
                tmp = value64_movefs(p);
            else
                tmp = value64_createfs(p);
        break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "type %d %s isn't suppoted", typ, value64_typename(typ) );
        break;
    }
    return tmp;
}

value64_ConverterFunc conv_matrix[VALUE64_TYPE_COUNT][VALUE64_TYPE_COUNT] = {
    [VALUE64_INT] = {
        [VALUE64_LNG] = value64_convert_int_to_lng,
        [VALUE64_DBL] = value64_convert_int_to_dbl,
        [VALUE64_FS]  = value64_convert_int_to_fs,
        [VALUE64_STR] = value64_convert_int_to_str
    },
    [VALUE64_LNG] = {
        [VALUE64_INT] = value64_convert_lng_to_int,
        [VALUE64_DBL] = value64_convert_lng_to_dbl,
        [VALUE64_FS]  = value64_convert_lng_to_fs,
        [VALUE64_STR] = value64_convert_lng_to_str
    },
    [VALUE64_DBL] = {
        [VALUE64_INT] = value64_convert_dbl_to_int,
        [VALUE64_LNG] = value64_convert_dbl_to_lng,
        [VALUE64_FS]  = value64_convert_dbl_to_fs,
        [VALUE64_STR] = value64_convert_dbl_to_str
    },
    [VALUE64_FS] = {
        [VALUE64_INT] = value64_convert_fs_to_int,
        [VALUE64_LNG] = value64_convert_fs_to_lng,
        [VALUE64_DBL] = value64_convert_fs_to_dbl,
        [VALUE64_STR] = value64_convert_fs_to_str,
        [VALUE64_FS]  = value64_convert_fs_to_fs
    },
    [VALUE64_STR] = {
        [VALUE64_INT] = value64_convert_str_to_int,
        [VALUE64_LNG] = value64_convert_str_to_lng,
        [VALUE64_DBL] = value64_convert_str_to_dbl,
        [VALUE64_FS]  = value64_convert_str_to_fs,
        [VALUE64_STR] = value64_convert_str_to_str
    }
};

value64_ConverterMoveFunc conv_move_matrix[VALUE64_TYPE_COUNT][VALUE64_TYPE_COUNT] = {
    [VALUE64_FS] = {
        [VALUE64_STR] = value64_convert_move_fs_to_str,
        [VALUE64_FS]  = value64_convert_move_fs_to_fs
    },
    [VALUE64_STR] = {
        [VALUE64_FS]  = value64_convert_move_str_to_fs,
        [VALUE64_STR] = value64_convert_move_str_to_str
    }
};

unsigned long               value64_lhash(value64 value, value64_type typ){
    // probably it's better to calc hash by u64 attr (except fs for sure)
    value64      tmp = VALUE64_ZERO;
    switch (typ){
        case VALUE64_INT:
            tmp.u64 = (uint64_t) value64_int(value);
        break;
        case VALUE64_LNG:
            tmp.u64 = (uint64_t) value64_long(value);
        break;
        case VALUE64_DBL:
            tmp.dval = value64_dbl(value);
        break;
        case VALUE64_PTR:
            tmp.u64 = (uint64_t) value64_ptr(value);    // or just do nothing as for HSET_DBL
        break;
        case VALUE64_FS:
            return  hash_djb2(fs_str(value64_fs(value) ) );
        break;
        case VALUE64_STR:
            return  hash_djb2(value64_str(value) );
        break;
        default:
        break;
    }
    return hash_long(tmp.u64);
}
// search type id by type name
value64_type            value64_gettype(const char *str){
    if (str){
        for (size_t i = 0; i < COUNT(value64_info); i++) {
            if (strcmp(str, value64_info_get(i)->name) == 0)
                return (value64_type) i;
         }
    }
    return VALUE64_UNKNOWN;
}

// move to EXISTING object, that is NOT a constructor
// move switcher to EXISTING object
value64                     *value64_move(value64 *restrict target, value64 *restrict source, value64_type typ){
    invraisecode(target && source,  ERR_NULLABLE_PTR, "Null pointers %p %p", target, source);

    switch (typ){
        case VALUE64_FS:    // note: this's NOT the same as value64_movefs!
            target->fsval = fs_moveto_heap(source->fsval);  // no need to null source, fs_moveto_heap'll do that
            if (!target->fsval)
                userraiseint(ERR_UNABLE_ALLOCATE, "Unable to alloc new fs body");
        break;
        case VALUE64_DBL:
            target->dval = source->dval;
            source->dval = 0.0;
        break;
        default:    // ALL others type even VALUE64_STR follows the same logic!
            target->u64 = source->u64;
            source->u64 = 0L;   // u64 cover all types
        break;
    }
    return target;
}

// finders
int                         value64_search(value64 val, value64_type typ, const value64 *arr, int sz){
    invraisecode(arr || sz == 0, ERR_NULLABLE_PTR, "Null pointer while sz > 0 %p %d", arr, sz);

    value64_Comparator comp = value64_getComparator(typ);
    for (int i = 0; i < sz; i++)
        if (comp(val, arr[i]) == 0)
            return logsimpleret(i, "Found %d", i);
    return logsimpleerr(-1, "Not found"); // just a stub
}
int                         value64_revsearch(value64 val, value64_type typ, const value64 *arr, int sz){
    invraisecode(arr || sz == 0, ERR_NULLABLE_PTR, "Null pointer while sz > 0 %p %d", arr, sz);

    value64_Comparator comp = value64_getComparator(typ);
    for (int i = sz; i > 0; i--)
        if (comp(val, arr[i - 1]) == 0)
            return logsimpleret(i - 1, "Found reverse %d", i - 1); 
    return logsimpleerr(-1, "Not found"); // just a stub
}
// arr MUST be ordered
int                         value64_binsearch(value64 val, value64_type typ, const value64 *arr, int sz){
    //bsearch(const void *key, const void *base, size_t nel, size_t width, int (*compar) (const void *, const void *));
    invraisecode(arr || sz == 0, ERR_NULLABLE_PTR, "Null pointer while sz > 0 %p %d", arr, sz);
    if (sz == 0)
        return logsimpleerr(-1, "Noting to find, sz == 0");
    // const value64 *find = bsearch(&val, arr, 
    return -1;
}
// value comparators, low-level, no checking
int                         value64_int_comp(value64 v1, value64 v2) {
    return compare_int(v1.ival, v2.ival);
}

int                         value64_long_comp(value64 v1, value64 v2) {
    return compare_long(v1.lval, v2.lval);
}

int                         value64_dbl_comp(value64 v1, value64 v2) {
    return compare_dbl(v1.dval, v2.dval);
}

int                         value64_fs_comp(value64 v1, value64 v2) {
    return fs_cmp(v1.fsval, v2.fsval);
}

int                         value64_str_comp(value64 v1, value64 v2) {
    return strcmp(v1.sval, v2.sval);
}

int                         value64_ptr_comp(value64 v1, value64 v2) {
    return compare_ptr(v1.pval, v2.pval);
}
// value, reverse, low-level, no checking
int                         value64_int_rev_comp(value64 v1, value64 v2) {
    return -compare_int(v1.ival, v2.ival);
}

int                         value64_long_rev_comp(value64 v1, value64 v2) {
    return -compare_long(v1.lval, v2.lval);
}

int                         value64_dbl_rev_comp(value64 v1, value64 v2) {
    return -compare_dbl(v1.dval, v2.dval);
}

int                         value64_fs_rev_comp(value64 v1, value64 v2) {
    return -fs_cmp(v1.fsval, v2.fsval);
}

int                         value64_str_rev_comp(value64 v1, value64 v2) {
    return -strcmp(v1.sval, v2.sval);
}

int                         value64_ptr_rev_comp(value64 v1, value64 v2) {
    return -compare_ptr(v1.pval, v2.pval);
}

// common value comparator (slow, for single-use), NULL checking
int                     value64_compare(value64 v1, value64 v2, value64_type typ){
    switch (typ){
        case VALUE64_INT:
            return compare_int(v1.ival, v2.ival);
        break;
        case VALUE64_LNG:
            return compare_long(v1.lval, v2.lval);
        break;
        case VALUE64_DBL:
            return compare_dbl(v1.dval, v2.dval);
        break;
        case VALUE64_PTR:
            return compare_ptr(v1.pval, v2.pval);
        break;
        case VALUE64_FS:
            if (!v1.fsval || !v2.fsval)
                userraiseint(ERR_NULLABLE_PTR, "Null pointers %p %p", v1.fsval, v2.fsval);
            return fs_cmp(v1.fsval, v2.fsval);
        break;
        case VALUE64_STR:
            if (!v1.sval || !v2.sval)
                userraiseint(ERR_NULLABLE_PTR, "Null pointers %p %p", v1.sval, v2.sval);
            return strcmp(v1.sval, v2.sval);
        break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "%s: %d", value64_typename(typ), typ);
            return 0;
    }
}
// pointer comparator, switch fow now, but probably table-function is required
// slow, for single-use, with NUL-check
int                         value64_pt_compare(const value64* restrict v1, const value64 *restrict v2, value64_type typ){
    invraisecode(v1 != NULL && v2 != NULL, ERR_NULLABLE_PTR, "Null pointers %p %p", v1, v2);
    switch (typ){
        case VALUE64_INT:
            return value64_pint_comp(v1, v2); //compare_pint(&val1->ival, &val2->ival);
        case VALUE64_LNG:
            return value64_plong_comp(v1, v2);  //compare_plong(&val1->lval, &val2->lval);
        case VALUE64_DBL:
            return value64_pdbl_comp(v1, v2);   //compare_pdbl(&val1->dval, &val2->dval);
        case VALUE64_FS:
            invraisecode(v1->fsval != NULL && v2->fsval != NULL, ERR_NULLABLE_PTR, "Null pointers %p %p", v1->fsval, v2->fsval);
            return value64_pfs_comp(v1, v2);     //compare_fs(val1->fsval, val2->fsval);
        case VALUE64_STR:
            invraisecode(v1->sval != NULL && v2->sval != NULL, ERR_NULLABLE_PTR, "Null pointers %p %p", v1->sval, v2->sval);
            return value64_pstr_comp(v1, v2);   // compare_str(val1->strval, val2->strval);
        case VALUE64_PTR:
            return value64_pptr_comp(v1, v2); //compare_pptr(&val1->pval, &val2->pval);
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE_CONV, "No comparator for %d:%s",
                 typ, value64_typename(typ) );
    }
    return 0;
}
// pointer comparators, for qsort, bsearch etc...  LOW LEVEL, no checking for NULL
// just a wrapper over imline pointer comparators with (const void *,...
// They must use comparators from common.h and fs.h (for FS)
int                         value64_pint_comp(const void *restrict v1, const void *restrict v2){
    const value64 *val1 = (const value64 *) v1;
    const value64 *val2 = (const value64 *) v2;
    return compare_int(val1->ival, val2->ival); // from common.h
}
int                         value64_plong_comp(const void *restrict v1, const void *restrict v2){
    const value64 *val1 = (const value64 *) v1;
    const value64 *val2 = (const value64 *) v2;
    return compare_long(val1->lval, val2->lval);
}
int                         value64_pdbl_comp(const void *restrict v1, const void *restrict v2){
    const value64 *val1 = (const value64 *) v1;
    const value64 *val2 = (const value64 *) v2;
    return compare_dbl(val1->dval, val2->dval);
}
int                         value64_pptr_comp(const void *restrict v1, const void *restrict v2){
    const value64 *val1 = (const value64 *) v1;
    const value64 *val2 = (const value64 *) v2;
    return compare_ptr(val1->pval, val2->pval);
}
int                         value64_pstr_comp(const void *restrict v1, const void *restrict v2){
    const value64 *val1 = (const value64 *) v1;
    const value64 *val2 = (const value64 *) v2;
    return strcmp(val1->sval, val2->sval);
}
int                         value64_pfs_comp(const void *restrict v1, const void *restrict v2){
    const value64 *val1 = (const value64 *) v1;
    const value64 *val2 = (const value64 *) v2;
    return fs_cmp(val1->fsval, val2->fsval);
}
// reverse pointer comparators
int                         value64_pint_rev_comp(const void *restrict v1, const void *restrict v2){
    const value64 *val1 = (const value64 *) v1;
    const value64 *val2 = (const value64 *) v2;
    return -compare_int(val1->ival, val2->ival); // from common.h
}
int                         value64_plong_rev_comp(const void *restrict v1, const void *restrict v2){
    const value64 *val1 = (const value64 *) v1;
    const value64 *val2 = (const value64 *) v2;
    return -compare_long(val1->lval, val2->lval);
}
int                         value64_pdbl_rev_comp(const void *restrict v1, const void *restrict v2){
    const value64 *val1 = (const value64 *) v1;
    const value64 *val2 = (const value64 *) v2;
    return -compare_dbl(val1->dval, val2->dval);
}
int                         value64_pptr_rev_comp(const void *restrict v1, const void *restrict v2){
    const value64 *val1 = (const value64 *) v1;
    const value64 *val2 = (const value64 *) v2;
    return -compare_ptr(val1->pval, val2->pval);
}
int                         value64_pstr_rev_comp(const void *restrict v1, const void *restrict v2){
    const value64 *val1 = (const value64 *) v1;
    const value64 *val2 = (const value64 *) v2;
    return -strcmp(val1->sval, val2->sval);
}
int                         value64_pfs_rev_comp(const void *restrict v1, const void *restrict v2){
    const value64 *val1 = (const value64 *) v1;
    const value64 *val2 = (const value64 *) v2;
    return -fs_cmp(val1->fsval, val2->fsval);
}


// ----------------------------- CONVERTERS ----------------------------------------

// TODO: hardcoding, refactoring is required!!!
bool                        value64_is_convertable(value64 v, value64_type from, value64_type to) {
    if (from == VALUE64_LNG && to == VALUE64_INT)
        return is_long_int_range(value64_long(v));

    if (from == VALUE64_DBL && to == VALUE64_INT)
        return is_dbl_int_range(value64_dbl(v));

    if (from == VALUE64_DBL && to == VALUE64_LNG)
        return is_dbl_long_range(value64_dbl(v));
    return true;
}
// converted, COPY semantic
value64                     value64_convert(value64 v, value64_type from, value64_type to) {

    value64_ConverterFunc func = conv_matrix[from][to];
    if (func != NULL) {
        return logsimpleret(func(v), "Converted from %s to %s", value64_typename(from), value64_typename(to) );
    } else
        return logsimpleret(v, "No conv is required for %s -> %s", value64_typename(from), value64_typename(to) );
}
// --- Группа INT ---
value64                     value64_convert_int_to_lng(value64 v) {
    return  value64_createlong((long) value64_int(v) );
}
value64                     value64_convert_int_to_dbl(value64 v) {
    return  value64_createdbl((double) value64_int(v) );
}
value64                     value64_convert_int_to_fs(value64 v) {
    value64     result = VALUE64_ZERO;
    fs          tmp = fscopyf("%d", value64_int(v));
    result.fsval = fs_moveto_heap(&tmp);
    return result;
}
value64                     value64_convert_int_to_str(value64 v) {
    char        buf[100];       // CAn't use fs in STR
    snprintf(buf, sizeof(buf) - 1, "%d", value64_int(v));
    return value64_createstr(buf);
}
// --- Группа LNG ---
value64                     value64_convert_lng_to_int(value64 v) {
    if (!is_long_int_range(value64_long(v)) )
        userraiseint(ERR_OUT_OF_RANGE, "Long->int overflow");
    return value64_createint( (int) value64_long(v) );
}
value64                     value64_convert_lng_to_dbl(value64 v) {
    return value64_createdbl((double) value64_long(v) );
}
value64                     value64_convert_lng_to_fs(value64 v) {
    value64     result = VALUE64_ZERO;
    fs          tmp = fscopyf("%ld", value64_long(v) );
    result.fsval = fs_moveto_heap(&tmp);
    return result;
}
value64                     value64_convert_lng_to_str(value64 v) {
    char        buf[100];
    snprintf(buf, sizeof(buf) - 1, "%ld", value64_long(v) );
    return value64_createstr(buf);
}

// --- Группа DBL ---
value64                     value64_convert_dbl_to_int(value64 v) {
    if (!is_dbl_int_range(value64_dbl(v)))
        userraiseint(ERR_OUT_OF_RANGE, "Dbl->int overflow");
    return value64_createint((int) value64_dbl(v) );
}
value64                     value64_convert_dbl_to_lng(value64 v) {
    if (!is_dbl_long_range(value64_dbl(v)))
        userraiseint(ERR_OUT_OF_RANGE, "Dbl->long overflow");
    return value64_createlong((long) value64_dbl(v) );
}
value64                     value64_convert_dbl_to_fs(value64 v) {
    value64     result = VALUE64_ZERO;
    fs tmp = fscopyf("%g", value64_dbl(v) );       // context must be used! TODO:
    result.fsval = fs_moveto_heap(&tmp);
    return result;
}
value64                     value64_convert_dbl_to_str(value64 v) {
    char        buf[100];
    snprintf(buf, sizeof(buf), "%lf", value64_dbl(v));    // // context must be used! TODO:
    return value64_createstr(buf);
}

// --- Группа FS ---
value64                     value64_convert_fs_to_int(value64 v) {
    fs          *fsval = value64_fs(v);
    return value64_createint(fs_getint(fsval) );
}
value64                     value64_convert_fs_to_lng(value64 v) {
    fs          *fsval = value64_fs(v);
    return value64_createlong(fs_getlong(fsval) );
}
value64                     value64_convert_fs_to_dbl(value64 v) {
    fs          *fsval = value64_fs(v);
    return value64_createdbl(fs_getdouble(fsval) );
}
value64                     value64_convert_fs_to_str(value64 v) {
    fs          *fsval = value64_fs(v);
    return value64_createstr(fsval->v);
}
value64                     value64_convert_fs_to_fs(value64 v){
    fs          *fsval = value64_fs(v);
    return value64_createfs(fsval);
}
// --- Группа STR ---
value64                     value64_convert_str_to_int(value64 v) {
    char        *sval = value64_str(v);
    value64     result = VALUE64_ZERO;
    if (!try_parse_int(sval, &result.ival))
        userraiseint(ERR_INVALID_CONVERSION, "str->int fail");
    return result;
}
value64                     value64_convert_str_to_lng(value64 v) {
    char        *sval = value64_str(v);
    value64     result = VALUE64_ZERO;
    if (!try_parse_long(sval, &result.lval))
        userraiseint(ERR_INVALID_CONVERSION, "str->long fail");
    return result;
}
value64                     value64_convert_str_to_dbl(value64 v) {
    char        *sval = value64_str(v);
    value64     result = VALUE64_ZERO;
    if (!try_parse_double(sval, &result.dval))
        userraiseint(ERR_INVALID_CONVERSION, "str->double fail");
    return result;
}
value64                     value64_convert_str_to_fs(value64 v) {
    char        *sval = value64_str(v);
    value64     result = VALUE64_ZERO;
    result.fsval = fs_heapcopy(sval);
    return result;
}
value64                     value64_convert_str_to_str(value64 v) {
    char        *sval = value64_str(v);
    value64     result = VALUE64_ZERO;
    result = value64_createstr(sval);
    return result;
}

// converter, MOVE semantic
value64                     value64_convert_move(value64 *v, value64_type from, value64_type to) {
    value64_ConverterMoveFunc func = conv_move_matrix[from][to];
    if (func != NULL) {
        return logsimpleret(func(v), "Move converted from %s to %s", value64_typename(from), value64_typename(to) );
    } else
        userraiseint(ERR_UNSUPPORTED_TYPE_CONV, "from %d:%s to %d:%s",
                 from, value64_typename(from), to, value64_typename(to));
    return VALUE64_ZERO;
}
// FS
value64                     value64_convert_move_fs_to_str(value64 *v){
    value64     result = VALUE64_ZERO;
    result.sval = fs_movefrom_heapstr(&v->fsval);
    return result;
}
value64                     value64_convert_move_fs_to_fs(value64 *v){
    value64     result = VALUE64_ZERO;
    result.fsval = v->fsval;
    v->fsval = 0;  // NO FREE HERE
    return result;
}
// STR
value64                     value64_convert_move_str_to_fs(value64 *v){
    value64     result = VALUE64_ZERO;
    result.fsval = fs_moveto_heapstr(&v->sval);
    v->sval = NULL;
    return result;
}
value64                     value64_convert_move_str_to_str(value64 *v){
    value64     result = VALUE64_ZERO;
    result.sval = v->sval;
    v->sval = NULL;
    return result;
}
// ------------------------ PRINTERS/CHECKERS ---------------------------------------

void                        value64_fprint(FILE *restrict out, const char *restrict msg, value64 val, value64_type typ){
    if (out){
        if (msg)
            fprintf(out, "%s ", msg);
        switch (typ){
            case VALUE64_INT:
                fprintf(out, "%d", val.ival);
            break;
            case VALUE64_LNG:
                fprintf(out, "%ld", val.lval);
            break;
            case VALUE64_DBL:
                fprintf(out, "%lf", val.dval);
            break;
            case VALUE64_PTR:
                fprintf(out, "%p", val.pval);
            break;
            case VALUE64_STR:
                fprintf(out, "%s", val.sval);
            break;
            case VALUE64_FS:
                fs_fprint(out, val.fsval, 0);
            break;
            default:
                fprintf(out, "Unsupported %d!\n", typ);
            break;
        }
    }
}

// --------------------------------- SERIALIZATION -----------------------------------------

// ---------------------------------------- Testing ------------------------------------------
#ifdef VALUE64TESTING

#include "test.h"

//types for testing

// ------------------------- TEST init_free ---------------------------------

static TestStatus
tf_init_free(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. int */
    test_sub("subtest %d: value64 int", ++subnum);
    {
        value64 v = value64_createint(42);
        test_validatefree(
            v.ival == 42,
            value64_free(v, VALUE64_INT),
            "Int value mismatch: got %d, expected 42", v.ival
        );
        test_validatefree(
            value64_int(v) == 42,
            value64_free(v, VALUE64_INT),
            "Int value mismatch: got %d, expected 42", v.ival
        );
        // для int освобождение не требуется
        value64_free(v, VALUE64_INT);
    }

    /* 2. long */
    test_sub("subtest %d: value64 long", ++subnum);
    {
        value64 v = value64_createlong(1234567890L);
        test_validate(
            v.lval == 1234567890L,
            "Long value mismatch: got %ld, expected 1234567890", v.lval
        );
        test_validate(
            value64_long(v) == 1234567890L,
            "Long value mismatch: got %ld, expected 1234567890", v.lval
        );
    }

    /* 3. double */
    test_sub("subtest %d: value64 double", ++subnum);
    {
        value64 v = value64_createdbl(3.14159265);
        test_validate(
            fabs(v.dval - 3.14159265) < 0.00000001,
            "Double value mismatch: got %f, expected 3.14159265", v.dval
        );
        test_validate(
            fabs(value64_dbl(v) - 3.14159265) < 0.00000001,
            "Double value mismatch: got %f, expected 3.14159265", v.dval
        );
    }

    /* 4. pointer */
    test_sub("subtest %d: value64 pointer", ++subnum);
    {
        int x = 77;
        value64 v = value64_createptr(&x);
        test_validate(
            v.pval == &x,
            "Pointer mismatch: got %p, expected %p", v.pval, (void*)&x
        );
        test_validate(
            value64_ptr(v) == &x,
            "Pointer mismatch: got %p, expected %p", v.pval, (void*)&x
        );
    }

    /* 5. fs (copy) */
    test_sub("subtest %d: value64 createfs (copy)", ++subnum);
    {
        const char *text = "hello value64";
        fs orig = fscopy(text);
        value64 v = value64_createfs(&orig);

        test_validatefree(
            strcmp(fs_str(v.fsval), text) == 0,
            (fsfree(orig), fs_free(v.fsval)),
            "FS copy mismatch: got '%s', expected '%s'", fs_str(v.fsval), text
        );
        test_validatefree(
            strcmp(fs_str(value64_fs(v) ), text) == 0,
            (fsfree(orig), fs_free(v.fsval)),
            "FS copy mismatch: got '%s', expected '%s'", fs_str(v.fsval), text
        );
        test_validatefree(
            fs_bodyalloc(v.fsval),
            (fsfree(orig), fs_free(v.fsval)),
            "Copied fs must have FS_FLAG_BODYALLOC flag"
        );

        fsfree(orig);
        value64_freefs(v);
        fs_alloc_check(true);
    }

    /* 6. fs (move) */
    test_sub("subtest %d: value64 movefs", ++subnum);
    {
        const char *text = "move me";
        fs orig = fscopy(text);
        value64 v = value64_movefs(&orig);   // orig будет опустошён

        test_validatefree(
            strcmp(fs_str(v.fsval), text) == 0,
            value64_freefs(v),
            "Moved fs value mismatch: got '%s', expected '%s'", fs_str(v.fsval), text
        );
        test_validatefree(
            strcmp(fs_str(value64_fs(v) ), text) == 0,
            value64_freefs(v),
            "Moved fs value mismatch: got '%s', expected '%s'", fs_str(v.fsval), text
        );
        test_validatefree(
            fs_bodyalloc(v.fsval),
            value64_freefs(v),
            "Moved fs must have FS_FLAG_BODYALLOC flag"
        );
        // Проверяем, что оригинал действительно опустошён
        test_validatefree(
            fslen(orig) == 0 && fsstr(orig) == NULL,
            value64_freefs(v),
            "After move, original fs must be empty (len=%d, str=%p)", fslen(orig), (void*)fsstr(orig)
        );

        value64_freefs(v);
        fsfree(orig);   // orig пуст, но fsfree безопасен
        fs_alloc_check(true);
    }

    /* 7. Множественные вызовы и проверка утечек */
    test_sub("subtest %d: value64 multiple create/free (leak check)", ++subnum);
    {
        const char *words[] = {"one", "two", "three"};
        value64 vals[COUNT(words)];

        for (int i = 0; i < COUNT(words); i++) {
            vals[i] = value64_createfs(
                &(fs){ .v = (char*)words[i], .len = strlen(words[i]), .sz = 0, .flags = FS_FLAG_STATIC }
            );   // временный fs для создания копии
        }

        for (int i = 0; i < COUNT(words); i++) {
            test_validatefree(
                strcmp(fs_str(vals[i].fsval), words[i]) == 0,
                (value64_freefs(vals[0]), value64_freefs(vals[1]), value64_freefs(vals[2])),
                "FS %d mismatch: got '%s', expected '%s'", i, fs_str(vals[i].fsval), words[i]
            );
        }

        for (int i = 0; i < COUNT(words); i++)
            value64_freefs(vals[i]);
        fs_alloc_check(true);
    }
    test_sub("subtest %d: value64 str", ++subnum);
    {
        const char *text = "hello c-string";
        value64 v = value64_createstr(text);

        test_validatefree(
            strcmp(v.sval, text) == 0,
            value64_freestr(v),
            "Str copy mismatch: got '%s', expected '%s'", v.sval, text
        );
        test_validatefree(
            strcmp(value64_str(v), text) == 0,
            value64_freestr(v),
            "Str copy mismatch: got '%s', expected '%s'", v.sval, text
        );

        free(v.sval);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_pcopy_move ---------------------------------
static TestStatus
tf_point_init(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ---------- value64_pinit (copy) ---------- */

    /* 1. copy int */
    test_sub("subtest %d: pinit int", ++subnum);
    {
        int ival = 123;
        value64 v = value64_pinit(&ival, VALUE64_INT);
        test_validate(v.ival == 123, "Copy int: got %d, expected 123", v.ival);
    }

    /* 2. copy long */
    test_sub("subtest %d: pinit long", ++subnum);
    {
        long lval = 999999999L;
        value64 v = value64_pinit(&lval, VALUE64_LNG);
        test_validate(v.lval == 999999999L, "Copy long: got %ld, expected 999999999", v.lval);
    }

    /* 3. copy double */
    test_sub("subtest %d: pinit double", ++subnum);
    {
        double dval = 2.7182818;
        value64 v = value64_pinit(&dval, VALUE64_DBL);
        test_validate(fabs(v.dval - 2.7182818) < 0.0000001,
                      "Copy double: got %f, expected 2.7182818", v.dval);
    }

    /* 4. copy pointer */
    test_sub("subtest %d: pinit pointer", ++subnum);
    {
        int x = 5;
        void *ptr = &x;
        value64 v = value64_pinit(&ptr, VALUE64_PTR);
        test_validate(v.pval == ptr,
                      "Copy pointer: got %p, expected %p", v.pval, ptr);
    }

    /* 5. copy C-string */
    test_sub("subtest %d: pinit str", ++subnum);
    {
        const char *text = "copy-me";
        value64 v = value64_pinit(text, VALUE64_STR);
        test_validatefree(
            strcmp(v.sval, text) == 0,
            value64_freestr(v),
            "Copy str: got '%s', expected '%s'", v.sval, text
        );
        test_validatefree(
            v.sval != text,
            value64_freestr(v),
            "Copy str must have different address from original"
        );
        value64_freestr(v);
    }

    /* 6. copy fs */
    test_sub("subtest %d: pinit fs", ++subnum);
    {
        const char *text = "fs-copy";
        fs orig = fscopy(text);
        value64 v = value64_pinit(&orig, VALUE64_FS);

        test_validatefree(
            strcmp(fs_str(v.fsval), text) == 0,
            (fsfree(orig), value64_freefs(v)),
            "Copy fs: got '%s', expected '%s'", fs_str(v.fsval), text
        );
        test_validatefree(
            fs_bodyalloc(v.fsval),
            (fsfree(orig), value64_freefs(v)),
            "Copy fs must have FS_FLAG_BODYALLOC"
        );

        fsfree(orig);
        value64_freefs(v);
        fs_alloc_check(true);
    }

    /* ---------- value64_pmove (move) ---------- */

    /* 7. move int (семантика копирования, т.к. скаляр) */
    test_sub("subtest %d: pmove int", ++subnum);
    {
        int ival = -5;
        value64 v = value64_pmove(&ival, VALUE64_INT);
        test_validate(v.ival == -5, "Move int: got %d, expected -5", v.ival);
    }

    /* 8. move long */
    test_sub("subtest %d: pmove long", ++subnum);
    {
        long lval = -999999999L;
        value64 v = value64_pmove(&lval, VALUE64_LNG);
        test_validate(v.lval == -999999999L, "Move long: got %ld, expected -999999999", v.lval);
    }

    /* 9. move double */
    test_sub("subtest %d: pmove double", ++subnum);
    {
        double dval = -1.4142135;
        value64 v = value64_pmove(&dval, VALUE64_DBL);
        test_validate(fabs(v.dval - (-1.4142135)) < 0.0000001,
                      "Move double: got %f, expected -1.4142135", v.dval);
    }

    /* 10. move pointer  DISABLED
    test_sub("subtest %d: pmove pointer", ++subnum);
    {
        int x = 99;
        void *ptr = &x;
        value64 v = value64_pmove(&ptr, VALUE64_PTR);
        test_validate(v.pval == &x,
                      "Move pointer: got %p, expected %p", v.pval, (void*)&x);
    } */

    /* 11. move C-string (забирает владение) */
    test_sub("subtest %d: pmove str", ++subnum);
    {
        char *text = strdup("move-str");
        value64 v = value64_pmove(text, VALUE64_STR);
        test_validatefree(
            strcmp(v.sval, "move-str") == 0,
            free(v.sval),
            "Move str: got '%s', expected 'move-str'", v.sval
        );
        // text больше не владеет памятью, его нельзя освобождать
        free(v.sval);
    }

    /* 12. move fs (оригинал опустошается) */
    test_sub("subtest %d: pmove fs", ++subnum);
    {
        const char *text = "fs-move";
        fs orig = fscopy(text);
        value64 v = value64_pmove(&orig, VALUE64_FS);

        test_validatefree(
            strcmp(fs_str(v.fsval), text) == 0,
            fs_free(v.fsval),
            "Move fs: got '%s', expected '%s'", fs_str(v.fsval), text
        );
        test_validatefree(
            fs_bodyalloc(v.fsval),
            fs_free(v.fsval),
            "Move fs must have FS_FLAG_BODYALLOC"
        );
        test_validatefree(
            fslen(orig) == 0 && fsstr(orig) == NULL,
            fs_free(v.fsval),
            "After move, original fs must be empty (len=%d, str=%p)", fslen(orig), (void*)fsstr(orig)
        );

        fs_free(v.fsval);
        fsfree(orig);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_clone ---------------------------------

static TestStatus
tf_clone(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ---------- value64_create* (конструкторы) ---------- */

    /* 1. int */
    test_sub("subtest %d: create int", ++subnum);
    {
        value64 v = value64_createint(42);
        test_validate(v.ival == 42, "Create int: got %d, expected 42", v.ival);
    }

    /* 2. long */
    test_sub("subtest %d: create long", ++subnum);
    {
        value64 v = value64_createlong(1234567890L);
        test_validate(v.lval == 1234567890L, "Create long: got %ld, expected 1234567890", v.lval);
    }

    /* 3. double */
    test_sub("subtest %d: create double", ++subnum);
    {
        value64 v = value64_createdbl(2.718281828);
        test_validate(fabs(v.dval - 2.718281828) < 0.000000001,
                      "Create double: got %f, expected 2.718281828", v.dval);
    }

    /* 4. pointer */
    test_sub("subtest %d: create pointer", ++subnum);
    {
        int x = 77;
        value64 v = value64_createptr(&x);
        test_validate(v.pval == &x,
                      "Create pointer: got %p, expected %p", v.pval, (void*)&x);
    }

    /* 5. C-string (копирование) */
    test_sub("subtest %d: create str", ++subnum);
    {
        const char *text = "hello value64";
        value64 v = value64_createstr(text);

        test_validatefree(
            strcmp(v.sval, text) == 0,
            value64_free(v, VALUE64_STR),
            "Create str: got '%s', expected '%s'", v.sval, text
        );
        test_validatefree(
            v.sval != text,
            value64_free(v, VALUE64_STR),
            "Create str must have its own memory"
        );
        value64_free(v, VALUE64_STR);
    }

    /* 6. fs (копирование) */
    test_sub("subtest %d: create fs", ++subnum);
    {
        const char *text = "hello fs";
        fs orig = fscopy(text);
        value64 v = value64_createfs(&orig);

        test_validatefree(
            strcmp(fs_str(v.fsval), text) == 0,
            (fsfree(orig), value64_free(v, VALUE64_FS)),
            "Create fs: got '%s', expected '%s'", fs_str(v.fsval), text
        );
        test_validatefree(
            fs_bodyalloc(v.fsval),
            (fsfree(orig), value64_free(v, VALUE64_FS)),
            "Create fs must have FS_FLAG_BODYALLOC"
        );

        fsfree(orig);
        value64_free(v, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* ---------- value64_clone ---------- */

    /* 7. clone int */
    test_sub("subtest %d: clone int", ++subnum);
    {
        value64 orig = value64_createint(100);
        value64 copy = value64_clone(orig, VALUE64_INT);
        test_validate(copy.ival == 100, "Clone int: got %d, expected 100", copy.ival);
    }

    /* 8. clone long */
    test_sub("subtest %d: clone long", ++subnum);
    {
        value64 orig = value64_createlong(999999999L);
        value64 copy = value64_clone(orig, VALUE64_LNG);
        test_validate(copy.lval == 999999999L, "Clone long: got %ld, expected 999999999", copy.lval);
    }

    /* 9. clone double */
    test_sub("subtest %d: clone double", ++subnum);
    {
        value64 orig = value64_createdbl(1.6180339);
        value64 copy = value64_clone(orig, VALUE64_DBL);
        test_validate(fabs(copy.dval - 1.6180339) < 0.0000001,
                      "Clone double: got %f, expected 1.6180339", copy.dval);
    }

    /* 10. clone pointer */
    test_sub("subtest %d: clone pointer", ++subnum);
    {
        int x = 123;
        value64 orig = value64_createptr(&x);
        value64 copy = value64_clone(orig, VALUE64_PTR);
        test_validate(copy.pval == &x,
                      "Clone pointer: got %p, expected %p", copy.pval, (void*)&x);
    }

    /* 11. clone C-string */
    test_sub("subtest %d: clone str", ++subnum);
    {
        const char *text = "clone-string";
        value64 orig = value64_createstr(text);
        value64 copy = value64_clone(orig, VALUE64_STR);

        test_validatefree(
            strcmp(copy.sval, text) == 0,
            (value64_free(orig, VALUE64_STR), value64_free(copy, VALUE64_STR)),
            "Clone str: got '%s', expected '%s'", copy.sval, text
        );
        test_validatefree(
            copy.sval != orig.sval,
            (value64_free(orig, VALUE64_STR), value64_free(copy, VALUE64_STR)),
            "Clone str must have different address"
        );

        value64_free(orig, VALUE64_STR);
        value64_free(copy, VALUE64_STR);
    }

    /* 12. clone fs */
    test_sub("subtest %d: clone fs", ++subnum);
    {
        const char *text = "clone-fs";
        fs orig_fs = fscopy(text);
        value64 orig = value64_createfs(&orig_fs);
        value64 copy = value64_clone(orig, VALUE64_FS);

        test_validatefree(
            strcmp(fs_str(copy.fsval), text) == 0,
            (fsfree(orig_fs), value64_free(orig, VALUE64_FS), value64_free(copy, VALUE64_FS)),
            "Clone fs: got '%s', expected '%s'", fs_str(copy.fsval), text
        );
        test_validatefree(
            fs_bodyalloc(copy.fsval),
            (fsfree(orig_fs), value64_free(orig, VALUE64_FS), value64_free(copy, VALUE64_FS)),
            "Clone fs must have FS_FLAG_BODYALLOC"
        );
        test_validatefree(
            copy.fsval != orig.fsval,
            (fsfree(orig_fs), value64_free(orig, VALUE64_FS), value64_free(copy, VALUE64_FS)),
            "Clone fs must have different pointer"
        );

        fsfree(orig_fs);
        value64_free(orig, VALUE64_FS);
        value64_free(copy, VALUE64_FS);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_move ---------------------------------

static TestStatus
tf_move(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. move int */
    test_sub("subtest %d: move int", ++subnum);
    {
        value64 src = value64_createint(42);
        value64 dst = VALUE64_ZERO;
        value64 *ret = value64_move_int(&dst, &src);

        test_validate(ret == &dst, "move_int must return &dst");
        test_validate(value64_int(dst) == 42, "dst must be 42, got %d", value64_int(dst));
        test_validate(value64_int(src) == 0, "src must be 0 after move, got %d", value64_int(src));
    }

    /* 2. move long */
    test_sub("subtest %d: move long", ++subnum);
    {
        value64 src = value64_createlong(123456789L);
        value64 dst = VALUE64_ZERO;
        value64_move_long(&dst, &src);

        test_validate(value64_long(dst) == 123456789L, "dst mismatch, got %ld", value64_long(dst));
        test_validate(value64_long(src) == 0L, "src must be 0 after move, got %ld", value64_long(src));
    }

    /* 3. move double */
    test_sub("subtest %d: move double", ++subnum);
    {
        value64 src = value64_createdbl(3.1415);
        value64 dst = VALUE64_ZERO;
        value64_move_dbl(&dst, &src);

        test_validate(fabs(value64_dbl(dst) - 3.1415) < 0.0001, "dst mismatch, got %f", value64_dbl(dst));
        test_validate(fabs(value64_dbl(src) - 0.0) < 1e-12, "src must be 0.0 after move, got %f", value64_dbl(src));
    }

    /* 4. move pointer */
    test_sub("subtest %d: move pointer", ++subnum);
    {
        int x = 7;
        value64 src = value64_createptr(&x);
        value64 dst = VALUE64_ZERO;
        value64_move_ptr(&dst, &src);

        test_validate(value64_ptr(dst) == &x, "dst must point to x, got %p", value64_ptr(dst));
        test_validate(value64_ptr(src) == NULL, "src must be NULL after move, got %p", value64_ptr(src));
    }

    /* 5. move str */
    test_sub("subtest %d: move str", ++subnum);
    {
        const char *text = "movable string";
        value64 src = value64_createstr(text);
        value64 dst = VALUE64_ZERO;
        value64_move_str(&dst, &src);

        test_validatefree(
            value64_str(dst) != NULL && strcmp(value64_str(dst), text) == 0,
            value64_free(dst, VALUE64_STR),
            "dst must be '%s', got '%s'", text, value64_str(dst)
        );
        test_validatefree(
            value64_str(src) == NULL,
            value64_free(dst, VALUE64_STR),
            "src must be NULL after move, got %p", value64_str(src)
        );
        value64_free(dst, VALUE64_STR);
    }

    /* 6. move fs */
    test_sub("subtest %d: move fs", ++subnum);
    {
        const char *text = "fs-move-target";
        fs orig = fscopy(text);
        value64 src = value64_createfs(&orig);
        fsfree(orig);

        fs *src_fs_before = value64_fs(src);   // запоминаем указатель до move

        value64 dst = VALUE64_ZERO;
        value64_move_fs(&dst, &src);

        fs *dst_fs = value64_fs(dst);

        test_validatefree(
            dst_fs != NULL && strcmp(fs_str(dst_fs), text) == 0,
            value64_free(dst, VALUE64_FS),
            "dst must be '%s', got '%s'", text, fs_str(dst_fs)
        );
        test_validatefree(
            fs_bodyalloc(dst_fs),
            value64_free(dst, VALUE64_FS),
            "dst fs must have FS_FLAG_BODYALLOC"
        );
       // Убеждаемся, что dst получил новую память (не ту, что была у src)
        test_validatefree(
            dst_fs != src_fs_before,
            value64_free(dst, VALUE64_FS),
            "dst fs pointer must differ from original src pointer"
        );
        value64_free(dst, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* 7. multiple fs moves (leak check) */
    test_sub("subtest %d: multiple fs moves (leak check)", ++subnum);
    {
        const char *words[] = {"first", "second", "third"};
        value64 dst[3] = { VALUE64_ZERO, VALUE64_ZERO, VALUE64_ZERO };

        for (int i = 0; i < COUNT(words); i++) {
            fs orig = fscopy(words[i]);
            value64 src = value64_createfs(&orig);
            fsfree(orig);
            value64_move_fs(&dst[i], &src);
        }

        for (int i = 0; i < COUNT(words); i++) {
            fs *dst_fs = value64_fs(dst[i]);
            test_validatefree(
                strcmp(fs_str(dst_fs), words[i]) == 0,
                (value64_free(dst[0], VALUE64_FS), value64_free(dst[1], VALUE64_FS), value64_free(dst[2], VALUE64_FS)),
                "dst[%d] must be '%s', got '%s'", i, words[i], fs_str(dst_fs)
            );
        }
        for (int i = 0; i < COUNT(words); i++) {
            value64_free(dst[i], VALUE64_FS);
        }
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_lhash ---------------------------------

static TestStatus
tf_lhash(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. int */
    test_sub("subtest %d: hash int", ++subnum);
    {
        value64 v1 = value64_createint(42);
        value64 v2 = value64_createint(42);
        value64 v3 = value64_createint(100);

        unsigned long h1 = value64_lhash(v1, VALUE64_INT);
        unsigned long h2 = value64_lhash(v2, VALUE64_INT);
        unsigned long h3 = value64_lhash(v3, VALUE64_INT);

        test_validate(h1 == h2, "Same ints must have same hash: %lu vs %lu", h1, h2);
        test_validate(h1 != h3, "Different ints should differ: %lu vs %lu", h1, h3);
    }

    /* 2. long */
    test_sub("subtest %d: hash long", ++subnum);
    {
        value64 v1 = value64_createlong(999999999L);
        value64 v2 = value64_createlong(999999999L);
        value64 v3 = value64_createlong(0L);

        unsigned long h1 = value64_lhash(v1, VALUE64_LNG);
        unsigned long h2 = value64_lhash(v2, VALUE64_LNG);
        unsigned long h3 = value64_lhash(v3, VALUE64_LNG);

        test_validate(h1 == h2, "Same longs must have same hash");
        test_validate(h1 != h3, "Different longs should differ");
    }

    /* 3. double */
    test_sub("subtest %d: hash double", ++subnum);
    {
        value64 v1 = value64_createdbl(3.1415);
        value64 v2 = value64_createdbl(3.1415);
        value64 v3 = value64_createdbl(2.718);

        unsigned long h1 = value64_lhash(v1, VALUE64_DBL);
        unsigned long h2 = value64_lhash(v2, VALUE64_DBL);
        unsigned long h3 = value64_lhash(v3, VALUE64_DBL);

        test_validate(h1 == h2, "Same doubles must have same hash");
        test_validate(h1 != h3, "Different doubles should differ");
    }

    /* 4. pointer */
    test_sub("subtest %d: hash pointer", ++subnum);
    {
        int x = 1, y = 2;
        value64 v1 = value64_createptr(&x);
        value64 v2 = value64_createptr(&x);
        value64 v3 = value64_createptr(&y);

        unsigned long h1 = value64_lhash(v1, VALUE64_PTR);
        unsigned long h2 = value64_lhash(v2, VALUE64_PTR);
        unsigned long h3 = value64_lhash(v3, VALUE64_PTR);

        test_validate(h1 == h2, "Same pointers must have same hash");
        test_validate(h1 != h3, "Different pointers should differ");
    }

    /* 5. C-string */
    test_sub("subtest %d: hash str", ++subnum);
    {
        const char *text = "hash-me";
        value64 v1 = value64_createstr(text);
        value64 v2 = value64_createstr(text);
        value64 v3 = value64_createstr("other");

        unsigned long h1 = value64_lhash(v1, VALUE64_STR);
        unsigned long h2 = value64_lhash(v2, VALUE64_STR);
        unsigned long h3 = value64_lhash(v3, VALUE64_STR);

        test_validate(h1 == h2, "Same strings must have same hash");
        test_validate(h1 != h3, "Different strings should differ");

        value64_free(v1, VALUE64_STR);
        value64_free(v2, VALUE64_STR);
        value64_free(v3, VALUE64_STR);
    }

    /* 6. fs (с ненулевым содержимым) */
    test_sub("subtest %d: hash fs", ++subnum);
    {
        const char *text = "fs-hash";
        fs orig = fscopy(text);
        value64 v1 = value64_createfs(&orig);
        value64 v2 = value64_createfs(&orig);   // ещё одна копия
        fsfree(orig);

        value64 v3 = value64_createfs(&(fs){ .v = "different", .len = 9, .sz = 0, .flags = FS_FLAG_STATIC });

        unsigned long h1 = value64_lhash(v1, VALUE64_FS);
        unsigned long h2 = value64_lhash(v2, VALUE64_FS);
        unsigned long h3 = value64_lhash(v3, VALUE64_FS);

        test_validate(h1 == h2, "Same fs must have same hash");
        test_validate(h1 != h3, "Different fs should differ");

        value64_free(v1, VALUE64_FS);
        value64_free(v2, VALUE64_FS);
        value64_free(v3, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* 7. fs с NULL указателем (пустая строка) */
    test_sub("subtest %d: hash fs with NULL body", ++subnum);
    {
        fs empty = FSLITERAL("");                    // v == NULL
        value64 v = value64_createfs(&empty);

        unsigned long h = value64_lhash(v, VALUE64_FS);
        // Главное, чтобы не упало – hash_djb2 должен обработать NULL
        test_validate(h == 5381, "Hash of empty fs (v=NULL) must be 5381, got %lu", h);

        value64_free(v, VALUE64_FS);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_compare ---------------------------------

static TestStatus
tf_compare(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. compare int */
    test_sub("subtest %d: compare int", ++subnum);
    {
        value64 v1 = value64_createint(42);
        value64 v2 = value64_createint(42);
        value64 v3 = value64_createint(100);

        test_validate(value64_compare(v1, v2, VALUE64_INT) == 0, "Equal ints must return 0");
        test_validate(value64_compare(v1, v3, VALUE64_INT) != 0, "Different ints must not return 0");
    }

    /* 2. compare long */
    test_sub("subtest %d: compare long", ++subnum);
    {
        value64 v1 = value64_createlong(999999999L);
        value64 v2 = value64_createlong(999999999L);
        value64 v3 = value64_createlong(0L);

        test_validate(value64_compare(v1, v2, VALUE64_LNG) == 0, "Equal longs must return 0");
        test_validate(value64_compare(v1, v3, VALUE64_LNG) != 0, "Different longs must not return 0");
    }

    /* 3. compare double */
    test_sub("subtest %d: compare double", ++subnum);
    {
        value64 v1 = value64_createdbl(3.1415);
        value64 v2 = value64_createdbl(3.1415);
        value64 v3 = value64_createdbl(2.718);

        test_validate(value64_compare(v1, v2, VALUE64_DBL) == 0, "Equal doubles must return 0");
        test_validate(value64_compare(v1, v3, VALUE64_DBL) != 0, "Different doubles must not return 0");
    }

    /* 4. compare pointer */
    test_sub("subtest %d: compare pointer", ++subnum);
    {
        int x = 1, y = 2;
        value64 v1 = value64_createptr(&x);
        value64 v2 = value64_createptr(&x);
        value64 v3 = value64_createptr(&y);

        test_validate(value64_compare(v1, v2, VALUE64_PTR) == 0, "Same pointers must return 0");
        test_validate(value64_compare(v1, v3, VALUE64_PTR) != 0, "Different pointers must not return 0");
    }

    /* 5. compare C-string (returns bool) */
    test_sub("subtest %d: compare str", ++subnum);
    {
        const char *text = "compare-me";
        value64 v1 = value64_createstr(text);
        value64 v2 = value64_createstr(text);
        value64 v3 = value64_createstr("other");

        // value64_compare для строк возвращает true/false
        test_validate(value64_compare(v1, v2, VALUE64_STR) == 0, "Equal strings must return true");
        test_validate(value64_compare(v1, v3, VALUE64_STR) != 0, "Different strings must return false");

        value64_free(v1, VALUE64_STR);
        value64_free(v2, VALUE64_STR);
        value64_free(v3, VALUE64_STR);
    }

    /* 6. compare fs */
    test_sub("subtest %d: compare fs", ++subnum);
    {
        const char *text = "fs-cmp";
        fs orig = fscopy(text);
        value64 v1 = value64_createfs(&orig);
        value64 v2 = value64_createfs(&orig);
        fsfree(orig);

        value64 v3 = value64_createfs( &FSLITERAL("different") );

        test_validate(value64_compare(v1, v2, VALUE64_FS) == 0, "Equal fs must return 0");
        test_validate(value64_compare(v1, v3, VALUE64_FS) != 0, "Different fs must not return 0");

        value64_free(v1, VALUE64_FS);
        value64_free(v2, VALUE64_FS);
        value64_free(v3, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* 7. compare fs with empty (0-len string "") */
    test_sub("subtest %d: compare fs empty", ++subnum);
    {
        fs empty1 = FSLITERAL("");
        fs empty2 = FSLITERAL("");

        value64 v1 = value64_createfs(&empty1);
        value64 v2 = value64_createfs(&empty2);

        test_validate(value64_compare(v1, v2, VALUE64_FS) == 0, "Empty fs must be equal");

        value64_free(v1, VALUE64_FS);
        value64_free(v2, VALUE64_FS);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_convert ---------------------------------

static TestStatus
tf_convert(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== 1. INT → LONG ========== */
    test_sub("subtest %d: INT -> LONG", ++subnum);
    {
        value64 src = value64_createint(42);
        value64 dst = value64_convert(src, VALUE64_INT, VALUE64_LNG);
        test_validate(value64_long(dst) == 42L,
            "INT->LONG: expected 42, got %ld", value64_long(dst));
    }

    /* ========== 2. INT → DBL ========== */
    test_sub("subtest %d: INT -> DBL", ++subnum);
    {
        value64 src = value64_createint(42);
        value64 dst = value64_convert(src, VALUE64_INT, VALUE64_DBL);
        test_validate(fabs(value64_dbl(dst) - 42.0) < 0.0001,
            "INT->DBL: expected 42.0, got %f", value64_dbl(dst));
    }

    /* ========== 3. INT → FS ========== */
    test_sub("subtest %d: INT -> FS", ++subnum);
    {
        value64 src = value64_createint(42);
        value64 dst = value64_convert(src, VALUE64_INT, VALUE64_FS);
        fs *dst_fs = value64_fs(dst);
        test_validatefree(
            strcmp(fs_str(dst_fs), "42") == 0,
            value64_free(dst, VALUE64_FS),
            "INT->FS: expected '42', got '%s'", fs_str(dst_fs)
        );
        value64_free(dst, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* ========== 4. INT → STR ========== */
    test_sub("subtest %d: INT -> STR", ++subnum);
    {
        value64 src = value64_createint(42);
        value64 dst = value64_convert(src, VALUE64_INT, VALUE64_STR);
        test_validatefree(
            strcmp(value64_str(dst), "42") == 0,
            value64_free(dst, VALUE64_STR),
            "INT->STR: expected '42', got '%s'", value64_str(dst)
        );
        value64_free(dst, VALUE64_STR);
    }

    /* ========== 5. LONG → INT (в пределах) ========== */
    test_sub("subtest %d: LONG -> INT (in range)", ++subnum);
    {
        value64 src = value64_createlong(123456L);
        value64 dst = value64_convert(src, VALUE64_LNG, VALUE64_INT);
        test_validate(value64_int(dst) == 123456,
            "LONG->INT: expected 123456, got %d", value64_int(dst));
    }

    /* ========== 6. LONG → INT (переполнение) ========== */
    test_sub("subtest %d: LONG -> INT (overflow)", ++subnum);
    {
        value64 src = value64_createlong(2147483648L);  // > INT_MAX
        if (!try()) {
            value64 dst = value64_convert(src, VALUE64_LNG, VALUE64_INT);
            test_validate(false, "LONG->INT overflow must raise error, but returned %d", value64_int(dst));
        } else {
            test_validate(true, "LONG->INT overflow correctly raised error");
        }
    }

    /* ========== 7. LONG → DBL ========== */
    test_sub("subtest %d: LONG -> DBL", ++subnum);
    {
        value64 src = value64_createlong(999999999L);
        value64 dst = value64_convert(src, VALUE64_LNG, VALUE64_DBL);
        test_validate(fabs(value64_dbl(dst) - 999999999.0) < 0.0001,
            "LONG->DBL: expected 999999999.0, got %f", value64_dbl(dst));
    }

    /* ========== 8. LONG → FS ========== */
    test_sub("subtest %d: LONG -> FS", ++subnum);
    {
        value64 src = value64_createlong(-123456789L);
        value64 dst = value64_convert(src, VALUE64_LNG, VALUE64_FS);
        fs *dst_fs = value64_fs(dst);
        test_validatefree(
            strcmp(fs_str(dst_fs), "-123456789") == 0,
            value64_free(dst, VALUE64_FS),
            "LONG->FS: expected '-123456789', got '%s'", fs_str(dst_fs)
        );
        value64_free(dst, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* ========== 9. LONG → STR ========== */
    test_sub("subtest %d: LONG -> STR", ++subnum);
    {
        value64 src = value64_createlong(0L);
        value64 dst = value64_convert(src, VALUE64_LNG, VALUE64_STR);
        test_validatefree(
            strcmp(value64_str(dst), "0") == 0,
            value64_free(dst, VALUE64_STR),
            "LONG->STR: expected '0', got '%s'", value64_str(dst)
        );
        value64_free(dst, VALUE64_STR);
    }

    /* ========== 10. DBL → INT (в пределах) ========== */
    test_sub("subtest %d: DBL -> INT (in range)", ++subnum);
    {
        value64 src = value64_createdbl(3.14);
        value64 dst = value64_convert(src, VALUE64_DBL, VALUE64_INT);
        test_validate(value64_int(dst) == 3,
            "DBL->INT: expected 3, got %d", value64_int(dst));
    }

    /* ========== 11. DBL → INT (переполнение) ========== */
    test_sub("subtest %d: DBL -> INT (overflow)", ++subnum);
    {
        value64 src = value64_createdbl(1.0e30);
        if (!try()) {
            value64 dst = value64_convert(src, VALUE64_DBL, VALUE64_INT);
            test_validate(false, "DBL->INT overflow must raise error, but returned %d", value64_int(dst));
        } else {
            test_validate(true, "DBL->INT overflow correctly raised error");
        }
    }

    /* ========== 12. DBL → LONG (в пределах) ========== */
    test_sub("subtest %d: DBL -> LONG (in range)", ++subnum);
    {
        value64 src = value64_createdbl(2.71828);
        value64 dst = value64_convert(src, VALUE64_DBL, VALUE64_LNG);
        test_validate(value64_long(dst) == 2L,
            "DBL->LONG: expected 2, got %ld", value64_long(dst));
    }

    /* ========== 13. DBL → LONG (переполнение) ========== */
    test_sub("subtest %d: DBL -> LONG (overflow)", ++subnum);
    {
        value64 src = value64_createdbl(1.0e30);
        if (!try()) {
            value64 dst = value64_convert(src, VALUE64_DBL, VALUE64_LNG);
            test_validate(false, "DBL->LONG overflow must raise error, but returned %ld", value64_long(dst));
        } else {
            test_validate(true, "DBL->LONG overflow correctly raised error");
        }
    }

    /* ========== 14. DBL → FS ========== */
    test_sub("subtest %d: DBL -> FS", ++subnum);
    {
        value64 src = value64_createdbl(3.14159265);
        value64 dst = value64_convert(src, VALUE64_DBL, VALUE64_FS);
        fs *dst_fs = value64_fs(dst);
        // %g убирает лишние нули, проверяем, что строка начинается с "3.14159"
        test_validatefree(
            strncmp(fs_str(dst_fs), "3.14159", 7) == 0,
            value64_free(dst, VALUE64_FS),
            "DBL->FS: expected prefix '3.14159', got '%s'", fs_str(dst_fs)
        );
        value64_free(dst, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* ========== 15. DBL → STR ========== */
    test_sub("subtest %d: DBL -> STR", ++subnum);
    {
        value64 src = value64_createdbl(2.5);
        value64 dst = value64_convert(src, VALUE64_DBL, VALUE64_STR);
        // snprintf с "%lf" даст "2.500000"
        test_validatefree(
            strcmp(value64_str(dst), "2.500000") == 0,
            value64_free(dst, VALUE64_STR),
            "DBL->STR: expected '2.500000', got '%s'", value64_str(dst)
        );
        value64_free(dst, VALUE64_STR);
    }

    /* ========== 16. FS → INT (корректная строка) ========== */
    test_sub("subtest %d: FS -> INT (valid)", ++subnum);
    {
        fs tmp = fscopy("123");
        value64 src = value64_createfs(&tmp);
        fsfree(tmp);
        value64 dst = value64_convert(src, VALUE64_FS, VALUE64_INT);
        test_validatefree(
            value64_int(dst) == 123,
            value64_free(src, VALUE64_FS),
            "FS->INT: expected 123, got %d", value64_int(dst)
        );
        value64_free(src, VALUE64_FS);
    }

    /* ========== 17. FS → INT (некорректная строка) ========== */
    test_sub("subtest %d: FS -> INT (invalid)", ++subnum);
    {
        fs tmp = fscopy("abc");
        value64 src = value64_createfs(&tmp);
        fsfree(tmp);
        if (!try()) {
            value64 dst = value64_convert(src, VALUE64_FS, VALUE64_INT);
            test_validatefree(false, value64_free(src, VALUE64_FS),
                "FS->INT invalid must raise error, but returned %d", value64_int(dst));
        } else {
            test_validatefree(true, value64_free(src, VALUE64_FS),
                "FS->INT invalid correctly raised error");
        }
        value64_free(src, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* ========== 18. FS → LONG ========== */
    test_sub("subtest %d: FS -> LONG", ++subnum);
    {
        fs tmp = fscopy("-999999999");
        value64 src = value64_createfs(&tmp);
        fsfree(tmp);
        value64 dst = value64_convert(src, VALUE64_FS, VALUE64_LNG);
        test_validatefree(
            value64_long(dst) == -999999999L,
            value64_free(src, VALUE64_FS),
            "FS->LONG: expected -999999999, got %ld", value64_long(dst)
        );
        value64_free(src, VALUE64_FS);
    }

    /* ========== 19. FS → DBL ========== */
    test_sub("subtest %d: FS -> DBL", ++subnum);
    {
        fs tmp = fscopy("3.1415");
        value64 src = value64_createfs(&tmp);
        fsfree(tmp);
        value64 dst = value64_convert(src, VALUE64_FS, VALUE64_DBL);
        test_validatefree(
            fabs(value64_dbl(dst) - 3.1415) < 0.0001,
            value64_free(src, VALUE64_FS),
            "FS->DBL: expected ~3.1415, got %f", value64_dbl(dst)
        );
        value64_free(src, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* ========== 20. FS → FS (копирование) ========== */
    test_sub("subtest %d: FS -> FS (copy)", ++subnum);
    {
        fs tmp = fscopy("clone-me");
        value64 src = value64_createfs(&tmp);
        fsfree(tmp);
        value64 dst = value64_convert(src, VALUE64_FS, VALUE64_FS);
        fs *src_fs = value64_fs(src);
        fs *dst_fs = value64_fs(dst);
        test_validatefree(
            strcmp(fs_str(src_fs), fs_str(dst_fs)) == 0 && src_fs != dst_fs,
            (value64_free(src, VALUE64_FS), value64_free(dst, VALUE64_FS)),
            "FS->FS: copy must have same content but different pointer"
        );
        value64_free(src, VALUE64_FS);
        value64_free(dst, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* ========== 21. FS → STR ========== */
    test_sub("subtest %d: FS -> STR", ++subnum);
    {
        fs tmp = fscopy("to-string");
        value64 src = value64_createfs(&tmp);
        fsfree(tmp);
        value64 dst = value64_convert(src, VALUE64_FS, VALUE64_STR);
        test_validatefree(
            strcmp(value64_str(dst), "to-string") == 0,
            (value64_free(src, VALUE64_FS), value64_free(dst, VALUE64_STR)),
            "FS->STR: expected 'to-string', got '%s'", value64_str(dst)
        );
        value64_free(src, VALUE64_FS);
        value64_free(dst, VALUE64_STR);
    }

    /* ========== 22. STR → INT (корректная) ========== */
    test_sub("subtest %d: STR -> INT (valid)", ++subnum);
    {
        value64 src = value64_createstr("42");
        value64 dst = value64_convert(src, VALUE64_STR, VALUE64_INT);
        test_validatefree(
            value64_int(dst) == 42,
            value64_free(src, VALUE64_STR),
            "STR->INT: expected 42, got %d", value64_int(dst)
        );
        value64_free(src, VALUE64_STR);
    }

    /* ========== 23. STR → INT (некорректная) ========== */
    test_sub("subtest %d: STR -> INT (invalid)", ++subnum);
    {
        value64 src = value64_createstr("not-a-number");
        if (!try()) {
            value64 dst = value64_convert(src, VALUE64_STR, VALUE64_INT);
            test_validatefree(false, value64_free(src, VALUE64_STR),
                "STR->INT invalid must raise error, but returned %d", value64_int(dst));
        } else {
            test_validatefree(true, value64_free(src, VALUE64_STR),
                "STR->INT invalid correctly raised error");
        }
        value64_free(src, VALUE64_STR);
    }

    /* ========== 24. STR → LONG (корректная) ========== */
    test_sub("subtest %d: STR -> LONG (valid)", ++subnum);
    {
        value64 src = value64_createstr("-123456789");
        value64 dst = value64_convert(src, VALUE64_STR, VALUE64_LNG);
        test_validatefree(
            value64_long(dst) == -123456789L,
            value64_free(src, VALUE64_STR),
            "STR->LONG: expected -123456789, got %ld", value64_long(dst)
        );
        value64_free(src, VALUE64_STR);
    }

    /* ========== 25. STR → DBL (корректная) ========== */
    test_sub("subtest %d: STR -> DBL (valid)", ++subnum);
    {
        value64 src = value64_createstr("2.71828");
        value64 dst = value64_convert(src, VALUE64_STR, VALUE64_DBL);
        test_validatefree(
            fabs(value64_dbl(dst) - 2.71828) < 0.00001,
            value64_free(src, VALUE64_STR),
            "STR->DBL: expected ~2.71828, got %f", value64_dbl(dst)
        );
        value64_free(src, VALUE64_STR);
    }

    /* ========== 26. STR → STR (копирование) ========== */
    test_sub("subtest %d: STR -> STR (copy)", ++subnum);
    {
        value64 src = value64_createstr("copy-string");
        value64 dst = value64_convert(src, VALUE64_STR, VALUE64_STR);
        test_validatefree(
            strcmp(value64_str(src), value64_str(dst)) == 0 && value64_str(src) != value64_str(dst),
            (value64_free(src, VALUE64_STR), value64_free(dst, VALUE64_STR)),
            "STR->STR: copy must have same content but different pointer"
        );
        value64_free(src, VALUE64_STR);
        value64_free(dst, VALUE64_STR);
    }

    /* ========== 27. STR → FS ========== */
    test_sub("subtest %d: STR -> FS", ++subnum);
    {
        value64 src = value64_createstr("hello-fs");
        value64 dst = value64_convert(src, VALUE64_STR, VALUE64_FS);
        fs *dst_fs = value64_fs(dst);
        test_validatefree(
            strcmp(fs_str(dst_fs), "hello-fs") == 0,
            (value64_free(src, VALUE64_STR), value64_free(dst, VALUE64_FS)),
            "STR->FS: expected 'hello-fs', got '%s'", fs_str(dst_fs)
        );
        value64_free(src, VALUE64_STR);
        value64_free(dst, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* ========== 28. Граничные значения: INT_MAX → LONG → DBL → STR → FS и обратно ========== */
    test_sub("subtest %d: round-trip INT_MAX", ++subnum);
    {
        value64 src = value64_createint(INT_MAX);
        value64 tmp = value64_convert(src, VALUE64_INT, VALUE64_LNG);
        test_validate(value64_long(tmp) == (long)INT_MAX, "INT_MAX -> LONG mismatch");
        tmp = value64_convert(tmp, VALUE64_LNG, VALUE64_DBL);
        test_validate(fabs(value64_dbl(tmp) - (double)INT_MAX) < 10.0, "INT_MAX -> DBL mismatch");
        // дальше можно в STR и обратно, но не будем усложнять
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_convert_move -----------------------------

static TestStatus
tf_convert_move(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. STR -> STR (move) */
    test_sub("subtest %d: STR -> STR (move)", ++subnum);
    {
        value64 src = value64_createstr("hello");
        value64 dst = value64_convert_move(&src, VALUE64_STR, VALUE64_STR);
        test_validatefree(
            strcmp(value64_str(dst), "hello") == 0 && value64_str(src) == NULL,
            value64_free(dst, VALUE64_STR),
            "STR->STR: dst='%s', src must be NULL (got %p)",
            value64_str(dst), (void*)value64_str(src)
        );
        // Безопасно освобождаем пустой src
        value64_free(src, VALUE64_STR);
        value64_free(dst, VALUE64_STR);
    }

    /* 2. STR -> FS (move) */
    test_sub("subtest %d: STR -> FS (move)", ++subnum);
    {
        value64 src = value64_createstr("world");
        value64 dst = value64_convert_move(&src, VALUE64_STR, VALUE64_FS);
        fs *dst_fs = value64_fs(dst);
        test_validatefree(
            strcmp(fs_str(dst_fs), "world") == 0 && fs_bodyalloc(dst_fs),
            value64_free(dst, VALUE64_FS),
            "STR->FS: dst='%s', src.sval must be NULL (got %p)",
            fs_str(dst_fs), (void*)value64_str(src)
        );
        test_validate(value64_str(src) == NULL, "After move, src.sval must be NULL");
        value64_free(dst, VALUE64_FS);
        value64_free(src, VALUE64_STR);   // src уже пуст, безопасно
        fs_alloc_check(true);
    }

    /* 3. FS -> STR (move) */
    test_sub("subtest %d: FS -> STR (move)", ++subnum);
    {
        fs tmp = fscopy("fs-string");
        value64 src = value64_createfs(&tmp);
        //fstechprint(*value64_fs(src) );

        fsfree(tmp);
        value64 dst = value64_convert_move(&src, VALUE64_FS, VALUE64_STR);

        test_validatefree(
            strcmp(value64_str(dst), "fs-string") == 0,
            value64_free(dst, VALUE64_STR),
            "FS->STR: dst='%s', src.fsval must be NULL or emptied",
            value64_str(dst)
        );
        test_validate(
            value64_fs(src) == NULL || fs_len(value64_fs(src)) == 0,
            "src.fsval must be empty after move"
        );
        value64_free(dst, VALUE64_STR);
        value64_free(src, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* 4. FS -> FS (move) */
    test_sub("subtest %d: FS -> FS (move)", ++subnum);
    {
        fs tmp = fscopy("move-fs");
        value64 src = value64_createfs(&tmp);
        fsfree(tmp);
        value64 dst = value64_convert_move(&src, VALUE64_FS, VALUE64_FS);
        fs *dst_fs = value64_fs(dst);
        test_validatefree(
            strcmp(fs_str(dst_fs), "move-fs") == 0 && fs_bodyalloc(dst_fs) &&
            value64_fs(src) == NULL,
            value64_free(dst, VALUE64_FS),
            "FS->FS: dst='%s', src.fsval must be NULL (got %p)",
            fs_str(dst_fs), (void*)value64_fs(src)
        );
        value64_free(dst, VALUE64_FS);
        value64_free(src, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* 5. INT -> STR (неподдерживаемая комбинация, должна упасть) */
    test_sub("subtest %d: INT -> STR (must raise error)", ++subnum);
    {
        value64 src = value64_createint(42);
        if (!try()) {
            value64 dst = value64_convert_move(&src, VALUE64_INT, VALUE64_STR);
            test_validate(
                false,
                "INT->STR must raise error, but returned %p",
                (void*)value64_str(dst)
            );
        } else {
            test_validate(
                true,
                "INT->STR correctly raised error"
            );
        }
        // src не изменился, освобождать не нужно
    }

    /* 6. STR -> INT (неподдерживаемая комбинация) */
    test_sub("subtest %d: STR -> INT (must raise error)", ++subnum);
    {
        value64 src = value64_createstr("123");
        if (!try()) {
            value64 dst = value64_convert_move(&src, VALUE64_STR, VALUE64_INT);
            test_validatefree(
                false,
                value64_free(src, VALUE64_STR),
                "STR->INT must raise error, but returned %d", value64_int(dst)
            );
        } else {
            test_validatefree(
                true,
                value64_free(src, VALUE64_STR),
                "STR->INT correctly raised error"
            );
        }
        value64_free(src, VALUE64_STR);
    }

    /* 7. NULL source (должен упасть) */
    test_sub("subtest %d: NULL source", ++subnum);
    {
        if (!try()) {
            value64_convert_move(NULL, VALUE64_STR, VALUE64_FS);
            test_validate(
                false,
                "convert_move(NULL) must raise error"
            );
        } else {
            test_validate(
                true,
                "convert_move(NULL) correctly raised error"
            );
        }
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_is_convertable -----------------------------
static TestStatus
tf_is_convertable(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. INT -> LONG: допустимо */
    test_sub("subtest %d: INT->LNG (allowed)", ++subnum);
    {
        value64 v = value64_createint(42);
        test_validate(
            value64_is_convertable(v, VALUE64_INT, VALUE64_LNG),
            "INT->LNG must be convertable"
        );
    }

    /* 2. INT -> DBL: допустимо */
    test_sub("subtest %d: INT->DBL (allowed)", ++subnum);
    {
        value64 v = value64_createint(42);
        test_validate(
            value64_is_convertable(v, VALUE64_INT, VALUE64_DBL),
            "INT->DBL must be convertable"
        );
    }

    /* 3. INT -> INT: допустимо (identity) */
    test_sub("subtest %d: INT->INT (allowed)", ++subnum);
    {
        value64 v = value64_createint(42);
        test_validate(
            value64_is_convertable(v, VALUE64_INT, VALUE64_INT),
            "INT->INT must be convertable"
        );
    }

    /* 4. LONG -> INT (в пределах) – допустимо */
    test_sub("subtest %d: LNG->INT (in range)", ++subnum);
    {
        value64 v = value64_createlong(123456L);
        test_validate(
            value64_is_convertable(v, VALUE64_LNG, VALUE64_INT),
            "LNG->INT (in range) must be convertable"
        );
    }

    /* 5. LONG -> INT (переполнение) – НЕ допустимо */
    test_sub("subtest %d: LNG->INT (overflow)", ++subnum);
    {
        value64 v = value64_createlong(2147483648L);
        test_validate(
            !value64_is_convertable(v, VALUE64_LNG, VALUE64_INT),
            "LNG->INT overflow must NOT be convertable"
        );
    }

    /* 6. DBL -> INT (в пределах) – допустимо */
    test_sub("subtest %d: DBL->INT (in range)", ++subnum);
    {
        value64 v = value64_createdbl(3.14);
        test_validate(
            value64_is_convertable(v, VALUE64_DBL, VALUE64_INT),
            "DBL->INT (in range) must be convertable"
        );
    }

    /* 7. DBL -> INT (переполнение) – НЕ допустимо */
    test_sub("subtest %d: DBL->INT (overflow)", ++subnum);
    {
        value64 v = value64_createdbl(1.0e30);
        test_validate(
            !value64_is_convertable(v, VALUE64_DBL, VALUE64_INT),
            "DBL->INT overflow must NOT be convertable"
        );
    }

    /* 8. DBL -> INT (NaN) – НЕ допустимо */
    test_sub("subtest %d: DBL->INT (NaN)", ++subnum);
    {
        value64 v = value64_createdbl(NAN);
        test_validate(
            !value64_is_convertable(v, VALUE64_DBL, VALUE64_INT),
            "DBL->INT NaN must NOT be convertable"
        );
    }

    /* 9. DBL -> LONG (переполнение) – НЕ допустимо */
    test_sub("subtest %d: DBL->LNG (overflow)", ++subnum);
    {
        value64 v = value64_createdbl(1.0e30);
        test_validate(
            !value64_is_convertable(v, VALUE64_DBL, VALUE64_LNG),
            "DBL->LNG overflow must NOT be convertable"
        );
    }

    /* 10. FS -> INT: неизвестно, но не должно падать */
    test_sub("subtest %d: FS->INT (must not crash)", ++subnum);
    {
        fs tmp = fscopy("123");
        value64 v = value64_createfs(&tmp);
        fsfree(tmp);
        // Просто вызываем, результат не проверяем, но убеждаемся, что не упали
        test_validatefree(
            true,   // основное условие — дошли до этой точки
            value64_free(v, VALUE64_FS),
            "FS->INT must not crash (result was %s)",
            bool_str(value64_is_convertable(v, VALUE64_FS, VALUE64_INT))
        );
        value64_free(v, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* 11. STR -> DBL: не должно падать */
    test_sub("subtest %d: STR->DBL (must not crash)", ++subnum);
    {
        value64 v = value64_createstr("3.14");
        test_validatefree(
            true,
            value64_free(v, VALUE64_STR),
            "STR->DBL must not crash (result was %s)",
            bool_str(value64_is_convertable(v, VALUE64_STR, VALUE64_DBL))
        );
        value64_free(v, VALUE64_STR);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_value64_pt_compare -----------------------------
static TestStatus
tf_pt_compare(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ---------- INT ---------- */
    test_sub("subtest %d: cmp INT equal", ++subnum);
    {
        value64 a = value64_createint(42);
        value64 b = value64_createint(42);
        test_validate(
            value64_pt_compare(&a, &b, VALUE64_INT) == 0, 
            "42 must equal 42"
        );
    }

    test_sub("subtest %d: cmp INT less / greater", ++subnum);
    {
        value64 a = value64_createint(10);
        value64 b = value64_createint(20);
        test_validate(
            value64_pt_compare(&a, &b, VALUE64_INT) < 0,
            "10 must be less than 20"
        );
        test_validate(
            value64_pt_compare(&b, &a, VALUE64_INT) > 0,
            "20 must be greater than 10"
        );
    }

    /* ---------- LONG ---------- */
    test_sub("subtest %d: cmp LONG equal", ++subnum);
    {
        value64 a = value64_createlong(-999999L);
        value64 b = value64_createlong(-999999L);
        test_validate(
            value64_pt_compare(&a, &b, VALUE64_LNG) == 0,
            "-999999L must equal -999999L"
        );
    }

    test_sub("subtest %d: cmp LONG less / greater", ++subnum);
    {
        value64 a = value64_createlong(100L);
        value64 b = value64_createlong(200L);
        test_validate(
            value64_pt_compare(&a, &b, VALUE64_LNG) < 0,
            "100L must be less than 200L"
        );
        test_validate(
            value64_pt_compare(&b, &a, VALUE64_LNG) > 0,
            "200L must be greater than 100L"
        );
    }

    /* ---------- DBL ---------- */
    test_sub("subtest %d: cmp DBL equal", ++subnum);
    {
        value64 a = value64_createdbl(3.1415);
        value64 b = value64_createdbl(3.1415);
        test_validate(
            value64_pt_compare(&a, &b, VALUE64_DBL) == 0,
            "3.1415 must equal 3.1415"
        );
    }

    test_sub("subtest %d: cmp DBL less / greater", ++subnum);
    {
        value64 a = value64_createdbl(1.0);
        value64 b = value64_createdbl(2.0);
        test_validate(
            value64_pt_compare(&a, &b, VALUE64_DBL) < 0,
            "1.0 must be less than 2.0"
        );
        test_validate(
            value64_pt_compare(&b, &a, VALUE64_DBL) > 0,
            "2.0 must be greater than 1.0"
        );
    }

    test_sub("subtest %d: cmp DBL special (NaN, inf)", ++subnum);
    {
        value64 a = value64_createdbl(NAN);
        value64 b = value64_createdbl(NAN);
        test_validate(
            value64_pt_compare(&a, &b, VALUE64_DBL) == 0,
            "NaN must equal NaN (by implementation)"
        );

        value64 inf1 = value64_createdbl(INFINITY);
        value64 inf2 = value64_createdbl(INFINITY);
        test_validate(
            value64_pt_compare(&inf1, &inf2, VALUE64_DBL) == 0,
            "+inf must equal +inf"
        );

        value64 ninf = value64_createdbl(-INFINITY);
        test_validate(
            value64_pt_compare(&inf1, &ninf, VALUE64_DBL) > 0,
            "+inf must be greater than -inf"
        );
    }

    /* ---------- PTR ---------- */
    test_sub("subtest %d: cmp PTR equal / not equal", ++subnum);
    {
        int x = 1, y = 2;
        value64 a = value64_createptr(&x);
        value64 b = value64_createptr(&x);
        value64 c = value64_createptr(&y);

        test_validate(
            value64_pt_compare(&a, &b, VALUE64_PTR) == 0,
            "same address must be equal"
        );
        test_validate(
            value64_pt_compare(&a, &c, VALUE64_PTR) != 0,
            "different addresses must not be equal"
        );
    }

    /* ---------- STR ---------- */
    test_sub("subtest %d: cmp STR equal", ++subnum);
    {
        value64 a = value64_createstr("hello");
        value64 b = value64_createstr("hello");
        test_validatefree(
            value64_pt_compare(&a, &b, VALUE64_STR) == 0,
            (value64_free(a, VALUE64_STR), value64_free(b, VALUE64_STR)),
            "'hello' must equal 'hello'"
        );
        value64_freestr(a);
        value64_freestr(b);
    }

    test_sub("subtest %d: cmp STR not equal", ++subnum);
    {
        value64 a = value64_createstr("abc");
        value64 b = value64_createstr("xyz");
        test_validatefree(
            value64_pt_compare(&a, &b, VALUE64_STR) != 0,
            (value64_freestr(a), value64_freestr(b) ),
            "'abc' must not equal 'xyz'"
        );
        value64_freestr(a);
        value64_freestr(b);
    }

    /* ---------- FS ---------- */
    test_sub("subtest %d: cmp FS equal", ++subnum);
    {
        fs tmp = fscopy("fs-data");
        value64 a = value64_createfs(&tmp);
        value64 b = value64_createfs(&tmp);
        fsfree(tmp);

        test_validatefree(
            value64_pt_compare(&a, &b, VALUE64_FS) == 0,
            (value64_freefs(a), value64_freefs(b)),
            "fs 'fs-data' must equal itself"
        );
        value64_freefs(a);
        value64_freefs(b);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: cmp FS not equal", ++subnum);
    {
        fs tmp1 = fscopy("alpha");
        fs tmp2 = fscopy("beta");
        value64 a = value64_createfs(&tmp1);
        value64 b = value64_createfs(&tmp2);
        fsfree(tmp1);
        fsfree(tmp2);

        test_validatefree(
            value64_pt_compare(&a, &b, VALUE64_FS) != 0,
            (value64_freefs(a), value64_freefs(b)),
            "fs 'alpha' must not equal 'beta'"
        );
        value64_freefs(a);
        value64_freefs(b);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: cmp FS empty vs non-empty", ++subnum);
    {
        fs empty_str = fscopy("");
        value64 a = value64_createfs(&empty_str);
        fsfree(empty_str);

        fs tmp2 = fscopy("non-empty");
        value64 b = value64_createfs(&tmp2);
        fsfree(tmp2);

        test_validatefree(
            value64_pt_compare(&a, &b, VALUE64_FS) != 0,
            (value64_freestr(a), value64_freestr(b)),
            "empty fs must not equal non-empty"
        );
        value64_freefs(a);
        value64_freefs(b);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_(rev)search -----------------------------

static TestStatus
tf_search(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ---------- INT ---------- */
    test_sub("subtest %d: search INT – first", ++subnum);
    {
        value64 arr[] = { value64_createint(10), value64_createint(20), value64_createint(30) };
        test_validate(
            value64_search(value64_createint(10), VALUE64_INT, arr, COUNT(arr)) == 0,
            "10 must be at index 0"
        );
    }

    test_sub("subtest %d: search INT – middle", ++subnum);
    {
        value64 arr[] = { value64_createint(10), value64_createint(20), value64_createint(30) };
        test_validate(
            value64_search(value64_createint(20), VALUE64_INT, arr, COUNT(arr)) == 1,
            "20 must be at index 1"
        );
    }

    test_sub("subtest %d: search INT – last", ++subnum);
    {
        value64 arr[] = { value64_createint(10), value64_createint(20), value64_createint(30) };
        test_validate(
            value64_search(value64_createint(30), VALUE64_INT, arr, COUNT(arr)) == 2,
            "30 must be at index 2"
        );
    }

    test_sub("subtest %d: search INT – not found", ++subnum);
    {
        value64 arr[] = { value64_createint(10), value64_createint(20), value64_createint(30) };
        test_validate(
            value64_search(value64_createint(99), VALUE64_INT, arr, COUNT(arr)) == -1,
            "99 must not be found, return -1"
        );
    }

    test_sub("subtest %d: revsearch INT – last", ++subnum);
    {
        value64 arr[] = { value64_createint(10), value64_createint(20), value64_createint(30) };
        test_validate(
            value64_revsearch(value64_createint(30), VALUE64_INT, arr, COUNT(arr)) == 2,
            "30 must be at index 2 (reverse)"
        );
    }

    test_sub("subtest %d: revsearch INT – not found", ++subnum);
    {
        value64 arr[] = { value64_createint(10), value64_createint(20), value64_createint(30) };
        test_validate(
            value64_revsearch(value64_createint(99), VALUE64_INT, arr, COUNT(arr)) == -1,
            "99 must not be found (reverse), return -1"
        );
    }

    /* ---------- LONG ---------- */
    test_sub("subtest %d: search LONG – middle", ++subnum);
    {
        value64 arr[] = { value64_createlong(100L), value64_createlong(200L), value64_createlong(300L) };
        test_validate(
            value64_search(value64_createlong(200L), VALUE64_LNG, arr, COUNT(arr)) == 1,
            "200L must be at index 1"
        );
    }

    /* ---------- DBL ---------- */
    test_sub("subtest %d: search DBL – equal", ++subnum);
    {
        value64 arr[] = { value64_createdbl(1.5), value64_createdbl(2.5), value64_createdbl(3.5) };
        test_validate(
            value64_search(value64_createdbl(2.5), VALUE64_DBL, arr, COUNT(arr)) == 1,
            "2.5 must be at index 1"
        );
    }

    /* ---------- STR ---------- */
    test_sub("subtest %d: search STR – found", ++subnum);
    {
        value64 arr[] = { value64_createstr("apple"), value64_createstr("banana"), value64_createstr("cherry") };
        value64 key = value64_createstr("banana");
        int pos;
        test_validate(
            (pos = value64_search(key, VALUE64_STR, arr, COUNT(arr) ) ) == 1, 
            "'banana' must be at index 1, got %d", pos
        );
        value64_free(key, VALUE64_STR);
        for (int i = 0; i < COUNT(arr); i++) value64_free(arr[i], VALUE64_STR);
    }

    test_sub("subtest %d: revsearch STR – last", ++subnum);
    {
        value64 arr[] = { value64_createstr("apple"), value64_createstr("banana"), value64_createstr("cherry") };
        value64 key = value64_createstr("cherry");
        int pos;
        test_validate(
            (pos = value64_revsearch(key, VALUE64_STR, arr, COUNT(arr) ) ) == 2,
            "'cherry' must be at index 2 (reverse), got %d", pos
        );
        value64_free(key, VALUE64_STR);
        for (int i = 0; i < COUNT(arr); i++)
            value64_free(arr[i], VALUE64_STR);
    }

    /* ---------- FS ---------- */
    test_sub("subtest %d: search FS – found", ++subnum);
    {
        fs tmp1 = fscopy("alpha"), tmp2 = fscopy("beta"), tmp3 = fscopy("gamma");
        value64 arr[] = { value64_createfs(&tmp1), value64_createfs(&tmp2), value64_createfs(&tmp3) };
        fsfree(tmp1); fsfree(tmp2); fsfree(tmp3);

        fs key_fs = fscopy("beta");
        value64 key = value64_createfs(&key_fs);
        fsfree(key_fs);

        int pos;
        test_validate(
            (pos = value64_search(key, VALUE64_FS, arr, COUNT(arr) ) ) == 1,
            "'beta' must be at index 1, got %d", pos
        );

        value64_free(key, VALUE64_FS);
        for (int i = 0; i < COUNT(arr); i++)
            value64_free(arr[i], VALUE64_FS);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: revsearch FS – first", ++subnum);
    {
        fs tmp1 = fscopy("alpha"), tmp2 = fscopy("beta"), tmp3 = fscopy("gamma");
        value64 arr[] = { value64_createfs(&tmp1), value64_createfs(&tmp2), value64_createfs(&tmp3) };
        fsfree(tmp1); fsfree(tmp2); fsfree(tmp3);

        fs key_fs = fscopy("alpha");
        value64 key = value64_createfs(&key_fs);
        fsfree(key_fs);

        int pos;
        test_validate(
            (pos = value64_revsearch(key, VALUE64_FS, arr, COUNT(arr) ) ) == 0,
            "'alpha' must be at index 0 (reverse), got %d", pos
        );

        value64_free(key, VALUE64_FS);
        for (int i = 0; i < COUNT(arr); i++) value64_free(arr[i], VALUE64_FS);
        fs_alloc_check(true);
    }

    /* ---------- empty / edge ---------- */
    test_sub("subtest %d: search – empty array", ++subnum);
    {
        value64 key = value64_createint(1);
        test_validate(
            value64_search(key, VALUE64_INT, NULL, 0) == -1, 
            "empty array must return -1"
        );
    }

    test_sub("subtest %d: revsearch – empty array", ++subnum);
    {
        value64 key = value64_createint(1);
        test_validate(
            value64_revsearch(key, VALUE64_INT, NULL, 0) == -1,
            "empty array must return -1 (reverse)"
        );
    }
    /* 1. INT: присутствует */
    test_sub("subtest %d: value64_in – found", ++subnum);
    {
        value64 arr[] = { value64_createint(1), value64_createint(2), value64_createint(3) };
        test_validate(
            value64_in(value64_createint(2), VALUE64_INT, arr, COUNT(arr)),
            "2 must be in {1,2,3}"
        );
    }

    /* 2. INT: отсутствует */
    test_sub("subtest %d: value64_in – not found", ++subnum);
    {
        value64 arr[] = { value64_createint(1), value64_createint(2), value64_createint(3) };
        test_validate(
            !value64_in(value64_createint(99), VALUE64_INT, arr, COUNT(arr)),
            "99 must NOT be in {1,2,3}"
        );
    }

    /* 3. INT: notin (должно быть true) */
    test_sub("subtest %d: value64_notin – true", ++subnum);
    {
        value64 arr[] = { value64_createint(1), value64_createint(2), value64_createint(3) };
        test_validate(
            value64_notin(value64_createint(99), VALUE64_INT, arr, COUNT(arr)),
            "99 must be NOT in {1,2,3}"
        );
    }

    /* 4. INT: notin (должно быть false) */
    test_sub("subtest %d: value64_notin – false", ++subnum);
    {
        value64 arr[] = { value64_createint(1), value64_createint(2), value64_createint(3) };
        test_validate(
            !value64_notin(value64_createint(1), VALUE64_INT, arr, COUNT(arr)),
            "1 must be in {1,2,3}, so notin must be false"
        );
    }

    /* 5. Пустой массив – in возвращает false, notin возвращает true */
    test_sub("subtest %d: empty array", ++subnum);
    {
        test_validate(
            !value64_in(value64_createint(42), VALUE64_INT, NULL, 0),
            "in must be false for empty array"
        );
        test_validate(
            value64_notin(value64_createint(42), VALUE64_INT, NULL, 0),
            "notin must be true for empty array"
        );
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_get(Rev)Comparator -----------------------------

static TestStatus
tf_getComparator(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. INT comparator */
    test_sub("subtest %d: getComparator INT", ++subnum);
    {
        value64_Comparator cmp = value64_getComparator(VALUE64_INT);
        test_validate(cmp != NULL, "INT comparator must not be NULL");

        value64 a = value64_createint(10);
        value64 b = value64_createint(20);
        test_validate(cmp(a, b) < 0, "10 < 20 must be negative");
        test_validate(cmp(b, a) > 0, "20 > 10 must be positive");
        test_validate(cmp(a, a) == 0, "10 == 10 must be zero");
    }

    /* 2. INT rev comparator */
    test_sub("subtest %d: getRevComparator INT", ++subnum);
    {
        value64_RevComparator rcmp = value64_getRevComparator(VALUE64_INT);
        test_validate(rcmp != NULL, "INT rev comparator must not be NULL");

        value64 a = value64_createint(10);
        value64 b = value64_createint(20);
        test_validate(rcmp(a, b) > 0, "rev: 10 < 20 must give >0");
        test_validate(rcmp(b, a) < 0, "rev: 20 > 10 must give <0");
        test_validate(rcmp(a, a) == 0, "rev: 10 == 10 must be 0");
    }

    /* 3. LONG comparator */
    test_sub("subtest %d: getComparator LONG", ++subnum);
    {
        value64_Comparator cmp = value64_getComparator(VALUE64_LNG);
        test_validate(cmp != NULL, "LONG comparator must not be NULL");

        value64 a = value64_createlong(100L);
        value64 b = value64_createlong(200L);
        test_validate(cmp(a, b) < 0, "100 < 200 must be negative");
        test_validate(cmp(a, a) == 0, "100 == 100 must be zero");
    }

    /* 4. LONG rev comparator */
    test_sub("subtest %d: getRevComparator LONG", ++subnum);
    {
        value64_RevComparator rcmp = value64_getRevComparator(VALUE64_LNG);
        test_validate(rcmp != NULL, "LONG rev comparator must not be NULL");

        value64 a = value64_createlong(100L);
        value64 b = value64_createlong(200L);
        test_validate(rcmp(a, b) > 0, "rev: 100 < 200 must give >0");
    }

    /* 5. DBL comparator */
    test_sub("subtest %d: getComparator DBL", ++subnum);
    {
        value64_Comparator cmp = value64_getComparator(VALUE64_DBL);
        test_validate(cmp != NULL, "DBL comparator must not be NULL");

        value64 a = value64_createdbl(1.5);
        value64 b = value64_createdbl(2.5);
        test_validate(cmp(a, b) < 0, "1.5 < 2.5 must be negative");
        test_validate(cmp(a, a) == 0, "1.5 == 1.5 must be zero");
    }

    /* 6. DBL rev comparator */
    test_sub("subtest %d: getRevComparator DBL", ++subnum);
    {
        value64_RevComparator rcmp = value64_getRevComparator(VALUE64_DBL);
        test_validate(rcmp != NULL, "DBL rev comparator must not be NULL");

        value64 a = value64_createdbl(1.5);
        value64 b = value64_createdbl(2.5);
        test_validate(rcmp(a, b) > 0, "rev: 1.5 < 2.5 must give >0");
    }

    /* 7. FS comparator */
    test_sub("subtest %d: getComparator FS", ++subnum);
    {
        value64_Comparator cmp = value64_getComparator(VALUE64_FS);
        test_validate(cmp != NULL, "FS comparator must not be NULL");

        fs tmp1 = fscopy("alpha"), tmp2 = fscopy("beta");
        value64 a = value64_createfs(&tmp1), b = value64_createfs(&tmp2);
        fsfree(tmp1); fsfree(tmp2);

        test_validatefree(
            cmp(a, b) < 0,
            (value64_free(a, VALUE64_FS), value64_free(b, VALUE64_FS)),
            "FS: 'alpha' < 'beta' must be negative"
        );
        // проверка равенства
        fs tmp3 = fscopy("alpha");
        value64 a2 = value64_createfs(&tmp3);
        fsfree(tmp3);
        test_validatefree(
            cmp(a, a2) == 0,
            (value64_free(a, VALUE64_FS), value64_free(a2, VALUE64_FS)),
            "FS: 'alpha' == 'alpha' must be zero"
        );
        value64_free(a, VALUE64_FS); value64_free(b, VALUE64_FS); value64_free(a2, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* 8. FS rev comparator */
    test_sub("subtest %d: getRevComparator FS", ++subnum);
    {
        value64_RevComparator rcmp = value64_getRevComparator(VALUE64_FS);
        test_validate(rcmp != NULL, "FS rev comparator must not be NULL");

        fs tmp1 = fscopy("alpha"), tmp2 = fscopy("beta");
        value64 a = value64_createfs(&tmp1), b = value64_createfs(&tmp2);
        fsfree(tmp1); fsfree(tmp2);

        test_validatefree(
            rcmp(a, b) > 0,
            (value64_free(a, VALUE64_FS), value64_free(b, VALUE64_FS)),
            "FS rev: 'alpha' < 'beta' must give >0"
        );
        value64_free(a, VALUE64_FS); value64_free(b, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* 9. STR comparator */
    test_sub("subtest %d: getComparator STR", ++subnum);
    {
        value64_Comparator cmp = value64_getComparator(VALUE64_STR);
        test_validate(cmp != NULL, "STR comparator must not be NULL");

        value64 a = value64_createstr("hello");
        value64 b = value64_createstr("world");
        test_validatefree(
            cmp(a, b) < 0,
            (value64_free(a, VALUE64_STR), value64_free(b, VALUE64_STR)),
            "STR: 'hello' < 'world' must be negative"
        );
        value64_free(a, VALUE64_STR); value64_free(b, VALUE64_STR);
    }

    /* 10. STR rev comparator */
    test_sub("subtest %d: getRevComparator STR", ++subnum);
    {
        value64_RevComparator rcmp = value64_getRevComparator(VALUE64_STR);
        test_validate(rcmp != NULL, "STR rev comparator must not be NULL");

        value64 a = value64_createstr("hello");
        value64 b = value64_createstr("world");
        test_validatefree(
            rcmp(a, b) > 0,
            (value64_free(a, VALUE64_STR), value64_free(b, VALUE64_STR)),
            "STR rev: 'hello' < 'world' must give >0"
        );
        value64_free(a, VALUE64_STR); value64_free(b, VALUE64_STR);
    }

    /* 11. PTR comparator (требуется исправление getComparator) */
    test_sub("subtest %d: getComparator PTR", ++subnum);
    {
        // Предполагаем, что вы исправили VALUE64_PTR -> value64_ptr_comp
        value64_Comparator cmp = value64_getComparator(VALUE64_PTR);
        test_validate(cmp != NULL, "PTR comparator must not be NULL");

        int x = 1, y = 2;
        value64 a = value64_createptr(&x);
        value64 b = value64_createptr(&y);
        test_validate(cmp(a, b) != 0, "different addresses must not be zero");
        test_validate(cmp(a, a) == 0, "same address must be zero");
    }

    /* 12. PTR rev comparator */
    test_sub("subtest %d: getRevComparator PTR", ++subnum);
    {
        value64_RevComparator rcmp = value64_getRevComparator(VALUE64_PTR);
        test_validate(rcmp != NULL, "PTR rev comparator must not be NULL");

        int x = 1, y = 2;
        value64 a = value64_createptr(&x);
        value64 b = value64_createptr(&y);
        int direct = value64_getComparator(VALUE64_PTR)(a, b);
        test_validate(rcmp(a, b) == -direct, "PTR rev must negate direct result");
    }

    /* 13. Неподдерживаемый тип */
    test_sub("subtest %d: unsupported type (must raise error)", ++subnum);
    {
        if (!try()) {
            value64_Comparator cmp = value64_getComparator(VALUE64_UNKNOWN);
            test_validate(false, "Unsupported type must raise error, but returned %p", (void*)cmp);
        } else {
            test_validate(true, "Unsupported type correctly raised error");
        }
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_getP(Rev)Comparator -----------------------------

static TestStatus
tf_getPComparator(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ---------- 1. P_INT comparator ---------- */
    test_sub("subtest %d: getPComparator INT", ++subnum);
    {
        value64_PComparator cmp = value64_getPComparator(VALUE64_INT);
        test_validate(cmp != NULL, "INT P-comparator must not be NULL");

        value64 a = value64_createint(10);
        value64 b = value64_createint(20);
        test_validate(cmp(&a, &b) < 0, "10 < 20 must be negative");
        test_validate(cmp(&b, &a) > 0, "20 > 10 must be positive");
        test_validate(cmp(&a, &a) == 0, "10 == 10 must be zero");
    }

    /* 2. P_INT rev comparator */
    test_sub("subtest %d: getPRevComparator INT", ++subnum);
    {
        value64_PRevComparator rcmp = value64_getPRevComparator(VALUE64_INT);
        test_validate(rcmp != NULL, "INT P-rev-comparator must not be NULL");

        value64 a = value64_createint(10);
        value64 b = value64_createint(20);
        test_validate(rcmp(&a, &b) > 0, "rev: 10 < 20 must give >0");
        test_validate(rcmp(&b, &a) < 0, "rev: 20 > 10 must give <0");
        test_validate(rcmp(&a, &a) == 0, "rev: 10 == 10 must be 0");
    }

    /* ---------- 3. P_LONG comparator ---------- */
    test_sub("subtest %d: getPComparator LONG", ++subnum);
    {
        value64_PComparator cmp = value64_getPComparator(VALUE64_LNG);
        test_validate(cmp != NULL, "LONG P-comparator must not be NULL");

        value64 a = value64_createlong(100L);
        value64 b = value64_createlong(200L);
        test_validate(cmp(&a, &b) < 0, "100 < 200 must be negative");
        test_validate(cmp(&a, &a) == 0, "100 == 100 must be zero");
    }

    /* 4. P_LONG rev comparator */
    test_sub("subtest %d: getPRevComparator LONG", ++subnum);
    {
        value64_PRevComparator rcmp = value64_getPRevComparator(VALUE64_LNG);
        test_validate(rcmp != NULL, "LONG P-rev-comparator must not be NULL");

        value64 a = value64_createlong(100L);
        value64 b = value64_createlong(200L);
        test_validate(rcmp(&a, &b) > 0, "rev: 100 < 200 must give >0");
    }

    /* ---------- 5. P_DBL comparator ---------- */
    test_sub("subtest %d: getPComparator DBL", ++subnum);
    {
        value64_PComparator cmp = value64_getPComparator(VALUE64_DBL);
        test_validate(cmp != NULL, "DBL P-comparator must not be NULL");

        value64 a = value64_createdbl(1.5);
        value64 b = value64_createdbl(2.5);
        test_validate(cmp(&a, &b) < 0, "1.5 < 2.5 must be negative");
        test_validate(cmp(&a, &a) == 0, "1.5 == 1.5 must be zero");
    }

    /* 6. P_DBL rev comparator */
    test_sub("subtest %d: getPRevComparator DBL", ++subnum);
    {
        value64_PRevComparator rcmp = value64_getPRevComparator(VALUE64_DBL);
        test_validate(rcmp != NULL, "DBL P-rev-comparator must not be NULL");

        value64 a = value64_createdbl(1.5);
        value64 b = value64_createdbl(2.5);
        test_validate(rcmp(&a, &b) > 0, "rev: 1.5 < 2.5 must give >0");
    }

    /* ---------- 7. P_FS comparator ---------- */
    test_sub("subtest %d: getPComparator FS", ++subnum);
    {
        value64_PComparator cmp = value64_getPComparator(VALUE64_FS);
        test_validate(cmp != NULL, "FS P-comparator must not be NULL");

        fs tmp1 = fscopy("alpha"), tmp2 = fscopy("beta");
        value64 a = value64_createfs(&tmp1), b = value64_createfs(&tmp2);
        fsfree(tmp1); fsfree(tmp2);

        test_validatefree(
            cmp(&a, &b) < 0,
            (value64_free(a, VALUE64_FS), value64_free(b, VALUE64_FS)),
            "FS P-comparator: 'alpha' < 'beta' must be negative"
        );

        fs tmp3 = fscopy("alpha");
        value64 a2 = value64_createfs(&tmp3);
        fsfree(tmp3);
        test_validatefree(
            cmp(&a, &a2) == 0,
            (value64_free(a, VALUE64_FS), value64_free(a2, VALUE64_FS)),
            "FS P-comparator: 'alpha' == 'alpha' must be zero"
        );
        value64_free(a, VALUE64_FS); value64_free(b, VALUE64_FS); value64_free(a2, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* 8. P_FS rev comparator */
    test_sub("subtest %d: getPRevComparator FS", ++subnum);
    {
        value64_PRevComparator rcmp = value64_getPRevComparator(VALUE64_FS);
        test_validate(rcmp != NULL, "FS P-rev-comparator must not be NULL");

        fs tmp1 = fscopy("alpha"), tmp2 = fscopy("beta");
        value64 a = value64_createfs(&tmp1), b = value64_createfs(&tmp2);
        fsfree(tmp1); fsfree(tmp2);

        test_validatefree(
            rcmp(&a, &b) > 0,
            (value64_free(a, VALUE64_FS), value64_free(b, VALUE64_FS)),
            "FS P-rev: 'alpha' < 'beta' must give >0"
        );
        value64_free(a, VALUE64_FS); value64_free(b, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* ---------- 9. P_STR comparator ---------- */
    test_sub("subtest %d: getPComparator STR", ++subnum);
    {
        value64_PComparator cmp = value64_getPComparator(VALUE64_STR);
        test_validate(cmp != NULL, "STR P-comparator must not be NULL");

        value64 a = value64_createstr("hello");
        value64 b = value64_createstr("world");
        test_validatefree(
            cmp(&a, &b) < 0,
            (value64_free(a, VALUE64_STR), value64_free(b, VALUE64_STR)),
            "STR P-comparator: 'hello' < 'world' must be negative"
        );
        value64_free(a, VALUE64_STR); value64_free(b, VALUE64_STR);
    }

    /* 10. P_STR rev comparator */
    test_sub("subtest %d: getPRevComparator STR", ++subnum);
    {
        value64_PRevComparator rcmp = value64_getPRevComparator(VALUE64_STR);
        test_validate(rcmp != NULL, "STR P-rev-comparator must not be NULL");

        value64 a = value64_createstr("hello");
        value64 b = value64_createstr("world");
        test_validatefree(
            rcmp(&a, &b) > 0,
            (value64_free(a, VALUE64_STR), value64_free(b, VALUE64_STR)),
            "STR P-rev: 'hello' < 'world' must give >0"
        );
        value64_free(a, VALUE64_STR); value64_free(b, VALUE64_STR);
    }

    /* ---------- 11. P_PTR comparator ---------- */
    test_sub("subtest %d: getPComparator PTR", ++subnum);
    {
        value64_PComparator cmp = value64_getPComparator(VALUE64_PTR);
        test_validate(cmp != NULL, "PTR P-comparator must not be NULL");

        int x = 1, y = 2;
        value64 a = value64_createptr(&x);
        value64 b = value64_createptr(&y);
        test_validate(cmp(&a, &b) != 0, "different addresses must not be zero");
        test_validate(cmp(&a, &a) == 0, "same address must be zero");
    }

    /* 12. P_PTR rev comparator */
    test_sub("subtest %d: getPRevComparator PTR", ++subnum);
    {
        value64_PRevComparator rcmp = value64_getPRevComparator(VALUE64_PTR);
        test_validate(rcmp != NULL, "PTR P-rev-comparator must not be NULL");

        int x = 1, y = 2;
        value64 a = value64_createptr(&x);
        value64 b = value64_createptr(&y);
        int direct = value64_getPComparator(VALUE64_PTR)(&a, &b);
        test_validate(rcmp(&a, &b) == -direct, "PTR P-rev must negate direct result");
    }

    /* ---------- 13. Неподдерживаемый тип ---------- */
    test_sub("subtest %d: unsupported type (must raise error)", ++subnum);
    {
        if (!try()) {
            value64_PComparator cmp = value64_getPComparator(VALUE64_UNKNOWN);
            test_validate(false, "Unsupported type must raise error, but returned %p", (void*)cmp);
        } else {
            test_validate(true, "Unsupported type correctly raised error");
        }
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main(int argc, const char *argv[])
{

 logsimpleinit("Start");
    bool    runall = argc == 1;
    printf("%d\n", argc);

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
                testnew(.f2 = tf_init_free,        .num =  1, .name = "Simple init and validate test"              , .desc="", .mandatory=true)
              , testnew(.f2 = tf_point_init,       .num =  2, .name = "Simple value64_pcopy_move() test"           , .desc="", .mandatory=true)
              , testnew(.f2 = tf_clone,            .num =  3, .name = "Simple value64_clone() test"                , .desc="", .mandatory=true)
              , testnew(.f2 = tf_move,             .num =  4, .name = "Simple value64_move() test"                 , .desc="", .mandatory=true)
              , testnew(.f2 = tf_lhash,            .num =  5, .name = "Simple value64_lhash() test"                , .desc="", .mandatory=true)
              , testnew(.f2 = tf_compare,          .num =  6, .name = "Simple value64_compare() test"              , .desc="", .mandatory=true)
              , testnew(.f2 = tf_convert,          .num =  7, .name = "Simple value64_convert() test"              , .desc="", .mandatory=true)
              , testnew(.f2 = tf_convert_move,     .num =  8, .name = "Simple value64_convert_move() test"         , .desc="", .mandatory=true)
              , testnew(.f2 = tf_is_convertable,   .num =  9, .name = "Simple value64_is_convertable() test"       , .desc="", .mandatory=true)
              , testnew(.f2 = tf_pt_compare,       .num = 10, .name = "Simple value64_pt_compare() test"           , .desc="", .mandatory=true)
              , testnew(.f2 = tf_search,           .num = 11, .name = "Simple value64_(rev)search test"            , .desc="", .mandatory=true)
              , testnew(.f2 = tf_getComparator,    .num = 12, .name = "Simple value64_get(Rev)Comparator test"     , .desc="", .mandatory=true)
              , testnew(.f2 = tf_getPComparator,   .num = 13, .name = "Simple value64_getP(Rev)Comparator test"    , .desc="", .mandatory=true)
            );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HSETTESTING */


