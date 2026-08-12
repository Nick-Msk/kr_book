/********************************************************************
                    VALUE64GEN MODULE IMPLEMENTATION
********************************************************************/

#include "value64gen.h"

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

value64Gen                      value64GenInit(value64GenFunc func, value64_type type, 
                                               value64 initdata1, value64 initdata2) {
    invraisecode(func != NULL, ERR_NULLABLE_PTR, "Generation function can't be null");
    invraisecode(value64_checktype(type), ERR_UNSUPPORTED_TYPE, "value64 type %d isn't suppoted", type);

    value64Gen res = (value64Gen) {.fnext = func, .type = type, .counter = 0U, 
                                .data = { [0] = initdata1, [1] = initdata2 } };
    return res;
}        

// -------------------- ACCESS AND MODIFICATORS -------------------------------------

/**
 * @brief Returns the next generated value and advances the generator.
 *
 * Calls gen->next(gen) which produces the value and increments gen->counter
 * as needed (for series generators).  The caller takes ownership of the
 * returned value.
 *
 * @param gen  pointer to the generator (must not be NULL)
 * @return     the next value64 of the generator's type
 */
value64                         value64GenNext(value64Gen *gen)
{
    /* The next-function is responsible for incrementing counter when required. */
    gen->counter++;     // just for stats and LIMITS
    return gen->fnext(gen);
}


// ------------------------ pre-created func V64 typed ------------------------------

/**
 * @brief Standard generator for zero / empty values.
 *
 * Returns a "zero" value of the generator's type:
 * - numeric types → 0 / 0L / 0.0 / 0UL
 * - bool          → false
 * - char          → '\0'
 * - pointer       → NULL
 * - fs            → empty fs (via value64_createfs_asstr(""))
 * - str           → empty string (via value64_createstr(""))
 *
 * The counter is left unchanged.
 *
 * @param gen  pointer to the generator (type must be set)
 * @return     a zero value64 of the requested type (ownership passed to caller)
 */
value64 value64GenUnlimZero(value64Gen *gen)
{
    switch (gen->type) {
        case VALUE64_INT:
            return value64_createint(0);
        case VALUE64_LONG:
            return value64_createlong(0L);
        case VALUE64_DBL:
            return value64_createdbl(0.0);
        case VALUE64_ULONG:
            return value64_createulong(0UL);
        case VALUE64_BOOL:
            return value64_createbool(false);
        case VALUE64_CHR:
            return value64_createchar('\0');
        case VALUE64_PTR:
            return value64_createptr(NULL);
        case VALUE64_FS:
            return value64_createfs_asstr("");
        case VALUE64_STR:
            return value64_createstr("");
        default:
            /* unknown type – return literal zero */
            return LITERAL64_ZERO;
    }
}

/**
 * @brief Unchecked unlimited ascending series generator.
 *
 * Numeric types: data[0] holds the current value and is incremented.
 * Boolean:        data[0] toggles.
 * String types:   data[0] holds the current integer, data[1] holds an
 *                optional printf-template (STR or FS). If absent, "%d".
 *
 * @param gen  pointer to the generator
 * @return     the next value64 of the generator's type (ownership passed to caller)
 */
value64
value64UncheckGenUnlimAscSeries(value64Gen *gen)
{
    value64 result = V64GENREG0(gen);
    switch (gen->type) {
        case VALUE64_INT:
        case VALUE64_LONG:
        case VALUE64_DBL:
        case VALUE64_ULONG:
        case VALUE64_CHR:
            break;

        case VALUE64_BOOL:
            V64GENREG0(gen).bval = !V64GENREG0(gen).bval; // No reg API?
            return result;  // origin

        case VALUE64_STR:
        case VALUE64_FS: {
            int val = value64GenGetAsInt(gen, 0);

            const char *fmt = value64GenRegFs(gen, 1, "%d");
            fs tmp = fscopyf(fmt, val);

            if (gen->type == VALUE64_FS) 
                result = value64_movefs(&tmp); // VALUE64_FS
            else
                result = LITERAL64_STR(tmp.v); // STR  no strdup() here!
            break;
        }

        default:
            return userraise(LITERAL64_ZERO, ERR_UNSUPPORTED_TYPE, "Unsupported type %d", gen->type);
    }
    value64GenRegAdd(gen, 0, 1);
    return result;       
}

extern value64                  value64GenUnlimAscRnd(value64Gen *gen);
extern value64                  value64GenUnlimDescSeries(value64Gen *gen);
extern value64                  value64GenUnlimDescRnd(value64Gen *gen);
extern value64                  value64GenUnlimRandom(value64Gen *gen);

