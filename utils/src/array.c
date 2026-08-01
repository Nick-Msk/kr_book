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
const char              *g_save_format_char      = "%6d\t%c\n";
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
static int                      getelemsize(const Array *arr) {
    invraisecode(ERR_NULLABLE_PTR, arr != NULL, "Null pointer");
    int         elem_size = 0;
    ArrayType   typ = Array_gettype(*arr);
    switch (typ) {
        case ARRAY_INT:     elem_size = sizeof(int);        break;
        case ARRAY_LONG:    elem_size = sizeof(long);       break;
        case ARRAY_DOUBLE:  elem_size = sizeof(double);     break;
        case ARRAY_POINTER: elem_size = sizeof(void*);      break;
        case ARRAY_CHAR:    elem_size = sizeof(char);       break;
        case ARRAY_V64:     elem_size = sizeof(value64);    break;
        default:
            return logsimpleret(-1, "Unknown type %d", typ);
    }
    return elem_size;
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

    int bytes = newsz * getelemsize(arr);
    if (bytes < 0) 
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
 * @brief Writes a pointer into an array (plain or value64).
 * @see set_int_element
 */
static inline void set_char_element(Array a, int i, char val) {
    if (Array_isv64(a))
        //a.v64[i] = value64_createptr(val);
        userraiseint(ERR_UNSUPPORTED_TYPE, "V64 container don't support char type for now");
    else
        a.cv[i] = val;
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

/**
 * @brief Loads array elements from a text stream.
 *
 * The array header must already be read, and the array must be correctly
 * created before calling this function.
 *
 * @param in  input stream, already opened for reading
 * @param arr pointer to the array to fill
 * @return true on success, false on error
 */
static bool                 load_values(FILE *restrict in, Array *restrict arr) {
    int     tmp;
    //for (int i = 0; i < arr->len; i++){
    Array_pforeach_idx(arr, i) {
        switch (Array_gettype(*arr)) {
            case ARRAY_INT:
                fscanf(in, g_save_format_int, &tmp, arr->iv + i);
                break;
            case ARRAY_LONG:
                fscanf(in, g_save_format_long, &tmp, arr->lv + i);
                break;
            case ARRAY_DOUBLE:
                fscanf(in, "%d %lg\n", &tmp, arr->dv + i);
                break;
            case ARRAY_POINTER:
                fscanf(in, "%d %p\n", &tmp, arr->pv + i);
                break;
            case ARRAY_CHAR:
                fscanf(in, "%d %c\n", &tmp, arr->cv + i);
                break;
            case ARRAY_V64:
                value64_fload(in, &arr->v64[i], arr->v64type, true, NULL);
                break;
        }
    }
    return true;
}

/**
 * @brief Writes the array elements into a text stream.
 *
 * This function outputs only the element data, without header or footer.
 *
 * @param out output stream, already opened for writing
 * @param arr constant pointer to the array
 * @return number of bytes written
 */
static long                 save_values(FILE *restrict out, const Array *restrict arr) {
    long        total = 0L;
    ArrayType   typ = Array_gettype(*arr);
    for (int i = 0; i < arr->len; i++)
        switch (typ) {
            case ARRAY_INT:
                IOCHECKER(written, fprintf(out, g_save_format_int, i, arr->iv[i]) )
                    total += written;
                break;
            case ARRAY_LONG:
                IOCHECKER(written, fprintf(out, g_save_format_long, i, arr->lv[i]) )
                    total += written;
                break;
            case ARRAY_DOUBLE:
                IOCHECKER(written, fprintf(out, g_save_format_double, i, arr->dv[i]) )
                    total += written;
                break;
            case ARRAY_POINTER:
                IOCHECKER(written, fprintf(out, g_save_format_pointer, i, arr->pv[i]) )
                    total += written;
                break;
            case ARRAY_CHAR:
                IOCHECKER(written, fprintf(out, g_save_format_char, i, arr->cv[i]) )
                    total += written;
                break;
            case ARRAY_V64:
                IOCHECKER(written, value64_fsave(out, arr->v64[i], arr->v64type, true) )
                    total += written;
                break;
            default:
                break;
        }
    return total;
}
/**
 * @brief Writes the array elements into a fs.
 *
 * This function outputs only the element data, without header or footer.
 *
 * @param out pointer to initialized (via FS() at least) fs
 * @param arr constant pointer to the array
 * @return number of bytes written
 */
static long                 serialize_values(fs *restrict s, const Array *restrict arr) {
    long        total = 0L;
    ArrayType   typ = Array_gettype(*arr);
    //for (int i = 0; i < arr->len; i++)
    Array_pforeach_idx(arr, i) {
        switch (typ) {
            case ARRAY_INT:
                total += fs_sprintf_concat(s, g_save_format_int, i, arr->iv[i]);
                break;
            case ARRAY_LONG:
                total += fs_sprintf_concat(s, g_save_format_long, i, arr->lv[i]);
                break;
            case ARRAY_DOUBLE:
                total += fs_sprintf_concat(s, g_save_format_double, i, arr->dv[i]);
                break;
            case ARRAY_POINTER:
                total += fs_sprintf_concat(s, g_save_format_pointer, i, arr->pv[i]);
                break;
            case ARRAY_CHAR:
                total += fs_sprintf_concat(s, g_save_format_char, i, arr->cv[i]);
                break;
            case ARRAY_V64:
                total += value64_tostr(s, arr->v64[i], arr->v64type, VALUE64_2STR);
                break;
            default:
                logsimple("Unknown type %s", ArrayTypeName(typ));
                break;
        }
    }
    return total;
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
/// @param val  int value pointer
/// @param sign sign (1/-1) 
/// @return adjusted value
static inline int               incintrnd(int *val, int sign){
    return (*val += sign * (rndint(10) + 1) );
}
/// @brief      long incrementer (or dec)
/// @param val  long value pointer
/// @param sign sign (1/-1) 
/// @return adjusted value
static inline long              inclongrnd(long *val, int sign){
    return (*val += sign * (rndlong(10) + 1) );
}
/// @brief       double incrementer (or dec)
/// @param val   double value pointer
/// @param sign  sign (1/-1) 
/// @return adjusted value
static inline double           incdoublernd(double *val, int sign){
    return (*val += sign * (rnddbl(10) + g_array_dbl_increment) );
}
/// @brief       char incrementer (or dec)
/// @param val   char value pointer
/// @param sign  sign (1/-1) 
/// @return adjusted value or OUT_OF_RANGE exception
/// @note low level (no NULL pointer checking)
static inline char             incchar(char *val, int sign){
    if ( (toupper(*val) >= 'Z' && sign > 0) || (toupper(*val) <= 'A' && sign < 0) )
        userraiseint(ERR_OUT_OF_RANGE, "Out of range %c with direction %d", *val, sign);
    return (*val += sign * 1);  // just 1, no random here
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
        case ARRAY_CHAR: {
            char val = 'a';
            for (int i = from; i < to; i++, incchar(&val, 1) )
                set_char_element(a, i, val);
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
        case ARRAY_CHAR: {
            char val = 'z';
            for (int i = from; i < to; i++, incchar(&val, -1) )
                set_char_element(a, i, val);
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
        case ARRAY_CHAR:
            for (int i = from; i < to; i++) // iter??? TODO: check if it's correct
                set_char_element(a, i, '\0');
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
        case ARRAY_CHAR:
            for (int i = from; i < to; i++) // iter??? TODO: check if it's correct
                set_char_element(a, i, rndupperchar() );    // upper/lower must be in context.c
            break;
        case ARRAY_UNKNOWN: {
            switch (a.v64type) {
                case VALUE64_FS: {
                    fs s = FS();
                    for (int i = from; i < to; i++) {
                        fs_genrnd(&s, to - from + 1, 'A');
                        set_v64fs_element(a, i, &s);  
                    }
                    fsfree(s);
                    break;
                }
                case VALUE64_STR: {
                    fs s = FS();
                    for (int i = from; i < to; i++) {
                        fs_genrnd(&s, to - from + 1, 'A');
                        set_v64str_element(a, i, fsstr(s) );
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
        case ARRAY_CHAR: {
            char val = 'A';
            for (int i = from; i < to; i++)
                set_double_element(a, i, val++);    // can be ERR_OUT_OF_RANGE
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
        case ARRAY_CHAR: {
            char val = 'Z';
            for (int i = from; i < to; i++)
                set_double_element(a, i, val--);    // can be ERR_OUT_OF_RANGE
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

/**
 * @brief Shuffle array elements using the Fisher–Yates algorithm.
 * @param arr array (by value)
 */
void                        Array_shuffle(Array arr) {
    int elem_size = getelemsize(&arr);
    if (elem_size <= 0)
        userraiseint(ERR_UNSUPPORTED_TYPE, "unsupported type for shuffle %s", ArrayGettypeName(arr));
    // in case if arr.len < 2 that'll do nothing
    char    *data = arr.v;  // raw byte pointer
    for (int i = arr.len - 1; i > 0; i--) {
        int j = rndint(i);
        // swap elements at indices i and j
        item_exch(data + i * elem_size, data + j * elem_size, elem_size);   
    }
}

/**
 * @brief Checks whether two arrays are *not* equal.
 *
 * Arrays are equal if they have the same type, the same length, and all
 * elements compare equal.  The comparison is type‑aware:
 * - Numeric types (INT, LONG, DOUBLE, CHAR) compare directly.
 * - POINTER arrays compare the pointers themselves.
 * - V64 arrays delegate to value64_equals().
 *
 * @param arr1 pointer to the first array
 * @param arr2 pointer to the second array
 * @return true if the arrays differ in type, length, or any element,
 *         false if they are equal
 *
 * @throws ERR_NULLABLE_PTR if any of the pointers is NULL
 * @throws ERR_UNSUPPORTED_TYPE if the type is not handled
 */
bool ArrayNoteq(const Array *restrict arr1, const Array *restrict arr2) {
    invraisecode(ERR_NULLABLE_PTR, arr1 != NULL && arr2 != NULL,
                 "Null pointers %p %p", (void*) arr1, (void*) arr2);
    // raise exception if not equal
    ArrayCheckComparable(arr1, arr2);

    if (arr1->len != arr2->len)
        return true;   // different lengths -> not equal

#define ArrayNoteq_COMPARE_LOOP(field) \
    for (int i = 0; i < arr1->len; i++) \
        if (arr1->field[i] != arr2->field[i]) return true;        


    ArrayType typ = Array_gettype(*arr1);
    switch (typ) {
        case ARRAY_INT: 
            ArrayNoteq_COMPARE_LOOP(iv);
            break;
        case ARRAY_LONG:
            ArrayNoteq_COMPARE_LOOP(lv);
            break;
        case ARRAY_DOUBLE:
            ArrayNoteq_COMPARE_LOOP(dv);
            break;
        case ARRAY_POINTER:
            ArrayNoteq_COMPARE_LOOP(pv);
            break;
        case ARRAY_CHAR:
            ArrayNoteq_COMPARE_LOOP(cv);
            break;
        case ARRAY_V64:
            for (int i = 0; i < arr1->len; i++)
                if (!value64_equal(arr1->v64[i], arr2->v64[i], arr1->v64type))
                    return true;
            break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "Unsupported type for comparison: %s",
                         ArrayGettypeName(*arr1));
            return true;
    }
    return false;   // all elements equal
}

/**
 * @brief Sorts an array in ascending or descending order.
 *
 * Uses quicksort (qsort) with type‑specific comparators.
 * For ARRAY_V64 the comparator is obtained via value64_getPComparator()
 * / value64_getPRevComparator() according to the stored v64type.
 *
 * @param arr array (by value)
 * @param ord sort order: ARRAY_FILLTYPE_ASC or ARRAY_FILLTYPE_DESC
 *
 * @throws ERR_UNSUPPORTED_TYPE if the array type cannot be sorted
 */
void                                Array_qsort(Array arr, ArrayFillType ord) {
    int                 sz = getelemsize(&arr);
    if (sz <= 0)
        userraiseint(ERR_UNSUPPORTED_TYPE, "Unable to get type size %d/%s", Array_gettype(arr), ArrayGettypeName(arr));

    pointer_comparator  cmp = NULL;
   
    ArrayType typ = Array_gettype(arr);
    switch (typ) {
        case ARRAY_INT:     
            cmp = (ord == ARRAY_FILLTYPE_ASC) ? pint_cmp  : pint_revcmp;  
            break;
        case ARRAY_LONG:    cmp = (ord == ARRAY_FILLTYPE_ASC) ? plong_cmp : plong_revcmp; 
            break;
        case ARRAY_DOUBLE:  
            cmp = (ord == ARRAY_FILLTYPE_ASC) ? pdbl_cmp  : pdbl_revcmp;  
            break;
        case ARRAY_POINTER: 
            cmp = (ord == ARRAY_FILLTYPE_ASC) ? pptr_cmp  : pptr_revcmp;  
            break;
        case ARRAY_CHAR:    
            cmp = (ord == ARRAY_FILLTYPE_ASC) ? pchar_cmp : pchar_revcmp; 
        break;
        case ARRAY_V64:
            cmp = (ord == ARRAY_FILLTYPE_ASC) 
                    ? value64_getPComparator(arr.v64type)
                    : value64_getPRevComparator(arr.v64type);
        break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "Unsupported type %s", ArrayGettypeName(arr));
    }
    
    if (cmp)
        qsort(arr.v, arr.len, sz, cmp);
}

/**
 * @brief Binary search for an integer in a sorted INT array.
 *
 * The array must be of type ARRAY_INT and sorted in ascending order.
 *
 * @param arr array (by value)
 * @param val value to search for
 * @return index of the found element (>=0), or -1 if not found
 */
int                         ArrayBsearchIntCommon(Array arr, int val, bool acs) {
    if (!Array_isint(arr))
        userraiseint(ERR_UNSUPPORTED_TYPE, "ArrayBsearchInt requires ARRAY_INT");
    if (arr.len == 0)
        return -1;
    pointer_comparator cmp = acs ? pint_cmp : pint_revcmp;
    const int *found = (const int*) bsearch(&val, arr.iv, arr.len, sizeof(int), cmp);
    return found ? (int)(found - arr.iv) : -1;
}

/**
 * @brief Binary search for a long in a sorted LONG array.
 *
 * The array must be of type ARRAY_LONG and sorted in ascending order.
 *
 * @param arr array (by value)
 * @param val value to search for
 * @return index of the found element (>=0), or -1 if not found
 */
int                         ArrayBsearchLongCommon(Array arr, long val, bool acs) {
    if (!Array_islong(arr))
        userraiseint(ERR_UNSUPPORTED_TYPE, "ArrayBsearchLong requires ARRAY_LONG");
    if (arr.len == 0)
        return -1;
    pointer_comparator cmp = acs ? plong_cmp : plong_revcmp;
    const long *found = (const long*) bsearch(&val, arr.lv, arr.len, sizeof(long), cmp);
    return found ? (int)(found - arr.lv) : -1;
}

/**
 * @brief Binary search for a double in a sorted DOUBLE array.
 *
 * The array must be of type ARRAY_DOUBLE and sorted in ascending order.
 *
 * @param arr array (by value)
 * @param val value to search for
 * @return index of the found element (>=0), or -1 if not found
 */
int                         ArrayBsearchDblCommon(Array arr, double val, bool acs) {
    if (!Array_isdouble(arr))
        userraiseint(ERR_UNSUPPORTED_TYPE, "ArrayBsearchDbl requires ARRAY_DOUBLE");
    if (arr.len == 0)
        return -1;
    pointer_comparator cmp = acs ? pdbl_cmp : pdbl_revcmp;    
    const double *found = (const double*)bsearch(&val, arr.dv, arr.len, sizeof(double), cmp);
    return found ? (int)(found - arr.dv) : -1;
}
/**
 * @brief Binary search for a double in a sorted DOUBLE array.
 *
 * The array must be of type ARRAY_DOUBLE and sorted in ascending order.
 *
 * @param arr array (by value)
 * @param val value to search for
 * @return index of the found element (>=0), or -1 if not found
 */
int                         ArrayBsearchCharCommon(Array arr, char val, bool acs) {
    if (!Array_ischar(arr))
        userraiseint(ERR_UNSUPPORTED_TYPE, "Requires ARRAY_CHAR");
    if (arr.len == 0)
        return -1;
    pointer_comparator  cmp = acs ? pchar_cmp : pchar_revcmp;    
    const               char *found = (const char *) bsearch(&val, arr.cv, arr.len, sizeof(char), cmp);
    return found ? (int)(found - arr.cv) : -1;
}

/**
 * @brief Binary search for a value64 in a sorted V64 array.
 *
 * The array must be of type ARRAY_V64 and sorted in ascending or descending
 * order according to its v64type.
 *
 * @param arr array (by value)
 * @param val value to search for
 * @param asc true if array is sorted ascending, false if descending
 * @return index of the found element (>=0), or -1 if not found
 */
int                         ArrayBsearchV64Common(Array arr, value64 val, bool asc) {
    if (!Array_isv64(arr))
        userraiseint(ERR_UNSUPPORTED_TYPE, "ArrayBsearchV64 requires ARRAY_V64");
    if (arr.len == 0) return -1;

    if (asc)
        return value64_binsearch(val, arr.v64type, arr.v64, arr.len);
    else
        return value64_rev_binsearch(val, arr.v64type, arr.v64, arr.len);
}

// -----------------------------------------------------------------------------------------------------
// if condition is 0-ptr == ALL
int                         Array_foreach_proc(Array arr, Array_cond cond, Array_proc func){
    // TODO: use foreach here
    int     cnt = 0;
    //for (int i = 0; i < Arraylen(arr); i++)
    Array_foreach_idx(arr, i) {
        if (cond == NULL || cond(arr, i) ){
            if (func)
                func(arr, i);
            cnt++;
        }
    }
    return logsimpleret(cnt, "processed %d", cnt);
}

// -------------------------- (API) printers -----------------------

/**
 * @brief Prints the contents of an array to a file stream.
 *
 * Output format can be customised via the global variables
 * `g_custom_print_line` and `g_array_rec_line`.
 * For ARRAY_V64 the specialised printer `value64_techfprint()` is used.
 *
 * @param f     output stream (must be opened for writing)
 * @param val   array (by value)
 * @param limit maximum number of elements to print (0 = print all)
 * @return      number of characters printed
 */
int                         Array_fprint(FILE *f, Array val, int limit) {
    int cnt = 0, i;
    int array_rec_line = 20;      // default value

    limit = (limit == 0) ? val.len : (limit < val.len) ? limit : val.len;
    if (g_array_rec_line)
        array_rec_line = g_array_rec_line;

    cnt += fprintf(f, "Array (%s[%d of total %d]):\n",
                   ArrayTypeName(val.flags), limit, val.len);

    const char *custom = g_custom_print_line;

    for (i = 0; i < limit; i++) {
        switch (Array_gettype(val)) {
            case ARRAY_INT:
                cnt += fprintf(f, custom ? custom : "[%d - %6d]\t", i, val.iv[i]);
                break;
            case ARRAY_LONG:
                cnt += fprintf(f, custom ? custom : "[%ld - %6ld]\t", i, val.lv[i]);
                break;
            case ARRAY_DOUBLE:
                cnt += fprintf(f, custom ? custom : "[%d - %.8lg]\t", i, val.dv[i]);
                break;
            case ARRAY_POINTER:
                cnt += fprintf(f, custom ? custom : "[%p - %p]\t", i, val.pv[i]);
                break;
            case ARRAY_CHAR:
                cnt += fprintf(f, custom ? custom : "[%d - %c]\t", i, val.cv[i]);
                break;
            case ARRAY_V64:
                // custom format not supported for value64; always use dedicated printer
                value64_techfprint(f, val.v64[i], val.v64type, "");
                break;
            default:
                cnt += fprintf(f, "[%d - ?]\t", i);
                break;
        }

        if (((i + 1) % array_rec_line) == 0)
            cnt += fprintf(f, "\n");
    }

    if (i < val.len)
        cnt += fprintf(f, "and more (%d) ...\n", val.len - i);
    else
        cnt += fprintf(f, "\n");

    return cnt;
}

/**
 * @brief Saves array values to a text file, separated by a delimiter.
 *
 * Each element is written on a single line, with the delimiter appended.
 * For ARRAY_V64 the dedicated value64_fsave() is used.
 *
 * @param arr   array (by value)
 * @param fname file name
 * @param delim delimiter character
 * @return number of bytes written, or -1 on error
 */
long                        Array_savevalues(Array arr, const char *fname, char delim) {
    logenter("%s, [%c]", fname, delim);

    FILE *f = fopen(fname, "w");
    if (!f)
        return logerr(-1, "Can't open '%s' for writing", fname);

    long    total_written = 0;
    int     typ = Array_gettype(arr), status = 0;

    for (int i = 0; i < arr.len; i++) {
        if (i > 0) {
            if (fputc(delim, f) == EOF) {
                status = -1;
                break;
            }
            total_written++;
        }
        int     written = 0;
        switch (typ) {
            case ARRAY_INT:
                written = fprintf(f, "%d", arr.iv[i]);
                break;
            case ARRAY_LONG:
                written = fprintf(f, "%ld", arr.lv[i]);
                break;
            case ARRAY_DOUBLE:
                written = fprintf(f, "%12.12lf", arr.dv[i]);
                break;
            case ARRAY_POINTER:
                written = fprintf(f, "%p", arr.pv[i]);
                break;
            case ARRAY_CHAR:
                written = fprintf(f, "%c", arr.iv[i]);
                break;
            case ARRAY_V64:
                written = value64_fsave(f, arr.v64[i], arr.v64type, true);
                break;
            default:
                fclose(f);
                return userraise(-1, ERR_UNSUPPORTED_TYPE, "Unsupported type %s\n", ArrayGettypeName(arr));
        }
        if (written < 0) {
            status = -1;
            break;
        }
        total_written += written;
    }
    if (status != 0) {
        fclose(f);
        return logerr(-1, "Write error in '%s'", fname);
    }

    if (fclose(f) != 0) {
        return logerr(-1, "Error closing file '%s'", fname);
    }

    return logret(total_written, "Done %ld", total_written);
}

/**
 * @brief Saves an array to a text stream in the full ARRAY format.
 *
 * The format is:
 *   ARRAY: <type> / <v64type> : <count>\n
 *   <elements>
 *   ARRAY: DONE\n
 *
 * @param out output stream, already opened for writing
 * @param arr array (by value)
 * @return number of bytes written
 */

long                        ArrayFSave(FILE *out, Array arr) {  
    if (!out)
        return logsimpleret(0L, "Output is null"); 

    long        total_written = 0L;
    const char  *typ = ArrayTypeName(arr.flags);
    const char  *v64_type  = Array_isv64(arr) ? ArrayGetV64typeName(arr) : "NONV64_TYPE";

    IOCHECKER(written, fprintf(out, "ARRAY: %s / %s : %d\n", typ, v64_type, arr.len) )
        total_written += written;
    IOCHECKER(written, save_values(out, &arr) )
        total_written += written;
    IOCHECKER(written, fprintf(out, "ARRAY: DONE\n") )
        total_written += written;
    return total_written;
}

/**
 * @brief Saves an array to a file.
 *
 * Opens the file for writing, calls ArrayFSave(), and closes the file.
 *
 * @param arr   array (by value)
 * @param fname file path
 * @return number of bytes written, or a negative value on error
 */
long                        Array_save(Array arr, const char *fname) {
    logenter("%s", fname);

    FILE        *out = fopen(fname, "w");
    if (out == 0)
        return userraise(ERR_UNABLE_OPEN_FILE_WRITE, -1, "Can't open '%s' for write", fname);

    long        res = ArrayFSave(out, arr);
    fclose(out);

    if(res < 0)
        return logerr(res, "Unable to save array");
    else
        return logret(res, "Done %ld", res);
}

/**
 * @brief Loads an array from a text stream in the full ARRAY format.
 *
 * Reads the header, creates the array, fills its elements, and checks the
 * footer.
 *
 * @param in input stream, already opened for reading
 * @return loaded array, or an array with the error flag set
 */
Array                           ArrayFLoad(FILE *in) {
    invraisecode(ERR_NULLABLE_PTR, in != NULL, "Nullable input");

    int                 cnt = 0;
    char                typ[ARRAY_MAX_TYPE_STR], v64typ[ARRAY_MAX_TYPE_STR] = "";

    // Read header: "ARRAY: <type> / <v64type> : <count>"
    if (fscanf(in, "ARRAY: %" TOSTRING(ARRAY_MAX_TYPE_STR_WO_LAST) "s / %" TOSTRING(ARRAY_MAX_TYPE_STR_WO_LAST) "s : %d ", 
                typ, 
                v64typ, 
                &cnt) != 3)
        userraiseint(ERR_WRONG_INPUT_FORMAT, "Array header wrong format");

    Array arr = Array_init();       // zero-init
    ArrayType atype = ArrayTypeFromName(typ);

    switch (atype) {
        case ARRAY_V64: {
            value64_type vt = value64_gettype(v64typ);
            if (vt == VALUE64_UNKNOWN)
                userraiseint(ERR_WRONG_INPUT_FORMAT, "Array header V64 wrong format '%s'", v64typ);
            arr = V64Array_create(cnt, ARRAY_FILLTYPE_NONE, vt);
            break;
        }
        case ARRAY_INT:
            arr = IArray_create(cnt, ARRAY_FILLTYPE_NONE);
            break;
        case ARRAY_LONG:
            arr = LArray_create(cnt, ARRAY_FILLTYPE_NONE);
            break;
        case ARRAY_DOUBLE:
            arr = DArray_create(cnt, ARRAY_FILLTYPE_NONE);
            break;
        case ARRAY_POINTER:
            arr = PArray_create(cnt, ARRAY_FILLTYPE_NONE);
            break;
        case ARRAY_CHAR:
            arr = CArray_create(cnt, ARRAY_FILLTYPE_NONE);
            break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "Unsupported type '%s'", typ);
    }

    load_values(in, &arr);

    // Check footer "ARRAY: DONE"
    if (fscanf(in, "ARRAY: %" TOSTRING(ARRAY_MAX_TYPE_STR_WO_LAST) "s", typ) != 1 
            || strcmp(typ, "DONE") != 0)
        userraiseint(ERR_WRONG_INPUT_FORMAT, "Wrong final piece '%s'", typ);

    return arr;
}

/**
 * @brief Loads an array from a file.
 *
 * Opens the file for reading, calls ArrayFLoad(), and closes the file.
 *
 * @param fname file path
 * @return loaded array, or an array with the error flag set
 */
Array                       Array_load(const char *fname) {
    invraisecode(ERR_NULLABLE_PTR, fname != NULL, "Nullable fname");

    logenter("%s", fname);
    FILE    *in = fopen(fname, "r");

    if (in == 0)
        userraiseint(ERR_UNABLE_OPEN_FILE_READ, "Can't open for read '%s'", fname);
    
    Array arr = ArrayFLoad(in);
    
    fclose(in);
    return logret(arr, "Done %d", arr.len);
}

// -------------------------- (API) serialization -----------------------

long                        ArraySerialize(fs *restrict s, const Array *restrict arr) {
    invraisecode(ERR_NULLABLE_PTR, s != NULL && arr != NULL, 
            "Fs nullable or arr is null %p %p", s, arr);

    long        total_written = 0L;
    const char  *typ = ArrayTypeName(arr->flags);
    const char  *v64_type  = Array_isv64(*arr) ? ArrayGetV64typeName(*arr) : "NONV64_TYPE";

    total_written += fs_sprintf_concat(s, "ARRAY: %s / %s : %d\n", typ, v64_type, arr->len);
    total_written += serialize_values(s, arr);
    total_written += fs_sprintf_concat(s, "ARRAY: DONE\n");
    return total_written;
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
        int cnt = 10000;
        Array parr = PArray_create(cnt, ARRAY_FILLTYPE_ZERO);

        // fiil array manually
        for (int i = 0; i < parr.len; i++)
            parr.pv[i] = parr.pv + cnt - 1 - i;

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

    /* ---------- RND заполнение ---------- */
    test_sub("subtest %d: create RND‑filled STR array", ++subnum);
    {
        Array arr = V64Array_create(5, ARRAY_FILLTYPE_RND, VALUE64_STR);
        test_validatefree(
            arr.len == 5,
            Arrayfree(arr),
            "RND STR array: len=%d (expected 5)", arr.len
        );
        // проверяем, что строки непустые
        for (int i = 0; i < 5; i++) {
            const char *s = value64_str(arr.v64[i]);
            logmsg("VALUE64_STR: ARRAY_FILLTYPE_RND: %s", s);
            test_validatefree(
                s != NULL && strlen(s) > 0,
                Arrayfree(arr),
                "RND STR[%d] must be non‑empty, got '%s'", i, s ? s : "NULL"
            );
        }
    Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: create RND‑filled FS array", ++subnum);
    {
        Array arr = V64Array_create(5, ARRAY_FILLTYPE_RND, VALUE64_FS);
        test_validatefree(
            arr.len == 5,
            Arrayfree(arr),
            "RND FS array: len=%d (expected 5)", arr.len
        );
        for (int i = 0; i < 5; i++) {
            fs *f = value64_fs(arr.v64[i]);
            logmsg("VALUE64_FS: ARRAY_FILLTYPE_RND: %s", fs_str(f) );
            test_validatefree(
                f != NULL && fs_len(f) > 0,
                Arrayfree(arr),
                "RND FS[%d] must be non‑empty, got len=%d", i, f ? fs_len(f) : -1
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ---------- ASC / DESC с проверкой длин ---------- */
    test_sub("subtest %d: create ASC‑filled STR (lengths non‑decreasing)", ++subnum);
    {
        Array arr = V64Array_create(4, ARRAY_FILLTYPE_ASC, VALUE64_STR);
        test_validatefree(arr.len == 4, Arrayfree(arr), "len check");
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

    test_sub("subtest %d: create DESC‑filled FS (lengths non‑increasing)", ++subnum);
    {
        Array arr = V64Array_create(4, ARRAY_FILLTYPE_DESC, VALUE64_FS);
        test_validatefree(arr.len == 4, Arrayfree(arr), "len check");
        for (int i = 1; i < 4; i++) {
            test_validatefree(
                fs_len(value64_fs(arr.v64[i])) <= fs_len(value64_fs(arr.v64[i-1])),
                Arrayfree(arr),
                "DESC FS: length must be non‑increasing"
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ---------- ZERO (пустые строки) ---------- */
    test_sub("subtest %d: ZERO STR array must have empty strings", ++subnum);
    {
        Array arr = V64Array_create(3, ARRAY_FILLTYPE_ZERO, VALUE64_STR);
        for (int i = 0; i < 3; i++) {
            test_validatefree(
                strcmp(value64_str(arr.v64[i]), "") == 0,
                Arrayfree(arr),
                "ZERO STR[%d] must be empty", i
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

// ------------------------- TEST V64Array (STR / FS) sorting -------------------------
static TestStatus
tf_v64array_sort(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== STR sorting ========== */
    test_sub("subtest %d: STR sort ASC", ++subnum);
    {
        Array arr = V64Array_create(4, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        arr.v64[0] = value64_createstr("delta");
        arr.v64[1] = value64_createstr("alpha");
        arr.v64[2] = value64_createstr("charlie");
        arr.v64[3] = value64_createstr("beta");

        Array_qsort(arr, ARRAY_FILLTYPE_ASC);

        const char *expected[] = {"alpha", "beta", "charlie", "delta"};
        for (int i = 0; i < 4; i++) {
            test_validatefree(
                strcmp(value64_str(arr.v64[i]), expected[i]) == 0,
                Arrayfree(arr),
                "STR ASC [%d]: expected '%s', got '%s'",
                i, expected[i], value64_str(arr.v64[i])
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: STR sort DESC", ++subnum);
    {
        Array arr = V64Array_create(4, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        arr.v64[0] = value64_createstr("delta");
        arr.v64[1] = value64_createstr("alpha");
        arr.v64[2] = value64_createstr("charlie");
        arr.v64[3] = value64_createstr("beta");

        Array_qsort(arr, ARRAY_FILLTYPE_DESC);

        const char *expected[] = {"delta", "charlie", "beta", "alpha"};
        for (int i = 0; i < 4; i++) {
            test_validatefree(
                strcmp(value64_str(arr.v64[i]), expected[i]) == 0,
                Arrayfree(arr),
                "STR DESC [%d]: expected '%s', got '%s'",
                i, expected[i], value64_str(arr.v64[i])
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ========== FS sorting ========== */
    test_sub("subtest %d: FS sort ASC", ++subnum);
    {
        Array arr = V64Array_create(4, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        arr.v64[0] = value64_createfs_asstr("/zzz");
        arr.v64[1] = value64_createfs_asstr("/aaa");
        arr.v64[2] = value64_createfs_asstr("/mmm");
        arr.v64[3] = value64_createfs_asstr("/bbb");

        Array_qsort(arr, ARRAY_FILLTYPE_ASC);

        const char *expected[] = {"/aaa", "/bbb", "/mmm", "/zzz"};
        for (int i = 0; i < 4; i++) {
            test_validatefree(
                strcmp(fs_str(value64_fs(arr.v64[i])), expected[i]) == 0,
                Arrayfree(arr),
                "FS ASC [%d]: expected '%s', got '%s'",
                i, expected[i], fs_str(value64_fs(arr.v64[i]))
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS sort DESC", ++subnum);
    {
        Array arr = V64Array_create(4, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        arr.v64[0] = value64_createfs_asstr("/zzz");
        arr.v64[1] = value64_createfs_asstr("/aaa");
        arr.v64[2] = value64_createfs_asstr("/mmm");
        arr.v64[3] = value64_createfs_asstr("/bbb");

        Array_qsort(arr, ARRAY_FILLTYPE_DESC);

        const char *expected[] = {"/zzz", "/mmm", "/bbb", "/aaa"};
        for (int i = 0; i < 4; i++) {
            test_validatefree(
                strcmp(fs_str(value64_fs(arr.v64[i])), expected[i]) == 0,
                Arrayfree(arr),
                "FS DESC [%d]: expected '%s', got '%s'",
                i, expected[i], fs_str(value64_fs(arr.v64[i]))
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ========== граничные случаи ========== */

    test_sub("subtest %d: STR sort empty array", ++subnum);
    {
        Array arr = V64Array_create(0, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);   // не должно падать
        test_validatefree(
            arr.len == 0,
            Arrayfree(arr),
            "Empty STR array after sort must still be empty"
        );
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: STR sort single element", ++subnum);
    {
        Array arr = V64Array_create(1, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        arr.v64[0] = value64_createstr("single");
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        test_validatefree(
            strcmp(value64_str(arr.v64[0]), "single") == 0,
            Arrayfree(arr),
            "Single STR element must survive sorting"
        );
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: STR sort already sorted", ++subnum);
    {
        Array arr = V64Array_create(3, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        arr.v64[0] = value64_createstr("a");
        arr.v64[1] = value64_createstr("b");
        arr.v64[2] = value64_createstr("c");
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        const char *exp[] = {"a", "b", "c"};
        for (int i = 0; i < 3; i++) {
            test_validatefree(
                strcmp(value64_str(arr.v64[i]), exp[i]) == 0,
                Arrayfree(arr),
                "Already sorted STR [%d] must stay '%s'", i, exp[i]
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: STR sort with duplicates", ++subnum);
    {
        Array arr = V64Array_create(4, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        arr.v64[0] = value64_createstr("a");
        arr.v64[1] = value64_createstr("b");
        arr.v64[2] = value64_createstr("a");
        arr.v64[3] = value64_createstr("c");
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        const char *exp[] = {"a", "a", "b", "c"};
        for (int i = 0; i < 4; i++) {
            test_validatefree(
                strcmp(value64_str(arr.v64[i]), exp[i]) == 0,
                Arrayfree(arr),
                "Duplicates STR [%d] must be '%s'", i, exp[i]
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS sort empty array", ++subnum);
    {
        Array arr = V64Array_create(0, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        test_validatefree(arr.len == 0, Arrayfree(arr), "Empty FS array after sort must still be empty");
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS sort single element", ++subnum);
    {
        Array arr = V64Array_create(1, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        arr.v64[0] = value64_createfs_asstr("/only");
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        test_validatefree(
            strcmp(fs_str(value64_fs(arr.v64[0])), "/only") == 0,
            Arrayfree(arr),
            "Single FS element must survive sorting"
        );
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS sort already sorted", ++subnum);
    {
        Array arr = V64Array_create(3, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        arr.v64[0] = value64_createfs_asstr("/a");
        arr.v64[1] = value64_createfs_asstr("/b");
        arr.v64[2] = value64_createfs_asstr("/c");
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        const char *exp[] = {"/a", "/b", "/c"};
        for (int i = 0; i < 3; i++) {
            test_validatefree(
                strcmp(fs_str(value64_fs(arr.v64[i])), exp[i]) == 0,
                Arrayfree(arr),
                "Already sorted FS [%d] must stay '%s'", i, exp[i]
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS sort with duplicates", ++subnum);
    {
        Array arr = V64Array_create(4, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        arr.v64[0] = value64_createfs_asstr("/a");
        arr.v64[1] = value64_createfs_asstr("/b");
        arr.v64[2] = value64_createfs_asstr("/a");
        arr.v64[3] = value64_createfs_asstr("/c");
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        const char *exp[] = {"/a", "/a", "/b", "/c"};
        for (int i = 0; i < 4; i++) {
            test_validatefree(
                strcmp(fs_str(value64_fs(arr.v64[i])), exp[i]) == 0,
                Arrayfree(arr),
                "Duplicates FS [%d] must be '%s'", i, exp[i]
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST V64Array (STR / FS) save/load -------------------------
static TestStatus
tf_v64array_save_load(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== STR save/load ========== */
    test_sub("subtest %d: STR save/load", ++subnum);
    {
        const char *fname = "res/array/v64str.sv";

        // создаём массив и заполняем
        Array orig = V64Array_create(3, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        orig.v64[0] = value64_createstr("one");
        orig.v64[1] = value64_createstr("two");
        orig.v64[2] = value64_createstr("three");

        // сохраняем
        long written = Array_save(orig, fname);
        test_validatefree(
            written > 0,
            Arrayfree(orig),
            "STR save failed"
        );

        // загружаем
        Array loaded = Array_load(fname);
        test_validatefree(
            loaded.len == orig.len,
            (Arrayfree(orig), Arrayfree(loaded)),
            "STR load: len=%d, expected %d", loaded.len, orig.len
        );

        // сравниваем поэлементно
        for (int i = 0; i < orig.len; i++) {
            test_validatefree(
                strcmp(value64_str(orig.v64[i]), value64_str(loaded.v64[i])) == 0,
                (Arrayfree(orig), Arrayfree(loaded)),
                "STR[%d]: orig='%s', loaded='%s'",
                i, value64_str(orig.v64[i]), value64_str(loaded.v64[i])
            );
        }

        Arrayfree(orig);
        Arrayfree(loaded);
    }
    fs_alloc_check(true);

    /* ========== FS save/load ========== */
    test_sub("subtest %d: FS save/load", ++subnum);
    {
        const char *fname = "res/array/v64fs.sv";

        // создаём массив и заполняем
        Array orig = V64Array_create(3, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        orig.v64[0] = value64_createfs_asstr("/alpha");
        orig.v64[1] = value64_createfs_asstr("/beta");
        orig.v64[2] = value64_createfs_asstr("/gamma");

        // сохраняем
        long written = Array_save(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "FS save failed");

        // загружаем
        Array loaded = Array_load(fname);
        test_validatefree(
            loaded.len == orig.len,
            (Arrayfree(orig), Arrayfree(loaded)),
            "FS load: len=%d, expected %d", loaded.len, orig.len
        );

        // сравниваем поэлементно (строки, лежащие внутри fs)
        for (int i = 0; i < orig.len; i++) {
            fs *f_orig = value64_fs(orig.v64[i]);
            fs *f_load = value64_fs(loaded.v64[i]);
            test_validatefree(
                f_orig && f_load && strcmp(fs_str(f_orig), fs_str(f_load)) == 0,
                (Arrayfree(orig), Arrayfree(loaded)),
                "FS[%d]: orig='%s', loaded='%s'",
                i, f_orig ? fs_str(f_orig) : "NULL",
                f_load ? fs_str(f_load) : "NULL"
            );
        }

        Arrayfree(orig);
        Arrayfree(loaded);
    }
    fs_alloc_check(true);

    /* ========== STR save/load: граничные случаи ========== */

    test_sub("subtest %d: STR save/load empty array", ++subnum);
    {
        const char *fname = "res/array/v64str_empty.sv";

        Array orig = V64Array_create(0, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        long written = Array_save(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "STR empty save failed");

        Array loaded = Array_load(fname);
        test_validatefree(
            loaded.len == 0 && loaded.v64 == NULL,
            (Arrayfree(orig), Arrayfree(loaded)),
            "Loaded empty STR: len=%d, v64=%p (expected 0, NULL)", loaded.len, loaded.v64
        );

        Arrayfree(orig);
        Arrayfree(loaded);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: STR save/load single element", ++subnum);
    {
        const char *fname = "res/array/v64str_single.sv";

        Array orig = V64Array_create(1, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        orig.v64[0] = value64_createstr("single");
        long written = Array_save(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "STR single save failed");

        Array loaded = Array_load(fname);
        test_validatefree(
            loaded.len == 1,
            (Arrayfree(orig), Arrayfree(loaded)),
            "Loaded single STR: len=%d, expected 1", loaded.len
        );
        test_validatefree(
            strcmp(value64_str(orig.v64[0]), value64_str(loaded.v64[0])) == 0,
            (Arrayfree(orig), Arrayfree(loaded)),
            "STR single: orig='%s', loaded='%s'",
            value64_str(orig.v64[0]), value64_str(loaded.v64[0])
        );

        Arrayfree(orig);
        Arrayfree(loaded);
    }
    fs_alloc_check(true);

    /* ========== FS save/load: граничные случаи ========== */

    test_sub("subtest %d: FS save/load empty array", ++subnum);
    {
        const char *fname = "res/array/v64fs_empty.sv";

        Array orig = V64Array_create(0, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        long written = Array_save(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "FS empty save failed");

        Array loaded = Array_load(fname);
        test_validatefree(
            loaded.len == 0 && loaded.v64 == NULL,
            (Arrayfree(orig), Arrayfree(loaded)),
            "Loaded empty FS: len=%d, v64=%p (expected 0, NULL)", loaded.len, loaded.v64
        );

        Arrayfree(orig);
        Arrayfree(loaded);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS save/load single element", ++subnum);
    {
        const char *fname = "res/array/v64fs_single.sv";

        Array orig = V64Array_create(1, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        orig.v64[0] = value64_createfs_asstr("/only");
        long written = Array_save(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "FS single save failed");

        Array loaded = Array_load(fname);
        test_validatefree(
            loaded.len == 1,
            (Arrayfree(orig), Arrayfree(loaded)),
            "Loaded single FS: len=%d, expected 1", loaded.len
        );
        fs *f_orig = value64_fs(orig.v64[0]);
        fs *f_load = value64_fs(loaded.v64[0]);
        test_validatefree(
            f_orig && f_load && strcmp(fs_str(f_orig), fs_str(f_load)) == 0,
            (Arrayfree(orig), Arrayfree(loaded)),
            "FS single: orig='%s', loaded='%s'",
            f_orig ? fs_str(f_orig) : "NULL", f_load ? fs_str(f_load) : "NULL"
        );

        Arrayfree(orig);
        Arrayfree(loaded);
    }
    fs_alloc_check(true);

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST ArrayBsearch (INT / LONG / DBL / V64) -------------------------
static TestStatus
tf_array_bsearch(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== INT ========== */
    test_sub("subtest %d: INT find existing", ++subnum);
    {
        Array arr = IArray_create(10, ARRAY_FILLTYPE_ASC_SERIES); // 0,1,2,...,9
        int idx;
        test_validatefree(
            (idx = ArrayBsearchInt(arr, 5) ) == 5,
            Arrayfree(arr),
            "INT asc: expected idx=5, got %d", idx
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: INT find missing", ++subnum);
    {
        Array arr = IArray_create(10, ARRAY_FILLTYPE_ASC_SERIES);
        int idx;
        test_validatefree(
            (idx = ArrayBsearchInt(arr, 99)) == -1,
            Arrayfree(arr),
            "INT asc missing: expected -1, got %d", idx
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: INT find first / last", ++subnum);
    {
        Array arr = IArray_create(5, ARRAY_FILLTYPE_ASC_SERIES);
        test_validatefree(
            ArrayBsearchInt(arr, 0) == 0 && ArrayBsearchInt(arr, 4) == 4,
            Arrayfree(arr),
            "INT first/last: must be 0 and 4"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: INT rev search", ++subnum);
    {
        Array arr = IArray_create(10, ARRAY_FILLTYPE_DESC_SERIES); // 9,8,...,0
        int idx;
        test_validatefree(
            (idx = ArrayBsearchIntrev(arr, 5)) == 4,   // 9(0),8(1),7(2),6(3),5(4)
            Arrayfree(arr),
            "INT desc: expected idx=4, got %d", idx
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: INT empty array", ++subnum);
    {
        Array arr = IArray_create(0, ARRAY_FILLTYPE_NONE);
        test_validatefree(
            ArrayBsearchInt(arr, 5) == -1,
            Arrayfree(arr),
            "Empty INT: must return -1"
        );
        Arrayfree(arr);
    }

    /* ========== LONG ========== */
    test_sub("subtest %d: LONG find existing", ++subnum);
    {
        Array arr = LArray_create(10, ARRAY_FILLTYPE_ASC_SERIES);
        int idx ;
        test_validatefree(
            (idx = ArrayBsearchLong(arr, 7L) ) == 7,
            Arrayfree(arr),
            "LONG asc: expected idx=7, got %d", idx
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: LONG rev missing", ++subnum);
    {
        Array arr = LArray_create(10, ARRAY_FILLTYPE_DESC_SERIES);
        int idx;
        test_validatefree(
            (idx = ArrayBsearchLongRev(arr, 100L)) == -1,
            Arrayfree(arr),
            "LONG desc missing: expected -1, got %d", idx
        );
        Arrayfree(arr);
    }

    /* ========== DBL ========== */
    test_sub("subtest %d: DBL find first / last", ++subnum);
    {
        Array arr = DArray_create(5, ARRAY_FILLTYPE_ASC_SERIES); // 0.0,1.0,...,4.0
        test_validatefree(
            ArrayBsearchDbl(arr, 0.0) == 0 && ArrayBsearchDbl(arr, 4.0) == 4,
            Arrayfree(arr),
            "DBL first/last: must be 0 and 4"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: DBL rev search", ++subnum);
    {
        Array arr = DArray_create(5, ARRAY_FILLTYPE_DESC_SERIES); // 4.0,3.0,...,0.0
        int idx;
        test_validatefree(
            (idx = ArrayBsearchDblRev(arr, 2.0)) == 2,   // 4(0),3(1),2(2)
            Arrayfree(arr),
            "DBL desc: expected idx=2, got %d", idx
        );
        Arrayfree(arr);
    }

    /* ========== V64 (STR) ========== */
    test_sub("subtest %d: V64 STR find existing", ++subnum);
    {
        Array arr = V64Array_create(4, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        // явно создаём отсортированный массив: "a","b","c","d"
        arr.v64[0] = value64_createstr("a");
        arr.v64[1] = value64_createstr("b");
        arr.v64[2] = value64_createstr("c");
        arr.v64[3] = value64_createstr("d");

        value64 key = LITERAL64_STR("c");
        int idx;
        test_validatefree(
            (idx  = ArrayBsearchV64(arr, key)) == 2,
            Arrayfree(arr),
            "STR asc: expected idx=2, got %d", idx
        );
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: V64 STR rev missing", ++subnum);
    {
        Array arr = V64Array_create(4, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        // отсортированный по убыванию: "d","c","b","a"
        arr.v64[0] = value64_createstr("d");
        arr.v64[1] = value64_createstr("c");
        arr.v64[2] = value64_createstr("b");
        arr.v64[3] = value64_createstr("a");

        value64 key = LITERAL64_STR("x");
        int idx;
        test_validatefree(
            (idx = ArrayBsearchV64Rev(arr, key)) == -1,
            Arrayfree(arr),
            "STR desc missing: expected -1, got %d", idx
        );
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ========== V64 (FS) ========== */
    test_sub("subtest %d: V64 FS find existing", ++subnum);
    {
        Array arr = V64Array_create(4, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        arr.v64[0] = value64_createfs_asstr("/alpha");
        arr.v64[1] = value64_createfs_asstr("/beta");
        arr.v64[2] = value64_createfs_asstr("/gamma");
        arr.v64[3] = value64_createfs_asstr("/delta");

        // предварительно отсортируем
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        // после сортировки: alpha, beta, delta, gamma

        value64 key = value64_createfs_asstr("/beta");
        int idx;
        test_validatefree(
            (idx = ArrayBsearchV64(arr, key)) == 1,
            (Arrayfree(arr), value64_freefs(&key)),
            "FS asc: expected idx=1, got %d", idx
        );
        value64_freefs(&key);
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ========== Type mismatch (должно вызывать ошибку) ========== */
    test_sub("subtest %d: type mismatch raises SIGINT", ++subnum);
    {
        Array arr = IArray_create(3, ARRAY_FILLTYPE_ASC_SERIES);
        if (!try()) {
            ArrayBsearchLong(arr, 5L);   // LONG на INT массиве
            test_validatefree(false, Arrayfree(arr), "Should have raised SIGINT");
        } else {
            logsimple("Exception correctly raised on type mismatch");
        }
        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST CARray (char) create/fill/free -------------------------
static TestStatus
tf_carray_create_fill_free(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Создание пустого CHAR массива */
    test_sub("subtest %d: create empty CHAR array", ++subnum);
    {
        Array arr = CArray_create(0, ARRAY_FILLTYPE_NONE);
        test_validatefree(
            arr.len == 0 && arr.sz == 0 && arr.cv == NULL,
            Arrayfree(arr),
            "Empty CHAR array: len=%d, sz=%d, cv=%p (expected 0,0,NULL)",
            arr.len, arr.sz, (void*)arr.cv
        );
        Arrayfree(arr);
    }

    /* 2. Создание ZERO‑filled CHAR массива */
    test_sub("subtest %d: create ZERO‑filled CHAR array", ++subnum);
    {
        Array arr = CArray_create(5, ARRAY_FILLTYPE_ZERO);
        test_validatefree(
            arr.len == 5 && arr.sz >= 5,
            Arrayfree(arr),
            "ZERO CHAR array: len=%d, sz=%d (expected 5, >=5)", arr.len, arr.sz
        );
        // Проверяем, что все элементы — '\0'
        for (int i = 0; i < 5; i++) {
            test_validatefree(
                arr.cv[i] == '\0',
                Arrayfree(arr),
                "CHAR[%d] must be '\\0', got '%c'", i, arr.cv[i]
            );
        }
        Arrayfree(arr);
    }

    /* 3. Создание ASC‑filled CHAR массива (случайные буквы) */
    test_sub("subtest %d: create ASC‑filled CHAR array", ++subnum);
    {
        Array arr = CArray_create(4, ARRAY_FILLTYPE_ASC);
        test_validatefree(
            arr.len == 4,
            Arrayfree(arr),
            "ASC CHAR array: len=%d (expected 4)", arr.len
        );
        // Элементы не должны быть '\0' и должны следовать в алфавитном порядке
        for (int i = 1; i < 4; i++) {
            test_validatefree(
                arr.cv[i - 1] <= arr.cv[i],
                Arrayfree(arr),
                "ASC CHAR: must be non‑decreasing"
            );
        }
        Arrayfree(arr);
    }

    /* 4. Создание DESC‑filled CHAR массива (случайные буквы в обратном порядке) */
    test_sub("subtest %d: create DESC‑filled CHAR array", ++subnum);
    {
        Array arr = CArray_create(4, ARRAY_FILLTYPE_DESC);
        test_validatefree(
            arr.len == 4,
            Arrayfree(arr),
            "DESC CHAR array: len=%d (expected 4)", arr.len
        );
        // Элементы должны быть не возрастающими
        for (int i = 1; i < 4; i++) {
            test_validatefree(
                arr.cv[i - 1] >= arr.cv[i],
                Arrayfree(arr),
                "DESC CHAR: must be non‑increasing"
            );
        }
        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST CARray (char) sort -------------------------
static TestStatus
tf_carray_sort(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Сортировка по возрастанию */
    test_sub("subtest %d: CHAR sort ASC", ++subnum);
    {
        Array arr = CArray_create(6, ARRAY_FILLTYPE_RND);
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);

        for (int i = 1; i < arr.len; i++) {
            test_validatefree(
                arr.cv[i - 1] <= arr.cv[i],
                Arrayfree(arr),
                "ASC CHAR: cv[%d]='%c' > cv[%d]='%c'",
                i - 1, arr.cv[i - 1], i, arr.cv[i]
            );
        }
        Arrayfree(arr);
    }

    /* 2. Сортировка по убыванию */
    test_sub("subtest %d: CHAR sort DESC", ++subnum);
    {
        Array arr = CArray_create(6, ARRAY_FILLTYPE_RND);
        Array_qsort(arr, ARRAY_FILLTYPE_DESC);

        for (int i = 1; i < arr.len; i++) {
            test_validatefree(
                arr.cv[i - 1] >= arr.cv[i],
                Arrayfree(arr),
                "DESC CHAR: cv[%d]='%c' < cv[%d]='%c'",
                i - 1, arr.cv[i - 1], i, arr.cv[i]
            );
        }
        Arrayfree(arr);
    }

    /* 3. Пустой массив */
    test_sub("subtest %d: CHAR sort empty", ++subnum);
    {
        Array arr = CArray_create(0, ARRAY_FILLTYPE_NONE);
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);   // не должно упасть
        test_validatefree(arr.len == 0, Arrayfree(arr), "Empty array must stay empty after sort");
        Arrayfree(arr);
    }

    /* 4. Один элемент */
    test_sub("subtest %d: CHAR sort single element", ++subnum);
    {
        Array arr = CArray_create(1, ARRAY_FILLTYPE_NONE);
        arr.cv[0] = 'x';
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        test_validatefree(
            arr.cv[0] == 'x',
            Arrayfree(arr),
            "Single element 'x' must stay 'x', got '%c'", arr.cv[0]
        );
        Arrayfree(arr);
    }

    /* 5. Уже отсортированный */
    test_sub("subtest %d: CHAR sort already sorted", ++subnum);
    {
        Array arr = CArray_create(3, ARRAY_FILLTYPE_NONE);
        arr.cv[0] = 'a'; arr.cv[1] = 'b'; arr.cv[2] = 'c';
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        test_validatefree(
            arr.cv[0] == 'a' && arr.cv[1] == 'b' && arr.cv[2] == 'c',
            Arrayfree(arr),
            "Already sorted array must stay 'a','b','c'"
        );
        Arrayfree(arr);
    }

    /* 6. Дубликаты */
    test_sub("subtest %d: CHAR sort duplicates", ++subnum);
    {
        Array arr = CArray_create(4, ARRAY_FILLTYPE_NONE);
        arr.cv[0] = 'b'; arr.cv[1] = 'a'; arr.cv[2] = 'b'; arr.cv[3] = 'c';
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        test_validatefree(
            arr.cv[0] == 'a' && arr.cv[1] == 'b' && arr.cv[2] == 'b' && arr.cv[3] == 'c',
            Arrayfree(arr),
            "Duplicates must be sorted correctly"
        );
        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST ArrayBsearch for CHAR / V64 (STR / FS) -------------------------
static TestStatus
tf_array_bsearch_char(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== CHAR ========== */
    test_sub("subtest %d: CHAR find existing", ++subnum);
    {
        Array arr = CArray_create(5, ARRAY_FILLTYPE_NONE);
        arr.cv[0] = 'a'; arr.cv[1] = 'b'; arr.cv[2] = 'c'; arr.cv[3] = 'd'; arr.cv[4] = 'e';
        // массив уже отсортирован по возрастанию
        int idx = ArrayBsearchChar(arr, 'c');
        test_validatefree(
            idx == 2,
            Arrayfree(arr),
            "CHAR asc: expected idx=2, got %d", idx
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: CHAR find missing", ++subnum);
    {
        Array arr = CArray_create(5, ARRAY_FILLTYPE_NONE);
        arr.cv[0] = 'a'; arr.cv[1] = 'b'; arr.cv[2] = 'c'; arr.cv[3] = 'd'; arr.cv[4] = 'e';
        int idx = ArrayBsearchChar(arr, 'z');
        test_validatefree(
            idx == -1,
            Arrayfree(arr),
            "CHAR asc missing: expected -1, got %d", idx
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: CHAR rev search", ++subnum);
    {
        Array arr = CArray_create(5, ARRAY_FILLTYPE_NONE);
        arr.cv[0] = 'e'; arr.cv[1] = 'd'; arr.cv[2] = 'c'; arr.cv[3] = 'b'; arr.cv[4] = 'a';
        int idx = ArrayBsearchCharRev(arr, 'b');
        test_validatefree(
            idx == 3,   // e(0),d(1),c(2),b(3),a(4)
            Arrayfree(arr),
            "CHAR desc: expected idx=3, got %d", idx
        );
        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST Array save/load (CHAR) -------------------------
static TestStatus
tf_array_save_load_char(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== CHAR ========== */
    test_sub("subtest %d: CHAR save/load", ++subnum);
    {
        const char *fname = "res/array/carr.sv";

        // создаём массив и заполняем
        Array orig = CArray_create(5, ARRAY_FILLTYPE_NONE);
        orig.cv[0] = 'h'; orig.cv[1] = 'e'; orig.cv[2] = 'l';
        orig.cv[3] = 'l'; orig.cv[4] = 'o';

        // сохраняем
        long written = Array_save(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "CHAR save failed");

        // загружаем
        Array loaded = Array_load(fname);
        test_validatefree(
            loaded.len == orig.len,
            (Arrayfree(orig), Arrayfree(loaded)),
            "CHAR load: len=%d, expected %d", loaded.len, orig.len
        );

        // сравниваем поэлементно
        for (int i = 0; i < orig.len; i++) {
            test_validatefree(
                orig.cv[i] == loaded.cv[i],
                (Arrayfree(orig), Arrayfree(loaded)),
                "CHAR[%d]: orig='%c', loaded='%c'",
                i, orig.cv[i], loaded.cv[i]
            );
        }

        Arrayfree(orig);
        Arrayfree(loaded);
    }

    /* ========== CHAR: пустой массив ========== */
    test_sub("subtest %d: CHAR save/load empty", ++subnum);
    {
        const char *fname = "res/array/carr_empty.sv";

        Array orig = CArray_create(0, ARRAY_FILLTYPE_NONE);
        long written = Array_save(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "CHAR empty save failed");

        Array loaded = Array_load(fname);
        test_validatefree(
            loaded.len == 0 && loaded.cv == NULL,
            (Arrayfree(orig), Arrayfree(loaded)),
            "Loaded empty CHAR: len=%d, cv=%p (expected 0, NULL)", loaded.len, loaded.cv
        );

        Arrayfree(orig);
        Arrayfree(loaded);
    }

    /* ========== CHAR: один элемент ========== */
    test_sub("subtest %d: CHAR save/load single", ++subnum);
    {
        const char *fname = "res/array/carr_single.sv";

        Array orig = CArray_create(1, ARRAY_FILLTYPE_NONE);
        orig.cv[0] = 'Z';
        long written = Array_save(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "CHAR single save failed");

        Array loaded = Array_load(fname);
        test_validatefree(
            loaded.len == 1 && loaded.cv[0] == 'Z',
            (Arrayfree(orig), Arrayfree(loaded)),
            "Loaded single CHAR: expected 'Z', got '%c'", loaded.cv[0]
        );

        Arrayfree(orig);
        Arrayfree(loaded);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST ArrayEq / ArrayNoteq (all types, edge cases) -------------------------
static TestStatus
tf_array_eq_noteq(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== INT ========== */
    test_sub("subtest %d: INT equal", ++subnum);
    {
        Array a = IArray_create(3, ARRAY_FILLTYPE_NONE);
        Array b = IArray_create(3, ARRAY_FILLTYPE_NONE);
        a.iv[0] = 1; a.iv[1] = 2; a.iv[2] = 3;
        b.iv[0] = 1; b.iv[1] = 2; b.iv[2] = 3;
        test_validatefree(
            ArrayEq(&a, &b) && !ArrayNoteq(&a, &b),
            (Arrayfree(a), Arrayfree(b)),
            "Identical INT arrays must be equal"
        );
        Arrayfree(a); Arrayfree(b);
    }

    test_sub("subtest %d: INT not equal (different values)", ++subnum);
    {
        Array a = IArray_create(2, ARRAY_FILLTYPE_NONE);
        Array b = IArray_create(2, ARRAY_FILLTYPE_NONE);
        a.iv[0] = 10; a.iv[1] = 20;
        b.iv[0] = 10; b.iv[1] = 30;
        test_validatefree(
            !ArrayEq(&a, &b) && ArrayNoteq(&a, &b),
            (Arrayfree(a), Arrayfree(b)),
            "Different values must be not equal"
        );
        Arrayfree(a); Arrayfree(b);
    }

    test_sub("subtest %d: INT not equal (different lengths)", ++subnum);
    {
        Array a = IArray_create(2, ARRAY_FILLTYPE_NONE);
        Array b = IArray_create(3, ARRAY_FILLTYPE_NONE);
        test_validatefree(
            !ArrayEq(&a, &b) && ArrayNoteq(&a, &b),
            (Arrayfree(a), Arrayfree(b)),
            "Different lengths must be not equal"
        );
        Arrayfree(a); Arrayfree(b);
    }

    test_sub("subtest %d: INT empty arrays", ++subnum);
    {
        Array a = IArray_create(0, ARRAY_FILLTYPE_NONE);
        Array b = IArray_create(0, ARRAY_FILLTYPE_NONE);
        test_validatefree(
            ArrayEq(&a, &b) && !ArrayNoteq(&a, &b),
            (Arrayfree(a), Arrayfree(b)),
            "Empty INT arrays must be equal"
        );
        Arrayfree(a); Arrayfree(b);
    }

    /* ========== CHAR ========== */
    test_sub("subtest %d: CHAR equal", ++subnum);
    {
        Array a = CArray_create(2, ARRAY_FILLTYPE_NONE);
        Array b = CArray_create(2, ARRAY_FILLTYPE_NONE);
        a.cv[0] = 'x'; a.cv[1] = 'y';
        b.cv[0] = 'x'; b.cv[1] = 'y';
        test_validatefree(
            ArrayEq(&a, &b) && !ArrayNoteq(&a, &b),
            (Arrayfree(a), Arrayfree(b)),
            "Identical CHAR arrays must be equal"
        );
        Arrayfree(a); Arrayfree(b);
    }

    test_sub("subtest %d: CHAR not equal", ++subnum);
    {
        Array a = CArray_create(1, ARRAY_FILLTYPE_NONE);
        Array b = CArray_create(1, ARRAY_FILLTYPE_NONE);
        a.cv[0] = 'a';
        b.cv[0] = 'b';
        test_validatefree(
            !ArrayEq(&a, &b) && ArrayNoteq(&a, &b),
            (Arrayfree(a), Arrayfree(b)),
            "Different CHAR must be not equal"
        );
        Arrayfree(a); Arrayfree(b);
    }

    /* ========== V64 STR ========== */
    test_sub("subtest %d: V64 STR equal", ++subnum);
    {
        Array a = V64Array_create(2, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        Array b = V64Array_create(2, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        a.v64[0] = value64_createstr("hello");
        a.v64[1] = value64_createstr("world");
        b.v64[0] = value64_createstr("hello");
        b.v64[1] = value64_createstr("world");
        test_validatefree(
            ArrayEq(&a, &b) && !ArrayNoteq(&a, &b),
            (Arrayfree(a), Arrayfree(b)),
            "Identical STR arrays must be equal"
        );
        Arrayfree(a); Arrayfree(b);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: V64 STR not equal", ++subnum);
    {
        Array a = V64Array_create(1, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        Array b = V64Array_create(1, ARRAY_FILLTYPE_NONE, VALUE64_STR);
        a.v64[0] = value64_createstr("abc");
        b.v64[0] = value64_createstr("xyz");
        test_validatefree(
            !ArrayEq(&a, &b) && ArrayNoteq(&a, &b),
            (Arrayfree(a), Arrayfree(b)),
            "Different STR must be not equal"
        );
        Arrayfree(a); Arrayfree(b);
    }
    fs_alloc_check(true);

    /* ========== V64 FS ========== */
    test_sub("subtest %d: V64 FS equal", ++subnum);
    {
        Array a = V64Array_create(2, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        Array b = V64Array_create(2, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        a.v64[0] = value64_createfs_asstr("/tmp/a");
        a.v64[1] = value64_createfs_asstr("/tmp/b");
        b.v64[0] = value64_createfs_asstr("/tmp/a");
        b.v64[1] = value64_createfs_asstr("/tmp/b");
        test_validatefree(
            ArrayEq(&a, &b) && !ArrayNoteq(&a, &b),
            (Arrayfree(a), Arrayfree(b)),
            "Identical FS arrays must be equal"
        );
        Arrayfree(a); Arrayfree(b);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: V64 FS not equal", ++subnum);
    {
        Array a = V64Array_create(1, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        Array b = V64Array_create(1, ARRAY_FILLTYPE_NONE, VALUE64_FS);
        a.v64[0] = value64_createfs_asstr("/first");
        b.v64[0] = value64_createfs_asstr("/second");
        test_validatefree(
            !ArrayEq(&a, &b) && ArrayNoteq(&a, &b),
            (Arrayfree(a), Arrayfree(b)),
            "Different FS must be not equal"
        );
        Arrayfree(a); Arrayfree(b);
    }
    fs_alloc_check(true);

    /* ========== Type mismatch (must raise SIGINT) ========== */
    test_sub("subtest %d: type mismatch raises SIGINT", ++subnum);
    {
        Array a = IArray_create(1, ARRAY_FILLTYPE_NONE);
        Array b = CArray_create(1, ARRAY_FILLTYPE_NONE);
        if (!try()) {
            ArrayNoteq(&a, &b);
            test_validatefree(false, (Arrayfree(a), Arrayfree(b)),
                             "Type mismatch should have raised SIGINT");
        } else {
            logsimple("Exception correctly raised on type mismatch");
        }
        Arrayfree(a); Arrayfree(b);
    }

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
        TESTADD(tf_v64array_shrink_increase,    "V64Array (STR / FS) shrink / increase simple test"),
        TESTADD(tf_v64array_sort,               "V64Array (STR / FS) sorting simple test"),
        TESTADD(tf_v64array_save_load,          "V64Array STR/FS save/load simple test"),
        TESTADD(tf_array_bsearch,               "ArrayBsearch (INT / LONG / DBL / V64) simple test"),
        TESTADD(tf_carray_create_fill_free,     "CHAR create/fill/free simple test"),
        TESTADD(tf_carray_sort,                 "CHAR sorting simple test"),
        TESTADD(tf_array_bsearch_char,          "CHAR ArrayBsearch simple test"),
        TESTADD(tf_array_save_load_char,        "CHAR Array save/load simple test"),
        TESTADD(tf_array_eq_noteq,              "ArrayEq / ArrayNoteq (all types, edge cases)")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* ARRAYTESTING */

