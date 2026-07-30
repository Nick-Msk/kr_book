// only array.h
#include "array.h"


/********************************************************************
                 ARRAY MODULE IMPLEMENTATION
********************************************************************/

//  globals, can be changed by app

// TODO: context must be used for that
int                      g_array_rec_line        = 20;  // TODO: rework that to normal (in Array structure)
const char              *g_custom_print_line     = 0;   // TODO: rework that to normal (in Array structure)
const char              *g_save_format_double    = "%6d      %15.15lg\n";
const char              *g_save_format_int       = "%6d\t%6d\n";
const char              *g_save_format_long      = "%6d\t%6ld\n";
const char              *g_save_format_pointer   = "%6d\t%p\n";
// not possible to format v64 that way!
const double             g_array_dbl_increment   = 0.01;

// internal type

// ---------- pseudo-header for utility procedures -----------------

// ------------------------------ Utilities ------------------------

/// @brief free elements of array
/// @param arr pointer to array
/// @param from from
/// @param to to
static void                     freeelems(Array *arr, int from, int to) {
    invraisecode(ERR_NULLABLE_PTR, arr != NULL, "Null pointer");
    invraisecode(ERR_OUT_OF_RANGE, from >= 0 && to <= arr->sz, "Invalid range");
    if (arr->v64type == VALUE64_STR || arr->v64type == VALUE64_FS) {   
        for (int i = from; i < to; i++) {
            value64free(arr->v64[i], arr->v64type);
        }
        logsimple("freed %s  %d - %d", value64_typename(arr->v64type), from, to);
    }
} 
/// @brief increase or descrease size of array
/// @param arr pointer to array
/// @param newsz new size
/// @return 
static int                      increase(Array *arr, int newsz){
    invraisecode(ERR_NULLABLE_PTR, arr != NULL, "Null pointer");
    invraisecode(ERR_OUT_OF_RANGE, newsz >= 0, "Size must be >= 0");
    logenter("newsz %d", newsz);
    if (newsz == arr->sz)
        return logret(arr->sz, "No change sz %d", arr->sz);
    if (newsz > arr->sz)
        newsz = round_up_2(newsz);

    int bytes = newsz;
    // TODO: that need to be ref!
    if (Array_isint(*arr))
        bytes *= sizeof(int);
    else if (Array_islong(*arr))
        bytes *= sizeof(long);
    else if (Array_isdouble(*arr))
        bytes *= sizeof(double);
    else if (Array_ispointer(*arr))
        bytes *= sizeof(void *);
    else if (Array_isv64(*arr))
        bytes *= sizeof(value64);
    else
        return logerr(-1, "Unknown type");

    if (newsz < arr->len)
        freeelems(arr, newsz, arr->len);    
    logmsg("Arr: bytes=%d, sz=%d", bytes, newsz);
    void *p = NULL;  
    if (bytes > 0) {
        if ( (p = realloc(arr->v, bytes) ) == NULL)
            userraiseint(ERR_UNABLE_ALLOCATE, "Unable to allocate %d", bytes);
    } else
        free(arr->v);
    arr->v = p; // iv/dv/pv... is the same
    if (arr->len > newsz)   // shrink case
        arr->len = newsz;
    arr->sz = newsz;
    return logret(arr->sz, "New sz %d", arr->sz);
}

/**
 * @brief Writes an integer value into an array (plain or value64).
 *
 * If the array is a value64 array (Array_isv64), the value is wrapped
 * via value64_createint and stored in a.v64[i].  Otherwise it is written
 * directly to a.iv[i].
 *
 * @param a   array
 * @param i   element index
 * @param val value to write
 */
static inline void set_int_element(Array a, int i, int val) {
    if (Array_isv64(a))
        a.v64[i] = value64_createint(val);
    else
        a.iv[i] = val;
}

/**
 * @brief Writes a long integer into an array (plain or value64).
 * @see set_int_element
 */
static inline void set_long_element(Array a, int i, long val) {
    if (Array_isv64(a))
        a.v64[i] = value64_createlong(val);
    else
        a.lv[i] = val;
}

/**
 * @brief Writes a double into an array (plain or value64).
 * @see set_int_element
 */
static inline void set_double_element(Array a, int i, double val) {
    if (Array_isv64(a))
        a.v64[i] = value64_createdbl(val);
    else
        a.dv[i] = val;
}

/**
 * @brief Writes a pointer into an array (plain or value64).
 * @see set_int_element
 */
static inline void set_pointer_element(Array a, int i, void *val) {
    if (Array_isv64(a))
        a.v64[i] = value64_createptr(val);
    else
        a.pv[i] = val;
}
/**
 * @brief Writes an fs value into a value64 array.
 *
 * The array must be a value64 array (Array_isv64).  The fs object is
 * deep‑copied into the array element.
 *
 * @param a   value64 array
 * @param i   element index
 * @param val pointer to the fs object to copy
 */
static inline void set_v64fs_element(Array a, int i, const fs *val) {
    a.v64[i] = value64_createfs(val);
}

/**
 * @brief Writes a C‑string into a value64 array (owned copy).
 *
 * The array must be a value64 array (Array_isv64).  The string is
 * duplicated and stored as a VALUE64_STR element.
 *
 * @param a   value64 array
 * @param i   element index
 * @param val C‑string to copy
 */
static inline void set_v64str_element(Array a, int i, const char *val) {
    a.v64[i] = value64_createstr(val);
}



// -------------------------- (Utility) printers -------------------

// --------------------------- API ---------------------------------

// ------------- CONSTRUCTOTS/DESTRUCTORS --------------

// CREATE  and fill with method
Array                           Array_create(int cnt, ArrayFillType filltyp, ArrayType typ, value64_type vt){
    logenter("cnt %d, filltyp %s typ %s", cnt, ArrayFillTypeName(filltyp), ArrayTypeName(typ) );
    // TODO: refactor via Array_increase
    Array       res = Array_init(.flags = typ, .v64type = vt);
    if (cnt <= 0)
        return logerr(res, "sz = %d", res.sz);

    increase(&res, cnt);
    res.len = cnt;
    Array_fill(res, filltyp);
    return logret(res, "sz = %d", res.sz);
}
/// @brief free array
/// @param val pointer to array
/// @note: Array_free must not failed even if val == NULL
void                            Array_free(Array *val){
    if (val)        // Array_free must not failed even if val == NULL
        increase(val, 0);
    /*if (val && val->iv){
        freeelems(val, 0, val->len);
        free(val->iv);
        val->iv = 0;
    }*/
}
/// @brief        Array filler (formatter) using fill type
/// @param a  Array (by value now, will be reworked)
/// @param typ  Array type 
/// @param vt   V64 type, only for for V64
/// @return Count of formatter data
int                             Array_fill(Array a, ArrayFillType typ){
    return Array_fillrange(a, typ, 0, a.len);
}
/// @brief      int incrementer (or dec)
/// @param val  int value 
/// @param sign sign (1/-1) 
/// @return adjusted value
static inline int               incintrnd(int *val, int sign){
    return (*val += sign * (rndint(10) + 1) );
}
/// @brief      long incrementer (or dec)
/// @param val  long value 
/// @param sign sign (1/-1) 
/// @return adjusted value
static inline long              inclongrnd(long *val, int sign){
    return (*val += sign * (rndlong(10) + 1) );
}
/// @brief       double incrementer (or dec)
/// @param val   double value
/// @param sign  sign (1/-1) 
/// @return adjusted value
static inline double           incdoublernd(double *val, int sign){
    return (*val += sign * (rnddbl(10) + g_array_dbl_increment) );
}
/// @brief        const char* incrementer
/// @param s      buffer fs pointer 
/// @return       inctremented fs
static inline fs               *incfs(fs *s) {
    fs_catstr(s, "A");    // increment len by 1
    return s;
}
/// @brief        fs dec
/// @param s      buffer fs pointer 
/// @return       decremented fs
static inline fs               *decfs(fs *s) {
    fs_setlen(s, fs_len(s) - 1);    // dec len by 1
    return s;
}

