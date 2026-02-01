#include <limits.h>
#include "array.h"

/********************************************************************
                 <SKELETON> MODULE IMPLEMENTATION
********************************************************************/

//  globals, can be changed by app

int                      g_array_rec_line        = 20;  // TODO: rework that to normal (in Array structure)
const char              *g_custom_print_line     = 0;   // TODO: rework that to normal (in Array structure)
const char              *g_save_format_double    = "%6d      %15.15lg\n";
const char              *g_save_format_int       = "%6d\t%6d\n";

// internal type

// ---------- pseudo-header for utility procedures -----------------

// ------------------------------ Utilities ------------------------

// -------------------------- (Utility) printers -------------------

// --------------------------- API ---------------------------------


void                     Array_fill(Array a, ArrayFillType typ){
    int     initval;
    // fill
    switch (typ){
        case ARRAY_DESC:
            initval = 100 * a.len;   // hope it'll ne owerwelhm int
            int     dec_value = 10;          // for now!!! It'll be changed
            for (int i = 0; i < a.len; i++){
                if (Array_isint(a))
                    a.iv[i] = initval -= rndint(dec_value);
                else    // isdouble
                    a.dv[i] = initval -= rnddbl(dec_value);
            }
        break;
        case ARRAY_ACS:
            initval = a.len / 10;
            int     asc_value = 10;          // for now!!! It'll be changed
            for (int i = 0; i < a.len; i++){
                if (Array_isint(a))
                    a.iv[i] = initval += rndint(asc_value);
                else    // isdouble
                    a.dv[i] = initval += rnddbl(asc_value);
            }
        break;
        case ARRAY_RND:
        case ARRAY_ZERO:
            for (int i = 0; i < a.len; i++){ // iter??? TODO:
                switch (typ){
                    case ARRAY_RND:
                        if (Array_isint(a))
                            a.iv[i] = rndint(10 * a.len);
                        else
                            a.dv[i] = rnddbl(10 * a.len);
                    break;
                    case ARRAY_ZERO:
                        if (Array_isint(a))
                            a.iv[i] = 0;
                        else
                            a.dv[i] = 0.0;
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
        break;
    }
}

Array                           Array_shrink(Array arr, int newsz){
    logenter("newsz %d", newsz);
    int sz = newsz;
    if (Array_isint(arr))
        sz *= sizeof(int);
    else // double
        sz *= sizeof(double);
    void *p  = realloc(arr.iv, sz);
    if (p == 0){
        fprintf(stderr, "Unable to allocate %d", sz);
        //  userraisesig must be here TODO:
    } else {
        arr.iv = p;
        if (arr.len > newsz)
            arr.len = newsz;
        if (arr.sz > newsz)
            arr.sz = newsz;
    }
    return logret(arr, "shrinked to (len %d == sz %d)", arr.len, arr.sz);
}

// CREATE  and fill with method
// increase and shrink are reuiqred too
Array                           Array_create(int cnt, ArrayFillType filltyp, ArrayType typ){
    logenter("cnt %d, typ %s", cnt, ArrayFillTypeName(filltyp));
    Array       res = Array_init();
    res.flags      |= typ;
    res.sz          = round_up_2(cnt);   // 2^(x + 1)
    res.len         = cnt;
    int          sz = typ == ARRAY_INT ? res.sz * sizeof(int) : res.sz * sizeof(double);
    res.iv          = malloc(sz);   // iv == dv
    if (!res.iv){
        fprintf(stderr, "Unable to allocate %d bytes\n", sz);
        res.sz = INT_MIN;   // userraisesig hehe TODO:
    }
    Array_fill(res, filltyp);
    return logret(res, "sz = %d", res.sz);
}

void                    Array_free(Array *val){
    if (val && val->iv){
        free(val->iv);
        val->iv = 0;
    }
}

// -------------------------- (API) printers -----------------------

int                     Array_fprint(FILE *f, Array val, int limit){

    int         cnt = 0, i;
    int         array_rec_line = 20;    // default
    const char *custom_print_line;    // for int or double

    limit = (limit == 0)? val.len : (limit < val.len) ? limit : val.len;
    if (g_array_rec_line)
        array_rec_line = g_array_rec_line;

    cnt += fprintf(f, "Array (%s):\n", Array_isint(val) ? "int" : "dbl");
    for (i = 0; i < limit; i++){
        if (Array_isint(val) ){
            if (g_custom_print_line)
                custom_print_line = g_custom_print_line;
            else  // standard behavior
                custom_print_line = "[%d - %6d]\t";
            cnt += fprintf(f, custom_print_line, i, val.iv[i]);
        }
        else if (Array_isdouble(val) ){
            if (g_custom_print_line)
                custom_print_line = g_custom_print_line;
            else
                custom_print_line = "[%d - %.8g]\t";
            cnt += fprintf(f, custom_print_line, i, val.dv[i]);
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

long                        Array_save(Array arr, const char *fname){
    logenter("%s", fname);

    long    res = 0;
    FILE *f = fopen(fname, "w");
    if (f == 0){
        fprintf(stderr, "Unable to open %s for writinf\n", fname);
        return logerr(-1, "Can't open for write");
    }
    // g_save_format_double g_save_format_int
    const char *typ;
    if (Array_isint(arr))
        typ = "INT";
    else
        typ = "DBL";
    res = fprintf(f, "ARRAY: %s : %d\n", typ, arr.len);
    for (int i = 0; i < arr.len; i++)
        if (Array_isint(arr))
            res += fprintf(f, g_save_format_int, i, arr.iv[i]);    // TODO: think if shrink repeatable
        else if (Array_isdouble(arr))
            res += fprintf(f, g_save_format_double, i, arr.dv[i]);
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
    if (strcmp(typ, "INT") == 0)
        arr = IArray_create(cnt, ARRAY_NONE);
    else if (strcmp(typ, "DBL") == 0)
        arr = DArray_create(cnt, ARRAY_NONE);
    else {
        fprintf(stderr, "Unsupported format %s\n", typ);
        return logactret(fclose(f), arr, "failed, wrong format %s...", typ);
    }
    for (int i = 0; i < cnt; i++){
        if (Array_isint(arr))
            fscanf(f, g_save_format_int, &tmp, arr.iv + i); // tmp isn't used for now
        else if (Array_isdouble(arr) )
            fscanf(f, "%d %lg\n", &tmp, arr.dv + i);
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
        Array darr = DArray_create(100, ARRAY_ACS);
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
        Array iarr = IArray_create(100, ARRAY_ACS);
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

        Array iarr = DArray_create(100, ARRAY_ACS);

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

// -------------------------------------------------------------------

int
main(int argc, char *argv[])
{
    const char *logfilename = "log/array.log";

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false, 0, "Starting");

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Int/double creation/descr test"       , .desc = "", .mandatory=true)
      , testnew(.f2 = tf2, .num = 2, .name = "Int/double filling test"              , .desc = "", .mandatory=true)
      , testnew(.f2 = tf3, .num = 3, .name = "Shrink test"                          , .desc = "", .mandatory=true)
      , testnew(.f2 = tf4, .num = 4, .name = "Save/load int test"                   , .desc = "", .mandatory=true)
      , testnew(.f2 = tf5, .num = 5, .name = "Save/load dbl test"                   , .desc = "", .mandatory=true)
    );

    logclose("end...");
    return 0;
}



#endif /* ARRAYTESTING */