// ------------------------------------ ETC. ----------------------------------------

// validation always use DIRECT fprintf (no userraise) to be able to work
// if you want to log using logging system, exec value64GenValidate(logfile, gen);
bool                      value64GenValidate(FILE *restrict out, const value64Gen *restrict gen) {
    if (!gen) {
        if (out)
            fprintf(out, "Gen is null");
        return logsimpleret(false, "Gen is null\n");
    }
    if (!gen->fnext) {
        if (out)
            fprintf(out, "Next Function is null\n");
        return logsimpleret(false, "Next Function is null");
    }
    if (!value64_checktype(gen->type)) {
        if (out)
            fprintf(out, "Value64 type %d is incorrect\n", gen->type);
        return logsimpleret(false, "Value64 type %d is incorrect", gen->type);
    }
    for (int i = 0; i < VALUE64GENCOUNT; i++) {
        if (!value64_validate(out, gen->data[i], gen->type) ) {
            if (out)
                fprintf(out, "V64 data[%d] is incorrect\n", i);
            return logsimpleret(false, "V64 data[%d] is incorrect", i);
        }
    }
    return true;
}

// ---------------------------------------- Testing ------------------------------------------
#ifdef VALUE64GEN_TESTING

#include "test.h"

//types for testing

// ------------------------- TEST init_free ---------------------------------

