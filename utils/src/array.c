#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <float.h>

#include "array.h"
#include "error.h"
#include "checker.h"
#include "common.h"

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
const double             g_array_dbl_increment   = 0.01;

// internal type

// ---------- pseudo-header for utility procedures -----------------

// ------------------------------ Utilities ------------------------

static int                      increase(Array *arr, int newsz){
    logenter("newsz %d", newsz);
    if (newsz == arr->sz)
        return logret(arr->sz, "No change sz %d", arr->sz);
    if (newsz > arr->sz)
        newsz = round_up_2(newsz);

    int bytes = newsz;
    if (Array_isint(*arr))
        bytes *= sizeof(int);
    else if (Array_islong(*arr))
        bytes *= sizeof(long);
    else if (Array_isdouble(*arr))
        bytes *= sizeof(double);
    else if (Array_ispointer(*arr))
        bytes *= sizeof(void *);
    else
        return logerr(-1, "Unknown type");

    logmsg("Arr: bytes=%d, sz=%d", bytes, newsz);
    void *p  = realloc(arr->iv, bytes);
    if (p == 0)
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to allocate %d", bytes);
    else {
        arr->v = p; // iv/dv is the same
        if (arr->len > newsz)   // shrink case
            arr->len = newsz;
        arr->sz = newsz;
    }
    return logret(arr->sz, "New sz %d", arr->sz);
}

// -------------------------- (Utility) printers -------------------

// --------------------------- API ---------------------------------

// ------------- CONSTRUCTOTS/DESTRUCTORS --------------

// CREATE  and fill with method
Array                           Array_create(int cnt, ArrayFillType filltyp, ArrayType typ){
    logenter("cnt %d, filltyp %s typ %s", cnt, ArrayFillTypeName(filltyp), ArrayTypeName(typ) );
    // TODO: refactor via Array_increase
    Array       res = Array_init(.flags = typ);
    increase(&res, cnt);
    res.len         = cnt;
    Array_fill(res, filltyp);
    return logret(res, "sz = %d", res.sz);
}

void                            Array_free(Array *val){
    if (val && val->iv){
        free(val->iv);
        val->iv = 0;
    }
}

int                             Array_fill(Array a, ArrayFillType typ){
    return Array_fillrange(a, typ, 0, a.len);
}