/// @brief        ascending filler
/// @param a      array 
/// @param from   start index 
/// @param to     end index 
/// @return       count of filled elements
static int                      Array_fillrange_ASC(Array a, int from, int to){
    switch (ArrayGetV64mappedType(a) ) {
        case ARRAY_INT: {
            int val = 0;
            for (int i = from; i < to; i++, incintrnd(&val, 1) )
                set_int_element(a, i, val);
            break;
        }
        case ARRAY_LONG: {
            long val = 0;
            for (int i = from; i < to; i++, inclongrnd(&val, 1) )
                set_long_element(a, i, val);
            break;
        }
        case ARRAY_DOUBLE: {
            double val = 0.0;
            for (int i = from; i < to; i++, incdoublernd(&val, 1) )
                set_double_element(a, i, val);
            break;
        }
        // V64, which not mapped to ARRAY_TYPES
        case ARRAY_UNKNOWN: {
            switch (a.v64type) {
                case VALUE64_FS: {
                    fs s = FS();
                    // fs увеличивающейся длины
                    for (int i = from; i < to; i++) {
                        incfs(&s);   // длина i+1
                        set_v64fs_element(a, i, &s);  
                    }
                    fsfree(s);
                    break;
                }
                case VALUE64_STR: {
                    fs s = FS();    // buf
                    // C‑строки увеличивающейся длины
                    for (int i = from; i < to; i++) {
                        incfs(&s);   // длина i+1
                        set_v64str_element(a, i, fsstr(s));
                    }
                    fsfree(s);
                    break;
                }
                default:
                    userraiseint(ERR_ACTION_NOT_APPLICABLE, "Unsupported v64 type for ASC fill %s", ArrayGetV64typeName(a) );
                    break;
            }
            break;
        }
        default:
            userraiseint(ERR_ACTION_NOT_APPLICABLE, "Unsupported type for ASC fill %s", ArrayGettypeName(a) );
            break;
    }
    return to - from;
}
/// @brief        descending filler
/// @param a      array 
/// @param from   start index 
/// @param to     end index 
/// @return       count of filled elements
static int                      Array_fillrange_DESC(Array a, int from, int to){
    switch (ArrayGetV64mappedType(a) ) {
        case ARRAY_INT: {
            int val = 10 * a.len;   // hope it'll ne owerwelhm int;
            for (int i = from; i < to; i++, incintrnd(&val, -1) )
                set_int_element(a, i, val);
            break;
        }
        case ARRAY_LONG: {
            long val = 100L * a.len;
            for (int i = from; i < to; i++, inclongrnd(&val, -1) )
                set_long_element(a, i, val);
            break;
        }
        case ARRAY_DOUBLE: {
            double val = 0.0;
            for (int i = from; i < to; i++, incdoublernd(&val, -1) )
                set_double_element(a, i, val);
            break;
        }
        // V64, which not mapped to ARRAY_TYPES
        case ARRAY_UNKNOWN: {
            switch (a.v64type) {
                case VALUE64_FS: {
                    fs s = fscopyf("%*s", to - from + 1,  "A");    // buf
                    // fs desc length
                    for (int i = from; i < to; i++) {
                        decfs(&s);   // длина i+1
                        set_v64fs_element(a, i, &s);  
                    }
                    fsfree(s);
                    break;
                }
                case VALUE64_STR: {
                    fs s = fscopyf("%*s", to - from + 1,  "A");    // buf
                    // c-str desc length
                    for (int i = from; i < to; i++) {
                        decfs(&s);   // длина i+1
                        set_v64str_element(a, i, fsstr(s));
                    }
                    fsfree(s);
                    break;
                }
                default:
                    userraiseint(ERR_ACTION_NOT_APPLICABLE, "Unsupported v64 type for DESC fill %s", ArrayGetV64typeName(a) );
                    break;
            }
            break;
        }
        default:
            userraiseint(ERR_ACTION_NOT_APPLICABLE, "Unsupported type for DESC fill %s", ArrayGettypeName(a) );
            break;
    }
    return to - from;
}

/// @brief        zero filler
/// @param a      array 
/// @param from   start index 
/// @param to     end index 
/// @return       count of filled elements
static int                      Array_fillrange_ZERO(Array a, int from, int to){
    switch (ArrayGetV64mappedType(a) ) {
        case ARRAY_INT:
            for (int i = from; i < to; i++) // iter??? TODO: check if it's correct
                set_int_element(a, i, 0);
            break;
        case ARRAY_LONG:
            for (int i = from; i < to; i++) // iter??? TODO: check if it's correct
                set_long_element(a, i, 0L);
            break;
        case ARRAY_DOUBLE:
            for (int i = from; i < to; i++) // iter??? TODO: check if it's correct
                set_double_element(a, i, 0.0);
            break;
        case ARRAY_POINTER:
            for (int i = from; i < to; i++) // iter??? TODO: check if it's correct
                set_pointer_element(a, i, NULL);
            break;
        // not real type => container v64
        case ARRAY_UNKNOWN: {
            switch (a.v64type) {
                case VALUE64_FS: {
                    fs s = FSLITERAL("");
                    for (int i = from; i < to; i++)
                        set_v64fs_element(a, i, &s);  
                    break;
                }
                case VALUE64_STR: {
                    for (int i = from; i < to; i++)
                        set_v64str_element(a, i, "");
                    break;
                }
                default:
                    userraiseint(ERR_ACTION_NOT_APPLICABLE, "Unsupported v64 type for ZERO v64 type %s", ArrayGetV64typeName(a) );
                    break;
            }
            break;
        }
        default:
            userraiseint(ERR_ACTION_NOT_APPLICABLE, "Unsupported type for ZERO fill %s", ArrayGettypeName(a) );
            break;
    }
    return to - from;
}

static int                      Array_fillrange_NONE(Array a, int from, int to) {
    switch (ArrayGetV64mappedType(a) ) {
        case ARRAY_UNKNOWN: {
            switch (a.v64type) {
                case VALUE64_FS: {
                    fs s = FS();
                    for (int i = from; i < to; i++)
                        a.v64[i] = LITERAL64_FS(s);;  
                    break;
                }
                case VALUE64_STR: {
                    for (int i = from; i < to; i++)
                        a.v64[i] = LITERAL64_STR("");
                    break;
                }
                default:
                    userraiseint(ERR_ACTION_NOT_APPLICABLE, "Unsupported v64 type for ZERO v64 type %s", ArrayGetV64typeName(a) );
                    break;
            }
            break;
        }
        default:
        break;
    }
    return to - from;
}

/// @brief        random value filler
/// @param a      array 
/// @param from   start index 
/// @param to     end index 
/// @return       count of filled elements
static int                      Array_fillrange_RND(Array a, int from, int to) {
    switch (ArrayGetV64mappedType(a) ) {
        case ARRAY_INT:
            for (int i = from; i < to; i++) // iter??? TODO: check if it's correct
                 set_int_element(a, i, rndint(10 * (to - from + 1) ) );
            break;
        case ARRAY_LONG:
            for (int i = from; i < to; i++) // iter??? TODO: check if it's correct
                 set_long_element(a, i, rndlong(10 * (to - from + 1) ) );
            break;
        case ARRAY_DOUBLE:
            for (int i = from; i < to; i++) // iter??? TODO: check if it's correct
                set_double_element(a, i, rnddbl(10 * (to - from + 1) ) );
            break;
        case ARRAY_UNKNOWN: {
            switch (a.v64type) {
                case VALUE64_FS: {
                    fs s = FS();
                    for (int i = from; i < to; i++) {
                        // fs_genrnd(&s, from - to + 1, 'A'); TODO:
                        set_v64fs_element(a, i, &s);  
                    }
                    fsfree(s);
                    break;
                }
                case VALUE64_STR: {
                    fs s = FS();
                    for (int i = from; i < to; i++) {
                        // fs_genrnd(&s, from - to + 1, 'A'); TODO:
                        set_v64str_element(a, i, "");
                    }
                    fsfree(s);
                    break;
                }
                default:
                    userraiseint(ERR_ACTION_NOT_APPLICABLE, "Unsupported v64 type for ZERO v64 fill %s", ArrayGetV64typeName(a) );
                    break;
            }
            break;
        }
        default:
            userraiseint(ERR_ACTION_NOT_APPLICABLE, "Unsupported type for ZERO fill %s", ArrayGettypeName(a) );
            break;
    }
    return to - from;
}

/// @brief        ascending series filler
/// @param a      array 
/// @param from   start index 
/// @param to     end index 
/// @return       count of filled elements
static int                      Array_fillrange_ASC_SERIES(Array a, int from, int to){
    switch (ArrayGetV64mappedType(a) ) {
        case ARRAY_INT: {
            int val = from;
            for (int i = from; i < to; i++) // iter??? TODO: check if it's correct
                 set_int_element(a, i, val++);
            break;
        }
        case ARRAY_LONG: {
            long val = from;
            for (int i = from; i < to; i++)
                set_long_element(a, i, val++);
            break;
        }
        case ARRAY_DOUBLE: {
            double val = from + 0.0;
            for (int i = from; i < to; i++)
                set_double_element(a, i, val++);
            break;
        }
        default:
            userraiseint(ERR_ACTION_NOT_APPLICABLE, "Unsupported type for ASC fill %s", ArrayGettypeName(a) );
            break;
    }
    return to - from;
}

/// @brief        descending series filler
/// @param a      array 
/// @param from   start index 
/// @param to     end index 
/// @return       count of filled elements
static int                      Array_fillrange_DESC_SERIES(Array a, int from, int to){
    switch (ArrayGetV64mappedType(a) ) {
        case ARRAY_INT: {
            int val = to - 1;
            for (int i = from; i < to; i++) // iter??? TODO: check if it's correct
                 set_int_element(a, i, val--);
            break;
        }
        case ARRAY_LONG: {
            long val = to - 1;
            for (int i = from; i < to; i++)
                set_long_element(a, i, val--);
            break;
        }
        case ARRAY_DOUBLE: {
            double val = to - 1;
            for (int i = from; i < to; i++)
                set_double_element(a, i, val--);
            break;
        }
        default:
            userraiseint(ERR_ACTION_NOT_APPLICABLE, "Unsupported type for ASC fill %s", ArrayGettypeName(a) );
            break;
    }
    return to - from;
}