// ------------------------- TEST value64GenInit / value64GenFree -------------------------
static TestStatus
tf1_gen_init_free(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: init and free with Zero generator", ++subnum);
    {
        value64Gen gen = value64GenInit(value64GenUnlimZero, VALUE64_INT,
                                        LITERAL64_ZERO, LITERAL64_ZERO);
        test_validate(gen.fnext != NULL, "generator function must be set");
        test_validate(gen.type == VALUE64_INT, "type must be INT");
        test_validate(gen.counter == 0, "initial counter must be 0");

        value64GenFree(&gen);   // must not crash / leak
        fs_alloc_check(true);
    }

    test_sub("subtest %d: init and free with STR type", ++subnum);
    {
        value64Gen gen = value64GenInit(value64GenUnlimZero, VALUE64_STR,
                                        LITERAL64_ZERO, LITERAL64_ZERO);
        test_validate(gen.type == VALUE64_STR, "type must be STR");
        test_validate(gen.counter == 0, "initial counter must be 0");

        value64GenFree(&gen);   // data[] were zero, no dynamic memory to free
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST value64GenNext (zero) -------------------------
static TestStatus
tf2_gen_next_zero(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: Next with Zero generator (INT)", ++subnum);
    {
        value64Gen gen = value64GenInit0(value64GenUnlimZero, VALUE64_INT);

        // Zero generator ignores counter and always returns zero
        value64 v1 = value64GenNext(&gen);
        test_validate(value64_int(v1) == 0, "first call must return 0");
        test_validate(gen.counter == 1, "counter must be 1");

        value64 v2 = value64GenNext(&gen);
        test_validate(value64_int(v2) == 0, "second call must return 0");
        test_validate(gen.counter == 2, "counter must be 2");

        value64 v3 = value64GenNext(&gen);
        test_validate(value64_int(v3) == 0, "third call must return 0");
        test_validate(gen.counter == 3, "counter must be 3");

        value64GenFree(&gen);
    }

    test_sub("subtest %d: Next with Zero generator (STR)", ++subnum);
    {
        value64Gen gen = value64GenInit0(value64GenUnlimZero, VALUE64_STR);

        value64 v = value64GenNext(&gen);
        test_validatefree(
            value64_str(v) && strcmp(value64_str(v), "") == 0,
            value64free(v, VALUE64_STR),
            "Zero for STR must return empty string"
        );
        test_validate(gen.counter == 1, "counter must be 1");
        value64free(v, VALUE64_STR);

        value64free(v, VALUE64_STR);
        
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 3. Zero generator for FS – stress test (many allocations) */
    test_sub("subtest %d: Next with Zero generator (FS), 100 elements", ++subnum);
    {
        value64Gen gen = value64GenInit0(value64GenUnlimZero, VALUE64_FS);

        enum { N = 100 };
        value64 arr[N];

        for (int i = 0; i < N; i++) {
            arr[i] = value64GenNext(&gen);
        }

        /* Verify all elements are empty strings and no corruption occurred */
        for (int i = 0; i < N; i++) {
            test_validate(arr[i].fsval != NULL && fs_len(arr[i].fsval) == 0,
                        "FS[%d] must be empty", i);
            test_validate(gen.counter == N, "Must be %d", N);
        }

        /* Free all generated FS values */
        for (int i = 0; i < N; i++) {
            value64free(arr[i], VALUE64_FS);
        }

        value64GenFree(&gen);
        fs_alloc_check(true);   // must not detect any leak
    }
    /* Zero generator for STR – 300 empty strings */
    test_sub("subtest %d: Next with Zero generator (STR), 300 elements", ++subnum);
    {
        value64Gen gen = value64GenInit0(value64GenUnlimZero, VALUE64_STR);

        enum { N = 300 };
        value64 arr[N];
        for (int i = 0; i < N; i++) {
            arr[i] = value64GenNext(&gen);
        }

        for (int i = 0; i < N; i++) {
            test_validate(value64_str(arr[i]) != NULL && strlen(value64_str(arr[i])) == 0,
                          "STR[%d] must be empty", i);
        }
        test_validate(gen.counter == N, "Zero generator counter must be %d", N);

        for (int i = 0; i < N; i++) {
            value64free(arr[i], VALUE64_STR);
        }
        value64GenFree(&gen);
        fs_alloc_check(true);
    }
    test_sub("subtest %d: wrapper for INT", ++subnum);
    {
        value64Gen gen = value64GenCreatorUnlimZero(VALUE64_INT);
        for (int i = 0; i < 400; i++) {
            int res;
            test_validate(
                (res = value64GenNext(&gen).ival) == 0,
                "Iteration %d got %d", i, res
            );
        }
        value64GenFree(&gen);
    }
    test_sub("subtest %d: wrapper for FS", ++subnum);
    {
        value64Gen gen = value64GenCreatorUnlimZero(VALUE64_FS);
        for (int i = 0; i < 400; i++) {
            fs *res = value64GenNext(&gen).fsval;
            test_validatefree(
                strcmp(fs_str(res), "") == 0,
                (fs_free(res), value64GenFree(&gen) ),
                "Iteration %d got %s instead of \"\"", i, fs_str(res)
            );
            fs_free(res);
        }
        value64GenFree(&gen);
    }

    return TEST_PASSED;
}

// ------------------------- TEST value64Gen data register API -------------------------
static TestStatus
tf_gen_reg_api(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ---------- value64GenRegStr ---------- */
    test_sub("subtest %d: RegStr non-STR type returns default", ++subnum);
    {
        value64Gen gen = value64GenInit0(value64GenUnlimZero, VALUE64_INT);
        const char *res = value64GenRegStr(&gen, 0, "fallback");
        test_validate(strcmp(res, "fallback") == 0,
                      "non-STR must return default, got '%s'", res);
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: RegStr empty STR returns default", ++subnum);
    {
        value64Gen gen = value64GenInit(value64GenUnlimZero, VALUE64_STR,
                                        value64_createstr(""), LITERAL64_ZERO);
        const char *res = value64GenRegStr(&gen, 0, "default_empty");
        test_validate(strcmp(res, "default_empty") == 0,
                      "empty STR must return default, got '%s'", res);
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: RegStr non-empty STR returns stored string", ++subnum);
    {
        value64Gen gen = value64GenInit(value64GenUnlimZero, VALUE64_STR,
                                        value64_createstr("hello"), LITERAL64_ZERO);
        const char *res = value64GenRegStr(&gen, 0, "fallback");
        test_validate(strcmp(res, "hello") == 0,
                      "non-empty STR must return stored string, got '%s'", res);
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: RegStr NULL STR returns default", ++subnum);
    {
        value64Gen gen = value64GenInit0(value64GenUnlimZero, VALUE64_STR);
        // data[0].sval is NULL by default (LITERAL64_ZERO)
        const char *res = value64GenRegStr(&gen, 0, "fallback_null");
        test_validate(strcmp(res, "fallback_null") == 0,
                      "NULL STR must return default, got '%s'", res);
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* ---------- value64GenRegFs ---------- */
    test_sub("subtest %d: RegFs non-FS type returns default", ++subnum);
    {
        value64Gen gen = value64GenInit0(value64GenUnlimZero, VALUE64_INT);
        const char *res = value64GenRegFs(&gen, 0, "fs_fallback");
        test_validate(strcmp(res, "fs_fallback") == 0,
                      "non-FS must return default, got '%s'", res);
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: RegFs NULL fs returns default", ++subnum);
    {
        value64Gen gen = value64GenInit0(value64GenUnlimZero, VALUE64_FS);
        // data[0].fsval is NULL by default
        const char *res = value64GenRegFs(&gen, 0, "fs_default_null");
        logauto(res);
        test_validate(strcmp(res, "fs_default_null") == 0,
                      "NULL fs must return default, got '%s'", res);
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: RegFs non-empty FS returns stored string", ++subnum);
    {
        value64Gen gen = value64GenInit(value64GenUnlimZero, VALUE64_FS,
                                        value64_createfs_asstr("world"), LITERAL64_ZERO);
        const char *res = value64GenRegFs(&gen, 0, "fs_fallback");
        test_validate(strcmp(res, "world") == 0,
                      "non-empty FS must return stored string, got '%s'", res);
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* ---------- value64GenRegAdd ---------- */
    test_sub("subtest %d: RegAdd INT", ++subnum);
    {
        value64Gen gen = value64GenInit(value64GenUnlimZero, VALUE64_INT,
                                        value64_createint(10), LITERAL64_ZERO);
        value64 res = value64GenRegAdd(&gen, 0, 5);
        test_validate(value64_int(res) == 15 && value64_int(gen.data[0]) == 15,
                      "INT RegAdd failed: res=%d, reg=%d",
                      value64_int(res), value64_int(gen.data[0]));
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: RegAdd LONG", ++subnum);
    {
        value64Gen gen = value64GenInit(value64GenUnlimZero, VALUE64_LONG,
                                        value64_createlong(100L), LITERAL64_ZERO);
        value64 res = value64GenRegAdd(&gen, 0, 20);
        test_validate(value64_long(res) == 120L && value64_long(gen.data[0]) == 120L,
                      "LONG RegAdd failed: res=%ld, reg=%ld",
                      value64_long(res), value64_long(gen.data[0]));
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: RegAdd ULONG", ++subnum);
    {
        value64Gen gen = value64GenInit(value64GenUnlimZero, VALUE64_ULONG,
                                        value64_createulong(200UL), LITERAL64_ZERO);
        value64 res = value64GenRegAdd(&gen, 0, 30);
        test_validate(value64_ulong(res) == 230UL &&
                      value64_ulong(gen.data[0]) == 230UL,
                      "ULONG RegAdd failed: res=%lu, reg=%lu",
                      value64_ulong(res), value64_ulong(gen.data[0]));
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: RegAdd DBL", ++subnum);
    {
        value64Gen gen = value64GenInit(value64GenUnlimZero, VALUE64_DBL,
                                        value64_createdbl(10.5), LITERAL64_ZERO);
        value64 res = value64GenRegAdd(&gen, 0, 2);
        test_validate(fabs(value64_dbl(res) - 12.5) < 1e-9 &&
                      fabs(value64_dbl(gen.data[0]) - 12.5) < 1e-9,
                      "DBL RegAdd failed: res=%f, reg=%f",
                      value64_dbl(res), value64_dbl(gen.data[0]));
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: RegAdd unsupported type does not modify register", ++subnum);
    {
        // data[0] содержит непустую строку, чтобы value64_equal работала без NULL
        value64Gen gen = value64GenInit1(value64GenUnlimZero, VALUE64_STR,
                                        value64_createstr("hello"));
        value64 original = gen.data[0];   // "hello"
        value64 res = value64GenRegAdd(&gen, 0, 1);   // для STR не должно менять регистр

        // Теперь обе строки не NULL, можно использовать value64_equal
        test_validate(value64_equal(res, original, VALUE64_STR),
                    "RegAdd on STR must return unchanged register value");
        test_validate(value64_equal(gen.data[0], original, VALUE64_STR),
                    "RegAdd on STR must leave register unchanged");

        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST value64UncheckGenUnlimAscSeries -------------------------
static TestStatus
tf_gen_asc_series(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. INT ascending from 10 */
    test_sub("subtest %d: AscSeries INT from 10", ++subnum);
    {
        value64Gen gen = value64GenInit(value64UncheckGenUnlimAscSeries, VALUE64_INT,
                                        value64_createint(10), LITERAL64_ZERO);
        value64 v1 = value64GenNext(&gen);
        test_validate(value64_int(v1) == 10, "first must be 10");
        test_validate(gen.counter == 1, "counter must be 1");

        value64 v2 = value64GenNext(&gen);
        test_validate(value64_int(v2) == 11, "second must be 11");
        test_validate(gen.counter == 2, "counter must be 2");

        value64 v3 = value64GenNext(&gen);
        test_validate(value64_int(v3) == 12, "third must be 12");
        test_validate(gen.counter == 3, "counter must be 3");

        value64_free(&v1, VALUE64_INT);
        value64_free(&v2, VALUE64_INT);
        value64_free(&v3, VALUE64_INT);
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 2. LONG ascending from 100 */
    test_sub("subtest %d: AscSeries LONG from 100", ++subnum);
    {
        value64Gen gen = value64GenInit(value64UncheckGenUnlimAscSeries, VALUE64_LONG,
                                        value64_createlong(100L), LITERAL64_ZERO);
        value64 v1 = value64GenNext(&gen);
        test_validate(value64_long(v1) == 100L, "first must be 100");
        value64 v2 = value64GenNext(&gen);
        test_validate(value64_long(v2) == 101L, "second must be 101");
        value64 v3 = value64GenNext(&gen);
        test_validate(value64_long(v3) == 102L, "third must be 102");
        test_validate(gen.counter == 3, "counter must be 3");

        value64_free(&v1, VALUE64_LONG);
        value64_free(&v2, VALUE64_LONG);
        value64_free(&v3, VALUE64_LONG);
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 3. DBL ascending from 10.5 */
    test_sub("subtest %d: AscSeries DBL from 10.5", ++subnum);
    {
        value64Gen gen = value64GenInit(value64UncheckGenUnlimAscSeries, VALUE64_DBL,
                                        value64_createdbl(10.5), LITERAL64_ZERO);
        value64 v1 = value64GenNext(&gen);
        test_validate(fabs(value64_dbl(v1) - 10.5) < 1e-9, "first must be 10.5");
        value64 v2 = value64GenNext(&gen);
        test_validate(fabs(value64_dbl(v2) - 11.5) < 1e-9, "second must be 11.5");
        value64 v3 = value64GenNext(&gen);
        test_validate(fabs(value64_dbl(v3) - 12.5) < 1e-9, "third must be 12.5");
        test_validate(gen.counter == 3, "counter must be 3");

        value64_free(&v1, VALUE64_DBL);
        value64_free(&v2, VALUE64_DBL);
        value64_free(&v3, VALUE64_DBL);
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 4. ULONG ascending from 200 */
    test_sub("subtest %d: AscSeries ULONG from 200", ++subnum);
    {
        value64Gen gen = value64GenInit(value64UncheckGenUnlimAscSeries, VALUE64_ULONG,
                                        value64_createulong(200UL), LITERAL64_ZERO);
        value64 v1 = value64GenNext(&gen);
        test_validate(value64_ulong(v1) == 200UL, "first must be 200");
        value64 v2 = value64GenNext(&gen);
        test_validate(value64_ulong(v2) == 201UL, "second must be 201");
        value64 v3 = value64GenNext(&gen);
        test_validate(value64_ulong(v3) == 202UL, "third must be 202");
        test_validate(gen.counter == 3, "counter must be 3");

        value64_free(&v1, VALUE64_ULONG);
        value64_free(&v2, VALUE64_ULONG);
        value64_free(&v3, VALUE64_ULONG);
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 5. BOOL toggles */
    test_sub("subtest %d: AscSeries BOOL toggles", ++subnum);
    {
        value64Gen gen = value64GenInit(value64UncheckGenUnlimAscSeries, VALUE64_BOOL,
                                        value64_createbool(false), LITERAL64_ZERO);
        value64 v1 = value64GenNext(&gen);
        test_validate(value64_bool(v1) == false, "first must be false");
        value64 v2 = value64GenNext(&gen);
        test_validate(value64_bool(v2) == true, "second must be true");
        value64 v3 = value64GenNext(&gen);
        test_validate(value64_bool(v3) == false, "third must be false");
        test_validate(gen.counter == 3, "counter must be 3");

        value64_free(&v1, VALUE64_BOOL);
        value64_free(&v2, VALUE64_BOOL);
        value64_free(&v3, VALUE64_BOOL);
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 6. CHAR ascending from 'A' (now supported via RegAdd) */
    test_sub("subtest %d: AscSeries CHAR from 'A'", ++subnum);
    {
        value64Gen gen = value64GenInit(value64UncheckGenUnlimAscSeries, VALUE64_CHR,
                                        value64_createchar('A'), LITERAL64_ZERO);
        value64 v1 = value64GenNext(&gen);
        test_validate(value64_char(v1) == 'A', "first must be 'A'");
        value64 v2 = value64GenNext(&gen);
        test_validate(value64_char(v2) == 'B', "second must be 'B'");
        value64 v3 = value64GenNext(&gen);
        test_validate(value64_char(v3) == 'C', "third must be 'C'");
        test_validate(gen.counter == 3, "counter must be 3");

        value64_free(&v1, VALUE64_CHR);
        value64_free(&v2, VALUE64_CHR);
        value64_free(&v3, VALUE64_CHR);
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 7. STR without template (data[0]=int, data[1]=zero) */
    test_sub("subtest %d: AscSeries STR default template", ++subnum);
    {
        value64Gen gen = value64GenInit(value64UncheckGenUnlimAscSeries, VALUE64_STR,
                                        value64_createint(0), LITERAL64_ZERO);
        value64 v1 = value64GenNext(&gen);
        test_validate(strcmp(value64_str(v1), "0") == 0, "first must be '0'");
        value64_free(&v1, VALUE64_STR);

        value64 v2 = value64GenNext(&gen);
        test_validate(strcmp(value64_str(v2), "1") == 0, "second must be '1'");
        value64_free(&v2, VALUE64_STR);

        value64 v3 = value64GenNext(&gen);
        test_validate(strcmp(value64_str(v3), "2") == 0, "third must be '2'");
        value64_free(&v3, VALUE64_STR);

        test_validate(gen.counter == 3, "counter must be 3");
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 8. STR with template "item %d" starting at 3 (template is FS) */
    test_sub("subtest %d: AscSeries STR with template", ++subnum);
    {
        value64Gen gen = value64GenInit(value64UncheckGenUnlimAscSeries, VALUE64_STR,
                                        value64_createint(3), value64_createfs_asstr("item %d"));
        value64 v1 = value64GenNext(&gen);
        test_validate(strcmp(value64_str(v1), "item 3") == 0, "first must be 'item 3'");
        value64_free(&v1, VALUE64_STR);

        value64 v2 = value64GenNext(&gen);
        test_validate(strcmp(value64_str(v2), "item 4") == 0, "second must be 'item 4'");
        value64_free(&v2, VALUE64_STR);

        value64 v3 = value64GenNext(&gen);
        test_validate(strcmp(value64_str(v3), "item 5") == 0, "third must be 'item 5'");
        value64_free(&v3, VALUE64_STR);

        test_validate(gen.counter == 3, "counter must be 3");
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 9. FS without template (data[0]=int, data[1]=zero) */
    test_sub("subtest %d: AscSeries FS default template", ++subnum);
    {
        value64Gen gen = value64GenInit(value64UncheckGenUnlimAscSeries, VALUE64_FS,
                                        value64_createint(0), LITERAL64_ZERO);
        value64 v1 = value64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v1)), "0") == 0, "first must be '0'");
        value64_free(&v1, VALUE64_FS);

        value64 v2 = value64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v2)), "1") == 0, "second must be '1'");
        value64_free(&v2, VALUE64_FS);

        value64 v3 = value64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v3)), "2") == 0, "third must be '2'");
        value64_free(&v3, VALUE64_FS);

        test_validate(gen.counter == 3, "counter must be 3");
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 10. FS with template "val_%d" starting at 7 */
    test_sub("subtest %d: AscSeries FS with template", ++subnum);
    {
        value64Gen gen = value64GenInit(value64UncheckGenUnlimAscSeries, VALUE64_FS,
                                        value64_createint(7), value64_createfs_asstr("val_%d"));
        value64 v1 = value64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v1)), "val_7") == 0, "first must be 'val_7'");
        value64_free(&v1, VALUE64_FS);

        value64 v2 = value64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v2)), "val_8") == 0, "second must be 'val_8'");
        value64_free(&v2, VALUE64_FS);

        value64 v3 = value64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v3)), "val_9") == 0, "third must be 'val_9'");
        value64_free(&v3, VALUE64_FS);

        test_validate(gen.counter == 3, "counter must be 3");
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main(/* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        TESTADD(tf1_gen_init_free,           "Simple init and validate test")
      , TESTADD(tf2_gen_next_zero,           "value64GenNext (zero) simple test")
      , TESTADD(tf_gen_reg_api,              "value64Gen data register API simple test")
      , TESTADD(tf_gen_asc_series,           "value64UncheckGenUnlimAscSeries() simple test")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* VALUE64GEN_TESTING */

