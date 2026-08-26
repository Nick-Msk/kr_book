// only array.h
#include "array.h"


/********************************************************************
                 ARRAY MODULE IMPLEMENTATION
********************************************************************/

//  globals, can be changed by app

// TODO: context must be used for that
int                      g_array_rec_line        = 20;  // TODO: rework that to normal (in Array structure)
const char              *g_custom_print_line     = 0;   // TODO: rework that to normal (in Array structure)
// TODO: move into context
const char              *g_save_format_double    = "%6d      %15.15lg\n";
const char              *g_save_format_int       = "%6d\t%6d\n";
const char              *g_save_format_long      = "%6d\t%6ld\n";
const char              *g_save_format_pointer   = "%6d\t%p\n";
const char              *g_save_format_char      = "%6d\t%c\n";
// not possible to format v64 that way!
static const int        g_array_acs_rndinc      = 5;
static const int        g_array_desc_rndinc     = 5;


#define                         ARRAY_MAX_TYPE_STR          20
#define                         ARRAY_MAX_TYPE_STR_WO_LAST  19

// internal type

// ---------- pseudo-header for utility procedures -----------------

// ------------------------------ Utilities ------------------------

// factory-wrappers -- ACS (rnd) series
static v64Gen                   f_v64_acs_int(long c, long s, int i) { 
    return v64GenCreatorAscRnd(v64typedCreateInt(s), c, i); 
}
static v64Gen                   f_v64_acs_long(long c, long s, int i) {
    return v64GenCreatorAscRnd(v64typedCreateLong(s), c, i); 
}
static v64Gen                   f_v64_acs_ulong(long c, long s, int i) {
    return v64GenCreatorAscRnd(v64typedCreateULong(s), c, i); 
}
static v64Gen                   f_v64_acs_dbl(long c, long s, int i)  { 
    return v64GenCreatorAscRnd(v64typedCreateDbl(s), c, i); 
}
static v64Gen                   f_v64_acs_chr(long c, long s, int i)  { 
    return v64GenCreatorAscRnd(v64typedCreateChar(ucharmax(s)), c, i); 
}
static v64Gen                   f_v64_acs_bool(long c, long s, int i) { 
    return v64GenCreatorAscRnd(v64typedCreateBool(s != 0), c, i); 
}
static v64Gen                   f_v64_acs_str(long c, long s, int i)  { 
    return v64GenCreatorAscStrRnd(c, s, "%d", i); 
}
static v64Gen                   f_v64_acs_fs(long c, long s, int i)   { 
    return v64GenCreatorAscFsRnd(c, s, "%d", i); 
}
// factory-wrappers -- DESC (rnd) series
static v64Gen                   f_v64_desc_int(long c, long s, int i) { 
    return v64GenCreatorDescRnd(v64typedCreateInt(s), c, i); 
}
static v64Gen                   f_v64_desc_long(long c, long s, int i) {
    return v64GenCreatorDescRnd(v64typedCreateLong(s), c, i); 
}
static v64Gen                   f_v64_desc_ulong(long c, long s, int i) {
    return v64GenCreatorDescRnd(v64typedCreateULong(s), c, i); 
}
static v64Gen                   f_v64_desc_dbl(long c, long s, int i)  { 
    return v64GenCreatorDescRnd(v64typedCreateDbl(s), c, i); 
}
static v64Gen                   f_v64_desc_chr(long c, long s, int i)  { 
    return v64GenCreatorDescRnd(v64typedCreateChar(ucharmax(s)), c, i); 
}
static v64Gen                   f_v64_desc_bool(long c, long s, int i) { 
    return v64GenCreatorDescRnd(v64typedCreateBool(s != 0), c, i); 
}
static v64Gen                   f_v64_desc_str(long c, long s, int i)  { 
    return v64GenCreatorDescStrRnd(c, s, "%d", i); 
}
static v64Gen                   f_v64_desc_fs(long c, long s, int i)   { 
    return v64GenCreatorDescFsRnd(c, s, "%d", i); 
}
// factory-wrappers -- ACS series (1, 2, 3 ... etc)
static v64Gen                   f_v64_acs_series_int(long c, long s, int i) {
    (void) i;
    return v64GenCreatorAscSeries(v64typedCreateInt(s), c); 
}
static v64Gen                   f_v64_acs_series_long(long c, long s, int i) {
    (void) i;
    return v64GenCreatorAscSeries(v64typedCreateLong(s), c); 
}
static v64Gen                   f_v64_acs_series_ulong(long c, long s, int i) {
    (void) i;
    return v64GenCreatorAscSeries(v64typedCreateULong(s), c); 
}
static v64Gen                   f_v64_acs_series_dbl(long c, long s, int i)  { 
    (void) i;
    return v64GenCreatorAscSeries(v64typedCreateDbl(s), c); 
}
static v64Gen                   f_v64_acs_series_chr(long c, long s, int i)  { 
    (void) i;
    return v64GenCreatorAscSeries(v64typedCreateChar(ucharmax(s)), c); 
}
static v64Gen                   f_v64_acs_series_bool(long c, long s, int i) { 
    (void) i;
    return v64GenCreatorAscSeries(v64typedCreateBool(s != 0), c); 
}
static v64Gen                   f_v64_acs_series_str(long c, long s, int i) { 
    (void) i;
    return v64GenCreatorAscStrSeries(c, s, "%d"); 
}
static v64Gen                   f_v64_acs_series_fs(long c, long s, int i) {
    (void) i; 
    return v64GenCreatorAscFsSeries(c, s, "%d"); 
}
// factory-wrappers -- DESC series (5, 4, 3 ... etc)
static v64Gen                   f_v64_desc_series_int(long c, long s, int i) {
    (void) i;
    return v64GenCreatorDescSeries(v64typedCreateInt(s), c); 
}
static v64Gen                   f_v64_desc_series_long(long c, long s, int i) {
    (void) i;
    return v64GenCreatorDescSeries(v64typedCreateLong(s), c); 
}
static v64Gen                   f_v64_desc_series_ulong(long c, long s, int i) {
    (void) i;
    return v64GenCreatorDescSeries(v64typedCreateULong(s), c); 
}
static v64Gen                   f_v64_desc_series_dbl(long c, long s, int i)  { 
    (void) i;
    return v64GenCreatorDescSeries(v64typedCreateDbl(s), c); 
}
static v64Gen                   f_v64_desc_series_chr(long c, long s, int i)  { 
    (void) i;
    return v64GenCreatorDescSeries(v64typedCreateChar(ucharmax(s)), c); 
}
static v64Gen                   f_v64_desc_series_bool(long c, long s, int i) { 
    (void) i;
    return v64GenCreatorDescSeries(v64typedCreateBool(s != 0), c); 
}
static v64Gen                   f_v64_desc_series_str(long c, long s, int i) { 
    (void) i;
    return v64GenCreatorDescStrSeries(c, s, "%d"); 
}
static v64Gen                   f_v64_desc_series_fs(long c, long s, int i) {
    (void) i; 
    return v64GenCreatorDescFsSeries(c, s, "%d"); 
}
// --- RND Master Factories (Internal) ---
// Параметры: (count, start, increment)
static v64Gen                   f_v64_rnd_int(long c, long s, int i)  { 
    (void) s;
    return v64GenCreatorRnd(VALUE64_INT, c, i); 
}
static v64Gen                   f_v64_rnd_long(long c, long s, int i) { 
    (void) s;
    return v64GenCreatorRnd(VALUE64_LONG, c, i); 
}
static v64Gen                   f_v64_rnd_ulong(long c, long s, int i) { 
    (void) s;
    return v64GenCreatorRnd(VALUE64_ULONG, c, i); 
}
static v64Gen                   f_v64_rnd_dbl(long c, long s, int i) { 
    (void) s;
    return v64GenCreatorRnd(VALUE64_DBL, c, i); 
}
static v64Gen                   f_v64_rnd_chr(long c, long s, int i) { 
    (void) s;
    return v64GenCreatorRnd(VALUE64_CHR, c, i); 
}
static v64Gen                   f_v64_rnd_bool(long c, long s, int i) { 
    (void) s;
    (void) i;
    return v64GenCreatorRnd(VALUE64_BOOL, c, 1); 
}
static v64Gen                   f_v64_rnd_str(long c, long s, int i) { 
    (void) s;
    return v64GenCreatorStrRnd(c, "%d", i); 
}
static v64Gen                   f_v64_rnd_fs(long c, long s, int i) { 
    (void) s;
    return v64GenCreatorFsRnd(c, "%d", i); 
}
// --- SAFE_EMPTY Master Factories (Internal) ---
static v64Gen                   f_v64_empty_provider_fs(long c, long s, int i) {
    (void) i;
    (void) s;
    return v64GenCreatorNull(VALUE64_FS, c);
}
static v64Gen                   f_v64_empty_provider_str(long c, long s, int i) {
    (void) i;
    (void) s;
    return v64GenCreatorNull(VALUE64_STR, c);
}
// --- ZERO Master Factories (Internal) ---
static v64Gen                   f_v64_zero_provider_int(long c, long s, int i) {
    (void) s; (void) i;
    return v64GenCreatorZero(VALUE64_INT, c);
}
static v64Gen                   f_v64_zero_provider_long(long c, long s, int i) {
    (void) s; (void) i;
    return v64GenCreatorZero(VALUE64_LONG, c);
}
static v64Gen                   f_v64_zero_provider_ulong(long c, long s, int i) {
    (void) s; (void) i;
    return v64GenCreatorZero(VALUE64_ULONG, c);
}
static v64Gen                   f_v64_zero_provider_dbl(long c, long s, int i) {
    (void) s; (void) i;
    return v64GenCreatorZero(VALUE64_DBL, c);
}
static v64Gen                   f_v64_zero_provider_chr(long c, long s, int i) {
    (void) s; (void) i;
    return v64GenCreatorZero(VALUE64_CHR, c);
}
static v64Gen                   f_v64_zero_provider_bool(long c, long s, int i) {
    (void) s; (void) i;
    return v64GenCreatorZero(VALUE64_BOOL, c);
}
static v64Gen                   f_v64_zero_provider_str(long c, long s, int i) {
    (void) s; (void) i;
    return v64GenCreatorZero(VALUE64_STR, c);
}
static v64Gen                   f_v64_zero_provider_fs(long c, long s, int i) {
    (void) s; (void) i;
    return v64GenCreatorZero(VALUE64_FS, c);
}
static v64Gen                   f_v64_zero_provider_ptr(long c, long s, int i) {
    (void) s; (void) i;
    return v64GenCreatorZero(VALUE64_PTR, c);
}





/*v64Gen gen = v64GenCreatorZero(
            ArrayGetV64mapType(parr), to - from);
    int cnt = ArrayGenPumprange(parr, &gen, from, to);
*/
/*
// ZERO (всегда возвращает ноль для любого типа)
static v64Gen f_zero(value64_type vt, long c, long s, int i)     { 
    return v64GenCreatorZero(vt, c); 
} */

static const v64GenTypedFactory         ARRAYTYPEINTERFACE[][ARRAY_FILLTYPE_MAX] = {
    [VALUE64_INT]   = { [ARRAY_FILLTYPE_ASC]         = f_v64_acs_int,   
                        [ARRAY_FILLTYPE_DESC]        = f_v64_desc_int,
                        [ARRAY_FILLTYPE_ASC_SERIES]  = f_v64_acs_series_int,   
                        [ARRAY_FILLTYPE_DESC_SERIES] = f_v64_desc_series_int,
                        [ARRAY_FILLTYPE_RND]         = f_v64_rnd_int,
                        [ARRAY_FILLTYPE_ZERO]        = f_v64_zero_provider_int
                    },
    [VALUE64_LONG]  = { [ARRAY_FILLTYPE_ASC]         = f_v64_acs_long,  
                        [ARRAY_FILLTYPE_DESC]        = f_v64_desc_long,
                        [ARRAY_FILLTYPE_ASC_SERIES]  = f_v64_acs_series_long,  
                        [ARRAY_FILLTYPE_DESC_SERIES] = f_v64_desc_series_long,
                        [ARRAY_FILLTYPE_RND]         = f_v64_rnd_long,
                        [ARRAY_FILLTYPE_ZERO]        = f_v64_zero_provider_long
                    },
    [VALUE64_ULONG] = { [ARRAY_FILLTYPE_ASC]         = f_v64_acs_ulong, 
                        [ARRAY_FILLTYPE_DESC]        = f_v64_desc_ulong,
                        [ARRAY_FILLTYPE_ASC_SERIES]  = f_v64_acs_series_ulong, 
                        [ARRAY_FILLTYPE_DESC_SERIES] = f_v64_desc_series_ulong,
                        [ARRAY_FILLTYPE_RND]         = f_v64_rnd_ulong,
                        [ARRAY_FILLTYPE_ZERO]        = f_v64_zero_provider_ulong
                    },
    [VALUE64_DBL]   = { [ARRAY_FILLTYPE_ASC]         = f_v64_acs_dbl,   
                        [ARRAY_FILLTYPE_DESC]        = f_v64_desc_dbl,
                        [ARRAY_FILLTYPE_ASC_SERIES]  = f_v64_acs_series_dbl,   
                        [ARRAY_FILLTYPE_DESC_SERIES] = f_v64_desc_series_dbl,
                        [ARRAY_FILLTYPE_RND]         = f_v64_rnd_dbl,
                        [ARRAY_FILLTYPE_ZERO]        = f_v64_zero_provider_dbl
                    },
    [VALUE64_CHR]   = { [ARRAY_FILLTYPE_ASC]         = f_v64_acs_chr,   
                        [ARRAY_FILLTYPE_DESC]        = f_v64_desc_chr,
                        [ARRAY_FILLTYPE_ASC_SERIES]  = f_v64_acs_series_chr,   
                        [ARRAY_FILLTYPE_DESC_SERIES] = f_v64_desc_series_chr,
                        [ARRAY_FILLTYPE_RND]         = f_v64_rnd_chr,
                        [ARRAY_FILLTYPE_ZERO]        = f_v64_zero_provider_chr
                    },
    [VALUE64_BOOL]  = { [ARRAY_FILLTYPE_ASC]         = f_v64_acs_bool,  
                        [ARRAY_FILLTYPE_DESC]        = f_v64_desc_bool,
                        [ARRAY_FILLTYPE_ASC_SERIES]  = f_v64_acs_series_bool,  
                        [ARRAY_FILLTYPE_DESC_SERIES] = f_v64_desc_series_bool,
                        [ARRAY_FILLTYPE_RND]         = f_v64_rnd_bool,
                        [ARRAY_FILLTYPE_ZERO]        = f_v64_zero_provider_bool
                    },
    [VALUE64_STR]   = { [ARRAY_FILLTYPE_ASC]         = f_v64_acs_str,   
                        [ARRAY_FILLTYPE_DESC]        = f_v64_desc_str,
                        [ARRAY_FILLTYPE_ASC_SERIES]  = f_v64_acs_series_str,   
                        [ARRAY_FILLTYPE_DESC_SERIES] = f_v64_desc_series_str,
                        [ARRAY_FILLTYPE_RND]         = f_v64_rnd_str,
                        [ARRAY_FILLTYPE_SAFE_EMPTY]  = f_v64_empty_provider_str,
                        [ARRAY_FILLTYPE_ZERO]        = f_v64_zero_provider_str
                    },
    [VALUE64_FS]    = { [ARRAY_FILLTYPE_ASC]         = f_v64_acs_fs,    
                        [ARRAY_FILLTYPE_DESC]        = f_v64_desc_fs,
                        [ARRAY_FILLTYPE_ASC_SERIES]  = f_v64_acs_series_fs,    
                        [ARRAY_FILLTYPE_DESC_SERIES] = f_v64_desc_series_fs,
                        [ARRAY_FILLTYPE_RND]         = f_v64_rnd_fs,
                        [ARRAY_FILLTYPE_SAFE_EMPTY]  = f_v64_empty_provider_fs,
                        [ARRAY_FILLTYPE_ZERO]        = f_v64_zero_provider_fs
                    },
    [VALUE64_PTR]   = { [ARRAY_FILLTYPE_ZERO]        = f_v64_zero_provider_ptr
                    },
    [VALUE64_UNKNOWN] = {0} 
};

// // internal 
// static v64GenTypedFactory         *getTypedInterface(value64_type vt64) {
//     if (vt64 < 0 || vt64 >= COUNT(ARRAYTYPEINTERFACE))
//         vt64 = VALUE64_UNKNOWN;
//     return ARRAYTYPEINTERFACE + vt64;
// }
// internal
static v64GenTypedFactory          getTypedFillFactory(value64_type vt64, ArrayFillType ft) {
    if (vt64 < 0 || vt64 >= COUNT(ARRAYTYPEINTERFACE) || ft < 0 || ft >= ARRAY_FILLTYPE_MAX)
        NULL;
    return ARRAYTYPEINTERFACE[vt64][ft];
}

/**
 * @brief Allocates and initializes a new Array descriptor.
 * 
 * @details This is a low-level constructor that allocates memory for the 
 *          Array structure itself. It uses @ref Array_init to set the 
 *          initial state of the descriptor. 
 * 
 * @note This function does NOT allocate the data buffer (the `v` pointer). 
 *       It only prepares the container structure. The caller is responsible 
 *       for the lifetime of the returned pointer and must call @ref Array_free 
 *       to prevent memory leaks.
 * 
 * @param typ The primary type assigned to the array (e.g., ARRAY_INT, ARRAY_V64).
 * @param vt  The specialized value64 subtype (only relevant if `typ` is ARRAY_V64).
 * 
 * @return Array* A pointer to the newly allocated Array structure. 
 *         Returns NULL (via @ref userraise) if memory allocation fails.
 */
static inline Array             *arraycreate(ArrayType typ, value64_type vt) {
    Array *arr = malloc(sizeof(Array));
    if (!arr)
        return userraise(NULL, ERR_UNABLE_ALLOCATE, "Unable create Array structure");
    *arr = Array_init(.flags = typ, .v64type = vt);
    return arr;
}

static inline void             fixbysz(Array *parr, int *pos) {
    if (*pos < 0) { // TODO: remove that after switch to size_t
        logsimple("postion %d is out of bound, cut to %d", *pos, 0);
        *pos = 0;
    }
    if (*pos > Arraysz(parr)) {
        logsimple("postion %d is out of bound, cut to sz %d", *pos, Arraysz(parr));
        *pos = Arraysz(parr);
    }
}
static inline void             fixbylen(Array *parr, int *pos) {
    if (*pos < 0) { // TODO: remove that after switch to size_t
        logsimple("postion %d is out of bound, cut to %d", *pos, 0);
        *pos = 0;
    }
    if (*pos > Arraysz(parr)) {
        logsimple("postion %d is out of bound, cut to sz %d", *pos, Arraysz(parr));
        *pos = Arraysz(parr);
    }
}
static inline void              fixrangesbysz(Array *parr, int *from, int *to) {
    fixbysz(parr, from);
    fixbysz(parr, to);
    // from > to isn't checker for now
}
static inline void              fixrangesbylen(Array *parr, int *from, int *to) {
    fixbylen(parr, from);
    fixbylen(parr, to);
    // from > to isn't checker for now
}