// TODO: probably shoube be reworked to use switch (type) + separate code
int                             Array_fillrange(Array a, ArrayFillType typ, int from, int to){
    int         intval;
    long        longval;
    double      doubleval;  // TODO:
    // fill
    if (from < 0)
        from = 0;
    if (to > a.sz)  // to avoid overflow
        to = a.len;
    switch (typ){
        case ARRAY_DESC:
            intval = 100 * a.len;   // hope it'll ne owerwelhm int
            longval = 100 * a.len;
            doubleval = 100.0 * a.len;
            int     dec_value = 10;          // for now!!! It'll be changed
            for (int i = from; i < to; i++){
                if (Array_isint(a))
                    a.iv[i] = (intval -= rndint(dec_value) + 1);
                else if (Array_islong(a))
                    a.lv[i] = (longval -= rndlong(dec_value) + 1);
                else if (Array_isdouble(a))
                    a.dv[i] = doubleval -= (rnddbl(dec_value) + g_array_dbl_increment);
                else if (Array_ispointer(a))
                    userraiseint(ERR_ACTION_NOT_APPLICABLE, "ARRAY_ASC isn't appilcable");
            }
        break;
        case ARRAY_ASC:
            intval = a.len / 10;
            longval = a.len / 10;
            doubleval = a.len / 10.0;
            int     asc_value = 10;          // for now!!! It'll be changed
            for (int i = from; i < to; i++){
                if (Array_isint(a))
                    a.iv[i] = (intval += rndint(asc_value) + 1);
                else if (Array_islong(a))
                    a.lv[i] = (longval += rndlong(asc_value) + 1);
                else if (Array_isdouble(a))
                    a.dv[i] = doubleval += (rnddbl(asc_value)  + g_array_dbl_increment);
                else if (Array_ispointer(a))
                    userraiseint(ERR_ACTION_NOT_APPLICABLE, "ARRAY_ASC isn't appilcable");
            }
        break;
        case ARRAY_RND:
        case ARRAY_ZERO:
            for (int i = from; i < to; i++){ // iter??? TODO:
                switch (typ){
                    case ARRAY_RND:
                        if (Array_isint(a))
                            a.iv[i] = rndint(10 * a.len);
                        else if (Array_islong(a))
                            a.lv[i] = rndlong(10 * a.len);
                        else if (Array_isdouble(a))
                            a.dv[i] = rnddbl(10.0 * a.len);
                        else if (Array_ispointer(a))
                            a.pv[i] = (void *) rndulong(10000UL * a.len);
                    break;
                    case ARRAY_ZERO:
                        if (Array_isint(a))
                            a.iv[i] = 0;
                        else if (Array_islong(a))
                            a.lv[i] = 0L;
                        else if (Array_isdouble(a))
                            a.dv[i] = 0.0;
                        else if (Array_ispointer(a))
                            a.pv[i] = 0x0;
                    break;
                    default:
                    break;
                }
            }
        break;
        case ARRAY_NONE:
            // just do nothing
        break;
        case ARRAY_ASC_SERIES:
            intval      = 0;
            longval     = 0L;
            doubleval   = 0.0;
            // TODO: I need invraisenum(ERROR_..., condition, msg, ....);
            invraisecode(Array_isint(a) || Array_islong(a) || Array_isdouble(a), 
                ERR_UNSUPPORTED_TYPE, "Unsupported type %d", Array_gettype(a) );
            for (int i = from; i < to; i++){ // iter??? TODO:
                    if (Array_isint(a))
                        a.iv[i] = i + intval;
                    else if (Array_islong(a))
                        a.lv[i] = i + longval;
                    else if (Array_isdouble(a))
                        a.dv[i] = i + doubleval;
            }
        break;
        case ARRAY_DESC_SERIES:
            intval      = to - 1;
            longval     = to - 1;
            doubleval   = to - 1;
            // TODO: I need invraisenum(ERROR_NUMBER, condition, msg, ....);
            invraisecode(Array_isint(a) || Array_islong(a) || Array_isdouble(a), 
                ERR_UNSUPPORTED_TYPE, "Unsupported type %d", Array_gettype(a) );
            for (int i = to - 1; i >= from; i--){ // iter??? TODO:
                    if (Array_isint(a))
                        a.iv[i] = intval - i;
                    else if (Array_islong(a))
                        a.lv[i] = longval - i;
                    else if (Array_isdouble(a))
                        a.dv[i] = doubleval - i;
            }
        break;
        default:
            logsimple("Unsupported type %d", typ);
            to = from - 1;  // return -1
        break;
    }
    return logsimpleret(to - from, "Filled %s [%d-%d] by %s", ArrayTypeName(a.flags), from, to - 1, ArrayFillTypeName(typ) );
}

// -------------- ACCESS AND MODIFICATION --------------

Array                           Array_increase(Array arr, int newcnt){
    if (newcnt > Arraysz(arr) )
        increase(&arr, newcnt);
    Array_fillrange(arr, ARRAY_ZERO, arr.len, newcnt);
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
    }
}

