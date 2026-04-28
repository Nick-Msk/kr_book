#include <limits.h>
#include <time.h>
#include <stdlib.h>

#include "array.h"
#include "error.h"
#include "common.h"

/********************************************************************
                 ARRAY MODULE IMPLEMENTATION
********************************************************************/

//  globals, can be changed by app

int                      g_array_rec_line        = 20;  // TODO: rework that to normal (in Array structure)
const char              *g_custom_print_line     = 0;   // TODO: rework that to normal (in Array structure)
const char              *g_save_format_double    = "%6d      %15.15lg\n";
const char              *g_save_format_int       = "%6d\t%6d\n";
const char              *g_save_format_pointer   = "%6d\t%p\n";

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
    logenter("cnt %d, typ %s", cnt, ArrayFillTypeName(filltyp));
    // TODO: refactor via Array_increase
    Array       res = Array_init(.flags = typ);
    increase(&res, cnt);
    res.len         = cnt;
    Array_fill(res, filltyp);
    return logret(res, "sz = %d", res.sz);
}

void                    Array_free(Array *val){
    if (val && val->iv){
        free(val->iv);
        val->iv = 0;
    }
}

int                      Array_fill(Array a, ArrayFillType typ){
    return Array_fillrange(a, typ, 0, a.len);
}