/// @brief free value64 elements of array
/// @param arr pointer to array
/// @param from from
/// @param to to
static void                     freeV64elems(Array *parr, int from, int to) {
    invraisecode(ERR_NULLABLE_PTR, parr != NULL, "Null pointer");

    fixrangesbysz(parr, &from, &to);
    if (parr->v64type == VALUE64_STR || parr->v64type == VALUE64_FS) {   
        for (int i = from; i < to; i++) {
            value64free(parr->v64[i], parr->v64type);
        }
        logsimple("freed %s  %d - %d", value64_typename(parr->v64type), from, to);
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

    int bytes = newsz * ArrayGetelemsize(arr);
    if (bytes < 0) 
        return userraise(-1, ERR_UNKNOWN_TYPE, "Unknown type");

    if (newsz < arr->len)
        freeV64elems(arr, newsz, arr->len);    
    logmsg("Arr: bytes=%d, sz=%d", bytes, newsz);
    void *p = NULL;  
    if (bytes > 0) {
        if ( (p = realloc(arr->v, bytes) ) == NULL)
            userraise(-1, ERR_UNABLE_ALLOCATE, "Unable to allocate %d", bytes);
    } else
        free(arr->v);
    arr->v = p; // iv/dv/pv... is the same
    if (arr->len > newsz)   // shrink case, 0 if newsz == 0 (free)
        arr->len = newsz;
    arr->sz = newsz;
    return logret(arr->sz, "New sz %d", arr->sz);
}

/**
 * @brief Internal utility to shift a block of elements within the array buffer.
 *
 * This function uses memmove to safely handle overlapping memory regions.
 * It is a low-level primitive used for both deletions and insertions.
 *
 * @param parr      Pointer to the array.
 * @param dest_idx  The target starting index for the block.
 * @param src_idx   The original starting index of the block.
 * @param cnt       The number of elements to move.
 * @return          The number of elements moved.
 */
static int                  moveelem(Array *parr, int dest_idx, int src_idx, int cnt) {
    invraisecode(parr != NULL, ERR_NULLABLE_PTR, "Null array pointer");

    size_t es = ArrayGetelemsize(parr);

    // memmove is used to safely handle overlapping memory regions
    memmove((char *)parr->v + (dest_idx * es), 
            (char *)parr->v + (src_idx * es), 
            cnt * es);
    
    return cnt;
}

/**
 * @brief Loads array elements from a text stream.
 *
 * The array header must already be read, and the array must be correctly
 * created before calling this function.
 *
 * @param in  input stream, already opened for reading
 * @param arr pointer to the array to fill
 * @return positive value in suceess, -1 if failed 
 */
static int                          ArrayFileLoadValues(FILE *restrict in, Array *restrict parr) {
    ArrayType   typ = ArrayGettype(parr);
    fs          buf = FS();
    int         cnt = 0;
    
    Array_pforeach_idx(parr, i) {
        int         ind;
        if (fscanf(in, "%6d\t", &ind) != 1)
            return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Can't parse index");        
        if (ind < 0 || ind >= parr->len)
            return userraise(-1, ERR_OUT_OF_RANGE, "%d must be between 0 and %d", ind, parr->len);

        switch (typ) {
            case ARRAY_INT:
                if (fscanf(in, "%d\n", parr->iv + ind) != 1)
                    return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Unable to fscanf a int value");
                break;
            case ARRAY_LONG:
                if (fscanf(in, "%ld\n", parr->lv + ind) != 1)
                    return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Unable to fscanf a long value");
                break;
            case ARRAY_DOUBLE:
                if (fscanf(in, "%lg\n", parr->dv + ind) != 1)
                    return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Unable to fscanf a double value");
                break;
            case ARRAY_POINTER:
                if (fscanf(in, "%p\n", parr->pv + ind) != 1)
                    return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Unable to fscanf a ptr value");
                break;
            case ARRAY_CHAR:
                if (fscanf(in, "%c\n", parr->cv + ind) != 1)
                    return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Unable to fscanf a char value");
                break;
            case ARRAY_V64:
                if (value64_loadfile(in, &parr->v64[ind], parr->v64type, true, &buf) != 1)
                    return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Unable to fscanf a V64 containered value");
                break;
            default:
                return userraise(-1, ERR_UNSUPPORTED_TYPE, "%d", typ);
        }
        cnt++;
    }
    fsfree(buf);
    return logsimpleret(cnt, "Readed %d", cnt);
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
static long                         ArraySaveValues(FILE *restrict out, const Array *restrict parr) {
    long        total = 0L;
    ArrayType   typ = ArrayGettype(parr);
    Array_pforeach_idx(parr, i)
        switch (typ) {
            case ARRAY_INT:
                IOCHECKER(written, fprintf(out, g_save_format_int, i, parr->iv[i]), -1)
                    total += written;
                break;
            case ARRAY_LONG:
                IOCHECKER(written, fprintf(out, g_save_format_long, i, parr->lv[i]), -1)
                    total += written;
                break;
            case ARRAY_DOUBLE:
                IOCHECKER(written, fprintf(out, g_save_format_double, i, parr->dv[i]), -1)
                    total += written;
                break;
            case ARRAY_POINTER:
                IOCHECKER(written, fprintf(out, g_save_format_pointer, i, parr->pv[i]), -1)
                    total += written;
                break;
            case ARRAY_CHAR:
                IOCHECKER(written, fprintf(out, g_save_format_char, i, parr->cv[i]), -1)
                    total += written;
                break;
            case ARRAY_V64:
                IOCHECKER(written, fprintf(out, "%6d\t", i), -1)   // to supply format
                    total += written;
                IOCHECKER(written, value64_tofile(out, parr->v64[i], parr->v64type, true), -1)
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
static long                 ArraySerializeValues(fs *restrict s, const Array *restrict parr) {
    long        total = 0L;
    ArrayType   typ = ArrayGettype(parr);

    Array_pforeach_idx(parr, i) {
        switch (typ) {
            case ARRAY_INT:
                IOCHECKER(written, fs_sprintf_concat(s, g_save_format_int, i, parr->iv[i]), -1)
                    total += written;
                break;
            case ARRAY_LONG:
                IOCHECKER(written, fs_sprintf_concat(s, g_save_format_long, i, parr->lv[i]), -1)
                    total += written;
                break;
            case ARRAY_DOUBLE:
                IOCHECKER(written, fs_sprintf_concat(s, g_save_format_double, i, parr->dv[i]), -1)
                    total += written;
                break;
            case ARRAY_POINTER:
                IOCHECKER(written, fs_sprintf_concat(s, g_save_format_pointer, i, parr->pv[i]), -1)
                    total += written;
                break;
            case ARRAY_CHAR:
                IOCHECKER(written, fs_sprintf_concat(s, g_save_format_char, i, parr->cv[i]), -1)
                    total += written;
                break;
            case ARRAY_V64: {
                //fs tmp = FS();      // TODO: rework to append logic
                total += value64_tostr(s, parr->v64[i], parr->v64type, true);
               //fs_cat(s, tmp);
                //fsfree(tmp);
                break;
            }
            default:
                userraise(-1, ERR_UNKNOWN_TYPE, "Unknown type %s", ArrayTypeGetName(typ));
        }
    }
    return total;
}

/**
 * @brief Loads array element values from a text string.
 *
 * Reads values from the string pointed to by `*pdata`, advancing the
 * pointer accordingly.  The array must already be created with the
 * correct type and length.
 *
 * @param pdata pointer to a string pointer
 * @param arr   pointer to the array to fill
 * @return count of bytes read
 */
static long             ArrayFsLoadValues(const char *restrict initdata, Array *restrict parr) {
    const char     *data = initdata;
    ArrayType       typ = ArrayGettype(parr);
    fs              buf = FS();

    //for (int i = 0; i < arr->len; i++) { // foreach
    Array_pforeach_idx(parr, i) {
        char             *endptr;
        
        long             lind = strtol(data, &endptr, 10);
        if (data == endptr)
            return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Can't parse index");        
        if (lind < 0 || lind >= (long) parr->len)
            return userraise(-1, ERR_OUT_OF_RANGE, "%ld must be between 0 and %d", lind, parr->len);
        data = endptr;
        int              ind = lind;
        switch (typ) {
            case ARRAY_INT:
                parr->iv[ind] = strtol(data, &endptr, 10);
                if (data == endptr)
                    return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Can't parse int value");
                data = endptr;
                break;
            case ARRAY_LONG:
                parr->lv[ind] = strtol(data, &endptr, 10);
                if (data == endptr)
                    return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Can't parse long value");
                data = endptr;                break;
            case ARRAY_DOUBLE:
                parr->dv[ind] = strtod(data, &endptr);
                if (data == endptr)
                    return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Can't parse double value");
                data = endptr;                break;
            case ARRAY_CHAR:
                data = skip_leading_spaces_nl(data);
                
                if (*data == '\0') 
                    return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Unexpected EO Line");

                parr->cv[ind] = *data; // Используем ind!
                data++; 

                // Пропускаем пробелы после символа, чтобы подготовить data к следующему strtol
                data = skip_leading_spaces_nl(data);
                break;
            case ARRAY_V64: {
                data += value64_loadstr(data, &parr->v64[ind], parr->v64type, true, &buf);
                break;
            }
            default:
                return userraise(-1, ERR_UNSUPPORTED_TYPE, "%d", typ);   // unsupported type
        }
    }
    fsfree(buf);
    return data - initdata; // total read
}

static Array                  *ArrayParseHeaderFile(FILE *in) {
    int                 cnt = 0;
    char                typ[ARRAY_MAX_TYPE_STR], v64typ[ARRAY_MAX_TYPE_STR] = "";

    // Read header: "ARRAY: <type> / <v64type> : <count>"
    if (fscanf(in, "ARRAY: %" TOSTRING(ARRAY_MAX_TYPE_STR_WO_LAST) "s / %" TOSTRING(ARRAY_MAX_TYPE_STR_WO_LAST) "s : %d ", 
                typ, 
                v64typ, 
                &cnt) != 3)
        userraiseint(ERR_WRONG_INPUT_FORMAT, "Array header wrong format");

    Array *parr;// = arraycreateempty();  //Array_init();       // zero-init
    ArrayType atype =  ArrayTypeFromName(typ);
    switch (atype) {
        case ARRAY_V64: {
            value64_type vt = value64_gettype(v64typ);
            if (vt == VALUE64_UNKNOWN)
                userraiseint(ERR_WRONG_INPUT_FORMAT, "Array header V64 wrong format '%s'", v64typ);
            parr = V64Array_create(cnt, ARRAY_FILLTYPE_SAFE_EMPTY, vt);
            break;
        }
        case ARRAY_INT:
            parr = IArray_create(cnt, ARRAY_FILLTYPE_SAFE_EMPTY);
            break;
        case ARRAY_LONG:
            parr = LArray_create(cnt, ARRAY_FILLTYPE_SAFE_EMPTY);
            break;
        case ARRAY_DOUBLE:
            parr = DArray_create(cnt, ARRAY_FILLTYPE_SAFE_EMPTY);
            break;
        case ARRAY_POINTER:
            parr = PArray_create(cnt, ARRAY_FILLTYPE_SAFE_EMPTY);
            break;
        case ARRAY_CHAR:
            parr = CArray_create(cnt, ARRAY_FILLTYPE_SAFE_EMPTY);
            break;
        default:
            parr = NULL;
            break;
    }
    if (!parr)
        return userraise(parr, ERR_UNSUPPORTED_TYPE, "Unsupported type '%s'", typ);
    else
        return parr;
}

static Array                   *ArrayParseHeaderStr(const char **base) {
    // ---------- 1. Parse header ----------
    char            typ[ARRAY_MAX_TYPE_STR], v64typ[ARRAY_MAX_TYPE_STR] = "";
    int             cnt = 0, header_len = 0;
    Array           *parr = NULL;  // = Array_init();
    const char     *data = *base;

    if (sscanf(data, "ARRAY: %" TOSTRING(ARRAY_MAX_TYPE_STR_WO_LAST) "s / %" TOSTRING(ARRAY_MAX_TYPE_STR_WO_LAST) "s : %d %n", typ, v64typ, &cnt, &header_len) != 3) {
        return userraise(parr, ERR_WRONG_INPUT_FORMAT, "ArrayLoadfs: header mismatch");
    } 
    data += header_len;

    // ---------- Create empty array ----------
    ArrayType       atype = ArrayTypeFromName(typ);
    value64_type    vt = value64_gettype(v64typ);
    // will set error flag if case of anything
    parr = ArrayOnlyCreate(cnt, atype, vt);
    if (!parr)  // 
        return userraise(parr, ERR_UNSUPPORTED_TYPE, "ArrayLoadfs: unsupported type '%s'", typ);

    *base = data;
    return parr;
}

static bool                     ArrayParseFooterFile(FILE *in) {
    char            typ[ARRAY_MAX_TYPE_STR];
    if (fscanf(in, " ARRAY: %" TOSTRING(ARRAY_MAX_TYPE_STR_WO_LAST) "s", typ) != 1 || strcmp(typ, "DONE") != 0)
        return userraise(false, ERR_WRONG_INPUT_FORMAT, "Wrong final piece '%s'", typ);
    else
        return true;
}

static bool                     ArrayParseFooterStr(const char **base) {
    const char     *data = *base;
    int             footer_len = 0;
    if (sscanf(data, "ARRAY: DONE%n", &footer_len) != 1) {
        return userraise(false, ERR_WRONG_INPUT_FORMAT, 
            "ArrayLoadfs: footer mismatch '%.30s'", data);
    }
    data += footer_len;
    *base = data;
    return true;
}

// -------------------------- (Utility) printers -------------------

// --------------------------- API ---------------------------------
// ------------- CONSTRUCTOTS/DESTRUCTORS --------------

// CREATE  and fill with method
Array                          *Array_create(int cnt, ArrayFillType filltyp, ArrayType typ, value64_type vt){
    logenter("cnt %d, filltyp %s typ %s", cnt, ArrayFillTypeName(filltyp), ArrayTypeGetName(typ) );
    // TODO: refactor via ArrayIncrease
    Array   *res = arraycreate(typ, vt);      
    if (cnt <= 0)
        return userraise(res, ERR_WRONG_INPUT_PARAMETERS, "sz = %d must be > 0", res->sz);

    if (increase(res, cnt) < 0)
        return userraise(NULL, ERR_UNABLE_ALLOCATE, "Unable to allocate %d elems", cnt);
    else {
        res->len = cnt;
        Array_fill(res, filltyp);
    }
    return logret(res, "sz = %d, len = %d", res->sz, res->len );
}
/// @brief free array
/// @param val pointer to array
/// @note: Array_free must not failed even if val == NULL
void                           Array_free(Array *val){
    if (val) {      // Array_free must not failed even if val == NULL
        increase(val, 0);
        free(val);
        val = NULL;
    }
}
/// @brief        Array filler (formatter) using fill type
/// @param a  Array (by value now, will be reworked)
/// @param typ  Array type 
/// @param vt   V64 type, only for for V64
/// @return Count of formatter data
int                             Array_fill(Array *parr, ArrayFillType typ){
    invraisecode(parr != NULL, ERR_NULLABLE_PTR, "Null input");
    return ArrayFillRange(parr, typ, 0, parr->len);
}


/// @brief        ascending filler (random increase)
/// @param parr   array 
/// @param from   start index 
/// @param to     end index 
/// @return       count of filled elements
static int                      ArrayFillRange_ASC(Array *parr, int from, int to) {

    value64_type              vt64 = ArrayGetV64mapType(parr);
    v64GenTypedFactory        ti = getTypedFillFactory(vt64, ARRAY_FILLTYPE_ASC);

    if (ti) {
        v64Gen gen = ti(to - from, from, g_array_acs_rndinc);

        int cnt = ArrayGenPumprange(parr, &gen, from, to);
        v64GenFree(&gen);

        return cnt;
    } else 
        return userraise(-1, ERR_ACTION_NOT_APPLICABLE,
                            "Type mismatch: Type %d/%s (v64: %d/%s) does not support ASC fill",
                            ArrayGettype(parr), ArrayGetTypeName(parr),
                            parr->v64type, ArrayGetV64typeName(parr));
}

/// @brief        descending filler
/// @param parr   array 
/// @param from   start index 
/// @param to     end index 
/// @return       count of filled elements
static int                      ArrayFillRange_DESC(Array *parr, int from, int to) {

    value64_type              vt64 = ArrayGetV64mapType(parr);
    const int                 start_num = (to - from + 1) * g_array_desc_rndinc;
    v64GenTypedFactory        ti = getTypedFillFactory(vt64, ARRAY_FILLTYPE_DESC);

    if (ti) {
        v64Gen gen = ti(to - from, start_num, g_array_desc_rndinc);

        int cnt = ArrayGenPumprange(parr, &gen, from, to);
        v64GenFree(&gen);
    
        return cnt;
    } else {
        return userraise(-1, ERR_ACTION_NOT_APPLICABLE,
                            "Type mismatch: Type %d/%s (v64: %d/%s) does not support DESC fill",
                            ArrayGettype(parr), ArrayGetTypeName(parr),
                            parr->v64type, ArrayGetV64typeName(parr));
    }
}


/// @brief        zero filler
/// @param parr   array 
/// @param from   start index 
/// @param to     end index 
/// @return       count of filled elements
static int                      ArrayFillRange_ZERO(Array *parr, int from, int to){

    value64_type    vt64 = ArrayGetV64mapType(parr);

    v64GenTypedFactory ti = getTypedFillFactory(vt64, ARRAY_FILLTYPE_ZERO);

    if (ti) {
        // first param - c [ count ]
        v64Gen gen = ti(to - from, 0, 0);
        int cnt = ArrayGenPumprange(parr, &gen, from, to);
        v64GenFree(&gen);

        return cnt;
    } else
        return 0;
    /*v64Gen gen = v64GenCreatorZero(
            ArrayGetV64mapType(parr), to - from);
    int cnt = ArrayGenPumprange(parr, &gen, from, to);

    v64GenFree(&gen);
    return cnt;*/
}

/// @brief        none filler (fs & str)
/// @param parr   array 
/// @param from   start index 
/// @param to     end index 
/// @return       count of filled elements
/// @note         no generator here!
static int                      ArrayFillRange_SAFE_EMPTY(Array *parr, int from, int to) {
    value64_type    vt64 = ArrayGetV64mapType(parr);

    v64GenTypedFactory ti = getTypedFillFactory(vt64, ARRAY_FILLTYPE_SAFE_EMPTY);

    if (ti) {
        v64Gen gen = ti(to - from, 0, 0);
        int cnt = ArrayGenPumprange(parr, &gen, from, to);
        v64GenFree(&gen);

        return cnt;
    } else
        return 0;
}

/// @brief        random value filler
/// @param parr   array 
/// @param from   start index 
/// @param to     end index 
/// @return       count of filled elements
static int                      ArrayFillRange_RND(Array *parr, int from, int to) {
    const int       rnd_max = 10 * (to - from);
    const int       rndinc  = to - from;
    value64_type    vt64 = ArrayGetV64mapType(parr);

    v64GenTypedFactory ti = getTypedFillFactory(vt64, ARRAY_FILLTYPE_RND);

    if (ti) {
        v64Gen gen = ti(rnd_max, 0 /* not used*/, rndinc);
        int cnt = ArrayGenPumprange(parr, &gen, from, to);
        v64GenFree(&gen);

        return cnt;
    } else 
        return userraise(-1, ERR_ACTION_NOT_APPLICABLE,
                            "Unsupported v64 type for RND fill: %d/%s or  %d/%s",
                            ArrayGettype(parr), ArrayGetTypeName(parr),
                            parr->v64type, ArrayGetV64typeName(parr));
}

/// @brief        ascending series filler
/// @param a      array 
/// @param from   start index 
/// @param to     end index 
/// @return       count of filled elements
static int                      ArrayFillRange_ASC_SERIES(Array *parr, int from, int to){
    value64_type              vt64 = ArrayGetV64mapType(parr);
    v64GenTypedFactory        ti = getTypedFillFactory(vt64,  ARRAY_FILLTYPE_ASC_SERIES);

    if (ti) {
        v64Gen gen = ti(to - from, from, 1);

        int cnt = ArrayGenPumprange(parr, &gen, from, to);
        v64GenFree(&gen);

        return cnt;
    } else 
        return userraise(-1, ERR_ACTION_NOT_APPLICABLE,
                            "Type mismatch: Type %d/%s (v64: %d/%s) does not support ASC series fill",
                            ArrayGettype(parr), ArrayGetTypeName(parr),
                            parr->v64type, ArrayGetV64typeName(parr));
}

/// @brief        descending series filler
/// @param a      array 
/// @param from   start index 
/// @param to     end index 
/// @return       count of filled elements
static int                      ArrayFillRange_DESC_SERIES(Array *parr, int from, int to) {
    value64_type              vt64 = ArrayGetV64mapType(parr);
    v64GenTypedFactory        ti = getTypedFillFactory(vt64, ARRAY_FILLTYPE_DESC_SERIES);
    const int                 start_num = (to - 1); // the same as for scalar types

    if (ti) {
        v64Gen gen = ti(to - from, start_num, 1);

        int cnt = ArrayGenPumprange(parr, &gen, from, to);
        v64GenFree(&gen);

        return cnt;
    } else 
        return userraise(-1, ERR_ACTION_NOT_APPLICABLE,
                            "Type mismatch: Type %d/%s (v64: %d/%s) does not support ASC series fill",
                            ArrayGettype(parr), ArrayGetTypeName(parr),
                            parr->v64type, ArrayGetV64typeName(parr));
}

/// @brief Array filler
/// @param a base array
/// @param typ Array type 
/// @param from from (will be normilized if out of range)
/// @param to  to (will be normilized if out of range)
/// @return Count of formatter data
int                             ArrayFillRange(Array *parr, ArrayFillType filltyp, int from, int to) {
    logenter("%d - %d, %s (%s/v64: %s)", 
            from, to, ArrayFillTypeName(filltyp), ArrayGetTypeName(parr), ArrayGetV64typeName(parr) );
    
    fixrangesbysz(parr, &from, &to);

    // value64_type              vt64 = ArrayGetV64mapType(parr);
    // v64GenTypedFactory        ti = getTypedFillFactory(vt64,  filltyp);
    // if (ti) {
    //.   startnum = arrayGetInterface()
    //     TODO: startvalue method to generate startnum v64Gen gen = ti(to - from, start_num, 1);

    //     int cnt = ArrayGenPumprange(parr, &gen, from, to);
    //     v64GenFree(&gen);

    //     return cnt;
    // } else 
    //     return userraise(-1, ERR_ACTION_NOT_APPLICABLE,
    //                         "Type mismatch: Type %d/%s (v64: %d/%s) does not support ASC series fill",
    //                         ArrayGettype(parr), ArrayGetTypeName(parr),
    //                         parr->v64type, ArrayGetV64typeName(parr));

    int cnt;
    switch (filltyp) {
        case ARRAY_FILLTYPE_ASC:
            cnt = ArrayFillRange_ASC(parr, from, to);
            break;
        case ARRAY_FILLTYPE_DESC:
            cnt = ArrayFillRange_DESC(parr, from, to);
            break;
        case ARRAY_FILLTYPE_ZERO:
            cnt = ArrayFillRange_ZERO(parr, from, to);
            break;
        case ARRAY_FILLTYPE_RND:
            cnt = ArrayFillRange_RND(parr, from, to);
            break;
        case ARRAY_FILLTYPE_SAFE_EMPTY:
            // just do nothing for scalar types
            cnt = ArrayFillRange_SAFE_EMPTY(parr, from, to);
            break;
        case ARRAY_FILLTYPE_ASC_SERIES:
            cnt = ArrayFillRange_ASC_SERIES(parr, from, to);
            break;
        case ARRAY_FILLTYPE_DESC_SERIES:
            cnt = ArrayFillRange_DESC_SERIES(parr, from, to);
            break;
        default:
            return userraise(-1, ERR_ACTION_NOT_APPLICABLE, 
                "Not supported filltype %d/%s", filltyp, ArrayFillTypeName(filltyp));
    }

    return logret(cnt, "Filled %d", cnt);
}

// -------------- ACCESS AND MODIFICATION --------------

Array                          *ArrayIncrease(Array *parr, int newcnt){
    if (newcnt > Arraysz(parr) )
        increase(parr, newcnt);
    ArrayFillRange(parr, ARRAY_FILLTYPE_ZERO, parr->len, newcnt);
    parr->len = newcnt;
    return parr;
}

Array                          *ArrayShrink(Array *parr, int newsz){
    logenter("newsz %d", newsz);
    fixbysz(parr, &newsz);

    increase(parr, newsz);
    return logret(parr, "shrinked to (len %d == sz %d)", parr->len, parr->sz);
}

/**
 * @brief Shuffle array elements using the Fisher–Yates algorithm.
 * @param arr array (by value)
 */
Array                       *ArrayShuffle(Array *parr) {
    int elem_size = ArrayGetelemsize(parr);
    if (elem_size <= 0)
        userraise(NULL, ERR_UNSUPPORTED_TYPE, 
                        "unsupported type for shuffle %s", ArrayGetTypeName(parr));
    // in case if arr.len < 2 that'll do nothing
    char    *data = parr->v;  // raw byte pointer
    for (int i = parr->len - 1; i > 0; i--) {
        int j = rndint(i);
        // swap elements at indices i and j
        item_exch(data + i * elem_size, data + j * elem_size, elem_size);   
    }
    return parr;
}

/**
 * @brief Removes elements from the array within the specified range.
 * 
 * @details This function removes @p cnt elements starting from index @p from.
 * If the requested range `[from, from + cnt)` exceeds the current array length, 
 * the function will automatically adjust @p cnt to remove only elements up to 
 * the end of the array (clamping).
 * 
 * @note The function ensures no memory leaks by cleaning up the content of 
 *       removed elements (e.g., for V64 or pointer types) before shifting.
 * 
 * @param arr   Pointer to the array.
 * @param from  The starting index of the removal range.
 * @param cnt   The number of elements to remove.
 * 
 * @return      Pointer to the modified array. Returns the original array 
 *              unchanged if `cnt <= 0` or `from` is out of bounds.
 * 
 * @warning The function performs a linear time O(n) shift operation.
 */
Array                        *ArrayDel(Array *parr, int from, int cnt) {
    invraisecode(parr != NULL, ERR_NULLABLE_PTR, 
        "Null array pointer %p", parr);

    // 1. Validation: Ensure the range is within [0, parr->len) and cnt > 0
    if (cnt <= 0 || from < 0 || from >= parr->len) {
        return logsimpleret(parr, "Nothing to del from %d, cnt %d, len %d", from, cnt, parr->len);
    }
    if ( (from + cnt) > parr->len) {
        logsimple("since %d + %d > total len %d cnt reduced to %d", 
                from, cnt, parr->len, parr->len - from);
        cnt = parr->len - from;
    }

    // no checking slices for now TODO:
    // ArraySlice *psli;
    // if ( (psli = ArrayCheckslicesInterval(parr, from, cnt) ) != NULL )
    // return userraise(NULL, ERR_OUT_OF_RANGE, "Slice exists... %d - %d", psli->initpos, psli->len);

    // 2. Cleanup: Free the content of the elements being removed to prevent leaks for V64.
    freeV64elems(parr, from, cnt);

    // 3. Shift: Move the suffix (elements after the removed block) to the deletion point.
    // Number of elements to move = Total length - end of deleted block
    int elements_to_move = parr->len - (from + cnt);
    if (elements_to_move > 0) {
        moveelem(parr, from, from + cnt, elements_to_move);
    }

    // 4. Update length
    parr->len -= cnt;

    return parr;
}

/**
 * @brief Inserts @p cnt new elements into the array at the specified position.
 *
 * @details This function expands the array to accommodate the new elements and 
 * shifts existing elements to make room for them. The process follows these steps:
 * <ol>
 *   <li>Validates the insertion index and the count of elements to be added.</li>
 *   <li>Increases the array capacity (sz) to ensure enough space for new elements.</li>
 *   <li>Shifts the existing elements from the insertion point to the end of the 
 *       array to the right, creating a "gap".</li>
 *   <li>Fills the newly created gap with the specified @p ftyp pattern.</li>
 *   <li>Updates the array's logical length (@p len).</li>
 * </ol>
 *
 * @param parr  Pointer to the array where elements will be inserted.
 * @param from  The starting index for the new elements.
 * @param cnt   The number of new elements to be added.
 * @param ftyp  The fill pattern to use for the newly inserted elements.
 *
 * @return      Pointer to the modified array on success.
 * @return      Returns an error code/NULL if the operation fails (e.g., invalid 
 *              parameters or memory allocation failure).
 *
 * @note This operation has a time complexity of O(n), where n is the number 
 *       of elements shifted to the right.
 * @warning If an error occurs during the capacity expansion (reallocation), 
 *          the array remains unchanged and in its original valid state.
 */
Array                   *ArrayAdd(Array *parr, int from, int cnt, ArrayFillType ftyp) {
    // TODO:
    invraisecode(parr != NULL, ERR_NULLABLE_PTR, 
        "Null array pointer %p", parr);

    if (cnt <= 0 || from < 0 || from > parr->len)
        return logsimpleret(parr, "Nothing to add: from %d, cnt %d, len %d", from, cnt, parr->len);

    if (increase(parr, parr->len + cnt) < 0)
        return userraise(NULL, ERR_UNABLE_ALLOCATE, "Unable to fill range, from %d, cnt %d, len %d", from, cnt, parr->len);
    
    int     elements_to_move = parr->len - from;

    moveelem(parr, from + cnt, from, elements_to_move);
    parr->len += cnt;   // cnt > 0!!!

    ArrayFillRange(parr, ftyp, from, from + cnt);
        
    return parr;
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
bool                            ArrayNoteq(const Array *restrict parr1, const Array *restrict parr2) {
    invraisecode(ERR_NULLABLE_PTR, parr1 != NULL && parr2 != NULL,
                 "Null pointers %p %p", (void*) parr1, (void*) parr2);
    // raise exception if not equal
    ArrayCheckComparable(parr1, parr2);

    if (parr1->len != parr2->len)
        return true;   // different lengths -> not equal

#define ArrayNoteq_COMPARE_LOOP(field) \
    for (int i = 0; i < parr1->len; i++) \
        if (parr1->field[i] != parr2->field[i]) return true;        


    ArrayType typ = ArrayGettype(parr1);
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
            for (int i = 0; i < parr1->len; i++)
                if (!value64_equal(parr1->v64[i], parr2->v64[i], parr1->v64type))
                    return true;
            break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "Unsupported type for comparison: %s",
                         ArrayGetTypeName(parr1));
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
void                                Array_qsort(Array *parr, ArrayFillType ord) {
    int                 sz = ArrayGetelemsize(parr);
    if (sz <= 0) {
        userraiseint(ERR_UNSUPPORTED_TYPE, "Unable to get type size %d/%s", ArrayGettype(parr), ArrayGetTypeName(parr));
    }
    pointer_comparator  cmp = NULL;
   
    ArrayType typ = ArrayGettype(parr);
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
                    ? value64_getPComparator(parr->v64type)
                    : value64_getPRevComparator(parr->v64type);
        break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "Unsupported type %s", ArrayGetTypeName(parr));
    }
    
    if (cmp)
        qsort(parr->v, parr->len, sz, cmp);
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
int                         ArrayBsearchIntCommon(const Array *parr, int val, bool acs) {
    if (!ArrayIsint(parr))
        userraiseint(ERR_UNSUPPORTED_TYPE, "ArrayBsearchInt requires ARRAY_INT");
    if (parr->len == 0)
        return -1;
    pointer_comparator cmp = acs ? pint_cmp : pint_revcmp;
    const int *found = (const int*) bsearch(&val, parr->iv, parr->len, sizeof(int), cmp);
    return found ? (int)(found - parr->iv) : -1;
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
int                         ArrayBsearchLongCommon(const Array *parr, long val, bool acs) {
    if (!ArrayIslong(parr))
        userraiseint(ERR_UNSUPPORTED_TYPE, "ArrayBsearchLong requires ARRAY_LONG");
    if (parr->len == 0)
        return -1;
    pointer_comparator cmp = acs ? plong_cmp : plong_revcmp;
    const long *found = (const long*) bsearch(&val, parr->lv, parr->len, sizeof(long), cmp);
    return found ? (int)(found - parr->lv) : -1;
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
int                         ArrayBsearchDblCommon(const Array *parr, double val, bool acs) {
    if (!ArrayIsdouble(parr))
        userraiseint(ERR_UNSUPPORTED_TYPE, "ArrayBsearchDbl requires ARRAY_DOUBLE");
    if (parr->len == 0)
        return -1;
    pointer_comparator cmp = acs ? pdbl_cmp : pdbl_revcmp;    
    const double *found = (const double*)bsearch(&val, parr->dv, parr->len, sizeof(double), cmp);
    return found ? (int)(found - parr->dv) : -1;
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
int                         ArrayBsearchCharCommon(const Array *parr, char val, bool acs) {
    if (!ArrayIschar(parr))
        userraiseint(ERR_UNSUPPORTED_TYPE, "Requires ARRAY_CHAR");
    if (parr->len == 0)
        return -1;
    pointer_comparator  cmp = acs ? pchar_cmp : pchar_revcmp;    
    const               char *found = (const char *) bsearch(&val, parr->cv, parr->len, sizeof(char), cmp);
    return found ? (int)(found - parr->cv) : -1;
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
int                         ArrayBsearchV64Common(const Array *parr, value64 val, bool asc) {
    if (!ArrayIsV64(parr))
        userraiseint(ERR_UNSUPPORTED_TYPE, "ArrayBsearchV64 requires ARRAY_V64");
    if (parr->len == 0)
        return -1;

    if (asc)
        return value64_binsearch(val, parr->v64type, parr->v64, parr->len);
    else
        return value64_rev_binsearch(val, parr->v64type, parr->v64, parr->len);
}

// -----------------------------------------------------------------------------------------------------
// if condition is 0-ptr == ALL
int                         Array_foreach_proc(Array *restrict arr, Array_cond cond, Array_proc func){
    int     cnt = 0;
    //for (int i = 0; i < Arraylen(arr); i++)
    Array_pforeach_idx(arr, i) {
        if (cond == NULL || cond(arr, i) ){
            if (func)
                func(arr, i);
            cnt++;
        }
    }
    return logsimpleret(cnt, "processed %d", cnt);
}

/**
 * @brief Fills a specified range within a v64 Array using a generator.
 * 
 * This function takes elements from the provided v64 generator and writes them 
 * into the destination array starting from index @p from up to (but not including) 
 * index @p to. 
 * 
 * @note **Safety Features:**
 * - If @p from or @p to are outside the actual bounds of the array, they are 
 *   automatically clamped to the array's size to prevent memory corruption.
 * - The operation stops if the generator runs out of elements before the 
 *   specified range is completed.
 * - If @p from > @p to, the function returns an error via userraise.
 *
 * @param[in] parr Pointer to the destination Array (must be of type ARRAY_V64).
 * @param[in] gen  Pointer to the v64 generator providing the data.
 * @param[in] from The starting index for the fill operation (inclusive).
 * @param[in] to   The ending index for the fill operation (exclusive).
 * 
 * @return The number of elements successfully written to the array.
 *         Returns -1 (via userraise) if input pointers are NULL or indices are negative.
 */
static int                   ArrayGenPumprangeV64(Array *restrict parr, v64Gen *restrict gen, int from, int to) {    
    int cnt = 0;
    value64 *const end = parr->v64 + to;
    value64 *pv = parr->v64 + from;

    while (v64GenHasnext(gen) && pv < end) {
        *pv++ = v64GenNext(gen);
        cnt++;
    }
    return cnt; // cnt can be less than to - from
}
// intenal, no checking, size copy
static int                   ArrayGenPumprangeBySize(Array *restrict parr, v64Gen *restrict gen, int from, int to, int sz) {
    int             cnt = 0;
    char           *pv = (char *) parr->v + from * sz;
    char    *const end = (char *) parr->v + to * sz;

    while (v64GenHasnext(gen) && pv < end) {
        value64 tmp = v64GenNext(gen);   // сохраняем значение, чтобы не брать адрес rvalue

        switch (ArrayGettype(parr)) {
            case ARRAY_INT:
                *(int *)pv = tmp.ival;
                break;
            case ARRAY_LONG:
                *(long *)pv = tmp.lval;
                break;
            case ARRAY_DOUBLE:
                *(double *)pv = tmp.dval;
                break;
            case ARRAY_CHAR:
                *(char *)pv = tmp.cval;
                break;
            case ARRAY_POINTER:
                *(void **)pv = tmp.pval;
                break;
            default:
                return userraise(-1, ERR_UNSUPPORTED_TYPE,
                                 "Unsupported scalar type %d/%s",
                                 ArrayGettype(parr), ArrayGetTypeName(parr));
        }
        pv += sz;
        cnt++;
    }
    return cnt;
}

// pump gen data into parr, no convertatiob here! Types must be equal
int                          ArrayGenPumprangeScalar(Array *restrict parr, v64Gen *restrict gen, int from, int to) {
    invraisecode(parr != NULL && gen != NULL, ERR_NULLABLE_PTR, 
        "Null input %p %p", parr, gen);
    invraisecode(!ArrayIsV64(parr), ERR_UNSUPPORTED_TYPE, 
        "Only scalar supported by scalar pump, but not %d/%s", ArrayGettype(parr), ArrayGetTypeName(parr));
    
    if (ArrayGetV64mapType(parr) != gen->type) {
        return userraise(-1, ERR_TYPES_MISMATCH, 
            "%d/%s vs %d/%s", 
            ArrayGettype(parr), ArrayGetTypeName(parr),
            gen->type, value64_typename(gen->type));
    }
   
    return ArrayGenPumprangeBySize(parr, gen, from, to, ArrayGetelemsize(parr));
}

// entry point, check here!
// pump for v64 & scalar
int                          ArrayGenPumprange(Array *restrict parr, v64Gen *restrict gen, int from, int to) {
    invraisecode(parr != NULL && gen != NULL, ERR_NULLABLE_PTR, 
        "Null input %p %p", parr, gen);
    // fixing ranges
    fixrangesbysz(parr, &from, &to);

    int     cnt = 0;
    // v64 have universale pumper while every scalar type - it's own
    if (ArrayIsV64(parr))
        cnt = ArrayGenPumprangeV64(parr, gen, from, to);
    else
        cnt = ArrayGenPumprangeScalar(parr, gen, from, to);
    return logsimpleret(cnt, "Generated %d", cnt);
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
long                         Arrayfprint(FILE *restrict out, const Array *restrict val, int limit) {
    invraisecode(val != NULL, ERR_NULLABLE_PTR, "Input array is null");
    if (!out)
        return logsimpleerr(0L, "Output file is null");
    long    cnt = 0, i;
    int     array_rec_line = 20;      // default value

    limit = (limit == 0) ? val->len : (limit < val->len) ? limit : val->len;
    if (g_array_rec_line)
        array_rec_line = g_array_rec_line;

    cnt += fprintf(out, "Array (%s[%d of total %d]):\n",
                   ArrayTypeGetName(val->flags), limit, val->len);

    const char *custom = g_custom_print_line;

    for (i = 0; i < limit; i++) {
        switch (ArrayGettype(val)) {
            case ARRAY_INT:
                IOCHECKER(written, fprintf(out, custom ? custom : "[%d - %6d]\t", i, val->iv[i]), -1L)
                     cnt += written;
                break;
            case ARRAY_LONG:
                IOCHECKER(written, fprintf(out, custom ? custom : "[%ld - %6ld]\t", i, val->lv[i]), -1L)
                    cnt += written;
                break;
            case ARRAY_DOUBLE:
                IOCHECKER(written, fprintf(out, custom ? custom : "[%d - %.8lg]\t", i, val->dv[i]), -1L)
                    cnt += written;
                break;
            case ARRAY_POINTER:
                IOCHECKER(written, fprintf(out, custom ? custom : "[%p - %p]\t", i, val->pv[i]), -1L)
                    cnt += written;
                break;
            case ARRAY_CHAR:
                IOCHECKER(written, fprintf(out, custom ? custom : "[%d - %c]\t", i, val->cv[i]), -1L)
                    cnt += written;
                break;
            case ARRAY_V64:
                // custom format not supported for value64; always use dedicated printer
                IOCHECKER(written, value64_techfprint(out, val->v64[i], val->v64type, ""), -1L)
                    cnt += written;
                break;
            default:
                IOCHECKER(written, fprintf(out, "[%ld - ?]\t", i), -1L )
                    cnt += written;
                break;
        }

        if (((i + 1) % array_rec_line) == 0)
            cnt += fprintf(out, "\n");
    }

    if (i < val->len)
        cnt += fprintf(out, "and more (%ld) ...\n", val->len - i);
    else
        cnt += fprintf(out, "\n");

    return cnt;
}

/**
 * @brief Saves array values to a text file, separated by a delimiter.
 *
 * Each element is written on a single line, with the delimiter appended.
 * For ARRAY_V64 the dedicated value64_tofile() is used.
 *
 * @param arr   array (by value)
 * @param fname file name
 * @param delim delimiter character
 * @return number of bytes written, or -1 on error
 */
long                        ArraySaveFilevalues(const Array *restrict parr, const char *restrict fname, char delim) {
    logenter("%s, [%c]", fname, delim);

    FILE *f = fopen(fname, "w");
    if (!f)
        return userraise(-1L, ERR_UNABLE_OPEN_FILE_WRITE, "Can't open '%s' for writing", fname);

    long    total_written = 0;
    int     typ = ArrayGettype(parr), status = 0;

    for (int i = 0; i < parr->len; i++) {
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
                written = fprintf(f, "%d", parr->iv[i]);
                break;
            case ARRAY_LONG:
                written = fprintf(f, "%ld", parr->lv[i]);
                break;
            case ARRAY_DOUBLE:
                written = fprintf(f, "%12.12lf", parr->dv[i]);
                break;
            case ARRAY_POINTER:
                written = fprintf(f, "%p", parr->pv[i]);
                break;
            case ARRAY_CHAR:
                written = fprintf(f, "%c", parr->iv[i]);
                break;
            case ARRAY_V64:
                written = value64_tofile(f, parr->v64[i], parr->v64type, true);
                break;
            default:
                fclose(f);
                return userraise(-1L, ERR_UNSUPPORTED_TYPE, "Unsupported type %s\n", ArrayGetTypeName(parr));
        }
        if (written < 0) {
            status = -1;
            break;
        }
        total_written += written;
    }
    if (status != 0) {
        fclose(f);
        return userraise(-1L, ERR_STREAM_ERROR, "Write error in '%s'", fname);
    }

    if (fclose(f) != 0) {   // NOT SURE
        return userraise(-1L, ERR_STREAM_ERROR, "Error closing file '%s'", fname);
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

long                        ArraySaveFile(FILE *restrict out, const Array *restrict parr) {  
    invraisecode(parr != NULL, ERR_NULLABLE_PTR, "Array is null");
    if (!out)
        return logsimpleret(0L,  "Output is null"); 

    long        total_written = 0L;
    const char  *typ = ArrayGetTypeRealName(parr);
    const char  *v64_type  = ArrayIsV64(parr) ? ArrayGetV64typeName(parr) : "NONV64_TYPE";

    IOCHECKER(written, fprintf(out, "ARRAY: %s / %s : %d\n", typ, v64_type, parr->len), -1)
        total_written += written;
    IOCHECKER(written, ArraySaveValues(out, parr), -1)
        total_written += written;
    IOCHECKER(written, fprintf(out, "ARRAY: DONE\n"), -1)
        total_written += written;
    return total_written;
}

/**
 * @brief Saves an array to a file.
 *
 * Opens the file for writing, calls ArraySaveFile(), and closes the file.
 *
 * @param arr   array (by value)
 * @param fname file path
 * @return number of bytes written, or a negative value on error
 */
long                        ArraySaveFileName(const Array *parr, const char *fname) {
    logenter("%s", fname);

    FILE        *out = fopen(fname, "w");
    if (out == 0)
        return userraise(-1, ERR_UNABLE_OPEN_FILE_WRITE, "Can't open '%s' for write", fname);

    long        res = ArraySaveFile(out, parr);
    fclose(out);

    if(res < 0)
        return userraise(res, ERR_STREAM_ERROR, "Unable to save array");
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
 * @return loaded array, or NULL
 */
Array                           *ArrayLoadFile(FILE *in) {
    invraisecode(ERR_NULLABLE_PTR, in != NULL, "Nullable input");

    Array *parr = ArrayParseHeaderFile(in); 
    if (!parr)
        return userraise(parr, ERR_UNSUPPORTED_TYPE, "Unable to create array");

    if (ArrayFileLoadValues(in, parr) < 0) {
        Array_free(parr);
        userraise(parr, ERR_WRONG_INPUT_FORMAT, "Unable to read value from file");
    }

    if (!ArrayParseFooterFile(in) ) {
        Array_free(parr);
        userraise(parr, ERR_WRONG_INPUT_FORMAT, "Unable to finish create array");
    }

    return parr;
}

/**
 * @brief Loads an array from a file.
 *
 * Opens the file for reading, calls ArrayLoadFile(), and closes the file.
 *
 * @param fname file path
 * @return loaded array, or an array with the error flag set
 */
Array                       *ArrayLoadFileName(const char *fname) {
    invraisecode(ERR_NULLABLE_PTR, fname != NULL, "Nullable fname");

    logenter("%s", fname);
    FILE    *in = fopen(fname, "r");

    if (in == 0)
        userraiseint(ERR_UNABLE_OPEN_FILE_READ, "Can't open for read '%s'", fname);
    
    Array   *arr = ArrayLoadFile(in);
    
    fclose(in);
    return logret(arr, "Done %d", arr->len);
}

// -------------------------- (API) serialization -----------------------

long                        ArraySavefs(fs *restrict s, const Array *restrict parr) {
    invraisecode(ERR_NULLABLE_PTR, s != NULL && parr != NULL, 
            "Fs nullable or arr is null %p %p", s, parr);

    long        total_written = 0L;
    const char  *typ = ArrayTypeGetName(parr->flags);
    const char  *v64_type  = ArrayIsV64(parr) ? ArrayGetV64typeName(parr) : "NONV64_TYPE";

    total_written += fs_sprintf_concat(s, "ARRAY: %s / %s : %d\n", typ, v64_type, parr->len);
    total_written += ArraySerializeValues(s, parr);
    total_written += fs_sprintf_concat(s, "ARRAY: DONE\n");
    return total_written;
}



long                            ArrayLoadfs(const fs *restrict s, Array *restrict parr) {
    invraisecode(ERR_NULLABLE_PTR, fs_isnull(s),
                 "Nullable input %p", (void*) s);

    const char     *data = s->v;
    int             data_len;
    Array           *pa = ArrayParseHeaderStr(&data); 

    if (!pa)
        return userraise(-1, ERR_UNSUPPORTED_TYPE, "Unable to create array");

    if ( (data_len = ArrayFsLoadValues(data, pa) )  < 0) {
        Array_free(pa);
        userraiseint(ERR_WRONG_INPUT_FORMAT, "Unable to read value from str");
    } else
        data += data_len;   // shift

    if (!ArrayParseFooterStr(&data) ) {
        Array_free(pa);
        userraiseint(ERR_WRONG_INPUT_FORMAT, "Unable to finish create array");
    }

   
    if (parr)    // if arr is NULL then dump read
        *parr = *pa;
    return (long)(data - s->v);
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
    int subnum = 0;

    test_sub("subtest %d: double", ++subnum);
    {
        Array *arr = DArray_create(100, ARRAY_FILLTYPE_ZERO);
        for (int i = 0; i < arr->len; i++)
            test_validatefree(
                arr->dv[i] == 0.0,
                Arrayfree(arr),
                "%d: Element must be 0.0, but not %lf", i, arr->dv[i]
            );
        test_validatefree(
            ArrayIsvalid(arr),
            Arrayfree(arr),
            "Validation is failed"
        );
        Arrayfree(arr);
        test_validate(arr == NULL, "Array isn't freed (pointer must be NULL)");
    }

    test_sub("subtest %d: int", ++subnum);
    {
        Array *arr = IArray_create(100, ARRAY_FILLTYPE_ZERO);
        for (int i = 0; i < arr->len; i++)
            test_validatefree(
                arr->iv[i] == 0,
                Arrayfree(arr),
                "%d: Element must be 0 but not %d", i, arr->iv[i]
            );
        test_validatefree(
            ArrayIsvalid(arr),
            Arrayfree(arr),
            "Validation is failed"
        );
        Arrayfree(arr);
        test_validate(arr == NULL, "Array isn't freed (pointer must be NULL)");
    }

    test_sub("subtest %d: long", ++subnum);
    {
        Array *arr = LArray_create(100, ARRAY_FILLTYPE_ZERO);
        for (int i = 0; i < arr->len; i++)
            test_validatefree(
                arr->lv[i] == 0L,
                Arrayfree(arr),
                "%d: Element must be 0L but not %ld", i, arr->lv[i]
            );
        test_validatefree(
            ArrayIsvalid(arr),
            Arrayfree(arr),
            "Validation is failed"
        );
        Arrayfree(arr);
        test_validate(arr == NULL, "Array isn't freed (pointer must be NULL)");
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 2 ---------------------------------
static TestStatus
tf2(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: double asc/desc", ++subnum);
    {
        Array *arr = DArray_create(100, ARRAY_FILLTYPE_ASC);
        // ASC check
        for (int i = 0; i < arr->len - 1; i++)
            test_validatefree(
                arr->dv[i] <= arr->dv[i + 1],
                Arrayfree(arr),
                "ASC violation: arr[%d]=%f > arr[%d]=%f",
                i, arr->dv[i], i + 1, arr->dv[i + 1]
            );
        // refill to DESC
        Array_fill(arr, ARRAY_FILLTYPE_DESC);
        // DESC check
        for (int i = 0; i < arr->len - 1; i++)
            test_validatefree(
                arr->dv[i] >= arr->dv[i + 1],
                Arrayfree(arr),
                "DESC violation: arr[%d]=%f < arr[%d]=%f",
                i, arr->dv[i], i + 1, arr->dv[i + 1]
            );
        Arrayfree(arr);
    }

    test_sub("subtest %d: int asc/desc", ++subnum);
    {
        Array *arr = IArray_create(100, ARRAY_FILLTYPE_ASC);
        for (int i = 0; i < arr->len - 1; i++)
            test_validatefree(
                arr->iv[i] <= arr->iv[i + 1],
                Arrayfree(arr),
                "ASC violation: arr[%d]=%d > arr[%d]=%d",
                i, arr->iv[i], i + 1, arr->iv[i + 1]
            );
        Array_fill(arr, ARRAY_FILLTYPE_DESC);
        for (int i = 0; i < arr->len - 1; i++)
            test_validatefree(
                arr->iv[i] >= arr->iv[i + 1],
                Arrayfree(arr),
                "DESC violation: arr[%d]=%d < arr[%d]=%d",
                i, arr->iv[i], i + 1, arr->iv[i + 1]
            );
        Arrayfree(arr);
    }

    test_sub("subtest %d: long asc/desc", ++subnum);
    {
        Array *arr = LArray_create(100, ARRAY_FILLTYPE_ASC);
        for (int i = 0; i < arr->len - 1; i++)
            test_validatefree(
                arr->lv[i] <= arr->lv[i + 1],
                Arrayfree(arr),
                "ASC violation: arr[%d]=%ld > arr[%d]=%ld",
                i, arr->lv[i], i + 1, arr->lv[i + 1]
            );
        Array_fill(arr, ARRAY_FILLTYPE_DESC);
        for (int i = 0; i < arr->len - 1; i++)
            test_validatefree(
                arr->lv[i] >= arr->lv[i + 1],
                Arrayfree(arr),
                "DESC violation: arr[%d]=%ld < arr[%d]=%ld",
                i, arr->lv[i], i + 1, arr->lv[i + 1]
            );
        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}
/**
 * @brief Test suite for Array Shrink and Validation.
 * 
 * @details This test ensures that shrinking an array works correctly, 
 *          the data remains valid, and the internal properties (len, sz) 
 *          are updated properly.
 * 
 * @return TestStatus TEST_PASSED if all subtests pass, otherwise TEST_FAILED.
 */
// ------------------------- TEST 3 ---------------------------------
static TestStatus
tf3(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: double", ++subnum);
    {
        Array *arr = DArray_create(100, ARRAY_FILLTYPE_ASC);
        Arrayfprint(logfile, arr, 0);

        arr = ArrayShrink(arr, 10);
        test_validatefree(
            ArrayIsvalid(arr) && arr->len == 10 && arr->sz >= 10 && arr->dv != NULL,
            Arrayfree(arr),
            "Shrink failed: len=%d, sz=%d, v=%p", arr->len, arr->sz, (void*)arr->dv
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: int", ++subnum);
    {
        Array *arr = IArray_create(100, ARRAY_FILLTYPE_ASC);
        Arrayfprint(logfile, arr, 0);

        arr = ArrayShrink(arr, 10);
        test_validatefree(
            ArrayIsvalid(arr) && arr->len == 10 && arr->sz >= 10 && arr->iv != NULL,
            Arrayfree(arr),
            "Shrink failed: len=%d, sz=%d, v=%p", arr->len, arr->sz, (void*)arr->iv
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: long", ++subnum);
    {
        Array *arr = LArray_create(100, ARRAY_FILLTYPE_ASC);
        Arrayfprint(logfile, arr, 0);

        arr = ArrayShrink(arr, 10);
        test_validatefree(
            ArrayIsvalid(arr) && arr->len == 10 && arr->sz >= 10 && arr->lv != NULL,
            Arrayfree(arr),
            "Shrink failed: len=%d, sz=%d, v=%p", arr->len, arr->sz, (void*)arr->lv
        );
        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 4 ---------------------------------
static TestStatus
tf4(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: int save/load", ++subnum);
    {
        Array *arr = IArray_create(100, ARRAY_FILLTYPE_RND);

        test_validate(
            arr != NULL, 
            "Array not created"
        );

        const char *filename = "res/array/iarr.sv";

        long written = ArraySaveFileName(arr, filename);
        test_validatefree(
            written > 0, 
            Arrayfree(arr), 
            "Int save failed"
        );

        Array *loaded = ArrayLoadFileName(filename);
        test_validatefree(
            loaded != NULL && ArrayIsvalid(loaded) &&
            arr->len == loaded->len && arr->flags == loaded->flags,
            (Arrayfree(arr), Arrayfree(loaded)),
            "Length or flags mismatch: len %d vs %d, flags %d vs %d",
            arr->len, loaded->len, arr->flags, loaded->flags
        );

        // поэлементное сравнение
        for (int i = 0; i < arr->len; i++)
            test_validatefree(
                arr->iv[i] == loaded->iv[i],
                (Arrayfree(arr), Arrayfree(loaded)),
                "arr[%d] = %d != arr2[%d] = %d",
                i, arr->iv[i], i, loaded->iv[i]
            );

        Arrayfree(arr);
        Arrayfree(loaded);
    }

    test_sub("subtest %d: long save/load", ++subnum);
    {
        Array *arr = LArray_create(100, ARRAY_FILLTYPE_RND);
        const char *filename = "res/array/larr.sv";

        long written = ArraySaveFileName(arr, filename);
        test_validatefree(written > 0, Arrayfree(arr), "Long save failed");

        Array *loaded = ArrayLoadFileName(filename);
        test_validatefree(
            loaded != NULL && ArrayIsvalid(loaded) &&
            arr->len == loaded->len && arr->flags == loaded->flags,
            (Arrayfree(arr), Arrayfree(loaded)),
            "Length or flags mismatch: len %d vs %d, flags %d vs %d",
            arr->len, loaded->len, arr->flags, loaded->flags
        );

        for (int i = 0; i < arr->len; i++)
            test_validatefree(
                arr->lv[i] == loaded->lv[i],
                (Arrayfree(arr), Arrayfree(loaded)),
                "arr[%d] = %ld != arr2[%d] = %ld",
                i, arr->lv[i], i, loaded->lv[i]
            );

        Arrayfree(arr);
        Arrayfree(loaded);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 5 ---------------------------------
static TestStatus
tf5(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d", ++subnum);
    {
        Array *arr = DArray_create(100, ARRAY_FILLTYPE_RND);
        const char *filename = "res/array/darr.sv";

        Arrayfprint(logfile, arr, 0);
        long written = ArraySaveFileName(arr, filename);
        test_validatefree(written > 0, Arrayfree(arr), "Double save failed");

        Array *loaded = ArrayLoadFileName(filename);
        test_validatefree(
            loaded != NULL && ArrayIsvalid(loaded) &&
            arr->len == loaded->len && arr->flags == loaded->flags,
            (Arrayfree(arr), Arrayfree(loaded)),
            "Length or flags mismatch: len %d vs %d, flags %d vs %d",
            arr->len, loaded->len, arr->flags, loaded->flags
        );

        for (int i = 0; i < arr->len; i++)
            test_validatefree(
                fabs(arr->dv[i] - loaded->dv[i]) <= FLT_EPSILON / 100,
                (Arrayfree(arr), Arrayfree(loaded)),
                "arr[%d] = %15.15lf != arr2[%d] = %15.15lf",
                i, arr->dv[i], i, loaded->dv[i]
            );

        Arrayfree(arr);
        Arrayfree(loaded);
    }
    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 6 ---------------------------------
static TestStatus
tf6(const char *name)
{
    logenter("%s", name);
    int subnum = 0;
    g_custom_print_line = NULL;   // сбрасываем формат печати

    /* ---------- double ---------- */
    test_sub("subtest %d: double", ++subnum);
    {
        Array *arr = DArray_create(50, ARRAY_FILLTYPE_ASC);
        ArrayShuffle(arr);

        // проверяем, что порядок нарушен (не все элементы строго возрастают)
        bool ordered = true;
        for (int i = 0; i < arr->len - 1; i++) {
            if (arr->dv[i] > arr->dv[i + 1]) {
                ordered = false;
                break;
            }
        }
        test_validatefree(
            !ordered,
            Arrayfree(arr),
            "Double shuffle: array must not be perfectly ordered after shuffle"
        );
        Arrayfree(arr);
    }

    /* ---------- int ---------- */
    test_sub("subtest %d: int", ++subnum);
    {
        Array *arr = IArray_create(50, ARRAY_FILLTYPE_ASC);
        ArrayShuffle(arr);

        bool ordered = true;
        for (int i = 0; i < arr->len - 1; i++) {
            if (arr->iv[i] > arr->iv[i + 1]) {
                ordered = false;
                break;
            }
        }
        test_validatefree(
            !ordered,
            Arrayfree(arr),
            "Int shuffle: array must not be perfectly ordered"
        );
        Arrayfree(arr);
    }

    /* ---------- long ---------- */
    test_sub("subtest %d: long", ++subnum);
    {
        Array *arr = LArray_create(50, ARRAY_FILLTYPE_ASC);
        ArrayShuffle(arr);

        bool ordered = true;
        for (int i = 0; i < arr->len - 1; i++) {
            if (arr->lv[i] > arr->lv[i + 1]) {
                ordered = false;
                break;
            }
        }
        test_validatefree(
            !ordered,
            Arrayfree(arr),
            "Long shuffle: array must not be perfectly ordered"
        );
        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 7 ---------------------------------
static TestStatus
tf7(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ---------- double ---------- */
    test_sub("subtest %d: double asc/desc", ++subnum);
    {
        Array *arr = DArray_create(10000, ARRAY_FILLTYPE_RND);
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        for (int i = 1; i < arr->len; i++)
            test_validatefree(
                arr->dv[i - 1] <= arr->dv[i],
                Arrayfree(arr),
                "ASC violation: arr[%d]=%f > arr[%d]=%f",
                i - 1, arr->dv[i - 1], i, arr->dv[i]
            );

        Array_qsort(arr, ARRAY_FILLTYPE_DESC);
        for (int i = 1; i < arr->len; i++)
            test_validatefree(
                arr->dv[i - 1] >= arr->dv[i],
                Arrayfree(arr),
                "DESC violation: arr[%d]=%f < arr[%d]=%f",
                i - 1, arr->dv[i - 1], i, arr->dv[i]
            );
        Arrayfree(arr);
    }

    /* ---------- int ---------- */
    test_sub("subtest %d: int asc/desc", ++subnum);
    {
        Array *arr = IArray_create(100000, ARRAY_FILLTYPE_RND);
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        for (int i = 1; i < arr->len; i++)
            test_validatefree(
                arr->iv[i - 1] <= arr->iv[i],
                Arrayfree(arr),
                "ASC violation: arr[%d]=%d > arr[%d]=%d",
                i - 1, arr->iv[i - 1], i, arr->iv[i]
            );

        Array_qsort(arr, ARRAY_FILLTYPE_DESC);
        for (int i = 1; i < arr->len; i++)
            test_validatefree(
                arr->iv[i - 1] >= arr->iv[i],
                Arrayfree(arr),
                "DESC violation: arr[%d]=%d < arr[%d]=%d",
                i - 1, arr->iv[i - 1], i, arr->iv[i]
            );
        Arrayfree(arr);
    }

    /* ---------- long ---------- */
    test_sub("subtest %d: long asc/desc", ++subnum);
    {
        Array *arr = LArray_create(100000, ARRAY_FILLTYPE_RND);
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        for (int i = 1; i < arr->len; i++)
            test_validatefree(
                arr->lv[i - 1] <= arr->lv[i],
                Arrayfree(arr),
                "ASC violation: arr[%d]=%ld > arr[%d]=%ld",
                i - 1, arr->lv[i - 1], i, arr->lv[i]
            );

        Array_qsort(arr, ARRAY_FILLTYPE_DESC);
        for (int i = 1; i < arr->len; i++)
            test_validatefree(
                arr->lv[i - 1] >= arr->lv[i],
                Arrayfree(arr),
                "DESC violation: arr[%d]=%ld < arr[%d]=%ld",
                i - 1, arr->lv[i - 1], i, arr->lv[i]
            );
        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 8 ---------------------------------
static TestStatus
tf8(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: increase int array", ++subnum);
    {
        int initsz = 25;
        Array *arr = IArray_create(initsz, ARRAY_FILLTYPE_RND);

        arr = ArrayIncrease(arr, initsz * 3);

        test_validatefree(
            arr->len == initsz * 3,
            Arrayfree(arr),
            "Array length %d must be %d", arr->len, initsz * 3
        );
        for (int i = initsz; i < arr->len; i++)
            test_validatefree(
                arr->iv[i] == 0,
                Arrayfree(arr),
                "arr[%d] must be zero, but not %d", i, arr->iv[i]
            );
        Arrayfree(arr);
    }

    test_sub("subtest %d: increase double array", ++subnum);
    {
        int initsz = 25;
        Array *arr = DArray_create(initsz, ARRAY_FILLTYPE_RND);

        arr = ArrayIncrease(arr, initsz * 3);

        test_validatefree(
            arr->len == initsz * 3,
            Arrayfree(arr),
            "Array length %d must be %d", arr->len, initsz * 3
        );
        for (int i = initsz; i < arr->len; i++)
            test_validatefree(
                arr->dv[i] == 0.0,
                Arrayfree(arr),
                "arr[%d] must be zero, but not %lf", i, arr->dv[i]
            );
        Arrayfree(arr);
    }

    test_sub("subtest %d: increase long array", ++subnum);
    {
        int initsz = 25;
        Array *arr = LArray_create(initsz, ARRAY_FILLTYPE_RND);

        arr = ArrayIncrease(arr, initsz * 5);

        test_validatefree(
            arr->len == initsz * 5,
            Arrayfree(arr),
            "Array length %d must be %d", arr->len, initsz * 5
        );
        for (int i = initsz; i < arr->len; i++)
            test_validatefree(
                arr->lv[i] == 0L,
                Arrayfree(arr),
                "arr[%d] must be zero, but not %ld", i, arr->lv[i]
            );
        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 9 ---------------------------------
static TestStatus
tf9(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: creating pointer array", ++subnum);
    {
        Array *parr = PArray_create(100, ARRAY_FILLTYPE_ZERO);
        for (int i = 0; i < parr->len; i++)
            test_validatefree(
                parr->pv[i] == NULL,
                Arrayfree(parr),
                "Element %d must be NULL, but not %p", i, (void*)parr->pv[i]
            );
        test_validatefree(
            ArrayIsvalid(parr),
            Arrayfree(parr),
            "Validation is failed"
        );
        Arrayfree(parr);
    }

    test_sub("subtest %d: shrinking", ++subnum);
    {
        Array *parr = PArray_create(100, ARRAY_FILLTYPE_ZERO);
        int cnt = 10;
        parr = ArrayShrink(parr, cnt);
        test_validatefree(
            ArrayIsvalid(parr),
            Arrayfree(parr),
            "Validation is failed"
        );
        test_validatefree(
            parr->len == cnt && parr->sz >= cnt && parr->pv != NULL,
            Arrayfree(parr),
            "Shrink failed: len=%d, sz=%d, v=%p", parr->len, parr->sz, (void*)parr->pv
        );
        Arrayfree(parr);
    }

    test_sub("subtest %d: pointer array save/load", ++subnum);
    {
        const char *filename = "res/array/parr.sv";
        Array *parr = PArray_create(100, ARRAY_FILLTYPE_ZERO);
        ArraySaveFileName(parr, filename);

        Array *loaded = ArrayLoadFileName(filename);
        test_validatefree(
            ArrayIsvalid(loaded),
            (Arrayfree(parr), Arrayfree(loaded)),
            "Loaded array validation failed"
        );
        test_validatefree(
            parr->len == loaded->len && parr->flags == loaded->flags,
            (Arrayfree(parr), Arrayfree(loaded)),
            "Length or flags mismatch: len %d vs %d, flags %d vs %d",
            parr->len, loaded->len, parr->flags, loaded->flags
        );

        for (int i = 0; i < parr->len; i++)
            test_validatefree(
                parr->pv[i] == loaded->pv[i],
                (Arrayfree(parr), Arrayfree(loaded)),
                "arr[%d] = %p != arr2[%d] = %p",
                i, (void*)parr->pv[i], i, (void*)loaded->pv[i]
            );

        Arrayfree(parr);
        Arrayfree(loaded);
    }

    test_sub("subtest %d: pointer array sorting", ++subnum);
    {
        int cnt = 10000;
        Array *parr = PArray_create(cnt, ARRAY_FILLTYPE_ZERO);

        // fill array with descending addresses
        for (int i = 0; i < parr->len; i++)
            parr->pv[i] = parr->pv + cnt - 1 - i;

        Array_qsort(parr, ARRAY_FILLTYPE_ASC);
        for (int i = 1; i < parr->len; i++)
            test_validatefree(
                (uintptr_t)parr->pv[i - 1] <= (uintptr_t)parr->pv[i],
                Arrayfree(parr),
                "ASC violation: arr[%d]=%p > arr[%d]=%p",
                i - 1, (void*)parr->pv[i - 1], i, (void*)parr->pv[i]
            );

        Array_qsort(parr, ARRAY_FILLTYPE_DESC);
        for (int i = 1; i < parr->len; i++)
            test_validatefree(
                (uintptr_t)parr->pv[i - 1] >= (uintptr_t)parr->pv[i],
                Arrayfree(parr),
                "DESC violation: arr[%d]=%p < arr[%d]=%p",
                i - 1, (void*)parr->pv[i - 1], i, (void*)parr->pv[i]
            );
        Arrayfree(parr);
    }

    test_sub("subtest %d: increase pointer array", ++subnum);
    {
        int initsz = 25;
        Array *arr = PArray_create(initsz, ARRAY_FILLTYPE_ZERO);
        arr = ArrayIncrease(arr, initsz * 3);

        test_validatefree(
            arr->len == initsz * 3,
            Arrayfree(arr),
            "Array length %d must be %d", arr->len, initsz * 3
        );
        for (int i = initsz; i < arr->len; i++)
            test_validatefree(
                arr->pv[i] == NULL,
                Arrayfree(arr),
                "arr[%d] must be NULL, but not %p", i, (void*)arr->pv[i]
            );
        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
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
        Array   *arr = IArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr->iv[i] == i,
                Arrayfree(arr),
                "Int asc series: arr[%d] = %d, expected %d", i, arr->iv[i], i
            );
        }
    }

    /* 2. Int descending series */
    test_sub("subtest %d: int desc series", ++subnum);
    {
        int     cnt = 50;
        Array   *arr = IArray_create(cnt, ARRAY_FILLTYPE_DESC_SERIES);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            int expected = cnt - 1 - i;
            test_validatefree(
                arr->iv[i] == expected,
                Arrayfree(arr),
                "Int desc series: arr[%d] = %d, expected %d", i, arr->iv[i], expected
            );
        }
    }

    /* 3. Long ascending series */
    test_sub("subtest %d: long asc series", ++subnum);
    {
        int     cnt = 70;
        Array   *arr = LArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr->lv[i] == (long)i,
                Arrayfree(arr),
                "Long asc series: arr[%d] = %ld, expected %ld", i, arr->lv[i], (long)i
            );
        }
    }

    /* 4. Long descending series */
    test_sub("subtest %d: long desc series", ++subnum);
    {
        int     cnt = 40;
        Array   *arr = LArray_create(cnt, ARRAY_FILLTYPE_DESC_SERIES);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            long expected = (long)(cnt - 1 - i);
            test_validatefree(
                arr->lv[i] == expected,
                Arrayfree(arr),
                "Long desc series: arr[%d] = %ld, expected %ld", i, arr->lv[i], expected
            );
        }
    }

    /* 5. Double ascending series */
    test_sub("subtest %d: double asc series", ++subnum);
    {
        int     cnt = 30;
        Array   *arr = DArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr->dv[i] == (double)i,
                Arrayfree(arr),
                "Double asc series: arr[%d] = %f, expected %f", i, arr->dv[i], (double)i
            );
        }
    }

    /* 6. Double descending series */
    test_sub("subtest %d: double desc series", ++subnum);
    {
        int     cnt = 25;
        Array   *arr = DArray_create(cnt, ARRAY_FILLTYPE_DESC_SERIES);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            double expected = (double)(cnt - 1 - i);
            test_validatefree(
                arr->dv[i] == expected,
                Arrayfree(arr),
                "Double desc series: arr[%d] = %f, expected %f", i, arr->dv[i], expected
            );
        }
    }

    /* 7. Empty array */
    test_sub("subtest %d: empty series", ++subnum);
    {
        Array   *arr = IArray_create(0, ARRAY_FILLTYPE_ASC_SERIES);
        int     len = Arraylen(arr);
        test_validatefree(
            len == 0,
            Arrayfree(arr),
            "Empty array length = %d, expected 0", len
        );
    }

    /* 8. Unsupported type (pointers) – must raise error */
    test_sub("subtest %d: pointer series (unsupported)", ++subnum);
    {
        if (!try()) {
            Array *arr = PArray_create(10, ARRAY_FILLTYPE_ASC_SERIES);
            // We should not reach here
            test_validate(
                false,
                "Pointer series should have raised an error but didn't"
            );
            // to avoid unused variable warning, but it will never be used
            Array_print(arr, 0);
        } else {
            // Signal caught – expected behavior
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

    /* 1. Fill middle of int array with ascending series */
    test_sub("subtest %d: fill middle with asc series", ++subnum);
    {
        int     cnt = 50;
        Array   *arr = IArray_create(cnt, ARRAY_FILLTYPE_ZERO);
        int     from = 10, to = 20;

        ArrayFillRange(arr, ARRAY_FILLTYPE_ASC_SERIES, from, to);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        // Elements before 'from' and after 'to' must remain zero
        for (int i = 0; i < cnt; i++) {
            if (i >= from && i < to)
                continue;
            test_validatefree(
                arr->iv[i] == 0,
                Arrayfree(arr),
                "Element [%d] = %d, expected 0 (outside range)", i, arr->iv[i]
            );
        }

        // Inside the range, values must equal the index
        for (int i = from; i < to; i++) {
            test_validatefree(
                arr->iv[i] == i,
                Arrayfree(arr),
                "Element [%d] = %d, expected %d (inside range)", i, arr->iv[i], i
            );
        }
    }

    /* 2. Full fill of long array with descending series */
    test_sub("subtest %d: full fill with desc series (long)", ++subnum);
    {
        int     cnt = 30;
        Array   *arr = LArray_create(cnt, ARRAY_FILLTYPE_SAFE_EMPTY);
        ArrayFillRange(arr, ARRAY_FILLTYPE_DESC_SERIES, 0, cnt);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            long expected = (long)(cnt - 1 - i);
            test_validatefree(
                arr->lv[i] == expected,
                Arrayfree(arr),
                "Element [%d] = %ld, expected %ld", i, arr->lv[i], expected
            );
        }
    }

    /* 3. Empty range (from == to) – array remains unchanged */
    test_sub("subtest %d: from == to leaves array unchanged", ++subnum);
    {
        int     cnt = 20;
        Array   *arr = IArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);  // [0..19]
        ArrayFillRange(arr, ARRAY_FILLTYPE_RND, 5, 5);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr->iv[i] == i,
                Arrayfree(arr),
                "Element [%d] = %d, expected %d (unchanged after empty fill)", i, arr->iv[i], i
            );
        }
    }

    /* 4. Out-of-bounds – program must not crash */
    test_sub("subtest %d: out-of-bounds does not crash", ++subnum);
    {
        int     cnt = 10;
        Array   *arr = IArray_create(cnt, ARRAY_FILLTYPE_ZERO);
        ArrayFillRange(arr, ARRAY_FILLTYPE_ASC_SERIES, -5, cnt + 5);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "After out-of-bounds fill, length = %d, expected %d", len, cnt
        );
        // Content is not checked, as behavior is undefined
    }

    /* 5. Double array, ascending series fill in a sub-range */
    test_sub("subtest %d: double asc series fill range", ++subnum);
    {
        int     cnt = 25, from = 5, to = 15;
        Array   *arr = DArray_create(cnt, ARRAY_FILLTYPE_ZERO);
        ArrayFillRange(arr, ARRAY_FILLTYPE_ASC_SERIES, from, to);

        int     len = Arraylen(arr);
        test_validatefree(
            len == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", len, cnt
        );

        // Elements outside the range must remain zero
        for (int i = 0; i < cnt; i++) {
            if (i >= from && i < to) continue;
            test_validatefree(
                arr->dv[i] == 0.0,
                Arrayfree(arr),
                "Element [%d] = %f, expected 0.0 (outside range)", i, arr->dv[i]
            );
        }

        // Inside the range, values must equal the index
        for (int i = from; i < to; i++) {
            test_validatefree(
                arr->dv[i] == (double)i,
                Arrayfree(arr),
                "Element [%d] = %f, expected %f", i, arr->dv[i], (double)i
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
        Array   *arr = IArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);   // 0..9

        IArray_foreach(arr, elem) {
            if (*elem % 2 == 0)
                *elem /= 2;
            else
                *elem = 0;
        }

        test_validatefree(Arraylen(arr) == cnt, Arrayfree(arr), "Length mismatch");

        int expected[] = {0, 0, 1, 0, 2, 0, 3, 0, 4, 0};
        for (int i = 0; i < cnt; i++)
            test_validatefree(arr->iv[i] == expected[i], Arrayfree(arr),
                "int[%d]=%d expected %d", i, arr->iv[i], expected[i]);

        Arrayfree(arr);
    }

    /* 2. long array */
    test_sub("subtest %d: long array", ++subnum);
    {
        int     cnt = 8;
        Array   *arr = LArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);

        LArray_foreach(arr, elem) {
            if (*elem % 2 == 0)
                *elem /= 2;
            else
                *elem = 0;
        }

        test_validatefree(Arraylen(arr) == cnt, Arrayfree(arr), "Length mismatch");

        long expected[] = {0L, 0L, 1L, 0L, 2L, 0L, 3L, 0L};
        for (int i = 0; i < cnt; i++)
            test_validatefree(arr->lv[i] == expected[i], Arrayfree(arr),
                "long[%d]=%ld expected %ld", i, arr->lv[i], expected[i]);

        Arrayfree(arr);
    }

    /* 3. double array */
    test_sub("subtest %d: double array", ++subnum);
    {
        int     cnt = 6;
        Array   *arr = DArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);

        DArray_foreach(arr, elem) {
            if (fmod(*elem, 2.0) == 0.0)
                *elem /= 2.0;
            else
                *elem = 0.0;
        }

        test_validatefree(Arraylen(arr) == cnt, Arrayfree(arr), "Length mismatch");

        double expected[] = {0.0, 0.0, 1.0, 0.0, 2.0, 0.0};
        for (int i = 0; i < cnt; i++)
            test_validatefree(arr->dv[i] == expected[i], Arrayfree(arr),
                "double[%d]=%f expected %f", i, arr->dv[i], expected[i]);

        Arrayfree(arr);
    }

    /* 4. pointer array (no‑op) */
    test_sub("subtest %d: pointer array (no‑op)", ++subnum);
    {
        int     cnt = 3;
        Array   *arr = PArray_create(cnt, ARRAY_FILLTYPE_SAFE_EMPTY);
        arr->pv[0] = (void*)1; arr->pv[1] = (void*)2; arr->pv[2] = (void*)3;

        PArray_foreach(arr, elem) {
            // nothing
        }

        test_validatefree(Arraylen(arr) == cnt, Arrayfree(arr), "Length changed");
        test_validatefree(arr->pv[0] == (void*)1, Arrayfree(arr), "ptr[0] mismatch");
        test_validatefree(arr->pv[1] == (void*)2, Arrayfree(arr), "ptr[1] mismatch");
        test_validatefree(arr->pv[2] == (void*)3, Arrayfree(arr), "ptr[2] mismatch");

        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 13 ---------------------------------
static bool keep_if_index_not_multiple_of_3(Array *arr, int pos) {
    (void)arr;
    return (pos % 3) != 0;
}

static void square_int(Array *arr, int pos) {
    int val = arr->iv[pos];
    arr->iv[pos] = val * val;
}
static void square_long(Array *arr, int pos) {
    long val = arr->lv[pos];
    arr->lv[pos] = val * val;
}
static void mul_one_point_five_double(Array *arr, int pos) {
    arr->dv[pos] *= 1.5;
}

static TestStatus
tf13(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. int array */
    test_sub("subtest %d: int array (square non‑multiples of 3)", ++subnum);
    {
        int     cnt = 10;
        Array   *arr = IArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);

        Array_foreach_proc(arr, keep_if_index_not_multiple_of_3, square_int);

        test_validatefree(
            Arraylen(arr) == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", Arraylen(arr), cnt
        );

        int expected[] = {0, 1, 4, 3, 16, 25, 6, 49, 64, 9};
        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr->iv[i] == expected[i],
                Arrayfree(arr),
                "int proc: arr[%d] = %d, expected %d", i, arr->iv[i], expected[i]
            );
        }

        Arrayfree(arr);
    }

    /* 2. long array */
    test_sub("subtest %d: long array", ++subnum);
    {
        int     cnt = 8;
        Array   *arr = LArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);

        Array_foreach_proc(arr, keep_if_index_not_multiple_of_3, square_long);

        test_validatefree(
            Arraylen(arr) == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", Arraylen(arr), cnt
        );

        long expected[] = {0L, 1L, 4L, 3L, 16L, 25L, 6L, 49L};
        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr->lv[i] == expected[i],
                Arrayfree(arr),
                "long proc: arr[%d] = %ld, expected %ld", i, arr->lv[i], expected[i]
            );
        }

        Arrayfree(arr);
    }

    /* 3. double array */
    test_sub("subtest %d: double array", ++subnum);
    {
        int     cnt = 6;
        Array   *arr = DArray_create(cnt, ARRAY_FILLTYPE_ASC_SERIES);

        Array_foreach_proc(arr, keep_if_index_not_multiple_of_3, mul_one_point_five_double);

        test_validatefree(
            Arraylen(arr) == cnt,
            Arrayfree(arr),
            "Array length = %d, expected %d", Arraylen(arr), cnt
        );

        double expected[] = {0.0, 1.5, 3.0, 3.0, 6.0, 7.5};
        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                arr->dv[i] == expected[i],
                Arrayfree(arr),
                "double proc: arr[%d] = %f, expected %f", i, arr->dv[i], expected[i]
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
        Array *arr = V64Array_create(0, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        test_validatefree(
            arr->len == 0 && arr->sz == 0 && arr->v64 == NULL,
            Arrayfree(arr),
            "Empty STR array: len=%d, sz=%d, v64=%p (expected 0,0,NULL)",
            arr->len, arr->sz, (void*)arr->v64
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: create ZERO‑filled STR array", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ZERO, VALUE64_STR);
        test_validatefree(
            arr->len == 5 && arr->sz >= 5,
            Arrayfree(arr),
            "ZERO STR array: len=%d, sz=%d (expected 5, >=5)", arr->len, arr->sz
        );
        for (int i = 0; i < 5; i++) {
            test_validatefree(
                value64_str(arr->v64[i]) != NULL &&
                strcmp(value64_str(arr->v64[i]), "") == 0,
                Arrayfree(arr),
                "STR[%d] must be empty string, got '%s'",
                i, value64_str(arr->v64[i])
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);   // STR not related to FS, but kept for consistency

    test_sub("subtest %d: create NONE FS array", ++subnum);
    {
        Array *arrtmp = V64Array_create(2, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        Arrayfree(arrtmp);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: create ASC‑filled STR array", ++subnum);
    {
        Array *arr = V64Array_create(4, ARRAY_FILLTYPE_ASC, VALUE64_STR);
        test_validatefree(
            arr->len == 4,
            Arrayfree(arr),
            "ASC STR array: len=%d (expected 4)", arr->len
        );
        for (int i = 1; i < 4; i++) {
            test_validatefree(
                strlen(value64_str(arr->v64[i])) >= strlen(value64_str(arr->v64[i-1])),
                Arrayfree(arr),
                "ASC STR: length must be non‑decreasing"
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /*test_sub("subtest %d: create DESC‑filled STR array", ++subnum);
    {
        Array *arr = V64Array_create(4, ARRAY_FILLTYPE_DESC, VALUE64_STR);
        test_validatefree(
            arr->len == 4,
            Arrayfree(arr),
            "DESC STR array: len=%d (expected 4)", arr->len
        );
        for (int i = 1; i < 4; i++) {
            test_validatefree(
                strlen(value64_str(arr->v64[i])) <= strlen(value64_str(arr->v64[i-1])),
                Arrayfree(arr),
                "DESC STR: length must be non‑increasing"
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true); */    // because new v64 generator

    /* ---------- VALUE64_FS ---------- */
    test_sub("subtest %d: create empty FS array", ++subnum);
    {
        Array *arr = V64Array_create(0, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        test_validatefree(
            arr->len == 0 && arr->sz == 0 && arr->v64 == NULL,
            Arrayfree(arr),
            "Empty FS array: len=%d, sz=%d, v64=%p (expected 0,0,NULL)",
            arr->len, arr->sz, (void*)arr->v64
        );
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: create ZERO‑filled FS array", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ZERO, VALUE64_FS);
        test_validatefree(
            arr->len == 5 && arr->sz >= 5,
            Arrayfree(arr),
            "ZERO FS array: len=%d, sz=%d (expected 5, >=5)", arr->len, arr->sz
        );
        for (int i = 0; i < 5; i++) {
            fs *f = value64_fs(arr->v64[i]);
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
        Array *arr = V64Array_create(3, ARRAY_FILLTYPE_ASC, VALUE64_FS);
        test_validatefree(
            arr->len == 3,
            Arrayfree(arr),
            "ASC FS array: len=%d (expected 3)", arr->len
        );
        for (int i = 1; i < 3; i++) {
            test_validatefree(
                fs_len(value64_fs(arr->v64[i])) >= fs_len(value64_fs(arr->v64[i-1])),
                Arrayfree(arr),
                "ASC FS: length must be non‑decreasing"
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    
    /*test_sub("subtest %d: create DESC‑filled FS array", ++subnum);
    {
        Array *arr = V64Array_create(3, ARRAY_FILLTYPE_DESC, VALUE64_FS);
        test_validatefree(
            arr->len == 3,
            Arrayfree(arr),
            "DESC FS array: len=%d (expected 3)", arr->len
        );
        for (int i = 1; i < 3; i++) {
            test_validatefree(
                fs_len(value64_fs(arr->v64[i])) <= fs_len(value64_fs(arr->v64[i-1])),
                Arrayfree(arr),
                "DESC FS: length must be non‑increasing"
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true); */ // because new v64 generator

    /* ---------- RND fill ---------- */
    test_sub("subtest %d: create RND‑filled STR array", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_RND, VALUE64_STR);
        test_validatefree(
            arr->len == 5,
            Arrayfree(arr),
            "RND STR array: len=%d (expected 5)", arr->len
        );
        for (int i = 0; i < 5; i++) {
            const char *s = value64_str(arr->v64[i]);
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
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_RND, VALUE64_FS);
        test_validatefree(
            arr->len == 5,
            Arrayfree(arr),
            "RND FS array: len=%d (expected 5)", arr->len
        );
        for (int i = 0; i < 5; i++) {
            fs *f = value64_fs(arr->v64[i]);
            logmsg("VALUE64_FS: ARRAY_FILLTYPE_RND: %s", fs_str(f));
            test_validatefree(
                f != NULL && fs_len(f) > 0,
                Arrayfree(arr),
                "RND FS[%d] must be non‑empty, got len=%zu", i, f ? fs_len(f) : -1
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ---------- ASC / DESC with length check ---------- */
    test_sub("subtest %d: create ASC‑filled STR (lengths non‑decreasing)", ++subnum);
    {
        Array *arr = V64Array_create(4, ARRAY_FILLTYPE_ASC, VALUE64_STR);
        test_validatefree(arr->len == 4, Arrayfree(arr), "len check");
        for (int i = 1; i < 4; i++) {
            test_validatefree(
                strlen(value64_str(arr->v64[i])) >= strlen(value64_str(arr->v64[i-1])),
                Arrayfree(arr),
                "ASC STR: length must be non‑decreasing"
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /*test_sub("subtest %d: create DESC‑filled FS (lengths non‑increasing)", ++subnum);
    {
        Array *arr = V64Array_create(4, ARRAY_FILLTYPE_DESC, VALUE64_FS);
        test_validatefree(arr->len == 4, Arrayfree(arr), "len check");
        for (int i = 1; i < 4; i++) {
            test_validatefree(
                fs_len(value64_fs(arr->v64[i])) <= fs_len(value64_fs(arr->v64[i-1])),
                Arrayfree(arr),
                "DESC FS: length must be non‑increasing"
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true); */ // because new v64 generator

    /* ---------- ZERO (empty strings) ---------- */
    test_sub("subtest %d: ZERO STR array must have empty strings", ++subnum);
    {
        Array *arr = V64Array_create(3, ARRAY_FILLTYPE_ZERO, VALUE64_STR);
        for (int i = 0; i < 3; i++) {
            test_validatefree(
                strcmp(value64_str(arr->v64[i]), "") == 0,
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
        Array *arr = V64Array_create(3, ARRAY_FILLTYPE_ZERO, VALUE64_STR);
        // заполним явно, чтобы потом проверять
        arr->v64[0] = value64_createstr("first");
        arr->v64[1] = value64_createstr("second");
        arr->v64[2] = value64_createstr("third");

        // увеличиваем до 6
        arr = ArrayIncrease(arr, 6);
        test_validatefree(
            arr->len == 6 && arr->sz >= 6,
            Arrayfree(arr),
            "After increase len=%d (expected 6)", arr->len
        );
        // новые элементы должны быть пустыми строками
        for (int i = 3; i < 6; i++) {
            test_validatefree(
                value64_str(arr->v64[i]) != NULL && strcmp(value64_str(arr->v64[i]), "") == 0,
                Arrayfree(arr),
                "STR[%d] must be empty after increase", i
            );
        }

        // уменьшаем обратно до 3
        arr = ArrayShrink(arr, 3);
        test_validatefree(
            arr->len == 3 && arr->sz >= 3,
            Arrayfree(arr),
            "After shrink len=%d (expected 3)", arr->len
        );
        // старые элементы должны сохраниться
        test_validatefree(
            strcmp(value64_str(arr->v64[0]), "first") == 0 &&
            strcmp(value64_str(arr->v64[1]), "second") == 0 &&
            strcmp(value64_str(arr->v64[2]), "third") == 0,
            Arrayfree(arr),
            "STR elements must survive shrink"
        );

        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ========== FS array: increase then shrink ========== */
    test_sub("subtest %d: FS increase + shrink", ++subnum);
    {
        Array *arr = V64Array_create(2, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        // заполним явно
        arr->v64[0] = value64_createfs_asstr("/first");
        arr->v64[1] = value64_createfs_asstr("/second");

        // увеличиваем до 4
        arr = ArrayIncrease(arr, 4);
        test_validatefree(
            arr->len == 4 && arr->sz >= 4,
            Arrayfree(arr),
            "After increase len=%d (expected 4)", arr->len
        );
        // новые элементы – пустые fs
        for (int i = 2; i < 4; i++) {
            fs *f = value64_fs(arr->v64[i]);
            test_validatefree(
                f != NULL && fs_len(f) == 0,
                Arrayfree(arr),
                "FS[%d] must be empty after increase", i
            );
        }

        // уменьшаем до 1
        arr = ArrayShrink(arr, 1);
        test_validatefree(
            arr->len == 1 && arr->sz >= 1,
            Arrayfree(arr),
            "After shrink len=%d (expected 1)", arr->len
        );
        // первый элемент должен сохраниться
        test_validatefree(
            strcmp(fs_str(value64_fs(arr->v64[0])), "/first") == 0,
            Arrayfree(arr),
            "FS[0] must survive shrink"
        );

        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ========== Shrink to zero ========== */
    test_sub("subtest %d: shrink to zero", ++subnum);
    {
        Array *arr = V64Array_create(3, ARRAY_FILLTYPE_ZERO, VALUE64_STR);
        arr->v64[0] = value64_createstr("a");
        arr->v64[1] = value64_createstr("b");
        arr->v64[2] = value64_createstr("c");

        arr = ArrayShrink(arr, 0);
        test_validatefree(
            arr->len == 0 && arr->sz == 0,
            Arrayfree(arr),
            "Shrink to zero: len=%d sz=%d (expected 0,0)", arr->len, arr->sz
        );
        // v64 должен быть NULL (память освобождена)
        test_validatefree(
            arr->v64 == NULL,
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
        Array *arr = V64Array_create(4, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        arr->v64[0] = value64_createstr("delta");
        arr->v64[1] = value64_createstr("alpha");
        arr->v64[2] = value64_createstr("charlie");
        arr->v64[3] = value64_createstr("beta");

        Array_qsort(arr, ARRAY_FILLTYPE_ASC);

        const char *expected[] = {"alpha", "beta", "charlie", "delta"};
        for (int i = 0; i < 4; i++) {
            test_validatefree(
                strcmp(value64_str(arr->v64[i]), expected[i]) == 0,
                Arrayfree(arr),
                "STR ASC [%d]: expected '%s', got '%s'",
                i, expected[i], value64_str(arr->v64[i])
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: STR sort DESC", ++subnum);
    {
        Array *arr = V64Array_create(4, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        arr->v64[0] = value64_createstr("delta");
        arr->v64[1] = value64_createstr("alpha");
        arr->v64[2] = value64_createstr("charlie");
        arr->v64[3] = value64_createstr("beta");

        Array_qsort(arr, ARRAY_FILLTYPE_DESC);

        const char *expected[] = {"delta", "charlie", "beta", "alpha"};
        for (int i = 0; i < 4; i++) {
            test_validatefree(
                strcmp(value64_str(arr->v64[i]), expected[i]) == 0,
                Arrayfree(arr),
                "STR DESC [%d]: expected '%s', got '%s'",
                i, expected[i], value64_str(arr->v64[i])
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ========== FS sorting ========== */
    test_sub("subtest %d: FS sort ASC", ++subnum);
    {
        Array *arr = V64Array_create(4, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        arr->v64[0] = value64_createfs_asstr("/zzz");
        arr->v64[1] = value64_createfs_asstr("/aaa");
        arr->v64[2] = value64_createfs_asstr("/mmm");
        arr->v64[3] = value64_createfs_asstr("/bbb");

        Array_qsort(arr, ARRAY_FILLTYPE_ASC);

        const char *expected[] = {"/aaa", "/bbb", "/mmm", "/zzz"};
        for (int i = 0; i < 4; i++) {
            test_validatefree(
                strcmp(fs_str(value64_fs(arr->v64[i])), expected[i]) == 0,
                Arrayfree(arr),
                "FS ASC [%d]: expected '%s', got '%s'",
                i, expected[i], fs_str(value64_fs(arr->v64[i]))
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS sort DESC", ++subnum);
    {
        Array *arr = V64Array_create(4, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        arr->v64[0] = value64_createfs_asstr("/zzz");
        arr->v64[1] = value64_createfs_asstr("/aaa");
        arr->v64[2] = value64_createfs_asstr("/mmm");
        arr->v64[3] = value64_createfs_asstr("/bbb");

        Array_qsort(arr, ARRAY_FILLTYPE_DESC);

        const char *expected[] = {"/zzz", "/mmm", "/bbb", "/aaa"};
        for (int i = 0; i < 4; i++) {
            test_validatefree(
                strcmp(fs_str(value64_fs(arr->v64[i])), expected[i]) == 0,
                Arrayfree(arr),
                "FS DESC [%d]: expected '%s', got '%s'",
                i, expected[i], fs_str(value64_fs(arr->v64[i]))
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    /* ========== edge cases ========== */

    test_sub("subtest %d: STR sort empty array", ++subnum);
    {
        Array *arr = V64Array_create(0, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);   // must not crash
        test_validatefree(
            arr->len == 0,
            Arrayfree(arr),
            "Empty STR array after sort must still be empty"
        );
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: STR sort single element", ++subnum);
    {
        Array *arr = V64Array_create(1, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        arr->v64[0] = value64_createstr("single");
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        test_validatefree(
            strcmp(value64_str(arr->v64[0]), "single") == 0,
            Arrayfree(arr),
            "Single STR element must survive sorting"
        );
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: STR sort already sorted", ++subnum);
    {
        Array *arr = V64Array_create(3, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        arr->v64[0] = value64_createstr("a");
        arr->v64[1] = value64_createstr("b");
        arr->v64[2] = value64_createstr("c");
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        const char *exp[] = {"a", "b", "c"};
        for (int i = 0; i < 3; i++) {
            test_validatefree(
                strcmp(value64_str(arr->v64[i]), exp[i]) == 0,
                Arrayfree(arr),
                "Already sorted STR [%d] must stay '%s'", i, exp[i]
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: STR sort with duplicates", ++subnum);
    {
        Array *arr = V64Array_create(4, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        arr->v64[0] = value64_createstr("a");
        arr->v64[1] = value64_createstr("b");
        arr->v64[2] = value64_createstr("a");
        arr->v64[3] = value64_createstr("c");
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        const char *exp[] = {"a", "a", "b", "c"};
        for (int i = 0; i < 4; i++) {
            test_validatefree(
                strcmp(value64_str(arr->v64[i]), exp[i]) == 0,
                Arrayfree(arr),
                "Duplicates STR [%d] must be '%s'", i, exp[i]
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS sort empty array", ++subnum);
    {
        Array *arr = V64Array_create(0, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        test_validatefree(arr->len == 0, Arrayfree(arr), "Empty FS array after sort must still be empty");
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS sort single element", ++subnum);
    {
        Array *arr = V64Array_create(1, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        arr->v64[0] = value64_createfs_asstr("/only");
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        test_validatefree(
            strcmp(fs_str(value64_fs(arr->v64[0])), "/only") == 0,
            Arrayfree(arr),
            "Single FS element must survive sorting"
        );
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS sort already sorted", ++subnum);
    {
        Array *arr = V64Array_create(3, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        arr->v64[0] = value64_createfs_asstr("/a");
        arr->v64[1] = value64_createfs_asstr("/b");
        arr->v64[2] = value64_createfs_asstr("/c");
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        const char *exp[] = {"/a", "/b", "/c"};
        for (int i = 0; i < 3; i++) {
            test_validatefree(
                strcmp(fs_str(value64_fs(arr->v64[i])), exp[i]) == 0,
                Arrayfree(arr),
                "Already sorted FS [%d] must stay '%s'", i, exp[i]
            );
        }
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS sort with duplicates", ++subnum);
    {
        Array *arr = V64Array_create(4, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        arr->v64[0] = value64_createfs_asstr("/a");
        arr->v64[1] = value64_createfs_asstr("/b");
        arr->v64[2] = value64_createfs_asstr("/a");
        arr->v64[3] = value64_createfs_asstr("/c");
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        const char *exp[] = {"/a", "/a", "/b", "/c"};
        for (int i = 0; i < 4; i++) {
            test_validatefree(
                strcmp(fs_str(value64_fs(arr->v64[i])), exp[i]) == 0,
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
tf_v64ArraySaveFile_load(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== STR save/load ========== */
    test_sub("subtest %d: STR save/load", ++subnum);
    {
        const char *fname = "res/array/v64str.sv";

        Array *orig = V64Array_create(3, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        orig->v64[0] = value64_createstr("one");
        orig->v64[1] = value64_createstr("two");
        orig->v64[2] = value64_createstr("three");

        long written = ArraySaveFileName(orig, fname);
        test_validatefree(
            written > 0,
            Arrayfree(orig),
            "STR save failed"
        );

        Array *loaded = ArrayLoadFileName(fname);
        test_validatefree(
            loaded->len == orig->len,
            (Arrayfree(orig), Arrayfree(loaded)),
            "STR load: len=%d, expected %d", loaded->len, orig->len
        );

        for (int i = 0; i < orig->len; i++) {
            test_validatefree(
                strcmp(value64_str(orig->v64[i]), value64_str(loaded->v64[i])) == 0,
                (Arrayfree(orig), Arrayfree(loaded)),
                "STR[%d]: orig='%s', loaded='%s'",
                i, value64_str(orig->v64[i]), value64_str(loaded->v64[i])
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

        Array *orig = V64Array_create(3, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        orig->v64[0] = value64_createfs_asstr("/alpha");
        orig->v64[1] = value64_createfs_asstr("/beta");
        orig->v64[2] = value64_createfs_asstr("/gamma");

        long written = ArraySaveFileName(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "FS save failed");

        Array *loaded = ArrayLoadFileName(fname);
        test_validatefree(
            loaded->len == orig->len,
            (Arrayfree(orig), Arrayfree(loaded)),
            "FS load: len=%d, expected %d", loaded->len, orig->len
        );

        for (int i = 0; i < orig->len; i++) {
            fs *f_orig = value64_fs(orig->v64[i]);
            fs *f_load = value64_fs(loaded->v64[i]);
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

    /* ========== STR save/load: edge cases ========== */

    test_sub("subtest %d: STR save/load empty array", ++subnum);
    {
        const char *fname = "res/array/v64str_empty.sv";

        Array *orig = V64Array_create(0, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        long written = ArraySaveFileName(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "STR empty save failed");

        Array *loaded = ArrayLoadFileName(fname);
        test_validatefree(
            loaded->len == 0 && loaded->v64 == NULL,
            (Arrayfree(orig), Arrayfree(loaded)),
            "Loaded empty STR: len=%d, v64=%p (expected 0, NULL)", loaded->len, (void*)loaded->v64
        );

        Arrayfree(orig);
        Arrayfree(loaded);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: STR save/load single element", ++subnum);
    {
        const char *fname = "res/array/v64str_single.sv";

        Array *orig = V64Array_create(1, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        orig->v64[0] = value64_createstr("single");
        long written = ArraySaveFileName(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "STR single save failed");

        Array *loaded = ArrayLoadFileName(fname);
        test_validatefree(
            loaded->len == 1,
            (Arrayfree(orig), Arrayfree(loaded)),
            "Loaded single STR: len=%d, expected 1", loaded->len
        );
        test_validatefree(
            strcmp(value64_str(orig->v64[0]), value64_str(loaded->v64[0])) == 0,
            (Arrayfree(orig), Arrayfree(loaded)),
            "STR single: orig='%s', loaded='%s'",
            value64_str(orig->v64[0]), value64_str(loaded->v64[0])
        );

        Arrayfree(orig);
        Arrayfree(loaded);
    }
    fs_alloc_check(true);

    /* ========== FS save/load: edge cases ========== */

    test_sub("subtest %d: FS save/load empty array", ++subnum);
    {
        const char *fname = "res/array/v64fs_empty.sv";

        Array *orig = V64Array_create(0, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        long written = ArraySaveFileName(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "FS empty save failed");

        Array *loaded = ArrayLoadFileName(fname);
        test_validatefree(
            loaded->len == 0 && loaded->v64 == NULL,
            (Arrayfree(orig), Arrayfree(loaded)),
            "Loaded empty FS: len=%d, v64=%p (expected 0, NULL)", loaded->len, (void*)loaded->v64
        );

        Arrayfree(orig);
        Arrayfree(loaded);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS save/load single element", ++subnum);
    {
        const char *fname = "res/array/v64fs_single.sv";

        Array *orig = V64Array_create(1, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        orig->v64[0] = value64_createfs_asstr("/only");
        long written = ArraySaveFileName(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "FS single save failed");

        Array *loaded = ArrayLoadFileName(fname);
        test_validatefree(
            loaded->len == 1,
            (Arrayfree(orig), Arrayfree(loaded)),
            "Loaded single FS: len=%d, expected 1", loaded->len
        );
        fs *f_orig = value64_fs(orig->v64[0]);
        fs *f_load = value64_fs(loaded->v64[0]);
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
        Array *arr = IArray_create(10, ARRAY_FILLTYPE_ASC_SERIES); // 0,1,2,...,9
        int idx;
        test_validatefree(
            (idx = ArrayBsearchInt(arr, 5)) == 5,
            Arrayfree(arr),
            "INT asc: expected idx=5, got %d", idx
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: INT find missing", ++subnum);
    {
        Array *arr = IArray_create(10, ARRAY_FILLTYPE_ASC_SERIES);
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
        Array *arr = IArray_create(5, ARRAY_FILLTYPE_ASC_SERIES);
        test_validatefree(
            ArrayBsearchInt(arr, 0) == 0 && ArrayBsearchInt(arr, 4) == 4,
            Arrayfree(arr),
            "INT first/last: must be 0 and 4"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: INT rev search", ++subnum);
    {
        Array *arr = IArray_create(10, ARRAY_FILLTYPE_DESC_SERIES); // 9,8,...,0
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
        Array *arr = IArray_create(0, ARRAY_FILLTYPE_SAFE_EMPTY);
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
        Array *arr = LArray_create(10, ARRAY_FILLTYPE_ASC_SERIES);
        int idx;
        test_validatefree(
            (idx = ArrayBsearchLong(arr, 7L)) == 7,
            Arrayfree(arr),
            "LONG asc: expected idx=7, got %d", idx
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: LONG rev missing", ++subnum);
    {
        Array *arr = LArray_create(10, ARRAY_FILLTYPE_DESC_SERIES);
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
        Array *arr = DArray_create(5, ARRAY_FILLTYPE_ASC_SERIES); // 0.0,1.0,...,4.0
        test_validatefree(
            ArrayBsearchDbl(arr, 0.0) == 0 && ArrayBsearchDbl(arr, 4.0) == 4,
            Arrayfree(arr),
            "DBL first/last: must be 0 and 4"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: DBL rev search", ++subnum);
    {
        Array *arr = DArray_create(5, ARRAY_FILLTYPE_DESC_SERIES); // 4.0,3.0,...,0.0
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
        Array *arr = V64Array_create(4, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        arr->v64[0] = value64_createstr("a");
        arr->v64[1] = value64_createstr("b");
        arr->v64[2] = value64_createstr("c");
        arr->v64[3] = value64_createstr("d");

        value64 key = LITERAL64_STR("c");
        int idx;
        test_validatefree(
            (idx = ArrayBsearchV64(arr, key)) == 2,
            Arrayfree(arr),
            "STR asc: expected idx=2, got %d", idx
        );
        Arrayfree(arr);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: V64 STR rev missing", ++subnum);
    {
        Array *arr = V64Array_create(4, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        arr->v64[0] = value64_createstr("d");
        arr->v64[1] = value64_createstr("c");
        arr->v64[2] = value64_createstr("b");
        arr->v64[3] = value64_createstr("a");

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
        Array *arr = V64Array_create(4, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        arr->v64[0] = value64_createfs_asstr("/alpha");
        arr->v64[1] = value64_createfs_asstr("/beta");
        arr->v64[2] = value64_createfs_asstr("/gamma");
        arr->v64[3] = value64_createfs_asstr("/delta");

        Array_qsort(arr, ARRAY_FILLTYPE_ASC);

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

    /* ========== Type mismatch (must raise error) ========== */
    test_sub("subtest %d: type mismatch raises SIGINT", ++subnum);
    {
        Array *arr = IArray_create(3, ARRAY_FILLTYPE_ASC_SERIES);
        if (!try()) {
            ArrayBsearchLong(arr, 5L);
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
        Array *arr = CArray_create(0, ARRAY_FILLTYPE_SAFE_EMPTY);
        test_validatefree(
            arr->len == 0 && arr->sz == 0 && arr->cv == NULL,
            Arrayfree(arr),
            "Empty CHAR array: len=%d, sz=%d, cv=%p (expected 0,0,NULL)",
            arr->len, arr->sz, (void*)arr->cv
        );
        Arrayfree(arr);
    }

    /* 2. Создание ZERO‑filled CHAR массива */
    test_sub("subtest %d: create ZERO‑filled CHAR array", ++subnum);
    {
        Array *arr = CArray_create(5, ARRAY_FILLTYPE_ZERO);
        test_validatefree(
            arr->len == 5 && arr->sz >= 5,
            Arrayfree(arr),
            "ZERO CHAR array: len=%d, sz=%d (expected 5, >=5)", arr->len, arr->sz
        );
        // Проверяем, что все элементы — '\0'
        for (int i = 0; i < 5; i++) {
            test_validatefree(
                arr->cv[i] == '\0',
                Arrayfree(arr),
                "CHAR[%d] must be '\\0', got '%c'", i, arr->cv[i]
            );
        }
        Arrayfree(arr);
    }

    /* 3. Создание ASC‑filled CHAR массива (случайные буквы) */
    test_sub("subtest %d: create ASC‑filled CHAR array", ++subnum);
    {
        Array *arr = CArray_create(4, ARRAY_FILLTYPE_ASC);
        test_validatefree(
            arr->len == 4,
            Arrayfree(arr),
            "ASC CHAR array: len=%d (expected 4)", arr->len
        );
        // Элементы не должны быть '\0' и должны следовать в алфавитном порядке
        for (int i = 1; i < 4; i++) {
            test_validatefree(
                arr->cv[i - 1] <= arr->cv[i],
                Arrayfree(arr),
                "ASC CHAR: must be non‑decreasing"
            );
        }
        Arrayfree(arr);
    }

    /* 4. Создание DESC‑filled CHAR массива (случайные буквы в обратном порядке) */
    test_sub("subtest %d: create DESC‑filled CHAR array", ++subnum);
    {
        Array *arr = CArray_create(4, ARRAY_FILLTYPE_DESC);
        test_validatefree(
            arr->len == 4,
            Arrayfree(arr),
            "DESC CHAR array: len=%d (expected 4)", arr->len
        );
        // Элементы должны быть не возрастающими
        for (int i = 1; i < 4; i++) {
            test_validatefree(
                arr->cv[i - 1] >= arr->cv[i],
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
        Array *arr = CArray_create(6, ARRAY_FILLTYPE_RND);
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);

        for (int i = 1; i < arr->len; i++) {
            test_validatefree(
                arr->cv[i - 1] <= arr->cv[i],
                Arrayfree(arr),
                "ASC CHAR: cv[%d]='%c' > cv[%d]='%c'",
                i - 1, arr->cv[i - 1], i, arr->cv[i]
            );
        }
        Arrayfree(arr);
    }

    /* 2. Сортировка по убыванию */
    test_sub("subtest %d: CHAR sort DESC", ++subnum);
    {
        Array *arr = CArray_create(6, ARRAY_FILLTYPE_RND);
        Array_qsort(arr, ARRAY_FILLTYPE_DESC);

        for (int i = 1; i < arr->len; i++) {
            test_validatefree(
                arr->cv[i - 1] >= arr->cv[i],
                Arrayfree(arr),
                "DESC CHAR: cv[%d]='%c' < cv[%d]='%c'",
                i - 1, arr->cv[i - 1], i, arr->cv[i]
            );
        }
        Arrayfree(arr);
    }

    /* 3. Пустой массив */
    test_sub("subtest %d: CHAR sort empty", ++subnum);
    {
        Array *arr = CArray_create(0, ARRAY_FILLTYPE_SAFE_EMPTY);
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);   // не должно упасть
        test_validatefree(arr->len == 0, Arrayfree(arr), "Empty array must stay empty after sort");
        Arrayfree(arr);
    }

    /* 4. Один элемент */
    test_sub("subtest %d: CHAR sort single element", ++subnum);
    {
        Array *arr = CArray_create(1, ARRAY_FILLTYPE_SAFE_EMPTY);
        arr->cv[0] = 'x';
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        test_validatefree(
            arr->cv[0] == 'x',
            Arrayfree(arr),
            "Single element 'x' must stay 'x', got '%c'", arr->cv[0]
        );
        Arrayfree(arr);
    }

    /* 5. Уже отсортированный */
    test_sub("subtest %d: CHAR sort already sorted", ++subnum);
    {
        Array *arr = CArray_create(3, ARRAY_FILLTYPE_SAFE_EMPTY);
        arr->cv[0] = 'a'; arr->cv[1] = 'b'; arr->cv[2] = 'c';
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        test_validatefree(
            arr->cv[0] == 'a' && arr->cv[1] == 'b' && arr->cv[2] == 'c',
            Arrayfree(arr),
            "Already sorted array must stay 'a','b','c'"
        );
        Arrayfree(arr);
    }

    /* 6. Дубликаты */
    test_sub("subtest %d: CHAR sort duplicates", ++subnum);
    {
        Array *arr = CArray_create(4, ARRAY_FILLTYPE_SAFE_EMPTY);
        arr->cv[0] = 'b'; arr->cv[1] = 'a'; arr->cv[2] = 'b'; arr->cv[3] = 'c';
        Array_qsort(arr, ARRAY_FILLTYPE_ASC);
        test_validatefree(
            arr->cv[0] == 'a' && arr->cv[1] == 'b' && arr->cv[2] == 'b' && arr->cv[3] == 'c',
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
        Array *arr = CArray_create(5, ARRAY_FILLTYPE_SAFE_EMPTY);
        arr->cv[0] = 'a'; arr->cv[1] = 'b'; arr->cv[2] = 'c'; arr->cv[3] = 'd'; arr->cv[4] = 'e';
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
        Array *arr = CArray_create(5, ARRAY_FILLTYPE_SAFE_EMPTY);
        arr->cv[0] = 'a'; arr->cv[1] = 'b'; arr->cv[2] = 'c'; arr->cv[3] = 'd'; arr->cv[4] = 'e';
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
        Array *arr = CArray_create(5, ARRAY_FILLTYPE_SAFE_EMPTY);
        arr->cv[0] = 'e'; arr->cv[1] = 'd'; arr->cv[2] = 'c'; arr->cv[3] = 'b'; arr->cv[4] = 'a';
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
tf_ArraySaveFile_load_char(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== CHAR ========== */
    test_sub("subtest %d: CHAR save/load", ++subnum);
    {
        const char *fname = "res/array/carr.sv";

        // создаём массив и заполняем
        Array *orig = CArray_create(5, ARRAY_FILLTYPE_SAFE_EMPTY);
        orig->cv[0] = 'h'; orig->cv[1] = 'e'; orig->cv[2] = 'l';
        orig->cv[3] = 'l'; orig->cv[4] = 'o';

        // сохраняем
        long written = ArraySaveFileName(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "CHAR save failed");

        // загружаем
        Array *loaded = ArrayLoadFileName(fname);
        test_validatefree(
            loaded->len == orig->len,
            (Arrayfree(orig), Arrayfree(loaded)),
            "CHAR load: len=%d, expected %d", loaded->len, orig->len
        );

        // сравниваем поэлементно
        for (int i = 0; i < orig->len; i++) {
            test_validatefree(
                orig->cv[i] == loaded->cv[i],
                (Arrayfree(orig), Arrayfree(loaded)),
                "CHAR[%d]: orig='%c', loaded='%c'",
                i, orig->cv[i], loaded->cv[i]
            );
        }

        Arrayfree(orig);
        Arrayfree(loaded);
    }

    /* ========== CHAR: пустой массив ========== */
    test_sub("subtest %d: CHAR save/load empty", ++subnum);
    {
        const char *fname = "res/array/carr_empty.sv";

        Array *orig = CArray_create(0, ARRAY_FILLTYPE_SAFE_EMPTY);
        long written = ArraySaveFileName(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "CHAR empty save failed");

        Array *loaded = ArrayLoadFileName(fname);
        test_validatefree(
            loaded->len == 0 && loaded->cv == NULL,
            (Arrayfree(orig), Arrayfree(loaded)),
            "Loaded empty CHAR: len=%d, cv=%p (expected 0, NULL)", loaded->len, (void*)loaded->cv
        );

        Arrayfree(orig);
        Arrayfree(loaded);
    }

    /* ========== CHAR: один элемент ========== */
    test_sub("subtest %d: CHAR save/load single", ++subnum);
    {
        const char *fname = "res/array/carr_single.sv";

        Array *orig = CArray_create(1, ARRAY_FILLTYPE_SAFE_EMPTY);
        orig->cv[0] = 'Z';
        long written = ArraySaveFileName(orig, fname);
        test_validatefree(written > 0, Arrayfree(orig), "CHAR single save failed");

        Array *loaded = ArrayLoadFileName(fname);
        test_validatefree(
            loaded->len == 1 && loaded->cv[0] == 'Z',
            (Arrayfree(orig), Arrayfree(loaded)),
            "Loaded single CHAR: expected 'Z', got '%c'", loaded->cv[0]
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
        Array *a = IArray_create(3, ARRAY_FILLTYPE_SAFE_EMPTY);
        Array *b = IArray_create(3, ARRAY_FILLTYPE_SAFE_EMPTY);
        a->iv[0] = 1; a->iv[1] = 2; a->iv[2] = 3;
        b->iv[0] = 1; b->iv[1] = 2; b->iv[2] = 3;
        test_validatefree(
            ArrayEq(a, b) && !ArrayNoteq(a, b),
            (Arrayfree(a), Arrayfree(b)),
            "Identical INT arrays must be equal"
        );
        Arrayfree(a); Arrayfree(b);
    }

    test_sub("subtest %d: INT not equal (different values)", ++subnum);
    {
        Array *a = IArray_create(2, ARRAY_FILLTYPE_SAFE_EMPTY);
        Array *b = IArray_create(2, ARRAY_FILLTYPE_SAFE_EMPTY);
        a->iv[0] = 10; a->iv[1] = 20;
        b->iv[0] = 10; b->iv[1] = 30;
        test_validatefree(
            !ArrayEq(a, b) && ArrayNoteq(a, b),
            (Arrayfree(a), Arrayfree(b)),
            "Different values must be not equal"
        );
        Arrayfree(a); Arrayfree(b);
    }

    test_sub("subtest %d: INT not equal (different lengths)", ++subnum);
    {
        Array *a = IArray_create(2, ARRAY_FILLTYPE_SAFE_EMPTY);
        Array *b = IArray_create(3, ARRAY_FILLTYPE_SAFE_EMPTY);
        test_validatefree(
            !ArrayEq(a, b) && ArrayNoteq(a, b),
            (Arrayfree(a), Arrayfree(b)),
            "Different lengths must be not equal"
        );
        Arrayfree(a); Arrayfree(b);
    }

    test_sub("subtest %d: INT empty arrays", ++subnum);
    {
        Array *a = IArray_create(0, ARRAY_FILLTYPE_SAFE_EMPTY);
        Array *b = IArray_create(0, ARRAY_FILLTYPE_SAFE_EMPTY);
        test_validatefree(
            ArrayEq(a, b) && !ArrayNoteq(a, b),
            (Arrayfree(a), Arrayfree(b)),
            "Empty INT arrays must be equal"
        );
        Arrayfree(a); Arrayfree(b);
    }

    /* ========== CHAR ========== */
    test_sub("subtest %d: CHAR equal", ++subnum);
    {
        Array *a = CArray_create(2, ARRAY_FILLTYPE_SAFE_EMPTY);
        Array *b = CArray_create(2, ARRAY_FILLTYPE_SAFE_EMPTY);
        a->cv[0] = 'x'; a->cv[1] = 'y';
        b->cv[0] = 'x'; b->cv[1] = 'y';
        test_validatefree(
            ArrayEq(a, b) && !ArrayNoteq(a, b),
            (Arrayfree(a), Arrayfree(b)),
            "Identical CHAR arrays must be equal"
        );
        Arrayfree(a); Arrayfree(b);
    }

    test_sub("subtest %d: CHAR not equal", ++subnum);
    {
        Array *a = CArray_create(1, ARRAY_FILLTYPE_SAFE_EMPTY);
        Array *b = CArray_create(1, ARRAY_FILLTYPE_SAFE_EMPTY);
        a->cv[0] = 'a';
        b->cv[0] = 'b';
        test_validatefree(
            !ArrayEq(a, b) && ArrayNoteq(a, b),
            (Arrayfree(a), Arrayfree(b)),
            "Different CHAR must be not equal"
        );
        Arrayfree(a); Arrayfree(b);
    }

    /* ========== V64 STR ========== */
    test_sub("subtest %d: V64 STR equal", ++subnum);
    {
        Array *a = V64Array_create(2, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        Array *b = V64Array_create(2, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        a->v64[0] = value64_createstr("hello");
        a->v64[1] = value64_createstr("world");
        b->v64[0] = value64_createstr("hello");
        b->v64[1] = value64_createstr("world");
        test_validatefree(
            ArrayEq(a, b) && !ArrayNoteq(a, b),
            (Arrayfree(a), Arrayfree(b)),
            "Identical STR arrays must be equal"
        );
        Arrayfree(a); Arrayfree(b);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: V64 STR not equal", ++subnum);
    {
        Array *a = V64Array_create(1, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        Array *b = V64Array_create(1, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        a->v64[0] = value64_createstr("abc");
        b->v64[0] = value64_createstr("xyz");
        test_validatefree(
            !ArrayEq(a, b) && ArrayNoteq(a, b),
            (Arrayfree(a), Arrayfree(b)),
            "Different STR must be not equal"
        );
        Arrayfree(a); Arrayfree(b);
    }
    fs_alloc_check(true);

    /* ========== V64 FS ========== */
    test_sub("subtest %d: V64 FS equal", ++subnum);
    {
        Array *a = V64Array_create(2, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        Array *b = V64Array_create(2, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        a->v64[0] = value64_createfs_asstr("/tmp/a");
        a->v64[1] = value64_createfs_asstr("/tmp/b");
        b->v64[0] = value64_createfs_asstr("/tmp/a");
        b->v64[1] = value64_createfs_asstr("/tmp/b");
        test_validatefree(
            ArrayEq(a, b) && !ArrayNoteq(a, b),
            (Arrayfree(a), Arrayfree(b)),
            "Identical FS arrays must be equal"
        );
        Arrayfree(a); Arrayfree(b);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: V64 FS not equal", ++subnum);
    {
        Array *a = V64Array_create(1, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        Array *b = V64Array_create(1, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        a->v64[0] = value64_createfs_asstr("/first");
        b->v64[0] = value64_createfs_asstr("/second");
        test_validatefree(
            !ArrayEq(a, b) && ArrayNoteq(a, b),
            (Arrayfree(a), Arrayfree(b)),
            "Different FS must be not equal"
        );
        Arrayfree(a); Arrayfree(b);
    }
    fs_alloc_check(true);

    /* ========== Type mismatch (must raise SIGINT) ========== */
    test_sub("subtest %d: type mismatch raises SIGINT", ++subnum);
    {
        Array *a = IArray_create(1, ARRAY_FILLTYPE_SAFE_EMPTY);
        Array *b = CArray_create(1, ARRAY_FILLTYPE_SAFE_EMPTY);
        if (!try()) {
            ArrayNoteq(a, b);
            test_validatefree(false, (Arrayfree(a), Arrayfree(b)),
                             "Type mismatch should have raised SIGINT");
        } else {
            logsimple("Exception correctly raised on type mismatch");
        }
        Arrayfree(a); Arrayfree(b);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST ArrayDel simple test -------------------------
static TestStatus
tf_ArrayDel(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== ArrayDel ========== */
    test_sub("subtest %d: ArrayDel middle one element", ++subnum);
    {
        Array *arr = IArray_create(4, ARRAY_FILLTYPE_ASC_SERIES);
        arr->iv[0] = 10; arr->iv[1] = 20; arr->iv[2] = 30; arr->iv[3] = 40;
        arr->len = 4;

        arr = ArrayDel(arr, 1, 1);
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            arr->iv[0] == 10 && arr->iv[1] == 30 && arr->iv[2] == 40,
            Arrayfree(arr),
            "Del middle 1: expected [10,30,40] len=3, got len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayDel middle multiple elements", ++subnum);
    {
        Array *arr = IArray_create(4, ARRAY_FILLTYPE_ASC_SERIES);
        arr->iv[0] = 10; arr->iv[1] = 20; arr->iv[2] = 30; arr->iv[3] = 40;
        arr->len = 4;

        arr = ArrayDel(arr, 1, 2);
        test_validatefree(
            arr != NULL && Arraylen(arr) == 2 &&
            arr->iv[0] == 10 && arr->iv[1] == 40,
            Arrayfree(arr),
            "Del middle 2: expected [10,40] len=2, got len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayDel from start", ++subnum);
    {
        Array *arr = IArray_create(4, ARRAY_FILLTYPE_ASC_SERIES);
        arr->iv[0] = 10; arr->iv[1] = 20; arr->iv[2] = 30; arr->iv[3] = 40;
        arr->len = 4;

        arr = ArrayDel(arr, 0, 1);
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            arr->iv[0] == 20 && arr->iv[1] == 30 && arr->iv[2] == 40,
            Arrayfree(arr),
            "Del start: expected [20,30,40] len=3, got len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayDel from end", ++subnum);
    {
        Array *arr = IArray_create(4, ARRAY_FILLTYPE_ASC_SERIES);
        arr->iv[0] = 10; arr->iv[1] = 20; arr->iv[2] = 30; arr->iv[3] = 40;
        arr->len = 4;

        arr = ArrayDel(arr, 3, 1);
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            arr->iv[0] == 10 && arr->iv[1] == 20 && arr->iv[2] == 30,
            Arrayfree(arr),
            "Del end: expected [10,20,30] len=3, got len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayDel all elements", ++subnum);
    {
        Array *arr = IArray_create(4, ARRAY_FILLTYPE_ASC_SERIES);
        arr->iv[0] = 10; arr->iv[1] = 20; arr->iv[2] = 30; arr->iv[3] = 40;
        arr->len = 4;

        arr = ArrayDel(arr, 0, 4);
        test_validatefree(
            arr != NULL && Arraylen(arr) == 0,
            Arrayfree(arr),
            "Del all: expected empty array, got len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayDel zero count", ++subnum);
    {
        Array *arr = IArray_create(3, ARRAY_FILLTYPE_ASC_SERIES);
        arr->iv[0] = 1; arr->iv[1] = 2; arr->iv[2] = 3;
        arr->len = 3;

        arr = ArrayDel(arr, 1, 0);
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            arr->iv[0] == 1 && arr->iv[1] == 2 && arr->iv[2] == 3,
            Arrayfree(arr),
            "Del zero: array must remain unchanged, len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayDel from beyond length", ++subnum);
    {
        Array *arr = IArray_create(3, ARRAY_FILLTYPE_ASC_SERIES);
        arr->iv[0] = 1; arr->iv[1] = 2; arr->iv[2] = 3;
        arr->len = 3;

        arr = ArrayDel(arr, 5, 1);
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            arr->iv[0] == 1 && arr->iv[1] == 2 && arr->iv[2] == 3,
            Arrayfree(arr),
            "Del beyond: array must remain unchanged, len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayDel cnt not exceeds length", ++subnum);
    {
        Array *arr = IArray_create(4, ARRAY_FILLTYPE_ASC_SERIES);
        arr->iv[0] = 10; arr->iv[1] = 20; arr->iv[2] = 30; arr->iv[3] = 40;
        //arr->len = 4;

        arr = ArrayDel(arr, 2, 2 /*10*/ );   // удалит 30,40
        test_validatefree(
            arr != NULL && Arraylen(arr) == 2 &&
            arr->iv[0] == 10 && arr->iv[1] == 20,
            Arrayfree(arr),
            "Del over: expected [10,20] len=2, got len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayDel cnt exceeds length", ++subnum);
    {
        Array *arr = IArray_create(4, ARRAY_FILLTYPE_ASC_SERIES);
        arr->iv[0] = 10; arr->iv[1] = 20; arr->iv[2] = 30; arr->iv[3] = 40;
        //arr->len = 4;

        arr = ArrayDel(arr, 2, 10);   // удалит 30,40
        test_validatefree(
            arr != NULL && Arraylen(arr) == 2 &&
            arr->iv[0] == 10 && arr->iv[1] == 20,
            Arrayfree(arr),
            "Del over: expected [10,20] len=2, got len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST ArrayAdd simple test -------------------------
static TestStatus
tf_ArrayAdd(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== ArrayAdd ========== */
    test_sub("subtest %d: ArrayAdd one element in middle", ++subnum);
    {
        Array *arr = IArray_create(3, ARRAY_FILLTYPE_ASC_SERIES); // [0,1,2]
        arr->iv[0] = 10; arr->iv[1] = 20; arr->iv[2] = 30;

        arr = ArrayAdd(arr, 1, 1, ARRAY_FILLTYPE_ZERO);
        arr->iv[1] = 99;
        test_validatefree(
            arr != NULL && Arraylen(arr) == 4 &&
            arr->iv[0] == 10 && arr->iv[1] == 99 && arr->iv[2] == 20 && arr->iv[3] == 30,
            Arrayfree(arr),
            "Add middle 1: expected [10,99,20,30] len=4, got len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayAdd multiple in middle", ++subnum);
    {
        Array *arr = IArray_create(2, ARRAY_FILLTYPE_ASC_SERIES); // [0,1]
        arr->iv[0] = 5; arr->iv[1] = 15;

        arr = ArrayAdd(arr, 1, 2, ARRAY_FILLTYPE_ZERO);
        arr->iv[1] = 7;
        arr->iv[2] = 10;
        test_validatefree(
            arr != NULL && Arraylen(arr) == 4 &&
            arr->iv[0] == 5 && arr->iv[1] == 7 && arr->iv[2] == 10 && arr->iv[3] == 15,
            Arrayfree(arr),
            "Add middle 2: expected [5,7,10,15] len=4, got len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayAdd at beginning", ++subnum);
    {
        Array *arr = IArray_create(2, ARRAY_FILLTYPE_ASC_SERIES); // [0,1]
        arr->iv[0] = 20; arr->iv[1] = 30;

        arr = ArrayAdd(arr, 0, 1, ARRAY_FILLTYPE_ZERO);
        arr->iv[0] = 10;
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            arr->iv[0] == 10 && arr->iv[1] == 20 && arr->iv[2] == 30,
            Arrayfree(arr),
            "Add start: expected [10,20,30] len=3, got len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayAdd at end", ++subnum);
    {
        Array *arr = IArray_create(2, ARRAY_FILLTYPE_ASC_SERIES); // [0,1]
        arr->iv[0] = 1; arr->iv[1] = 2;

        arr = ArrayAdd(arr, 2, 1, ARRAY_FILLTYPE_ZERO);  // from == len
        arr->iv[2] = 3;
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            arr->iv[0] == 1 && arr->iv[1] == 2 && arr->iv[2] == 3,
            Arrayfree(arr),
            "Add end: expected [1,2,3] len=3, got len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayAdd zero cnt (no-op)", ++subnum);
    {
        Array *arr = IArray_create(3, ARRAY_FILLTYPE_ASC_SERIES); // [0,1,2]
        arr->iv[0] = 1; arr->iv[1] = 2; arr->iv[2] = 3;

        arr = ArrayAdd(arr, 1, 0, ARRAY_FILLTYPE_ZERO);
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            arr->iv[0] == 1 && arr->iv[1] == 2 && arr->iv[2] == 3,
            Arrayfree(arr),
            "Add zero: array unchanged, len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayAdd from out of bounds (no-op)", ++subnum);
    {
        Array *arr = IArray_create(3, ARRAY_FILLTYPE_ASC_SERIES); // [0,1,2]
        arr->iv[0] = 1; arr->iv[1] = 2; arr->iv[2] = 3;

        arr = ArrayAdd(arr, 5, 1, ARRAY_FILLTYPE_ZERO);
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            arr->iv[0] == 1 && arr->iv[1] == 2 && arr->iv[2] == 3,
            Arrayfree(arr),
            "Add out of bounds: array unchanged, len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: ArrayAdd to empty array", ++subnum);
    {
        // Создаем массив с нулевой длиной
        Array *arr = IArray_create(0, ARRAY_FILLTYPE_ASC_SERIES); 

        arr = ArrayAdd(arr, 0, 3, ARRAY_FILLTYPE_ZERO);
        arr->iv[0] = 10; arr->iv[1] = 20; arr->iv[2] = 30;

        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            arr->iv[0] == 10 && arr->iv[1] == 20 && arr->iv[2] == 30,
            Arrayfree(arr),
            "Add to empty: expected [10,20,30] len=3, got len=%d", Arraylen(arr)
        );
        Arrayfree(arr);
    }

    /* ========== V64 ArrayAdd (STR type) ========== */
    test_sub("subtest %d: V64(STR) ArrayAdd one element middle", ++subnum);
    {
        Array *arr = V64Array_create(3, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        arr->v64[0] = value64_createstr("a");
        arr->v64[1] = value64_createstr("b");
        arr->v64[2] = value64_createstr("c");
        arr->len = 3;

        arr = ArrayAdd(arr, 1, 1, ARRAY_FILLTYPE_ZERO);
        // ZERO для STR заполнит пустой строкой ""
        test_validatefree(
            arr != NULL && Arraylen(arr) == 4 &&
            strcmp(value64_str(arr->v64[0]), "a") == 0 &&
            strcmp(value64_str(arr->v64[1]), "") == 0 &&   // заполнено ZERO
            strcmp(value64_str(arr->v64[2]), "b") == 0 &&
            strcmp(value64_str(arr->v64[3]), "c") == 0,
            Arrayfree(arr),
            "V64(STR) Add middle 1: expected [\"a\",\"\",\"b\",\"c\"]"
        );
        // перезапишем
        value64free(arr->v64[1], VALUE64_STR);
        arr->v64[1] = value64_createstr("X");
        test_validatefree(
            strcmp(value64_str(arr->v64[1]), "X") == 0,
            Arrayfree(arr),
            "V64(STR) Add middle 1: after set idx 1 = X"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: V64(STR) ArrayAdd multiple middle", ++subnum);
    {
        Array *arr = V64Array_create(2, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        arr->v64[0] = value64_createstr("first");
        arr->v64[1] = value64_createstr("last");
        arr->len = 2;

        arr = ArrayAdd(arr, 1, 2, ARRAY_FILLTYPE_ZERO);
        // после сдвига: [0]="first", [1]="", [2]="", [3]="last"
        value64free(arr->v64[1], VALUE64_STR);
        value64free(arr->v64[2], VALUE64_STR);
        arr->v64[1] = value64_createstr("middle1");
        arr->v64[2] = value64_createstr("middle2");

        test_validatefree(
            arr != NULL && Arraylen(arr) == 4 &&
            strcmp(value64_str(arr->v64[0]), "first") == 0 &&
            strcmp(value64_str(arr->v64[1]), "middle1") == 0 &&
            strcmp(value64_str(arr->v64[2]), "middle2") == 0 &&
            strcmp(value64_str(arr->v64[3]), "last") == 0,
            Arrayfree(arr),
            "V64(STR) Add middle 2: expected [first,middle1,middle2,last]"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: V64(STR) ArrayAdd at beginning", ++subnum);
    {
        Array *arr = V64Array_create(2, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        arr->v64[0] = value64_createstr("world");
        arr->v64[1] = value64_createstr("!");
        arr->len = 2;

        arr = ArrayAdd(arr, 0, 1, ARRAY_FILLTYPE_ZERO);
        value64free(arr->v64[0], VALUE64_STR);
        arr->v64[0] = value64_createstr("Hello");
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            strcmp(value64_str(arr->v64[0]), "Hello") == 0 &&
            strcmp(value64_str(arr->v64[1]), "world") == 0 &&
            strcmp(value64_str(arr->v64[2]), "!") == 0,
            Arrayfree(arr),
            "V64(STR) Add start: expected [Hello,world,!]"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: V64(STR) ArrayAdd at end", ++subnum);
    {
        Array *arr = V64Array_create(2, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        arr->v64[0] = value64_createstr("x");
        arr->v64[1] = value64_createstr("y");
        arr->len = 2;

        arr = ArrayAdd(arr, 2, 1, ARRAY_FILLTYPE_ZERO);
        value64free(arr->v64[2], VALUE64_STR);
        arr->v64[2] = value64_createstr("z");
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            strcmp(value64_str(arr->v64[0]), "x") == 0 &&
            strcmp(value64_str(arr->v64[1]), "y") == 0 &&
            strcmp(value64_str(arr->v64[2]), "z") == 0,
            Arrayfree(arr),
            "V64(STR) Add end: expected [x,y,z]"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: V64(STR) ArrayAdd zero cnt (no-op)", ++subnum);
    {
        Array *arr = V64Array_create(3, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        arr->v64[0] = value64_createstr("1");
        arr->v64[1] = value64_createstr("2");
        arr->v64[2] = value64_createstr("3");
        arr->len = 3;

        arr = ArrayAdd(arr, 1, 0, ARRAY_FILLTYPE_ZERO);
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            strcmp(value64_str(arr->v64[0]), "1") == 0 &&
            strcmp(value64_str(arr->v64[1]), "2") == 0 &&
            strcmp(value64_str(arr->v64[2]), "3") == 0,
            Arrayfree(arr),
            "V64(STR) Add zero: array unchanged"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: V64(STR) ArrayAdd out of bounds (no-op)", ++subnum);
    {
        Array *arr = V64Array_create(3, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        arr->v64[0] = value64_createstr("a");
        arr->v64[1] = value64_createstr("b");
        arr->v64[2] = value64_createstr("c");
        arr->len = 3;

        arr = ArrayAdd(arr, 5, 1, ARRAY_FILLTYPE_ZERO);
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            strcmp(value64_str(arr->v64[0]), "a") == 0 &&
            strcmp(value64_str(arr->v64[1]), "b") == 0 &&
            strcmp(value64_str(arr->v64[2]), "c") == 0,
            Arrayfree(arr),
            "V64(STR) Add out of bounds: array unchanged"
        );
        Arrayfree(arr);
    }

    /* ========== V64(FS) ArrayAdd ========== */
    test_sub("subtest %d: V64(FS) ArrayAdd one element middle", ++subnum);
    {
        Array *arr = V64Array_create(3, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        arr->v64[0] = value64_createfs_asstr("a");
        arr->v64[1] = value64_createfs_asstr("b");
        arr->v64[2] = value64_createfs_asstr("c");
        arr->len = 3;

        arr = ArrayAdd(arr, 1, 1, ARRAY_FILLTYPE_ZERO);
        // ZERO для FS создаёт пустую строку ""
        test_validatefree(
            arr != NULL && Arraylen(arr) == 4 &&
            strcmp(fs_str(value64_fs(arr->v64[0])), "a") == 0 &&
            strcmp(fs_str(value64_fs(arr->v64[1])), "") == 0 &&  // ZERO
            strcmp(fs_str(value64_fs(arr->v64[2])), "b") == 0 &&
            strcmp(fs_str(value64_fs(arr->v64[3])), "c") == 0,
            Arrayfree(arr),
            "V64(FS) Add middle 1: expected [\"a\",\"\",\"b\",\"c\"]"
        );
        // заменяем пустую строку на "X"
        value64free(arr->v64[1], VALUE64_FS);
        arr->v64[1] = value64_createfs_asstr("X");
        test_validatefree(
            strcmp(fs_str(value64_fs(arr->v64[1])), "X") == 0,
            Arrayfree(arr),
            "V64(FS) Add middle 1: after set idx 1 = X"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: V64(FS) ArrayAdd multiple middle", ++subnum);
    {
        Array *arr = V64Array_create(2, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        arr->v64[0] = value64_createfs_asstr("first");
        arr->v64[1] = value64_createfs_asstr("last");
        arr->len = 2;

        arr = ArrayAdd(arr, 1, 2, ARRAY_FILLTYPE_ZERO);
        // после сдвига: [0]="first", [1]="", [2]="", [3]="last"
        value64free(arr->v64[1], VALUE64_FS);
        value64free(arr->v64[2], VALUE64_FS);
        arr->v64[1] = value64_createfs_asstr("middle1");
        arr->v64[2] = value64_createfs_asstr("middle2");

        test_validatefree(
            arr != NULL && Arraylen(arr) == 4 &&
            strcmp(fs_str(value64_fs(arr->v64[0])), "first") == 0 &&
            strcmp(fs_str(value64_fs(arr->v64[1])), "middle1") == 0 &&
            strcmp(fs_str(value64_fs(arr->v64[2])), "middle2") == 0 &&
            strcmp(fs_str(value64_fs(arr->v64[3])), "last") == 0,
            Arrayfree(arr),
            "V64(FS) Add middle 2: expected [first,middle1,middle2,last]"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: V64(FS) ArrayAdd at beginning", ++subnum);
    {
        Array *arr = V64Array_create(2, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        arr->v64[0] = value64_createfs_asstr("world");
        arr->v64[1] = value64_createfs_asstr("!");
        arr->len = 2;

        arr = ArrayAdd(arr, 0, 1, ARRAY_FILLTYPE_ZERO);
        value64free(arr->v64[0], VALUE64_FS);
        arr->v64[0] = value64_createfs_asstr("Hello");
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            strcmp(fs_str(value64_fs(arr->v64[0])), "Hello") == 0 &&
            strcmp(fs_str(value64_fs(arr->v64[1])), "world") == 0 &&
            strcmp(fs_str(value64_fs(arr->v64[2])), "!") == 0,
            Arrayfree(arr),
            "V64(FS) Add start: expected [Hello,world,!]"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: V64(FS) ArrayAdd at end", ++subnum);
    {
        Array *arr = V64Array_create(2, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        arr->v64[0] = value64_createfs_asstr("x");
        arr->v64[1] = value64_createfs_asstr("y");
        arr->len = 2;

        arr = ArrayAdd(arr, 2, 1, ARRAY_FILLTYPE_ZERO);
        value64free(arr->v64[2], VALUE64_FS);
        arr->v64[2] = value64_createfs_asstr("z");
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            strcmp(fs_str(value64_fs(arr->v64[0])), "x") == 0 &&
            strcmp(fs_str(value64_fs(arr->v64[1])), "y") == 0 &&
            strcmp(fs_str(value64_fs(arr->v64[2])), "z") == 0,
            Arrayfree(arr),
            "V64(FS) Add end: expected [x,y,z]"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: V64(FS) ArrayAdd zero cnt (no-op)", ++subnum);
    {
        Array *arr = V64Array_create(3, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        arr->v64[0] = value64_createfs_asstr("1");
        arr->v64[1] = value64_createfs_asstr("2");
        arr->v64[2] = value64_createfs_asstr("3");
        arr->len = 3;

        arr = ArrayAdd(arr, 1, 0, ARRAY_FILLTYPE_ZERO);
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            strcmp(fs_str(value64_fs(arr->v64[0])), "1") == 0 &&
            strcmp(fs_str(value64_fs(arr->v64[1])), "2") == 0 &&
            strcmp(fs_str(value64_fs(arr->v64[2])), "3") == 0,
            Arrayfree(arr),
            "V64(FS) Add zero: array unchanged"
        );
        Arrayfree(arr);
    }

    test_sub("subtest %d: V64(FS) ArrayAdd out of bounds (no-op)", ++subnum);
    {
        Array *arr = V64Array_create(3, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        arr->v64[0] = value64_createfs_asstr("a");
        arr->v64[1] = value64_createfs_asstr("b");
        arr->v64[2] = value64_createfs_asstr("c");
        arr->len = 3;

        arr = ArrayAdd(arr, 5, 1, ARRAY_FILLTYPE_ZERO);
        test_validatefree(
            arr != NULL && Arraylen(arr) == 3 &&
            strcmp(fs_str(value64_fs(arr->v64[0])), "a") == 0 &&
            strcmp(fs_str(value64_fs(arr->v64[1])), "b") == 0 &&
            strcmp(fs_str(value64_fs(arr->v64[2])), "c") == 0,
            Arrayfree(arr),
            "V64(FS) Add out of bounds: array unchanged"
        );
        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST ArrayFillRange_ZERO with V64 generator -------------------------
static TestStatus
tf26_array_v64_zero_fill_all(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. INT */
    test_sub("subtest %d: V64 ZERO fill for INT", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ZERO, VALUE64_INT);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < 5; i++) {
            test_validatefree(value64_int(arr->v64[i]) == 0,
                              Arrayfree(arr), "INT[%d] must be 0", i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 2. LONG */
    test_sub("subtest %d: V64 ZERO fill for LONG", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ZERO, VALUE64_LONG);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < 5; i++) {
            test_validatefree(value64_long(arr->v64[i]) == 0L,
                              Arrayfree(arr), "LONG[%d] must be 0", i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 3. ULONG */
    test_sub("subtest %d: V64 ZERO fill for ULONG", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ZERO, VALUE64_ULONG);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < 5; i++) {
            test_validatefree(value64_ulong(arr->v64[i]) == 0UL,
                              Arrayfree(arr), "ULONG[%d] must be 0", i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 4. DBL */
    test_sub("subtest %d: V64 ZERO fill for DBL", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ZERO, VALUE64_DBL);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < 5; i++) {
            test_validatefree(fabs(value64_dbl(arr->v64[i]) - 0.0) < 1e-9,
                              Arrayfree(arr), "DBL[%d] must be 0.0", i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 5. CHR */
    test_sub("subtest %d: V64 ZERO fill for CHR", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ZERO, VALUE64_CHR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < 5; i++) {
            test_validatefree(value64_char(arr->v64[i]) == '\0',
                              Arrayfree(arr), "CHR[%d] must be \\0", i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 6. BOOL */
    test_sub("subtest %d: V64 ZERO fill for BOOL", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ZERO, VALUE64_BOOL);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < 5; i++) {
            test_validatefree(value64_bool(arr->v64[i]) == false,
                              Arrayfree(arr), "BOOL[%d] must be false", i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 7. PTR (NULL) */
    test_sub("subtest %d: V64 ZERO fill for PTR", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ZERO, VALUE64_PTR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < 5; i++) {
            test_validatefree(value64_ptr(arr->v64[i]) == NULL,
                              Arrayfree(arr), "PTR[%d] must be NULL", i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 8. FS (пустой fs) */
    test_sub("subtest %d: V64 ZERO fill for FS", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ZERO, VALUE64_FS);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < 5; i++) {
            test_validatefree(
                value64_fs(arr->v64[i]) != NULL && fs_len(value64_fs(arr->v64[i])) == 0,
                Arrayfree(arr), "FS[%d] must be empty", i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 9. STR (пустая строка) */
    test_sub("subtest %d: V64 ZERO fill for STR", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ZERO, VALUE64_STR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < 5; i++) {
            test_validatefree(
                value64_str(arr->v64[i]) != NULL && strcmp(value64_str(arr->v64[i]), "") == 0,
                Arrayfree(arr), "STR[%d] must be empty string", i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST ArrayFillRange_ASC_SERIES with V64 generator -------------------------
static TestStatus
tf27_array_v64_asc_series_fill_all(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: V64 ASC fill for INT", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ASC_SERIES, VALUE64_INT);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");
        for (int i = 0; i < Arraylen(arr); i++) {
            test_validatefree(value64_int(arr->v64[i]) == i,
                              Arrayfree(arr), "INT[%d] must be %d, got %d", i, i, value64_int(arr->v64[i]));
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: V64 ASC fill for LONG", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ASC_SERIES, VALUE64_LONG);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");
        for (int i = 0; i < Arraylen(arr); i++) {
            test_validatefree(value64_long(arr->v64[i]) == i,
                              Arrayfree(arr), "LONG[%d] must be %d", i, i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: V64 ASC fill for ULONG", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ASC_SERIES, VALUE64_ULONG);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");
        for (int i = 0; i < Arraylen(arr); i++) {
            test_validatefree(value64_ulong(arr->v64[i]) == (unsigned long)i,
                              Arrayfree(arr), "ULONG[%d] must be %d", i, i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: V64 ASC fill for DBL", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ASC_SERIES, VALUE64_DBL);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");
        for (int i = 0; i < Arraylen(arr); i++) {
            test_validatefree(fabs(value64_dbl(arr->v64[i]) - i) < 1e-9,
                              Arrayfree(arr), "DBL[%d] must be %d", i, i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: V64 ASC fill for CHR", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ASC_SERIES, VALUE64_CHR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");
        for (int i = 0; i < Arraylen(arr); i++) {
            test_validatefree(value64_char(arr->v64[i]) == (char)i,
                              Arrayfree(arr), "CHR[%d] must be %d", i, i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: V64 ASC fill for BOOL", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ASC_SERIES, VALUE64_BOOL);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");
        for (int i = 0; i < Arraylen(arr); i++) {
            bool expected = (i % 2 != 0);
            test_validatefree(value64_bool(arr->v64[i]) == expected,
                              Arrayfree(arr), "BOOL[%d] must be %s", i, expected ? "true" : "false");
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: V64 ASC fill for STR", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ASC_SERIES, VALUE64_STR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");
        for (int i = 0; i < Arraylen(arr); i++) {
            char expected[16];
            snprintf(expected, sizeof(expected), "%d", i);
            test_validatefree(strcmp(value64_str(arr->v64[i]), expected) == 0,
                              Arrayfree(arr), "STR[%d] must be '%s'", i, expected);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: V64 ASC fill for FS", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_ASC_SERIES, VALUE64_FS);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");
        for (int i = 0; i < Arraylen(arr); i++) {
            char expected[16];
            snprintf(expected, sizeof(expected), "%d", i);
            test_validatefree(fs_cmpstr(value64_fs(arr->v64[i]), expected) == 0,
                              Arrayfree(arr), "FS[%d] must be '%s'", i, expected);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST ArrayFillRange_ASC with V64 generator (random increase) -------------------------
static TestStatus
tf28_array_v64_asc_fill_all_random(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. INT */
    test_sub("subtest %d: V64 ASC random fill for INT", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_ASC, VALUE64_INT);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            int diff = value64_int(arr->v64[i]) - value64_int(arr->v64[i-1]);
            test_validatefree(diff >= 1 && diff <= g_array_acs_rndinc,
                              Arrayfree(arr),
                              "INT difference at %d must be 1..%d, got %d", 
                              i, g_array_acs_rndinc, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 2. LONG */
    test_sub("subtest %d: V64 ASC random fill for LONG", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_ASC, VALUE64_LONG);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            long diff = value64_long(arr->v64[i]) - value64_long(arr->v64[i-1]);
            test_validatefree(diff >= 1 && diff <= g_array_acs_rndinc,
                              Arrayfree(arr),
                              "LONG difference at %d must be 1..%d, got %ld", 
                              i, g_array_acs_rndinc, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 3. ULONG */
    test_sub("subtest %d: V64 ASC random fill for ULONG", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_ASC, VALUE64_ULONG);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            unsigned long diff = value64_ulong(arr->v64[i]) - value64_ulong(arr->v64[i-1]);
            test_validatefree(diff >= 1 && diff <= g_array_acs_rndinc,
                              Arrayfree(arr),
                              "ULONG difference at %d must be 1..%d, got %lu", 
                              i, g_array_acs_rndinc, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 4. DBL */
    test_sub("subtest %d: V64 ASC random fill for DBL", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_ASC, VALUE64_DBL);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            double diff = value64_dbl(arr->v64[i]) - value64_dbl(arr->v64[i-1]);
            test_validatefree(diff >= 1.0 && diff <= g_array_acs_rndinc + 0.0,
                              Arrayfree(arr),
                              "DBL difference at %d must be 1.0..%g, got %f", 
                              i, g_array_acs_rndinc + 0.0, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 5. CHR */
    test_sub("subtest %d: V64 ASC random fill for CHR", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_ASC, VALUE64_CHR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            int diff = (unsigned char)value64_char(arr->v64[i]) - (unsigned char)value64_char(arr->v64[i-1]);
            test_validatefree(diff >= 1 && diff <= g_array_acs_rndinc,
                              Arrayfree(arr),
                              "CHR difference at %d must be 1..%d, got %d", 
                              i, g_array_acs_rndinc, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 6. BOOL – просто проверяем, что созданы допустимые значения */
    test_sub("subtest %d: V64 ASC random fill for BOOL", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_ASC, VALUE64_BOOL);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < Arraylen(arr); i++) {
            test_validatefree(value64_bool(arr->v64[i]) == true || value64_bool(arr->v64[i]) == false,
                              Arrayfree(arr),
                              "BOOL[%d] must be true or false", i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 7. STR */
    test_sub("subtest %d: V64 ASC random fill for STR", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_ASC, VALUE64_STR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            int prev, curr;
            test_validatefree(sscanf(value64_str(arr->v64[i-1]), "%d", &prev) == 1 &&
                              sscanf(value64_str(arr->v64[i]), "%d", &curr) == 1,
                              Arrayfree(arr),
                              "STR[%d] parse error", i);
            int diff = curr - prev;
            test_validatefree(diff >= 1 && diff <= g_array_acs_rndinc,
                              Arrayfree(arr),
                              "STR difference at %d must be 1..%d, got %d", 
                              i, g_array_acs_rndinc, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 8. FS */
    test_sub("subtest %d: V64 ASC random fill for FS", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_ASC, VALUE64_FS);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            int prev, curr;
            test_validatefree(sscanf(fs_str(value64_fs(arr->v64[i-1])), "%d", &prev) == 1 &&
                              sscanf(fs_str(value64_fs(arr->v64[i])), "%d", &curr) == 1,
                              Arrayfree(arr),
                              "FS[%d] parse error", i);
            int diff = curr - prev;
            test_validatefree(diff >= 1 && diff <= g_array_acs_rndinc,
                              Arrayfree(arr),
                              "FS difference at %d must be 1..%d, got %d", 
                              i, g_array_acs_rndinc, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST ArrayFillRange_DESC with V64 generator (random decrease) -------------------------
static TestStatus
tf29_array_v64_desc_fill_all_random(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. INT */
    test_sub("subtest %d: V64 DESC random fill for INT", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_DESC, VALUE64_INT);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            int diff = value64_int(arr->v64[i-1]) - value64_int(arr->v64[i]);
            test_validatefree(diff >= 1 && diff <= g_array_desc_rndinc,
                              Arrayfree(arr),
                              "INT difference at %d must be 1..%d, got %d", 
                              i, g_array_desc_rndinc, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 2. LONG */
    test_sub("subtest %d: V64 DESC random fill for LONG", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_DESC, VALUE64_LONG);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            long diff = value64_long(arr->v64[i-1]) - value64_long(arr->v64[i]);
            test_validatefree(diff >= 1 && diff <= g_array_desc_rndinc,
                              Arrayfree(arr),
                              "LONG difference at %d must be 1..%d, got %ld", 
                              i, g_array_desc_rndinc, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 3. ULONG */
    test_sub("subtest %d: V64 DESC random fill for ULONG", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_DESC, VALUE64_ULONG);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            unsigned long diff = value64_ulong(arr->v64[i-1]) - value64_ulong(arr->v64[i]);
            test_validatefree(diff >= 1 && diff <= g_array_desc_rndinc,
                              Arrayfree(arr),
                              "ULONG difference at %d must be 1..%d, got %lu", 
                              g_array_desc_rndinc, i, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 4. DBL */
    test_sub("subtest %d: V64 DESC random fill for DBL", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_DESC, VALUE64_DBL);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            double diff = value64_dbl(arr->v64[i-1]) - value64_dbl(arr->v64[i]);
            test_validatefree(diff >= 1.0 && diff <= g_array_desc_rndinc + 0.0,
                              Arrayfree(arr),
                              "DBL difference at %d must be 1.0..%g, got %f", 
                              i, g_array_desc_rndinc + 0.0, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 5. CHR */
    test_sub("subtest %d: V64 DESC random fill for CHR", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_DESC, VALUE64_CHR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            int diff = (unsigned char)value64_char(arr->v64[i-1]) - (unsigned char)value64_char(arr->v64[i]);
            test_validatefree(diff >= 1 && diff <= g_array_desc_rndinc,
                              Arrayfree(arr),
                              "CHR difference at %d must be 1..%d, got %d", 
                              i, g_array_desc_rndinc, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 6. BOOL – просто проверяем, что созданы допустимые значения */
    test_sub("subtest %d: V64 DESC random fill for BOOL", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_DESC, VALUE64_BOOL);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < Arraylen(arr); i++) {
            test_validatefree(value64_bool(arr->v64[i]) == true || value64_bool(arr->v64[i]) == false,
                              Arrayfree(arr),
                              "BOOL[%d] must be true or false", i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 7. STR */
    test_sub("subtest %d: V64 DESC random fill for STR", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_DESC, VALUE64_STR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            int prev, curr;
            test_validatefree(sscanf(value64_str(arr->v64[i-1]), "%d", &prev) == 1 &&
                              sscanf(value64_str(arr->v64[i]), "%d", &curr) == 1,
                              Arrayfree(arr),
                              "STR[%d] parse error", i);
            int diff = prev - curr;
            test_validatefree(diff >= 1 && diff <= g_array_desc_rndinc,
                              Arrayfree(arr),
                              "STR difference at %d must be 1..%d, got %d", 
                              i, g_array_desc_rndinc, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 8. FS */
    test_sub("subtest %d: V64 DESC random fill for FS", ++subnum);
    {
        Array *arr = V64Array_create(8, ARRAY_FILLTYPE_DESC, VALUE64_FS);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 1; i < Arraylen(arr); i++) {
            int prev, curr;
            test_validatefree(sscanf(fs_str(value64_fs(arr->v64[i-1])), "%d", &prev) == 1 &&
                              sscanf(fs_str(value64_fs(arr->v64[i])), "%d", &curr) == 1,
                              Arrayfree(arr),
                              "FS[%d] parse error", i);
            int diff = prev - curr;
            test_validatefree(diff >= 1 && diff <= g_array_desc_rndinc,
                              Arrayfree(arr),
                              "FS difference at %d must be 1..%d, got %d", 
                              i, g_array_desc_rndinc, diff);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST ArrayFillRange_DESC_SERIES with V64 generator -------------------------
static TestStatus
tf30_array_v64_desc_series_fill_all(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. INT */
    test_sub("subtest %d: V64 DESC_SERIES fill for INT", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_DESC_SERIES, VALUE64_INT);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < Arraylen(arr); i++) {
            int expected = N - 1 - i;
            test_validatefree(value64_int(arr->v64[i]) == expected,
                              Arrayfree(arr),
                              "INT[%d] must be %d, got %d", i, expected, value64_int(arr->v64[i]));
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 2. LONG */
    test_sub("subtest %d: V64 DESC_SERIES fill for LONG", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_DESC_SERIES, VALUE64_LONG);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < Arraylen(arr); i++) {
            long expected = N - 1 - i;
            test_validatefree(value64_long(arr->v64[i]) == expected,
                              Arrayfree(arr),
                              "LONG[%d] must be %ld, got %ld", i, expected, value64_long(arr->v64[i]));
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 3. ULONG */
    test_sub("subtest %d: V64 DESC_SERIES fill for ULONG", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_DESC_SERIES, VALUE64_ULONG);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < Arraylen(arr); i++) {
            unsigned long expected = N - 1 - i;
            test_validatefree(value64_ulong(arr->v64[i]) == expected,
                              Arrayfree(arr),
                              "ULONG[%d] must be %lu, got %lu", i, expected, value64_ulong(arr->v64[i]));
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 4. DBL */
    test_sub("subtest %d: V64 DESC_SERIES fill for DBL", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_DESC_SERIES, VALUE64_DBL);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < Arraylen(arr); i++) {
            double expected = N - 1 - i;
            test_validatefree(fabs(value64_dbl(arr->v64[i]) - expected) < 1e-9,
                              Arrayfree(arr),
                              "DBL[%d] must be %f, got %f", i, expected, value64_dbl(arr->v64[i]));
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 5. CHR */
    test_sub("subtest %d: V64 DESC_SERIES fill for CHR", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_DESC_SERIES, VALUE64_CHR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        char expected =  N - 1;   // 7, 6, ..., 0
        for (int i = 0; i < Arraylen(arr); i++) {
            test_validatefree(value64_char(arr->v64[i]) == expected,
                              Arrayfree(arr),
                              "CHR[%d] must be '%c'/%d, got '%c'/%d", 
                              i, expected, expected, value64_char(arr->v64[i]), value64_char(arr->v64[i]));
            expected--;
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 6. BOOL */
    test_sub("subtest %d: V64 DESC_SERIES fill for BOOL", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_DESC_SERIES, VALUE64_BOOL);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < Arraylen(arr); i++) {
            test_validatefree(value64_bool(arr->v64[i]) == true || value64_bool(arr->v64[i]) == false,
                              Arrayfree(arr),
                              "BOOL[%d] must be true or false", i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 7. STR */
    test_sub("subtest %d: V64 DESC_SERIES fill for STR", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_DESC_SERIES, VALUE64_STR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < Arraylen(arr); i++) {
            char expected[16];
            snprintf(expected, sizeof(expected), "%d", N - 1 - i);
            test_validatefree(strcmp(value64_str(arr->v64[i]), expected) == 0,
                              Arrayfree(arr),
                              "STR[%d] must be '%s', got '%s'", i, expected, value64_str(arr->v64[i]));
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 8. FS */
    test_sub("subtest %d: V64 DESC_SERIES fill for FS", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_DESC_SERIES, VALUE64_FS);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        for (int i = 0; i < Arraylen(arr); i++) {
            char expected[16];
            snprintf(expected, sizeof(expected), "%d", N - 1 - i);
            test_validatefree(fs_cmpstr(value64_fs(arr->v64[i]), expected) == 0,
                              Arrayfree(arr),
                              "FS[%d] must be '%s', got '%s'", i, expected, fs_str(value64_fs(arr->v64[i])));
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST ArrayFillRange_RND with V64 generator -------------------------
static TestStatus
tf31_array_v64_rnd_fill_all(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. INT */
    test_sub("subtest %d: V64 RND fill for INT", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_RND, VALUE64_INT);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        int prev = value64_int(arr->v64[0]);
        bool all_same = true;
        for (int i = 0; i < Arraylen(arr); i++) {
            int val = value64_int(arr->v64[i]);
            test_validatefree(val >= 0 && val <= 10 * N,
                              Arrayfree(arr),
                              "INT[%d] out of range: %d", i, val);
            if (i > 0 && val != prev) all_same = false;
            prev = val;
        }
        test_validatefree(!all_same, Arrayfree(arr), "INT values should not be all identical");
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 2. LONG */
    test_sub("subtest %d: V64 RND fill for LONG", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_RND, VALUE64_LONG);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        long prev = value64_long(arr->v64[0]);
        bool all_same = true;
        for (int i = 0; i < Arraylen(arr); i++) {
            long val = value64_long(arr->v64[i]);
            test_validatefree(val >= 0 && val <= 10 * N,
                              Arrayfree(arr),
                              "LONG[%d] out of range: %ld", i, val);
            if (i > 0 && val != prev) all_same = false;
            prev = val;
        }
        test_validatefree(!all_same, Arrayfree(arr), "LONG values should not be all identical");
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 3. ULONG */
    test_sub("subtest %d: V64 RND fill for ULONG", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_RND, VALUE64_ULONG);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        unsigned long prev = value64_ulong(arr->v64[0]);
        bool all_same = true;
        for (int i = 0; i < Arraylen(arr); i++) {
            unsigned long val = value64_ulong(arr->v64[i]);
            test_validatefree(val >= 0 && val <= 10UL * N,
                              Arrayfree(arr),
                              "ULONG[%d] out of range: %lu", i, val);
            if (i > 0 && val != prev) all_same = false;
            prev = val;
        }
        test_validatefree(!all_same, Arrayfree(arr), "ULONG values should not be all identical");
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 4. DBL */
    test_sub("subtest %d: V64 RND fill for DBL", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_RND, VALUE64_DBL);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        double prev = value64_dbl(arr->v64[0]);
        bool all_same = true;
        for (int i = 0; i < Arraylen(arr); i++) {
            double val = value64_dbl(arr->v64[i]);
            test_validatefree(val >= 0.0 && val <= 10.0 * N,
                              Arrayfree(arr),
                              "DBL[%d] out of range: %f", i, val);
            if (i > 0 && fabs(val - prev) > 1e-9) all_same = false;
            prev = val;
        }
        test_validatefree(!all_same, Arrayfree(arr), "DBL values should not be all identical");
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 5. CHR */
    test_sub("subtest %d: V64 RND fill for CHR", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_RND, VALUE64_CHR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        unsigned char prev = (unsigned char)value64_char(arr->v64[0]);
        bool all_same = true;
        for (int i = 0; i < Arraylen(arr); i++) {
            unsigned char val = (unsigned char)value64_char(arr->v64[i]);
            test_validatefree(val <= 10 * N,
                              Arrayfree(arr),
                              "CHR[%d] out of range: %u", i, val);
            if (i > 0 && val != prev) all_same = false;
            prev = val;
        }
        test_validatefree(!all_same, Arrayfree(arr), "CHR values should not be all identical");
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 6. BOOL */
    test_sub("subtest %d: V64 RND fill for BOOL", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_RND, VALUE64_BOOL);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        bool saw_true = false, saw_false = false;
        for (int i = 0; i < Arraylen(arr); i++) {
            bool b = value64_bool(arr->v64[i]);
            test_validatefree(b == true || b == false,
                              Arrayfree(arr),
                              "BOOL[%d] must be true or false", i);
            if (b) saw_true = true;
            else saw_false = true;
        }
        test_validatefree(saw_true && saw_false,
                          Arrayfree(arr),
                          "BOOL values should include both true and false");
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 7. STR */
    test_sub("subtest %d: V64 RND fill for STR", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_RND, VALUE64_STR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        int prev = -1;
        bool all_same = true;
        for (int i = 0; i < Arraylen(arr); i++) {
            const char *s = value64_str(arr->v64[i]);
            test_validatefree(s != NULL && s[0] != '\0',
                              Arrayfree(arr),
                              "STR[%d] must be non-empty", i);
            int num;
            test_validatefree(sscanf(s, "%d", &num) == 1,
                              Arrayfree(arr),
                              "STR[%d] must be numeric, got '%s'", i, s);
            test_validatefree(num >= 0 && num <= 10 * N,
                              Arrayfree(arr),
                              "STR[%d] out of range: %d", i, num);
            if (i > 0 && num != prev) all_same = false;
            prev = num;
        }
        test_validatefree(!all_same, Arrayfree(arr), "STR values should not be all identical");
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 8. FS */
    test_sub("subtest %d: V64 RND fill for FS", ++subnum);
    {
        const int N = 8;
        Array *arr = V64Array_create(N, ARRAY_FILLTYPE_RND, VALUE64_FS);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        int prev = -1;
        bool all_same = true;
        for (int i = 0; i < Arraylen(arr); i++) {
            const char *s = fs_str(value64_fs(arr->v64[i]));
            test_validatefree(s != NULL && s[0] != '\0',
                              Arrayfree(arr),
                              "FS[%d] must be non-empty", i);
            int num;
            test_validatefree(sscanf(s, "%d", &num) == 1,
                              Arrayfree(arr),
                              "FS[%d] must be numeric, got '%s'", i, s);
            test_validatefree(num >= 0 && num <= 10 * N,
                              Arrayfree(arr),
                              "FS[%d] out of range: %d", i, num);
            if (i > 0 && num != prev) all_same = false;
            prev = num;
        }
        test_validatefree(!all_same, Arrayfree(arr), "FS values should not be all identical");
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST ArrayFillRange_SAFE_EMPTY -------------------------
static TestStatus
tf32_array_safe_empty(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. STR array: все элементы должны быть NULL-строками */
    test_sub("subtest %d: SAFE_EMPTY for STR", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_STR);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        int cnt = ArrayFillRange_SAFE_EMPTY(arr, 0, Arraylen(arr));
        test_validatefree(cnt == Arraylen(arr), Arrayfree(arr),
                          "expected %d filled, got %d", Arraylen(arr), cnt);

        for (int i = 0; i < Arraylen(arr); i++) {
            test_validatefree(value64_str(arr->v64[i]) == NULL,
                              Arrayfree(arr),
                              "STR[%d] must be NULL", i);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 2. FS array: все элементы должны быть пустыми fs (не NULL) */
    test_sub("subtest %d: SAFE_EMPTY for FS", ++subnum);
    {
        Array *arr = V64Array_create(5, ARRAY_FILLTYPE_SAFE_EMPTY, VALUE64_FS);
        test_validatefree(arr != NULL, Arrayfree(arr), "V64Array_create failed");

        int cnt = ArrayFillRange_SAFE_EMPTY(arr, 0, Arraylen(arr));
        test_validatefree(cnt == Arraylen(arr), Arrayfree(arr),
                          "expected %d filled, got %d", Arraylen(arr), cnt);

        for (int i = 0; i < Arraylen(arr); i++) {
            fs *f = value64_fs(arr->v64[i]);

            test_validatefree(f != NULL && fs_isempty(f),
                            Arrayfree(arr),
                            "FS[%d] must be non-NULL %p and == FS() ", i, f
            );
            if (f)
                test_validatefree(fs_len(f) == 0, Arrayfree(arr),
                                  "FS[%d] must be empty, len=%zu", i, fs_len(f));
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    /* 3. Скалярный массив: SAFE_EMPTY ничего не делает, возвращает 0 */
    test_sub("subtest %d: SAFE_EMPTY for scalar", ++subnum);
    {
        Array *arr = IArray_create(5, ARRAY_FILLTYPE_ASC_SERIES);
        test_validatefree(arr != NULL, Arrayfree(arr), "IArray_create failed");

        int cnt = ArrayFillRange_SAFE_EMPTY(arr, 0, Arraylen(arr));
        test_validatefree(cnt == 0, Arrayfree(arr),
                          "expected 0, got %d", cnt);

        // Значения должны остаться прежними (заполнены ASC_SERIES)
        for (int i = 0; i < Arraylen(arr); i++) {
            test_validatefree(arr->iv[i] == i,
                              Arrayfree(arr),
                              "INT[%d] must be %d, got %d", i, i, arr->iv[i]);
        }
        Arrayfree(arr);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// -------------------------------------------------------------------
int
main( /*int argc, char *argv[] */ )
{
    logsimpleinit("Start");

    testenginestd(
        TESTADD(tf1,                                    "Int/double creation/descr test")
      , TESTADD(tf2,                                    "Int/double filling test")
      , TESTADD(tf3,                                    "Shrink test")
      , TESTADD(tf4,                                    "Save/load int test")
      , TESTADD(tf5,                                    "Save/load dbl test")
      , TESTADD(tf6,                                    "Shuffle array(dbl/int) simple test")
      , TESTADD(tf7,                                    "Sort array(dbl/int) simple test")
      , TESTADD(tf8,                                    "ArrayIncrease simple test")
      , TESTADD(tf9,                                    "PArray simple test")
      , TESTADD(tf10,                                   "Creation with ARRAY_(DE)ASC_SERIES simple test")
      , TESTADD(tf11,                                   "ArrayFillRange simple test")
      , TESTADD(tf12,                                   "Array_foreach macro simple test")
      , TESTADD(tf13,                                   "Array_foreach_prod simple test")
      , TESTADD(tf_v64array_str_fs,                     "V64Array (STR / FS) simple test")
      , TESTADD(tf_v64array_shrink_increase,            "V64Array (STR / FS) shrink / increase simple test")
      , TESTADD(tf_v64array_sort,                       "V64Array (STR / FS) sorting simple test")
      , TESTADD(tf_v64ArraySaveFile_load,               "V64Array STR/FS save/load simple test")
      , TESTADD(tf_array_bsearch,                       "ArrayBsearch (INT / LONG / DBL / V64) simple test")
      , TESTADD(tf_carray_create_fill_free,             "CHAR create/fill/free simple test")
      , TESTADD(tf_carray_sort,                         "CHAR sorting simple test")
      , TESTADD(tf_array_bsearch_char,                  "CHAR ArrayBsearch simple test")
      , TESTADD(tf_ArraySaveFile_load_char,             "CHAR Array save/load simple test")
      , TESTADD(tf_array_eq_noteq,                      "ArrayEq / ArrayNoteq (all types, edge cases)")
      , TESTADD(tf_ArrayDel,                            "ArrayDel simple test")
      , TESTADD(tf_ArrayAdd,                            "ArrayAdd simple test")
      , TESTADD(tf26_array_v64_zero_fill_all,           "ArrayFillRange_ZERO with V64 generator")
      , TESTADD(tf27_array_v64_asc_series_fill_all,     "ArrayFillRange_ASC_SERIES with V64 generator")
      , TESTADD(tf28_array_v64_asc_fill_all_random,     "ArrayFillRange_ASC_RND with V64 generator (random increase)")
      , TESTADD(tf29_array_v64_desc_fill_all_random,    "ArrayFillRange_DESC_RND with V64 generators (random decrease)")
      , TESTADD(tf30_array_v64_desc_series_fill_all,    "ArrayFillRange_DESC_SERIES with V64 generator")
      , TESTADD(tf31_array_v64_rnd_fill_all,            "ArrayFillRange_RND with V64 generators all types")
      , TESTADD(tf32_array_safe_empty,                  "ArrayFillRange_SAFE_EMPTY simple test")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* ARRAYTESTING */