// TODO: probably shoube be reworked to use switch (type) + separate code
void                     Array_qsort(Array arr, ArrayFillType ord){
    int  sz = 0;
    int (*cmp)(const void *, const void *) = 0;
    if (Array_isint(arr) ){
        sz = sizeof(int);
        if (ord == ARRAY_ASC)
            cmp = pint_cmp;
        else
            cmp = pint_revcmp;
    } else if (Array_islong(arr) ){
        sz = sizeof(long);
        if (ord == ARRAY_ASC){
            cmp = plong_cmp;
            //logsimple("COMPARATOR: sz %d, ARRAY_ASC", sz);
        }
        else
            cmp = plong_revcmp;
    } else if (Array_isdouble(arr) ){
        sz = sizeof(double);
        if (ord == ARRAY_ASC)
            cmp = pdbl_cmp;
        else
            cmp = pdbl_revcmp;
    } else if (Array_ispointer(arr) ){
        sz = sizeof(void *);
        if (ord == ARRAY_ASC)
            cmp = pptr_cmp;
        else
            cmp = pptr_revcmp;
    }
    if (sz)
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
        } else if (Array_isdouble(val) ){
            if (g_custom_print_line)    // TODO: refactor that!
                custom_print_line = g_custom_print_line;
            else
                custom_print_line = "[%p - %p]\t";
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
        fprintf(stderr, "Unable to open %s for writinf\n", fname);
        Array_seterror(arr);
        return logerr(arr, "Can't read");
    }

    char typ[20];
    fscanf(f, "ARRAY: %s : %d", typ, &cnt);
    if (strcmp(typ, "ARRAY_INT") == 0)
        arr = IArray_create(cnt, ARRAY_NONE);
    else if (strcmp(typ, "ARRAY_LONG") == 0)
        arr = LArray_create(cnt, ARRAY_NONE);
    else if (strcmp(typ, "ARRAY_DOUBLE") == 0)
        arr = DArray_create(cnt, ARRAY_NONE);
    else if (strcmp(typ, "ARRAY_POINTER") == 0)
        arr = PArray_create(cnt, ARRAY_NONE);
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
    }
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
        Array darr = DArray_create(100, ARRAY_ZERO);
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
        Array iarr = IArray_create(100, ARRAY_ZERO);
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
        Array larr = LArray_create(100, ARRAY_ZERO);
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
        Array darr = DArray_create(100, ARRAY_ASC);
        for (int i = 0; i < darr.len - 1; i++)
            if (darr.dv[i] > darr.dv[i + 1])
                return logactret(Arrayfree(darr), TEST_FAILED, "Violation for ACS gen: arr[%d] = %f > arr[%d+1] = %f", i, darr.dv[i], i, darr.dv[i + 1]);

        test_sub("subtest %d: double desc", ++subnum);
        Array_fill(darr, ARRAY_DESC);
        for (int i = 0; i < darr.len - 1; i++)
            if (darr.dv[i] < darr.dv[i + 1])
                return logactret(Arrayfree(darr), TEST_FAILED, "Violation for DESC gen: arr[%d] = %f < arr[%d+1] = %f", i, darr.dv[i], i, darr.dv[i + 1]);
        Arrayfree(darr);
    }
    test_sub("subtest %d: int asc", ++subnum);
    {
        Array iarr = IArray_create(100, ARRAY_ASC);
        for (int i = 0; i < iarr.len - 1; i++)
            if (iarr.iv[i] > iarr.iv[i + 1])
                return logactret(Arrayfree(iarr), TEST_FAILED, "Violation for ACS gen: arr[%d] = %d > arr[%d+1] = %d", i, iarr.iv[i], i, iarr.iv[i + 1]);

        test_sub("subtest %d: int desc", ++subnum);
        Array_fill(iarr, ARRAY_DESC);
        for (int i = 0; i < iarr.len - 1; i++)
            if (iarr.iv[i] < iarr.iv[i + 1])
                return logactret(Arrayfree(iarr), TEST_FAILED, "Violation for DESC gen: arr[%d] = %d < arr[%d+1] = %d", i, iarr.iv[i], i, iarr.iv[i + 1]);
        Arrayfree(iarr);
    }
    test_sub("subtest %d: long asc", ++subnum);
    {
        Array larr = LArray_create(100, ARRAY_ASC);
        for (int i = 0; i < larr.len - 1; i++)
            if (larr.lv[i] > larr.lv[i + 1])
                return logactret(Arrayfree(larr), TEST_FAILED, "Violation for ACS gen: arr[%d] = %ld > arr[%d+1] = %ld", i, larr.lv[i], i, larr.lv[i + 1]);

        test_sub("subtest %d: long desc", ++subnum);
        Array_fill(larr, ARRAY_DESC);
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
        Array darr = DArray_create(100, ARRAY_ASC);

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
        Array iarr = IArray_create(100, ARRAY_ASC);

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
        Array larr = LArray_create(100, ARRAY_ASC);

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
        Array iarr = IArray_create(100, ARRAY_RND);
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
        Array larr = LArray_create(100, ARRAY_RND);
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
        Array darr = DArray_create(100, ARRAY_RND);
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
        Array darr = DArray_create(50, ARRAY_ASC);

        Array_shuffle(darr);
        g_custom_print_line = 0;
        Array_print(darr, 0);
        Arrayfree(darr);
    }
    test_sub("subtest %d: int", ++subnum);
    {
        Array iarr = IArray_create(50, ARRAY_ASC);
        Array_shuffle(iarr);
        Array_print(iarr, 0);
        Arrayfree(iarr);
    }
    test_sub("subtest %d: long", ++subnum);
    {
        Array larr = LArray_create(50, ARRAY_ASC);
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
        Array darr = DArray_create(10000, ARRAY_RND);

        Array_qsort(darr, ARRAY_ASC);
        //Array_print(darr, 50);
        // check asc
        for (int i = 1; i < darr.len; i++)
            if (darr.dv[i - 1] > darr.dv[i])
                return logactret(Arrayfree(darr), TEST_FAILED, "array[%d] = %f > array[%d] = %f, should be <=", i - 1, darr.dv[i - 1], i, darr.dv[i]);

        test_sub("subtest %d: double desc", ++subnum);
        Array_qsort(darr, ARRAY_DESC);
        for (int i = 1; i < darr.len; i++)
            if (darr.dv[i - 1] < darr.dv[i])
                return logactret(Arrayfree(darr), TEST_FAILED, "array[%d] = %f < array[%d] = %f, should be >=", i - 1, darr.dv[i - 1], i, darr.dv[i]);
        Arrayfree(darr);
    }
    test_sub("subtest %d: int acs", ++subnum);
    {

        Array iarr = IArray_create(100000, ARRAY_RND);
        Array_qsort(iarr, ARRAY_ASC);
        // check asc
        for (int i = 1; i < iarr.len; i++)
            if (iarr.iv[i - 1] > iarr.iv[i])
                return logactret(Arrayfree(iarr), TEST_FAILED, "array[%d] = %d > array[%d] = %d, should be <=", i - 1, iarr.iv[i - 1], i, iarr.iv[i]);

        test_sub("subtest %d: int desc", ++subnum);
        Array_qsort(iarr, ARRAY_DESC);
        // check desc
        for (int i = 1; i < iarr.len; i++)
            if (iarr.iv[i - 1] < iarr.iv[i])
                return logactret(Arrayfree(iarr), TEST_FAILED, "array[%d] = %d < array[%d] = %d, should be >=", i - 1, iarr.iv[i - 1], i, iarr.iv[i]);
        //Array_print(iarr, 50);
        Arrayfree(iarr);
    }
    test_sub("subtest %d: long asc", ++subnum);
    {

        Array larr = LArray_create(100000, ARRAY_RND);
        Array_qsort(larr, ARRAY_ASC);
        Array_print(larr, 50);
        // check asc
        for (int i = 1; i < larr.len; i++)
            if (larr.lv[i - 1] > larr.lv[i])
                return logactret(Arrayfree(larr), TEST_FAILED, "array[%d] = %ld > array[%d] = %ld, should be <=", i - 1, larr.lv[i - 1], i, larr.lv[i]);

        test_sub("subtest %d: long desc", ++subnum);
        Array_qsort(larr, ARRAY_DESC);
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
        Array arr = IArray_create(initsz, ARRAY_RND);

        arr = Array_increase(arr, initsz * 3);

        test_validatefree(Arraylen(arr) == initsz * 3, Arrayfree(arr), "Array length %d must be %d", Arraylen(arr), initsz * 3);
        for (int i = initsz; i < Arraylen(arr); i++){
            test_validatefree(arr.iv[i] == 0, Arrayfree(arr), "arr[%d] must be zero,  but not %d", i, arr.iv[i]);
        }

        Arrayfree(arr);
    }
    test_sub("subtest %d increas double array", ++subnum);
    {

        int initsz = 25;
        Array arr = DArray_create(initsz, ARRAY_RND);

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
        Array arr = LArray_create(initsz, ARRAY_RND);

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
        Array   parr = PArray_create(cnt, ARRAY_ZERO);

        for (int i = 0; i < parr.len; i++)
            test_validatefree(parr.pv[i] == 0x0, Arrayfree(parr),
                "Element %d must be 0x0 but not %p", i, parr.pv[i]);
        test_validatefree(Array_isvalid(parr), Arrayfree(parr),
                "Validation is failed");

        Arrayfree(parr);
    }
    test_sub("subtest %d shrinking", ++subnum);
    {

        Array   parr = PArray_create(100, ARRAY_ZERO);

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
        Array parr = PArray_create(100, ARRAY_RND);

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

        Array parr = PArray_create(10000, ARRAY_ZERO);

        // fiil array manually
        for (int i = 0; i < parr.len; i++)
            parr.pv[i] = (void **) rndulong(parr.len * 10000);

        Array_qsort(parr, ARRAY_ASC);
        //Array_print(darr, 50);
        // check asc
        for (int i = 1; i < parr.len; i++)
            test_validatefree(parr.pv[i - 1] <= parr.pv[i], Arrayfree(parr),
                            "array[%d] = %p > array[%d] = %p, should be <=", i - 1, parr.pv[i - 1], i, parr.pv[i]);
        // resort descending
        Array_qsort(parr, ARRAY_DESC);
        for (int i = 1; i < parr.len; i++)
            test_validatefree(parr.pv[i - 1] >= parr.pv[i], Arrayfree(parr),
                            "array[%d] = %p < array[%d] = %p, should be >=", i - 1, parr.pv[i - 1], i, parr.pv[i]);
        Arrayfree(parr);
    }
    test_sub("subtest %d increase pointer array", ++subnum);
    {
        int initsz = 25;
        Array arr = PArray_create(initsz, ARRAY_RND);

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
        Array   arr = IArray_create(cnt, ARRAY_ASC_SERIES);

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
        Array   arr = IArray_create(cnt, ARRAY_DESC_SERIES);

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
        Array   arr = LArray_create(cnt, ARRAY_ASC_SERIES);

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
        Array   arr = LArray_create(cnt, ARRAY_DESC_SERIES);

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
        Array   arr = DArray_create(cnt, ARRAY_ASC_SERIES);

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
        Array   arr = DArray_create(cnt, ARRAY_DESC_SERIES);

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
        Array   arr = IArray_create(0, ARRAY_ASC_SERIES);
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
            Array arr = PArray_create(10, ARRAY_ASC_SERIES);
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
        Array   arr = IArray_create(cnt, ARRAY_ZERO);
        int     from = 10, to = 20;

        Array_fillrange(arr, ARRAY_ASC_SERIES, from, to);

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
        Array   arr = LArray_create(cnt, ARRAY_NONE);
        Array_fillrange(arr, ARRAY_DESC_SERIES, 0, cnt);

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
        Array   arr = IArray_create(cnt, ARRAY_ASC_SERIES);  // [0..19]
        Array_fillrange(arr, ARRAY_RND, 5, 5);

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
        Array   arr = IArray_create(cnt, ARRAY_ZERO);
        Array_fillrange(arr, ARRAY_ASC_SERIES, -5, cnt + 5);

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
        Array   arr = DArray_create(cnt, ARRAY_ZERO);
        Array_fillrange(arr, ARRAY_ASC_SERIES, from, to);

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
        Array   arr = IArray_create(cnt, ARRAY_ASC_SERIES);   // 0..9

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
        Array   arr = LArray_create(cnt, ARRAY_ASC_SERIES);

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
        Array   arr = DArray_create(cnt, ARRAY_ASC_SERIES);

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
        Array   arr = PArray_create(cnt, ARRAY_NONE);
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
        Array   arr = IArray_create(cnt, ARRAY_ASC_SERIES);   // 0,1,2,3,4,5,6,7,8,9

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
        Array   arr = LArray_create(cnt, ARRAY_ASC_SERIES);   // 0L..7L

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
        Array   arr = DArray_create(cnt, ARRAY_ASC_SERIES);   // 0.0..5.0

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

// -------------------------------------------------------------------
int
main( /*int argc, char *argv[] */ )
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 =  tf1, .num =  1, .name = "Int/double creation/descr test"                 , .desc = "", .mandatory=true)
      , testnew(.f2 =  tf2, .num =  2, .name = "Int/double filling test"                        , .desc = "", .mandatory=true)
      , testnew(.f2 =  tf3, .num =  3, .name = "Shrink test"                                    , .desc = "", .mandatory=true)
      , testnew(.f2 =  tf4, .num =  4, .name = "Save/load int test"                             , .desc = "", .mandatory=true)
      , testnew(.f2 =  tf5, .num =  5, .name = "Save/load dbl test"                             , .desc = "", .mandatory=true)
      , testnew(.f2 =  tf6, .num =  6, .name = "Shuffle array(dbl/int) simple test"             , .desc = "", .mandatory=true)
      , testnew(.f2 =  tf7, .num =  7, .name = "Sort array(dbl/int) simple test"                , .desc = "", .mandatory=true)
      , testnew(.f2 =  tf8, .num =  8, .name = "Array_increase simple test"                     , .desc = "", .mandatory=true)
      , testnew(.f2 =  tf9, .num =  9, .name = "PArray simple test"                             , .desc = "", .mandatory=true)
      , testnew(.f2 = tf10, .num = 10, .name = "Creation with ARRAY_(DE)ASC_SERIES simple test" , .desc = "", .mandatory=true)
      , testnew(.f2 = tf11, .num = 11, .name = "Array_fillrange simple test"                    , .desc = "", .mandatory=true)
      , testnew(.f2 = tf12, .num = 12, .name = "Array_foreach macro simple test"                , .desc = "", .mandatory=true)
      , testnew(.f2 = tf13, .num = 13, .name = "Array_foreach_prod simple test"                 , .desc = "", .mandatory=true)
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* ARRAYTESTING */