// TODO: probably shoube be reworked to use switch (type) + separate code
int                      Array_fillrange(Array a, ArrayFillType typ, int from, int to){
    int     initval;
    // fill
    if (from < 0)
        from = 0;
    if (to > a.len)
        to = a.len;
    switch (typ){
        case ARRAY_DESC:
            initval = 100 * a.len;   // hope it'll ne owerwelhm int
            int     dec_value = 10;          // for now!!! It'll be changed
            for (int i = 0; i < a.len; i++){
                if (Array_isint(a))
                    a.iv[i] = initval -= rndint(dec_value);
                else if (Array_isdouble(a))
                    a.dv[i] = initval -= rnddbl(dec_value);
                else if (Array_ispointer(a))
                    userraiseint(ERR_ACTION_NOT_APPLICABLE, "ARRAY_ASC isn't appilcable");
            }
        break;
        case ARRAY_ASC:
            initval = a.len / 10;
            int     asc_value = 10;          // for now!!! It'll be changed
            for (int i = 0; i < a.len; i++){
                if (Array_isint(a))
                    a.iv[i] = initval += rndint(asc_value);
                else if (Array_isdouble(a))
                    a.dv[i] = initval += rnddbl(asc_value);
                else if (Array_ispointer(a))
                    userraiseint(ERR_ACTION_NOT_APPLICABLE, "ARRAY_ASC isn't appilcable");
            }
        break;
        case ARRAY_RND:
        case ARRAY_ZERO:
            for (int i = 0; i < a.len; i++){ // iter??? TODO:
                switch (typ){
                    case ARRAY_RND:
                        if (Array_isint(a))
                            a.iv[i] = rndint(10 * a.len);
                        else if (Array_isdouble(a))
                            a.dv[i] = rnddbl(10 * a.len);
                        else if (Array_ispointer(a))
                            a.pv[i] = (void *) rndulong(10 * a.len);
                    break;
                    case ARRAY_ZERO:
                        if (Array_isint(a))
                            a.iv[i] = 0;
                        else if (Array_ispointer(a))
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
        default:
            logsimple("Unsupported type %d", typ);
            to = from - 1;  // return -1
        break;
    }
    return logsimpleret(to - from, "Filled %d by %s", to - from, ArrayFillTypeName(typ) );
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

// -------------------------- (API) printers -----------------------

int                     Array_fprint(FILE *f, Array val, int limit){

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
        else if (Array_isdouble(val) ){
            if (g_custom_print_line)    // TODO: refactor that!
                custom_print_line = g_custom_print_line;
            else
                custom_print_line = "[%d - %.8g]\t";
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

    res = fprintf(f, "ARRAY: %s : %d\n", typ, arr.len);
    for (int i = 0; i < arr.len; i++)
        if (Array_isint(arr))
            res += fprintf(f, g_save_format_int, i, arr.iv[i]);    // TODO: think if shrink repeatable
        else if (Array_isdouble(arr))
            res += fprintf(f, g_save_format_double, i, arr.dv[i]);
        else if (Array_ispointer(arr))
            res += fprintf(f, g_save_format_pointer, i, arr.pv[i]);
    res = fprintf(f, "ARRAY: DONE");
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
        test_sub("subtest %d", ++subnum);
        Array darr = DArray_create(100, ARRAY_ZERO);
        if (darr.dv[99] != 0.0)
            return logret(TEST_FAILED, "Element must be 0.0. but not %f", darr.dv[99]);
        if (!Array_isvalid(darr))
            return logret(TEST_FAILED, "Validation is failed");
        Arrayfree(darr);
        if (darr.dv != 0)
            return logret(TEST_FAILED, "Array is'nt freed");
    }
    {
        test_sub("subtest %d", ++subnum);
        Array iarr = IArray_create(100, ARRAY_ZERO);
        if (iarr.iv[99] != 0)
            return logret(TEST_FAILED, "Element must be 0.0. but not %d", iarr.iv[99]);
        if (!Array_isvalid(iarr))
            return logret(TEST_FAILED, "Validation is failed");
        Arrayfree(iarr);
        if (iarr.dv != 0)
            return logret(TEST_FAILED, "Array is'nt freed");
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 2 ---------------------------------
static TestStatus
tf2(const char *name){
    logenter("%s", name);

    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);
        Array darr = DArray_create(100, ARRAY_ASC);
        for (int i = 0; i < darr.len - 1; i++)
            if (darr.dv[i] > darr.dv[i + 1])
                return logactret(Arrayfree(darr), TEST_FAILED, "Violation for ACS gen: arr[%d] = %f > arr[%d+1] = %f", i, darr.dv[i], i, darr.dv[i + 1]);

        test_sub("subtest %d", ++subnum);
        Array_fill(darr, ARRAY_DESC);
        for (int i = 0; i < darr.len - 1; i++)
            if (darr.dv[i] < darr.dv[i + 1])
                return logactret(Arrayfree(darr), TEST_FAILED, "Violation for DESC gen: arr[%d] = %f < arr[%d+1] = %f", i, darr.dv[i], i, darr.dv[i + 1]);
        Arrayfree(darr);
    }

    {
        test_sub("subtest %d", ++subnum);
        Array iarr = IArray_create(100, ARRAY_ASC);
        for (int i = 0; i < iarr.len - 1; i++)
            if (iarr.iv[i] > iarr.iv[i + 1])
                return logactret(Arrayfree(iarr), TEST_FAILED, "Violation for ACS gen: arr[%d] = %d > arr[%d+1] = %d", i, iarr.iv[i], i, iarr.iv[i + 1]);

        test_sub("subtest %d", ++subnum);
        Array_fill(iarr, ARRAY_DESC);
        for (int i = 0; i < iarr.len - 1; i++)
            if (iarr.iv[i] < iarr.iv[i + 1])
                return logactret(Arrayfree(iarr), TEST_FAILED, "Violation for DESC gen: arr[%d] = %d < arr[%d+1] = %d", i, iarr.iv[i], i, iarr.iv[i + 1]);
        Arrayfree(iarr);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 3 ---------------------------------
static TestStatus
tf3(const char *name){
    logenter("%s", name);

    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);

        Array iarr = DArray_create(100, ARRAY_ASC);

        Array_fprint(logfile, iarr, 0);

        iarr = Array_shrink(iarr, 10);
        if (!Array_isvalid(iarr))
            return logactret(Arrayfree(iarr), TEST_FAILED, "Validation is failed");

        if (! inv(iarr.len == 10 && iarr.sz == 10 && iarr.iv != 0, "Broken array!") )
            return logactret(Arrayfree(iarr), TEST_FAILED, "Validatation is failed, len %d - sz %d - v %p", iarr.len, iarr.sz, iarr.iv);
        Arrayfree(iarr);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 4 ---------------------------------
static TestStatus
tf4(const char *name){
    logenter("%s", name);

    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);

        Array iarr = IArray_create(100, ARRAY_RND);

        Array_save(iarr, "log/arr.sv");

        Array iarr2 = Array_load("log/arr.sv");

        if (!Array_isvalid(iarr2))
            return logactret(Arrayfree(iarr), TEST_FAILED, "Validation is failed");

        test_sub("subtest %d", ++subnum);
        if (!inv(iarr.len == iarr2.len && iarr.flags == iarr2.flags, "not equal") )
            return logactret( (Array_free(&iarr), Array_free(&iarr2) ), TEST_FAILED, "Not equal len %d - %d, flags %d - %d",
                iarr.len, iarr2.len, iarr.flags, iarr2.flags);

        test_sub("subtest %d", ++subnum);
        for (int i = 0; i < iarr.len; i++)
            if (iarr.iv[i] != iarr2.iv[i])
                return logacterr( (Array_free(&iarr), Array_free(&iarr2) ), TEST_FAILED,
                                "arr[%d] = %d != arr2[%d] = %d", i, iarr.iv[i], i, iarr2.iv[i]);

        Array_free(&iarr);
        Array_free(&iarr2);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 5 ---------------------------------
static TestStatus
tf5(const char *name){
    logenter("%s", name);

    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);

        Array darr = DArray_create(100, ARRAY_RND);

        Array_print(darr, 0);
        Array_save(darr, "log/darr.sv");

        Array darr2 = Array_load("log/darr.sv");

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
    {
        test_sub("subtest %d", ++subnum);

        Array darr = DArray_create(50, ARRAY_ASC);

        Array_shuffle(darr);
        g_custom_print_line = 0;
        Array_print(darr, 0);
        Arrayfree(darr);

        test_sub("subtest %d", ++subnum);

        Array iarr = IArray_create(50, ARRAY_ASC);
        Array_shuffle(iarr);
        Array_print(iarr, 0);
        Arrayfree(iarr);
    }
    return logret(TEST_MANUAL, "done"); // TEST_FAILED
}

// ------------------------- TEST 7 ---------------------------------
static TestStatus
tf7(const char *name){
    logenter("%s", name);

    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);

        Array darr = DArray_create(10000, ARRAY_RND);

        Array_qsort(darr, ARRAY_ASC);
        //Array_print(darr, 50);
        // check asc
        for (int i = 1; i < darr.len; i++)
            if (darr.dv[i - 1] > darr.dv[i])
                return logactret(Arrayfree(darr), TEST_FAILED, "array[%d] = %f > array[%d] = %f, should be <=", i - 1, darr.dv[i - 1], i, darr.dv[i]);

        test_sub("subtest %d", ++subnum);
        Array_qsort(darr, ARRAY_DESC);
        for (int i = 1; i < darr.len; i++)
            if (darr.dv[i - 1] < darr.dv[i])
                return logactret(Arrayfree(darr), TEST_FAILED, "array[%d] = %f < array[%d] = %f, should be >=", i - 1, darr.dv[i - 1], i, darr.dv[i]);
        Arrayfree(darr);

        test_sub("subtest %d", ++subnum);

        Array iarr = IArray_create(100000, ARRAY_RND);
        Array_qsort(iarr, ARRAY_ASC);
        // check asc
        for (int i = 1; i < iarr.len; i++)
            if (iarr.iv[i - 1] > iarr.iv[i])
                return logactret(Arrayfree(iarr), TEST_FAILED, "array[%d] = %d > array[%d] = %d, should be <=", i - 1, iarr.iv[i - 1], i, iarr.iv[i]);

        test_sub("subtest %d", ++subnum);
        Array_qsort(iarr, ARRAY_DESC);
        // check desc
        for (int i = 1; i < iarr.len; i++)
            if (iarr.iv[i - 1] < iarr.iv[i])
                return logactret(Arrayfree(iarr), TEST_FAILED, "array[%d] = %d < array[%d] = %d, should be >=", i - 1, iarr.iv[i - 1], i, iarr.iv[i]);
        //Array_print(iarr, 50);
        Arrayfree(iarr);
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
            test_validatefree(arr.dv[i] == 0.0, Arrayfree(arr), "arr[%d] must be zero,  but not %f", i, arr.dv[i]);
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
    {
        test_sub("subtest %d increase pointer array", ++subnum);
        
        TODO:
        Arrayfree(arr);
    }

    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// -------------------------------------------------------------------
int
main( /*int argc, char *argv[] */ )
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Int/double creation/descr test"       , .desc = "", .mandatory=true)
      , testnew(.f2 = tf2, .num = 2, .name = "Int/double filling test"              , .desc = "", .mandatory=true)
      , testnew(.f2 = tf3, .num = 3, .name = "Shrink test"                          , .desc = "", .mandatory=true)
      , testnew(.f2 = tf4, .num = 4, .name = "Save/load int test"                   , .desc = "", .mandatory=true)
      , testnew(.f2 = tf5, .num = 5, .name = "Save/load dbl test"                   , .desc = "", .mandatory=true)
      , testnew(.f2 = tf6, .num = 6, .name = "Shuffle array(dbl/int) simple test"   , .desc = "", .mandatory=true)
      , testnew(.f2 = tf7, .num = 7, .name = "Sort array(dbl/int) simple test"      , .desc = "", .mandatory=true)
      , testnew(.f2 = tf8, .num = 8, .name = "Array_increase simple test"           , .desc = "", .mandatory=true)
      , testnew(.f2 = tf9, .num = 9, .name = "PArray simple test"                   , .desc = "", .mandatory=true)
    );

    return logret(0, "end...");  // as replace of logclose()
}



#endif /* ARRAYTESTING */