/// @brief Array filler
/// @param a base array
/// @param typ Array type 
/// @param from from (will be normilized if out of range)
/// @param to  to (will be normilized if out of range)
/// @return Count of formatter data
int                             Array_fillrange(Array a, ArrayFillType typ, int from, int to) {
    logenter("%d - %d, %s (v64: %s)", from, to, ArrayFillTypeName(typ), ArrayGetV64typeName(a) );
    // Нормализация границ
    if (from < 0) {
        from = 0;
        logmsg("'from' was negative, normalized to 0");
    }
    if (to > a.sz){
        to = a.sz;
        logmsg("'to' was out of range - normalized to sz %d", a.sz);
    }
    switch (typ) {
        case ARRAY_FILLTYPE_ASC:
            // ----- Заполнение по возрастанию -----
            Array_fillrange_ASC(a, from, to);
            break;
        case ARRAY_FILLTYPE_DESC:
            // ----- Заполнение по убыванию -----
            Array_fillrange_DESC(a, from, to);
            break;
        case ARRAY_FILLTYPE_ZERO:
            Array_fillrange_ZERO(a, from, to);
            break;
        case ARRAY_FILLTYPE_RND:
            Array_fillrange_RND(a, from, to);
            break;
        case ARRAY_FILLTYPE_NONE:
            // just do nothing for scalar types
            Array_fillrange_NONE(a, from, to);
            break;
        case ARRAY_FILLTYPE_ASC_SERIES:
            Array_fillrange_ASC_SERIES(a, from, to);
            break;
        case ARRAY_FILLTYPE_DESC_SERIES:
            Array_fillrange_DESC_SERIES(a, from, to);
            break;
        default:
            userraiseint(ERR_ACTION_NOT_APPLICABLE, "Not supported filltype %s", ArrayFillTypeName(typ));
    }

    return logret(to - from, "Filled %d", to - from);
}

// -------------- ACCESS AND MODIFICATION --------------

Array                           Array_increase(Array arr, int newcnt){
    if (newcnt > Arraysz(arr) )
        increase(&arr, newcnt);
    Array_fillrange(arr, ARRAY_FILLTYPE_ZERO, arr.len, newcnt);
    arr.len = newcnt;
    return arr;
}

Array                           Array_shrink(Array arr, int newsz){
    logenter("newsz %d", newsz);
    if (newsz < 0)
        newsz = 0;
    if (newsz > arr.sz)
        newsz = arr.sz;
    increase(&arr, newsz);
    return logret(arr, "shrinked to (len %d == sz %d)", arr.len, arr.sz);
}

// TODO: probably shoube be reworked to use switch (type) + separate code
void                     Array_shuffle(Array arr){
    srand(time(NULL) );
    for (int i = arr.len - 1; i > 0; i--){
        int j = rndint(i);
        if (Array_isint(arr) )
            int_exch(arr.iv + i, arr.iv + j);
        else if (Array_islong(arr) )
            long_exch(arr.lv + i, arr.lv + j);
        else if (Array_isdouble(arr) )
            dbl_exch(arr.dv + i, arr.dv + j);
        else if (Array_ispointer(arr) )
            ptr_exch(arr.pv + i, arr.pv + j);
        else if (Array_isv64(arr))
            v64_exch(arr.v64 + i, arr.v64 + j);
        else
            logsimple("unsupported type for shuffle %s", ArrayGettypeName(arr) );
    }
}

// TODO: probably shoube be reworked to use switch (type) + separate code
void                     Array_qsort(Array arr, ArrayFillType ord){
    int  sz = 0;
    int (*cmp)(const void *, const void *) = 0;
    if (Array_isint(arr) ){
        sz = sizeof(int);
        if (ord == ARRAY_FILLTYPE_ASC)
            cmp = pint_cmp;
        else
            cmp = pint_revcmp;
    } else if (Array_islong(arr) ){
        sz = sizeof(long);
        if (ord == ARRAY_FILLTYPE_ASC){
            cmp = plong_cmp;
            //logsimple("COMPARATOR: sz %d, ARRAY_FILLTYPE_ASC", sz);
        }
        else
            cmp = plong_revcmp;
    } else if (Array_isdouble(arr) ) {
        sz = sizeof(double);
        if (ord == ARRAY_FILLTYPE_ASC)
            cmp = pdbl_cmp;
        else
            cmp = pdbl_revcmp;
    } else if (Array_ispointer(arr) ) {
        sz = sizeof(void *);
        if (ord == ARRAY_FILLTYPE_ASC)
            cmp = pptr_cmp;
        else
            cmp = pptr_revcmp;
    } else if (Array_isv64(arr) ) {
        // TODO:
        sz = sizeof(value64);
        if (ord == ARRAY_FILLTYPE_ASC)
            cmp = value64_getPComparator(arr.v64type);
        else
            cmp = value64_getPRevComparator(arr.v64type);
    } else {
        logsimple("Array_fill: unsupported type %s", ArrayGettypeName(arr) );
        return;
    }
    if (sz && cmp)
        qsort(arr.v, arr.len, sz, cmp);
}
// if condition is 0-ptr == ALL
int                         Array_foreach_proc(Array arr, Array_cond cond, Array_proc func){
    // TODO: use foreach here
    int     cnt = 0;
    for (int i = 0; i < Arraylen(arr); i++)
        if (cond == 0 || cond(arr, i) ){
            if (func)
                func(arr, i);
            cnt++;
        }
    return logsimpleret(cnt, "processed %d", cnt);
}

// -------------------------- (API) printers -----------------------

int                         Array_fprint(FILE *f, Array val, int limit){

    int         cnt = 0, i;
    int         array_rec_line = 20;    // default
    const char *custom_print_line;    // for int or double

    limit = (limit == 0)? val.len : (limit < val.len) ? limit : val.len;
    if (g_array_rec_line)
        array_rec_line = g_array_rec_line;

    cnt += fprintf(f, "Array (%s[%d of total %d]):\n", ArrayTypeName(val.flags), limit, val.len);
    for (i = 0; i < limit; i++){
        if (Array_isint(val) ){
            if (g_custom_print_line)    // TODO: refactor that!
                custom_print_line = g_custom_print_line;
            else  // standard behavior
                custom_print_line = "[%d - %6d]\t";
            cnt += fprintf(f, custom_print_line, i, val.iv[i]);
        }
        else if (Array_islong(val) ){
            if (g_custom_print_line)    // TODO: refactor that!
                custom_print_line = g_custom_print_line;
            else  // standard behavior
                custom_print_line = "[%ld - %6ld]\t";
            cnt += fprintf(f, custom_print_line, i, val.lv[i]);
        }
        else if (Array_isdouble(val) ){
            if (g_custom_print_line)    // TODO: refactor that!
                custom_print_line = g_custom_print_line;
            else
                custom_print_line = "[%d - %.8lg]\t";
            cnt += fprintf(f, custom_print_line, i, val.dv[i]);
        } else if (Array_ispointer(val) ){
            if (g_custom_print_line)    // TODO: refactor that!
                custom_print_line = g_custom_print_line;
            else
                custom_print_line = "[%p - %p]\t";
            cnt += fprintf(f, custom_print_line, i, val.dv[i]);
        } else if (Array_isv64(val) ) {
            // no custom here!
            value64_techfprint(f, val.v64[i], val.v64type, "");
        }
        // delim
        if ( ( (i + 1) % array_rec_line) == 0){
            cnt += fprintf(f, "\n");
        }
    }
    if (i < val.len)
        cnt += fprintf(f, "and more (%d) ...\n", val.len - i);
    else
        cnt += fprintf(f, "\n");
    return cnt;
}
// save only values by delimeter
long                        Array_savevalues(Array arr, const char *fname, char delim){
    logenter("%s, [%c]", fname, delim);

    long    res = 0;
    FILE   *f = fopen(fname, "w");
    if (f == 0){
        fprintf(stderr, "Unable to open %s for writinf\n", fname);
        return logerr(-1, "Can't open for write");
    }
    for (int i = 0; i < arr.len; i++)
        if (Array_isint(arr))
            res += fprintf(f, "%d%c", arr.iv[i], delim);
        else if (Array_islong(arr))
            res += fprintf(f, "%ld%c", arr.lv[i], delim);
        else if (Array_isdouble(arr))
            res += fprintf(f, "%12.12f%c", arr.dv[i], delim);
        else if (Array_ispointer(arr))
            res += fprintf(f, "%p%c", arr.pv[i], delim);
        else if (Array_isv64(arr))
            res += value64_fsave(f, arr.v64[i], arr.v64type, true);
    fclose(f);
    return logret(res, "Done %ld", res);
}

long                        Array_save(Array arr, const char *fname){
    logenter("%s", fname);

    long         res = 0;
    FILE        *f = fopen(fname, "w");
    if (f == 0){
        fprintf(stderr, "Unable to open %s for writinf\n", fname);
        return logerr(-1, "Can't open for write");
    }
    // g_save_format_double g_save_format_int
    const char  *typ = ArrayTypeName(arr.flags);

    res += fprintf(f, "ARRAY: %s : %d\n", typ, arr.len);
    for (int i = 0; i < arr.len; i++)
        if (Array_isint(arr))
            res += fprintf(f, g_save_format_int, i, arr.iv[i]);    // TODO: think if shrink repeatable
        else if (Array_islong(arr))
            res += fprintf(f, g_save_format_long, i, arr.lv[i]);    // TODO: think if shrink repeatable
        else if (Array_isdouble(arr))
            res += fprintf(f, g_save_format_double, i, arr.dv[i]);
        else if (Array_ispointer(arr))
            res += fprintf(f, g_save_format_pointer, i, arr.pv[i]);
        else if (Array_isv64(arr))
            res += value64_fsave(f, arr.v64[i], arr.v64type, true);
    res += fprintf(f, "ARRAY: DONE\n");
    fclose(f);
    return logret(res, "Done %ld", res);
}

Array                       Array_load(const char *fname){
    logenter("%s", fname);

    int    cnt = 0, tmp;
    FILE *f = fopen(fname, "r");
    Array arr = Array_init();
    if (f == 0){
        fprintf(stderr, "Unable to open %s for read\n", fname);
        Array_seterror(arr);
        return logerr(arr, "Can't read");
    }

    char typ[20];
    fscanf(f, "ARRAY: %s : %d", typ, &cnt);
    fs s = FS();
    if (strcmp(typ, "ARRAY_INT") == 0)
        arr = IArray_create(cnt, ARRAY_FILLTYPE_NONE);
    else if (strcmp(typ, "ARRAY_LONG") == 0)
        arr = LArray_create(cnt, ARRAY_FILLTYPE_NONE);
    else if (strcmp(typ, "ARRAY_DOUBLE") == 0)
        arr = DArray_create(cnt, ARRAY_FILLTYPE_NONE);
    else if (strcmp(typ, "ARRAY_POINTER") == 0)
        arr = PArray_create(cnt, ARRAY_FILLTYPE_NONE);
    else if (strcmp(typ, "ARRAY_V64") == 0)
        // TODO: rework probably VALUE64_UNKNOWN
        arr = V64Array_create(cnt, ARRAY_FILLTYPE_NONE, VALUE64_UNKNOWN);
    else {
        fprintf(stderr, "Unsupported format %s\n", typ);
        return logactret(fclose(f), arr, "failed, wrong format %s...", typ);
    }
    for (int i = 0; i < cnt; i++){
        if (Array_isint(arr))
            fscanf(f, g_save_format_int, &tmp, arr.iv + i); // tmp isn't used for now
        else if (Array_islong(arr))
            fscanf(f, g_save_format_long, &tmp, arr.lv + i);
        else if (Array_isdouble(arr) )
            fscanf(f, "%d %lg\n", &tmp, arr.dv + i);
        else if (Array_ispointer(arr) )
            fscanf(f, "%d %p\n", &tmp, arr.pv + i);
        else if (Array_isv64(arr) )
            value64_fload(f, &arr.v64[i], arr.v64type, true, &s);
    }
    fsfree(s);
    // TODO: probably checking for ARRAY: DONE must be here
    fclose(f);
    return logret(arr, "Done %d", cnt);
}
// -------------------------------Testing --------------------------

#ifdef ARRAYTESTING

//#include <signal.h>
#include <float.h>
#include <math.h>
#include "test.h"
#include "checker.h"

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d: double", ++subnum);
        Array darr = DArray_create(100, ARRAY_FILLTYPE_ZERO);
        for (int i = 0; i < darr.len; i++)
            if (darr.dv[i] != 0.0)
                return logret(TEST_FAILED, "%d: Element must be 0.0. but not %lf", i, darr.dv[i]);
        if (!Array_isvalid(darr))
            return logret(TEST_FAILED, "Validation is failed");
        Arrayfree(darr);
        if (darr.dv != 0)
            return logret(TEST_FAILED, "Array is'nt freed");
    }
    {
        test_sub("subtest %d: int", ++subnum);
        Array iarr = IArray_create(100, ARRAY_FILLTYPE_ZERO);
        for (int i = 0; i < iarr.len; i++)
            if (iarr.iv[i] != 0)
                return logret(TEST_FAILED, "%d: Element must be 0 but not %d", i, iarr.iv[i]);
        if (!Array_isvalid(iarr))
            return logret(TEST_FAILED, "Validation is failed");
        Arrayfree(iarr);
        if (iarr.dv != 0)
            return logret(TEST_FAILED, "Array is'nt freed");
    }
    {
        test_sub("subtest %d: long", ++subnum);
        Array larr = LArray_create(100, ARRAY_FILLTYPE_ZERO);
        for (int i = 0; i < larr.len; i++)
            if (larr.lv[i] != 0)
                return logret(TEST_FAILED, "%d: Element must be 0L but not %ld", i, larr.lv[i]);
        if (!Array_isvalid(larr))
            return logret(TEST_FAILED, "Validation is failed");
        Arrayfree(larr);
        if (larr.dv != 0)
            return logret(TEST_FAILED, "Array is'nt freed");
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 2 ---------------------------------
static TestStatus
tf2(const char *name){

    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: double asc", ++subnum);
    {
        Array darr = DArray_create(100, ARRAY_FILLTYPE_ASC);
        for (int i = 0; i < darr.len - 1; i++)
            if (darr.dv[i] > darr.dv[i + 1])
                return logactret(Arrayfree(darr), TEST_FAILED, "Violation for ACS gen: arr[%d] = %f > arr[%d+1] = %f", i, darr.dv[i], i, darr.dv[i + 1]);

        test_sub("subtest %d: double desc", ++subnum);
        Array_fill(darr, ARRAY_FILLTYPE_DESC);
        for (int i = 0; i < darr.len - 1; i++)
            if (darr.dv[i] < darr.dv[i + 1])
                return logactret(Arrayfree(darr), TEST_FAILED, "Violation for DESC gen: arr[%d] = %f < arr[%d+1] = %f", i, darr.dv[i], i, darr.dv[i + 1]);
        Arrayfree(darr);
    }
    test_sub("subtest %d: int asc", ++subnum);
    {
        Array iarr = IArray_create(100, ARRAY_FILLTYPE_ASC);
        for (int i = 0; i < iarr.len - 1; i++)
            if (iarr.iv[i] > iarr.iv[i + 1])
                return logactret(Arrayfree(iarr), TEST_FAILED, "Violation for ACS gen: arr[%d] = %d > arr[%d+1] = %d", i, iarr.iv[i], i, iarr.iv[i + 1]);

        test_sub("subtest %d: int desc", ++subnum);
        Array_fill(iarr, ARRAY_FILLTYPE_DESC);
        for (int i = 0; i < iarr.len - 1; i++)
            if (iarr.iv[i] < iarr.iv[i + 1])
                return logactret(Arrayfree(iarr), TEST_FAILED, "Violation for DESC gen: arr[%d] = %d < arr[%d+1] = %d", i, iarr.iv[i], i, iarr.iv[i + 1]);
        Arrayfree(iarr);
    }
    test_sub("subtest %d: long asc", ++subnum);
    {
        Array larr = LArray_create(100, ARRAY_FILLTYPE_ASC);
        for (int i = 0; i < larr.len - 1; i++)
            if (larr.lv[i] > larr.lv[i + 1])
                return logactret(Arrayfree(larr), TEST_FAILED, "Violation for ACS gen: arr[%d] = %ld > arr[%d+1] = %ld", i, larr.lv[i], i, larr.lv[i + 1]);

        test_sub("subtest %d: long desc", ++subnum);
        Array_fill(larr, ARRAY_FILLTYPE_DESC);
        for (int i = 0; i < larr.len - 1; i++)
            if (larr.lv[i] < larr.lv[i + 1])
                return logactret(Arrayfree(larr), TEST_FAILED, "Violation for DESC gen: arr[%d] = %ld < arr[%d+1] = %ld", i, larr.lv[i], i, larr.lv[i + 1]);
        Arrayfree(larr);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 3 ---------------------------------
static TestStatus
tf3(const char *name){

    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: double", ++subnum);
    {
        Array darr = DArray_create(100, ARRAY_FILLTYPE_ASC);

        Array_fprint(logfile, darr, 0);

        darr = Array_shrink(darr, 10);
        if (!Array_isvalid(darr))
            return logactret(Arrayfree(darr), TEST_FAILED, "Validation is failed");

        if (! inv(darr.len == 10 && darr.sz == 10 && darr.iv != 0, "Broken array!") )
            return logactret(Arrayfree(darr), TEST_FAILED, "Validatation is failed, len %d - sz %d - v %p", darr.len, darr.sz, darr.dv);
        Arrayfree(darr);
    }
    test_sub("subtest %d: int", ++subnum);
    {
        Array iarr = IArray_create(100, ARRAY_FILLTYPE_ASC);

        Array_fprint(logfile, iarr, 0);

        iarr = Array_shrink(iarr, 10);
        if (!Array_isvalid(iarr))
            return logactret(Arrayfree(iarr), TEST_FAILED, "Validation is failed");

        if (! inv(iarr.len == 10 && iarr.sz == 10 && iarr.iv != 0, "Broken array!") )
            return logactret(Arrayfree(iarr), TEST_FAILED, "Validatation is failed, len %d - sz %d - v %p", iarr.len, iarr.sz, iarr.iv);
        Arrayfree(iarr);
    }
    test_sub("subtest %d: long", ++subnum);
    {
        Array larr = LArray_create(100, ARRAY_FILLTYPE_ASC);

        Array_fprint(logfile, larr, 0);

        larr = Array_shrink(larr, 10);
        if (!Array_isvalid(larr))
            return logactret(Arrayfree(larr), TEST_FAILED, "Validation is failed");

        if (! inv(larr.len == 10 && larr.sz == 10 && larr.iv != 0, "Broken array!") )
            return logactret(Arrayfree(larr), TEST_FAILED, "Validatation is failed, len %d - sz %d - v %p", larr.len, larr.sz, larr.dv);
        Arrayfree(larr);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 4 ---------------------------------
static TestStatus
tf4(const char *name){

    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: int save/load", ++subnum);
    {
        Array iarr = IArray_create(100, ARRAY_FILLTYPE_RND);
        const char *filename =  "res/array/iarr.sv";

        Array_save(iarr, filename);

        Array iarr2 = Array_load(filename);

        if (!Array_isvalid(iarr2))
            return logactret(Arrayfree(iarr), TEST_FAILED, "Validation is failed");

        test_sub("subtest %d: check", ++subnum);
        if (!inv(iarr.len == iarr2.len && iarr.flags == iarr2.flags, "not equal") )
            return logactret( (Array_free(&iarr), Array_free(&iarr2) ), TEST_FAILED, "Not equal len %d - %d, flags %d - %d",
                iarr.len, iarr2.len, iarr.flags, iarr2.flags);

        test_sub("subtest %d: check2", ++subnum);
        for (int i = 0; i < iarr.len; i++)
            if (iarr.iv[i] != iarr2.iv[i])
                return logacterr( (Array_free(&iarr), Array_free(&iarr2) ), TEST_FAILED,
                                "arr[%d] = %d != arr2[%d] = %d", i, iarr.iv[i], i, iarr2.iv[i]);

        Array_free(&iarr);
        Array_free(&iarr2);
    }
    test_sub("subtest %d: long save/load", ++subnum);
    {
        Array larr = LArray_create(100, ARRAY_FILLTYPE_RND);
        const char *filename =  "res/array/larr.sv";

        Array_save(larr, filename);

        Array larr2 = Array_load(filename);

        if (!Array_isvalid(larr2))
            return logactret(Arrayfree(larr), TEST_FAILED, "Validation is failed");

        test_sub("subtest %d: check", ++subnum);
        if (!inv(larr.len == larr2.len && larr.flags == larr2.flags, "not equal") )
            return logactret( (Array_free(&larr), Array_free(&larr2) ), TEST_FAILED, "Not equal len %d - %d, flags %d - %d",
                larr.len, larr2.len, larr.flags, larr2.flags);

        test_sub("subtest %d: check2", ++subnum);
        for (int i = 0; i < larr.len; i++)
            if (larr.lv[i] != larr2.lv[i])
                return logacterr( (Array_free(&larr), Array_free(&larr2) ), TEST_FAILED,
                                "arr[%d] = %ld != arr2[%d] = %ld", i, larr.lv[i], i, larr2.lv[i]);

        Array_free(&larr);
        Array_free(&larr2);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 5 ---------------------------------
static TestStatus
tf5(const char *name){

    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d", ++subnum);
    {
        Array darr = DArray_create(100, ARRAY_FILLTYPE_RND);
        const char *filename =  "res/array/darr.sv";

        Array_print(darr, 0);
        Array_save(darr, filename);

        Array darr2 = Array_load(filename);

        if (!Array_isvalid(darr2))
            return logactret(Arrayfree(darr), TEST_FAILED, "Validation is failed");

        test_sub("subtest %d", ++subnum);
        if (!inv(darr.len == darr2.len && darr.flags == darr2.flags, "not equal") )
            return logactret( (Array_free(&darr), Array_free(&darr2) ), TEST_FAILED, "Not equal len %d - %d, flags %d - %d",
                darr.len, darr2.len, darr.flags, darr2.flags);

        test_sub("subtest %d", ++subnum);
        for (int i = 0; i < darr.len; i++)
            if (fabs(darr.dv[i] - darr2.dv[i]) > FLT_EPSILON / 100)
                return logacterr( (Array_free(&darr), Array_free(&darr2) ), TEST_FAILED,
                                "arr[%d] = %15.15lf != arr2[%d] = %15.15lf", i, darr.dv[i], i, darr2.dv[i]);

        Array_free(&darr);
        Array_free(&darr2);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 6 ---------------------------------
static TestStatus
tf6(const char *name){
    logenter("%s", name);

    int         subnum = 0;

    test_sub("subtest %d: double", ++subnum);
    {
        Array darr = DArray_create(50, ARRAY_FILLTYPE_ASC);

        Array_shuffle(darr);
        g_custom_print_line = 0;
        Array_print(darr, 0);
        Arrayfree(darr);
    }
    test_sub("subtest %d: int", ++subnum);
    {
        Array iarr = IArray_create(50, ARRAY_FILLTYPE_ASC);
        Array_shuffle(iarr);
        Array_print(iarr, 0);
        Arrayfree(iarr);
    }
    test_sub("subtest %d: long", ++subnum);
    {
        Array larr = LArray_create(50, ARRAY_FILLTYPE_ASC);
        Array_shuffle(larr);
        Array_print(larr, 0);
        Arrayfree(larr);
    }
    return logret(TEST_MANUAL, "done"); // TEST_FAILED
}

// ------------------------- TEST 7 ---------------------------------
static TestStatus
tf7(const char *name){

    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: double acs", ++subnum);
    {
        Array darr = DArray_create(10000, ARRAY_FILLTYPE_RND);

        Array_qsort(darr, ARRAY_FILLTYPE_ASC);
        //Array_print(darr, 50);
        // check asc
        for (int i = 1; i < darr.len; i++)
            if (darr.dv[i - 1] > darr.dv[i])
                return logactret(Arrayfree(darr), TEST_FAILED, "array[%d] = %f > array[%d] = %f, should be <=", i - 1, darr.dv[i - 1], i, darr.dv[i]);

        test_sub("subtest %d: double desc", ++subnum);
        Array_qsort(darr, ARRAY_FILLTYPE_DESC);
        for (int i = 1; i < darr.len; i++)
            if (darr.dv[i - 1] < darr.dv[i])
                return logactret(Arrayfree(darr), TEST_FAILED, "array[%d] = %f < array[%d] = %f, should be >=", i - 1, darr.dv[i - 1], i, darr.dv[i]);
        Arrayfree(darr);
    }
    test_sub("subtest %d: int acs", ++subnum);
    {

        Array iarr = IArray_create(100000, ARRAY_FILLTYPE_RND);
        Array_qsort(iarr, ARRAY_FILLTYPE_ASC);
        // check asc
        for (int i = 1; i < iarr.len; i++)
            if (iarr.iv[i - 1] > iarr.iv[i])
                return logactret(Arrayfree(iarr), TEST_FAILED, "array[%d] = %d > array[%d] = %d, should be <=", i - 1, iarr.iv[i - 1], i, iarr.iv[i]);

        test_sub("subtest %d: int desc", ++subnum);
        Array_qsort(iarr, ARRAY_FILLTYPE_DESC);
        // check desc
        for (int i = 1; i < iarr.len; i++)
            if (iarr.iv[i - 1] < iarr.iv[i])
                return logactret(Arrayfree(iarr), TEST_FAILED, "array[%d] = %d < array[%d] = %d, should be >=", i - 1, iarr.iv[i - 1], i, iarr.iv[i]);
        //Array_print(iarr, 50);
        Arrayfree(iarr);
    }
    test_sub("subtest %d: long asc", ++subnum);
    {

        Array larr = LArray_create(100000, ARRAY_FILLTYPE_RND);
        Array_qsort(larr, ARRAY_FILLTYPE_ASC);
        Array_print(larr, 50);
        // check asc
        for (int i = 1; i < larr.len; i++)
            if (larr.lv[i - 1] > larr.lv[i])
                return logactret(Arrayfree(larr), TEST_FAILED, "array[%d] = %ld > array[%d] = %ld, should be <=", i - 1, larr.lv[i - 1], i, larr.lv[i]);

        test_sub("subtest %d: long desc", ++subnum);
        Array_qsort(larr, ARRAY_FILLTYPE_DESC);
        // check desc
        for (int i = 1; i < larr.len; i++)
            if (larr.lv[i - 1] < larr.lv[i])
                return logactret(Arrayfree(larr), TEST_FAILED, "array[%d] = %ld < array[%d] = %ld, should be >=", i - 1, larr.lv[i - 1], i, larr.lv[i]);
        //Array_print(iarr, 50);
        Arrayfree(larr);
    }

    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 8 ---------------------------------
static TestStatus
tf8(const char *name){
    logenter("%s", name);

    int         subnum = 0;
    {
        test_sub("subtest %d increase int array", ++subnum);

        int initsz = 25;
        Array arr = IArray_create(initsz, ARRAY_FILLTYPE_RND);

        arr = Array_increase(arr, initsz * 3);

        test_validatefree(
            Arraylen(arr) == initsz * 3, 
            Arrayfree(arr), 
            "Array length %d must be %d", Arraylen(arr), 
            initsz * 3
        );
        for (int i = initsz; i < Arraylen(arr); i++){
            test_validatefree(
                arr.iv[i] == 0,
                Arrayfree(arr),
                "arr[%d] must be zero,  but not %d", i, arr.iv[i]
            );
        }

        Arrayfree(arr);
    }
    test_sub("subtest %d increas double array", ++subnum);
    {

        int initsz = 25;
        Array arr = DArray_create(initsz, ARRAY_FILLTYPE_RND);

        arr = Array_increase(arr, initsz * 3);

        test_validatefree(Arraylen(arr) == initsz * 3, Arrayfree(arr), "Array length %d must be %d", Arraylen(arr), initsz * 3);
        for (int i = initsz; i < Arraylen(arr); i++){
            test_validatefree(arr.dv[i] == 0.0, Arrayfree(arr), "arr[%d] must be zero,  but not %lf", i, arr.dv[i]);
        }

        Arrayfree(arr);
    }
    test_sub("subtest %d increas long array", ++subnum);
    {

        int initsz = 25;
        Array arr = LArray_create(initsz, ARRAY_FILLTYPE_RND);

        arr = Array_increase(arr, initsz * 5);

        test_validatefree(Arraylen(arr) == initsz * 5, Arrayfree(arr), "Array length %d must be %d", Arraylen(arr), initsz * 5);
        for (int i = initsz; i < Arraylen(arr); i++){
            test_validatefree(arr.lv[i] == 0L, Arrayfree(arr), "arr[%d] must be zero,  but not %ld", i, arr.lv[i]);
        }

        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 9 ---------------------------------
static TestStatus
tf9(const char *name){
    logenter("%s", name);

    int         subnum = 0;
    test_sub("subtest %d creating pointer array", ++subnum);
    {
        int     cnt = 100;
        Array   parr = PArray_create(cnt, ARRAY_FILLTYPE_ZERO);

        for (int i = 0; i < parr.len; i++)
            test_validatefree(parr.pv[i] == 0x0, Arrayfree(parr),
                "Element %d must be 0x0 but not %p", i, parr.pv[i]);
        test_validatefree(Array_isvalid(parr), Arrayfree(parr),
                "Validation is failed");

        Arrayfree(parr);
    }
    test_sub("subtest %d shrinking", ++subnum);
    {

        Array   parr = PArray_create(100, ARRAY_FILLTYPE_ZERO);

        int     cnt = 10;
        parr = Array_shrink(parr, cnt);
        test_validatefree(Array_isvalid(parr), Arrayfree(parr), "Validation is failed");

        test_validatefree(parr.len == cnt && parr.sz == cnt && parr.iv != 0, Arrayfree(parr),
                 "Validatation is failed, len %d - sz %d - v %p", parr.len, parr.sz, parr.pv);
        Arrayfree(parr);
    }
    test_sub("subtest %d, pointer array save/load", ++subnum);
    {
        const char *filename = "res/array/parr.sv";
        Array parr = PArray_create(100, ARRAY_FILLTYPE_ZERO);

        Array_save(parr, filename);

        Array parr2 = Array_load(filename);

        test_validatefree(Array_isvalid(parr2), (Arrayfree(parr), Arrayfree(parr2) ), "Validation is failed");

        test_validatefree(parr.len == parr2.len && parr.flags == parr2.flags,  (Arrayfree(parr), Arrayfree(parr2) ),
                "Not equal len %d - %d, flags %d - %d", parr.len, parr2.len, parr.flags, parr2.flags);

        for (int i = 0; i < parr.len; i++)
            test_validatefree(parr.pv[i] == parr2.pv[i], (Arrayfree(parr), Arrayfree(parr2) ),
                 "arr[%d] = %p != arr2[%d] = %p", i, parr.pv[i], i, parr2.pv[i]);

        Arrayfree(parr);
        Arrayfree(parr2);
    }
    test_sub("subtest %d, pointer array sorting", ++subnum);
    {

        Array parr = PArray_create(10000, ARRAY_FILLTYPE_ZERO);

        // fiil array manually
        for (int i = 0; i < parr.len; i++)
            parr.pv[i] = (void **) rndulong(parr.len * 10000);

        Array_qsort(parr, ARRAY_FILLTYPE_ASC);
        //Array_print(darr, 50);
        // check asc
        for (int i = 1; i < parr.len; i++)
            test_validatefree(parr.pv[i - 1] <= parr.pv[i], Arrayfree(parr),
                            "array[%d] = %p > array[%d] = %p, should be <=", i - 1, parr.pv[i - 1], i, parr.pv[i]);
        // resort descending
        Array_qsort(parr, ARRAY_FILLTYPE_DESC);
        for (int i = 1; i < parr.len; i++)
            test_validatefree(parr.pv[i - 1] >= parr.pv[i], Arrayfree(parr),
                            "array[%d] = %p < array[%d] = %p, should be >=", i - 1, parr.pv[i - 1], i, parr.pv[i]);
        Arrayfree(parr);
    }
    test_sub("subtest %d increase pointer array", ++subnum);
    {
        int initsz = 25;
        Array arr = PArray_create(initsz, ARRAY_FILLTYPE_ZERO);

        arr = Array_increase(arr, initsz * 3);

        test_validatefree(Arraylen(arr) == initsz * 3, Arrayfree(arr), "Array length %d must be %d", Arraylen(arr), initsz * 3);

        for (int i = initsz; i < Arraylen(arr); i++){
            test_validatefree(arr.pv[i] == 0x0, Arrayfree(arr), "arr[%d] must be null, but not %p", i, arr.pv[i]);
        }

        Arrayfree(arr);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 10 ---------------------------------
static TestStatus
tf10(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Int ascending series */
    test_sub("subtest %d: int asc series", ++subnum);
    {
        int     cnt = 100;
        Array   arr = IArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr.iv[i] == i,
                Arrayfree(arr),
                "Int asc series: arr[%d] = %d, expected %d", i, arr.iv[i], i
            );
        }
    }

    /* 2. Int descending series */
    test_sub("subtest %d: int desc series", ++subnum);
    {
        int     cnt = 50;
        Array   arr = IArray_create(cnt, ARRAY_FILLTYPE_DESC_SERIES);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            int expected = cnt - 1 - i;
            test_validatefree(
                arr.iv[i] == expected,
                Arrayfree(arr),
                "Int desc series: arr[%d] = %d, expected %d", i, arr.iv[i], expected
            );
        }
    }

    /* 3. Long ascending series */
    test_sub("subtest %d: long asc series", ++subnum);
    {
        int     cnt = 70;
        Array   arr = LArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr.lv[i] == (long)i,
                Arrayfree(arr),
                "Long asc series: arr[%d] = %ld, expected %ld", i, arr.lv[i], (long)i
            );
        }
    }

    /* 4. Long descending series */
    test_sub("subtest %d: long desc series", ++subnum);
    {
        int     cnt = 40;
        Array   arr = LArray_create(cnt, ARRAY_FILLTYPE_DESC_SERIES);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            long expected = (long)(cnt - 1 - i);
            test_validatefree(
                arr.lv[i] == expected,
                Arrayfree(arr),
                "Long desc series: arr[%d] = %ld, expected %ld", i, arr.lv[i], expected
            );
        }
    }

    /* 5. Double ascending series */
    test_sub("subtest %d: double asc series", ++subnum);
    {
        int     cnt = 30;
        Array   arr = DArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr.dv[i] == (double)i,
                Arrayfree(arr),
                "Double asc series: arr[%d] = %f, expected %f", i, arr.dv[i], (double)i
            );
        }
    }

    /* 6. Double descending series */
    test_sub("subtest %d: double desc series", ++subnum);
    {
        int     cnt = 25;
        Array   arr = DArray_create(cnt, ARRAY_FILLTYPE_DESC_SERIES);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            double expected = (double)(cnt - 1 - i);
            test_validatefree(
                arr.dv[i] == expected,
                Arrayfree(arr),
                "Double desc series: arr[%d] = %f, expected %f", i, arr.dv[i], expected
            );
        }
    }

    /* 7. Empty array */
    test_sub("subtest %d: empty series", ++subnum);
    {
        Array   arr = IArray_create(0, ARRAY_FILLTYPE_ASC_SERIES);
        int     len = Arraylen(arr);
        test_validatefree(
            len == 0,
            Arrayfree(arr),
            "Empty array length = %d, expected 0", len
        );
    }

    /* 8. Неподдерживаемый тип (указатели) – должен вернуть ошибку, но не упасть */
    test_sub("subtest %d: pointer series (unsupported)", ++subnum);
    {
        if (!try()) {
            Array arr = PArray_create(10, ARRAY_FILLTYPE_ASC_SERIES);
            // Если мы здесь, значит, программа не упала, хотя должна была вызвать invraise
            test_validate(
                false,
                "Pointer series should have raised an error but didn't"
            );
            // just to avoid warning unused variable 'arr' [-Wunused-variable], that NEVER be used
            Array_print(arr, 0);
        } else {
            // Сигнал перехвачен – это ожидаемое поведение
            // Ничего не освобождаем, так как массив, скорее всего, не создался
            test_validate(
                true,
                "Pointer series correctly raised error"
            );
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

    /* 1. Заполнение середины int массива возрастающей серией */
    test_sub("subtest %d: fill middle with asc series", ++subnum);
    {
        int     cnt = 50;
        Array   arr = IArray_create(cnt, ARRAY_FILLTYPE_ZERO);
        int     from = 10, to = 20;

        Array_fillrange(arr, ARRAY_FILLTYPE_ASC_SERIES, from, to);

        // Проверка общей длины
        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        // Элементы до from и после to должны остаться нулями
        for (int i = 0; i < cnt; i++) {
            if (i >= from && i < to)
                continue;
            test_validatefree(
                arr.iv[i] == 0,
                Arrayfree(arr),
                "Element [%d] = %d, expected 0 (outside range)", i, arr.iv[i]
            );
        }

        // Внутри диапазона значения равны индексу
        for (int i = from; i < to; i++) {
            test_validatefree(
                arr.iv[i] == i,
                Arrayfree(arr),
                "Element [%d] = %d, expected %d (inside range)", i, arr.iv[i], i
            );
        }
    }

    /* 2. Заполнение всего long массива убывающей серией */
    test_sub("subtest %d: full fill with desc series (long)", ++subnum);
    {
        int     cnt = 30;
        Array   arr = LArray_create(cnt, ARRAY_FILLTYPE_NONE);
        Array_fillrange(arr, ARRAY_FILLTYPE_DESC_SERIES, 0, cnt);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            long expected = (long)(cnt - 1 - i);
            test_validatefree(
                arr.lv[i] == expected,
                Arrayfree(arr),
                "Element [%d] = %ld, expected %ld", i, arr.lv[i], expected
            );
        }
    }

    /* 3. Пустой диапазон (from == to) – массив не меняется */
    test_sub("subtest %d: from == to leaves array unchanged", ++subnum);
    {
        int     cnt = 20;
        Array   arr = IArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);  // [0..19]
        Array_fillrange(arr, ARRAY_FILLTYPE_RND, 5, 5);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr.iv[i] == i,
                Arrayfree(arr),
                "Element [%d] = %d, expected %d (unchanged after empty fill)", i, arr.iv[i], i
            );
        }
    }

    /* 4. Выход за границы – программа не должна упасть */
    test_sub("subtest %d: out-of-bounds does not crash", ++subnum);
    {
        int     cnt = 10;
        Array   arr = IArray_create(cnt, ARRAY_FILLTYPE_ZERO);
        Array_fillrange(arr, ARRAY_FILLTYPE_ASC_SERIES, -5, cnt + 5);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "After out-of-bounds fill, length = %d, expected %d", len, cnt
        );
        // Дополнительно можно не проверять содержимое, так как поведение не определено
    }

    /* 5. Double массив, заполнение возрастающей серией в поддиапазоне */
    test_sub("subtest %d: double asc series fill range", ++subnum);
    {
        int     cnt = 25, from = 5, to = 15;
        Array   arr = DArray_create(cnt, ARRAY_FILLTYPE_ZERO);
        Array_fillrange(arr, ARRAY_FILLTYPE_ASC_SERIES, from, to);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        // Элементы вне диапазона остались нулями
        for (int i = 0; i < cnt; i++) {
            if (i >= from && i < to) continue;
            test_validatefree(
                arr.dv[i] == 0.0,
                Arrayfree(arr),
                "Element [%d] = %f, expected 0.0 (outside range)", i, arr.dv[i]
            );
        }

        // Внутри диапазона значения равны индексу
        for (int i = from; i < to; i++) {
            test_validatefree(
                arr.dv[i] == (double)i,
                Arrayfree(arr),
                "Element [%d] = %f, expected %f", i, arr.dv[i], (double)i
            );
        }
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 12 ---------------------------------

static TestStatus
tf12(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. int array */
    test_sub("subtest %d: int array (even half, odd zero)", ++subnum);
    {
        int     cnt = 10;
        Array   arr = IArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);   // 0..9

        IArray_foreach(arr, elem) {
            if (*elem % 2 == 0)
                *elem /= 2;
            else
                *elem = 0;
        }

        test_validatefree(Arraylen(arr) == cnt, Arrayfree(arr), "Length mismatch");

        int expected[] = {0, 0, 1, 0, 2, 0, 3, 0, 4, 0};
        for (int i = 0; i < cnt; i++)
            test_validatefree(arr.iv[i] == expected[i], Arrayfree(arr),
                "int[%d]=%d expected %d", i, arr.iv[i], expected[i]);

        Arrayfree(arr);
    }

    /* 2. long array */
    test_sub("subtest %d: long array", ++subnum);
    {
        int     cnt = 8;
        Array   arr = LArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);

        LArray_foreach(arr, elem) {
            if (*elem % 2 == 0)
                *elem /= 2;
            else
                *elem = 0;
        }

        test_validatefree(Arraylen(arr) == cnt, Arrayfree(arr), "Length mismatch");

        long expected[] = {0L, 0L, 1L, 0L, 2L, 0L, 3L, 0L};
        for (int i = 0; i < cnt; i++)
            test_validatefree(arr.lv[i] == expected[i], Arrayfree(arr),
                "long[%d]=%ld expected %ld", i, arr.lv[i], expected[i]);

        Arrayfree(arr);
    }

    /* 3. double array */
    test_sub("subtest %d: double array", ++subnum);
    {
        int     cnt = 6;
        Array   arr = DArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);

        DArray_foreach(arr, elem) {
            if (fmod(*elem, 2.0) == 0.0)
                *elem /= 2.0;
            else
                *elem = 0.0;
        }

        test_validatefree(Arraylen(arr) == cnt, Arrayfree(arr), "Length mismatch");

        double expected[] = {0.0, 0.0, 1.0, 0.0, 2.0, 0.0};
        for (int i = 0; i < cnt; i++)
            test_validatefree(arr.dv[i] == expected[i], Arrayfree(arr),
                "double[%d]=%f expected %f", i, arr.dv[i], expected[i]);

        Arrayfree(arr);
    }

    /* 4. pointer array (no‑op) */
    test_sub("subtest %d: pointer array (no‑op)", ++subnum);
    {
        int     cnt = 3;
        Array   arr = PArray_create(cnt, ARRAY_FILLTYPE_NONE);
        arr.pv[0] = (void*)1; arr.pv[1] = (void*)2; arr.pv[2] = (void*)3;

        PArray_foreach(arr, elem) {
            // ничего не делаем
        }

        test_validatefree(Arraylen(arr) == cnt, Arrayfree(arr), "Length changed");
        test_validatefree(arr.pv[0] == (void*)1, Arrayfree(arr), "ptr[0] mismatch");
        test_validatefree(arr.pv[1] == (void*)2, Arrayfree(arr), "ptr[1] mismatch");
        test_validatefree(arr.pv[2] == (void*)3, Arrayfree(arr), "ptr[2] mismatch");

        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 13 ---------------------------------

static bool keep_if_index_not_multiple_of_3(Array arr, int pos) {
    (void)arr;
    return (pos % 3) != 0;
}

static void square_int(Array arr, int pos) {
    int val = arr.iv[pos];
    arr.iv[pos] = val * val;
}
static void square_long(Array arr, int pos) {
    long val = arr.lv[pos];
    arr.lv[pos] = val * val;
}
static void mul_one_point_five_double(Array arr, int pos) {
    arr.dv[pos] *= 1.5;
}

static TestStatus
tf13(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. int array: возводим в квадрат, если индекс не кратен 3 */
    test_sub("subtest %d: int array (square non‑multiples of 3)", ++subnum);
    {
        int     cnt = 10;
        Array   arr = IArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);   // 0,1,2,3,4,5,6,7,8,9

        Array_foreach_proc(arr, keep_if_index_not_multiple_of_3, square_int);

        test_validatefree(
            Arraylen(arr) == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", Arraylen(arr), cnt
        );

        int expected[] = {0, 1, 4, 3, 16, 25, 6, 49, 64, 9};
        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr.iv[i] == expected[i],
                Arrayfree(arr),
                "int proc: arr[%d] = %d, expected %d", i, arr.iv[i], expected[i]
            );
        }

        Arrayfree(arr);
    }

    /* 2. long array */
    test_sub("subtest %d: long array", ++subnum);
    {
        int     cnt = 8;
        Array   arr = LArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);   // 0L..7L

        Array_foreach_proc(arr, keep_if_index_not_multiple_of_3, square_long);

        test_validatefree(
            Arraylen(arr) == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", Arraylen(arr), cnt
        );

        long expected[] = {0L, 1L, 4L, 3L, 16L, 25L, 6L, 49L};
        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr.lv[i] == expected[i],
                Arrayfree(arr),
                "long proc: arr[%d] = %ld, expected %ld", i, arr.lv[i], expected[i]
            );
        }

        Arrayfree(arr);
    }

    /* 3. double array: умножаем на 1.5, если индекс не кратен 3 */
    test_sub("subtest %d: double array", ++subnum);
    {
        int     cnt = 6;
        Array   arr = DArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);   // 0.0..5.0

        Array_foreach_proc(arr, keep_if_index_not_multiple_of_3, mul_one_point_five_double);

        test_validatefree(
            Arraylen(arr) == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", Arraylen(arr), cnt
        );

        double expected[] = {0.0, 1.5, 3.0, 3.0, 6.0, 7.5};
        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr.dv[i] == expected[i],
                Arrayfree(arr),
                "double proc: arr[%d] = %f, expected %f", i, arr.dv[i], expected[i]
            );
        }

        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST V64Array (STR / FS) -------------------------
static TestStatus
tf_v64array_str_fs(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ---------- VALUE64_STR ---------- */
    test_sub("subtest %d: create empty STR array", ++subnum);
    {
        Array arr = V64Array_create(0, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        test_validatefree(
            arr.len == 0 && arr.sz == 0 && arr.v64 == NULL,
            Arrayfree(arr),
            "Empty STR array: len=%d, sz=%d, v64=%p (expected 0,0,NULL)",
            arr.len, arr.sz, (void*)arr.v64
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: create ZERO‑filled STR array", ++subnum);
    {
        Array arr = V64Array_create(5, ARRAY_FILLTYPE_ZERO, VALUE64_STR);
        test_validatefree(
            arr.len == 5 && arr.sz >= 5,
            Arrayfree(arr),
            "ZERO STR array: len=%d, sz=%d (expected 5, >=5)", arr.len, arr.sz
        );
        // проверяем, что каждый элемент – пустая строка
        for (int i = 0; i < 5; i++) {
            test_validatefree(
                value64_str(arr.v64[i]) != NULL &&
                strcmp(value64_str(arr.v64[i]), "") == 0,
                Arrayfree(arr),
                "STR[%d] must be empty string, got '%s'",
                i, value64_str(arr.v64[i])
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);   // STR не связан с FS, но для порядка

    test_sub("subtest %d: create NONE fs array", ++subnum);
    {   
        Array arrtmp = V64Array_create(2, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        Arrayfree(arrtmp);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: create ASC‑filled STR array", ++subnum);
    {
        Array arr = V64Array_create(4, ARRAY_FILLTYPE_ASC, VALUE64_STR);
        test_validatefree(
            arr.len == 4,
            Arrayfree(arr),
            "ASC STR array: len=%d (expected 4)", arr.len
        );
        // строки должны быть непустыми и увеличиваться по длине
        for (int i = 1; i < 4; i++) {
            test_validatefree(
                strlen(value64_str(arr.v64[i])) >= strlen(value64_str(arr.v64[i-1])),
                Arrayfree(arr),
                "ASC STR: length must be non‑decreasing"
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: create DESC‑filled STR array", ++subnum);
    {
        Array arr = V64Array_create(4, ARRAY_FILLTYPE_DESC, VALUE64_STR);
        test_validatefree(
            arr.len == 4,
            Arrayfree(arr),
            "DESC STR array: len=%d (expected 4)", arr.len
        );
        // строки должны уменьшаться по длине (или быть пустыми к концу)
        for (int i = 1; i < 4; i++) {
            test_validatefree(
                strlen(value64_str(arr.v64[i])) <= strlen(value64_str(arr.v64[i-1])),
                Arrayfree(arr),
                "DESC STR: length must be non‑increasing"
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ---------- VALUE64_FS ---------- */
    test_sub("subtest %d: create empty FS array", ++subnum);
    {
        Array arr = V64Array_create(0, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        test_validatefree(
            arr.len == 0 && arr.sz == 0 && arr.v64 == NULL,
            Arrayfree(arr),
            "Empty FS array: len=%d, sz=%d, v64=%p (expected 0,0,NULL)",
            arr.len, arr.sz, (void*)arr.v64
        );
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: create ZERO‑filled FS array", ++subnum);
    {
        Array arr = V64Array_create(5, ARRAY_FILLTYPE_ZERO, VALUE64_FS);
        test_validatefree(
            arr.len == 5 && arr.sz >= 5,
            Arrayfree(arr),
            "ZERO FS array: len=%d, sz=%d (expected 5, >=5)", arr.len, arr.sz
        );
        // каждый элемент – FS с пустой строкой
        for (int i = 0; i < 5; i++) {
            fs *f = value64_fs(arr.v64[i]);
            test_validatefree(
                f != NULL && fs_len(f) == 0 && fs_str(f)[0] == '\0',
                Arrayfree(arr),
                "FS[%d] must be empty fs", i
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: create ASC‑filled FS array", ++subnum);
    {
        Array arr = V64Array_create(3, ARRAY_FILLTYPE_ASC, VALUE64_FS);
        test_validatefree(
            arr.len == 3,
            Arrayfree(arr),
            "ASC FS array: len=%d (expected 3)", arr.len
        );
        // длины FS должны возрастать
        for (int i = 1; i < 3; i++) {
            test_validatefree(
                fs_len(value64_fs(arr.v64[i])) >= fs_len(value64_fs(arr.v64[i-1])),
                Arrayfree(arr),
                "ASC FS: length must be non‑decreasing"
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: create DESC‑filled FS array", ++subnum);
    {
        Array arr = V64Array_create(3, ARRAY_FILLTYPE_DESC, VALUE64_FS);
        test_validatefree(
            arr.len == 3,
            Arrayfree(arr),
            "DESC FS array: len=%d (expected 3)", arr.len
        );
        // длины FS должны убывать
        for (int i = 1; i < 3; i++) {
            test_validatefree(
                fs_len(value64_fs(arr.v64[i])) <= fs_len(value64_fs(arr.v64[i-1])),
                Arrayfree(arr),
                "DESC FS: length must be non‑increasing"
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST V64Array (STR / FS) shrink / increase -------------------------
static TestStatus
tf_v64array_shrink_increase(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== STR array: increase then shrink ========== */
    test_sub("subtest %d: STR increase + shrink", ++subnum);
    {
        Array arr = V64Array_create(3, ARRAY_FILLTYPE_ZERO, VALUE64_STR);
        // заполним явно, чтобы потом проверять
        arr.v64[0] = value64_createstr("first");
        arr.v64[1] = value64_createstr("second");
        arr.v64[2] = value64_createstr("third");

        // увеличиваем до 6
        arr = Array_increase(arr, 6);
        test_validatefree(
            arr.len == 6 && arr.sz >= 6,
            Arrayfree(arr),
            "After increase len=%d (expected 6)", arr.len
        );
        // новые элементы должны быть пустыми строками
        for (int i = 3; i < 6; i++) {
            test_validatefree(
                value64_str(arr.v64[i]) != NULL && strcmp(value64_str(arr.v64[i]), "") == 0,
                Arrayfree(arr),
                "STR[%d] must be empty after increase", i
            );
        }

        // уменьшаем обратно до 3
        arr = Array_shrink(arr, 3);
        test_validatefree(
            arr.len == 3 && arr.sz >= 3,
            Arrayfree(arr),
            "After shrink len=%d (expected 3)", arr.len
        );
        // старые элементы должны сохраниться
        test_validatefree(
            strcmp(value64_str(arr.v64[0]), "first") == 0 &&
            strcmp(value64_str(arr.v64[1]), "second") == 0 &&
            strcmp(value64_str(arr.v64[2]), "third") == 0,
            Arrayfree(arr),
            "STR elements must survive shrink"
        );

        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ========== FS array: increase then shrink ========== */
    test_sub("subtest %d: FS increase + shrink", ++subnum);
    {
        //
        Array arr = V64Array_create(2, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        // заполним явно
        arr.v64[0] = value64_createfs_asstr("/first");
        arr.v64[1] = value64_createfs_asstr("/second");

        // увеличиваем до 4
        arr = Array_increase(arr, 4);
        test_validatefree(
            arr.len == 4 && arr.sz >= 4,
            Arrayfree(arr),
            "After increase len=%d (expected 4)", arr.len
        );
        // новые элементы – пустые fs
        for (int i = 2; i < 4; i++) {
            fs *f = value64_fs(arr.v64[i]);
            test_validatefree(
                f != NULL && fs_len(f) == 0,
                Arrayfree(arr),
                "FS[%d] must be empty after increase", i
            );
        }

        // уменьшаем до 1
        arr = Array_shrink(arr, 1);
        test_validatefree(
            arr.len == 1 && arr.sz >= 1,
            Arrayfree(arr),
            "After shrink len=%d (expected 1)", arr.len
        );
        // первый элемент должен сохраниться
        test_validatefree(
            strcmp(fs_str(value64_fs(arr.v64[0])), "/first") == 0,
            Arrayfree(arr),
            "FS[0] must survive shrink"
        );

        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ========== Shrink to zero ========== */
    test_sub("subtest %d: shrink to zero", ++subnum);
    {
        Array arr = V64Array_create(3, ARRAY_FILLTYPE_ZERO, VALUE64_STR);
        arr.v64[0] = value64_createstr("a");
        arr.v64[1] = value64_createstr("b");
        arr.v64[2] = value64_createstr("c");

        arr = Array_shrink(arr, 0);
        test_validatefree(
            arr.len == 0 && arr.sz == 0,
            Arrayfree(arr),
            "Shrink to zero: len=%d sz=%d (expected 0,0)", arr.len, arr.sz
        );
        // v64 должен быть NULL (память освобождена)
        test_validatefree(
            arr.v64 == NULL,
            Arrayfree(arr),
            "v64 must be NULL after shrinking to zero"
        );
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    return logret(TEST_PASSED, "done");
}

// -------------------------------------------------------------------
int
main( /*int argc, char *argv[] */ )
{
    logsimpleinit("Start");

    testenginestd(
        TESTADD(tf1,                            "Int/double creation/descr test"),
        TESTADD(tf2,                            "Int/double filling test"),
        TESTADD(tf3,                            "Shrink test"),
        TESTADD(tf4,                            "Save/load int test"),
        TESTADD(tf5,                            "Save/load dbl test"),
        TESTADD(tf6,                            "Shuffle array(dbl/int) simple test"),
        TESTADD(tf7,                            "Sort array(dbl/int) simple test"),
        TESTADD(tf8,                            "Array_increase simple test"),
        TESTADD(tf9,                            "PArray simple test"),
        TESTADD(tf10,                           "Creation with ARRAY_(DE)ASC_SERIES simple test"),
        TESTADD(tf11,                           "Array_fillrange simple test"),
        TESTADD(tf12,                           "Array_foreach macro simple test"),
        TESTADD(tf13,                           "Array_foreach_prod simple test"),
        TESTADD(tf_v64array_str_fs,             "V64Array (STR / FS) simple test"),
        TESTADD(tf_v64array_shrink_increase,    "V64Array (STR / FS) shrink / increase simple test")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* ARRAYTESTING */

