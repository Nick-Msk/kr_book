/********************************************************************
                    VALUE64GEN MODULE IMPLEMENTATION
********************************************************************/

#include "v64gen.h"

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

v64Gen                          v64GenInit(v64GenFunc func, value64_type type, v64typed reg0, v64typed reg1) {
    invraisecode(func != NULL, ERR_NULLABLE_PTR, "Generation function can't be null");
    invraisecode(value64_checktype(type), ERR_UNSUPPORTED_TYPE, "value64 type %d isn't supported", type);

    v64Gen res = (v64Gen) {
        .fnext   = func,
        .type    = type,
        .counter = 0U,
        .data    = { [0] = reg0, [1] = reg1 }
    };
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
value64                         v64GenNext(v64Gen *gen)
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
value64 v64GenUnlimZero(v64Gen *gen)    // TODO: need to be refactored via Table!
{
    switch (gen->type) {
        case VALUE64_INT:
            return value64_createint(0);    // TODO: ANY value here!
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

static value64
v64UncheckGenUnlimAscValue(v64Gen *gen, int value) {
    value64 result = V64GENREG0(gen).val;
    switch (gen->type) {        // type of output generation
        case VALUE64_INT:
        case VALUE64_LONG:
        case VALUE64_DBL:
        case VALUE64_ULONG:
        case VALUE64_CHR:
            break;

        case VALUE64_BOOL:
            // V64GENREG0(gen).bval = !V64GENREG0(gen).bval; // No reg API?
            v64typedBoolNegative(&V64GENREG0(gen));
            return result;  // origin

        case VALUE64_STR:
        case VALUE64_FS: {
            // this is NOT a simple V64GENREG0(gen).ival, this is type casting into int (w/o range checking for now)
            int         val = v64typedCastToInt(V64GENREG0(gen));       

            const char *fmt = v64typedNvlStr(V64GENREG1(gen), "%d"); //value64GenRegFs(gen, 1, "%d");
            fs          tmp = fscopyf(fmt, val);

            if (gen->type == VALUE64_FS) 
                result = value64_movefs(&tmp); // VALUE64_FS
            else {
                result = LITERAL64_STR(fs_movetostr(&tmp) );
            }
            break;
        }

        default:
            return userraise(LITERAL64_ZERO, ERR_UNSUPPORTED_TYPE, "Unsupported type %d", gen->type);
    }
    v64typedAdd(&V64GENREG0(gen), value);

    return result;
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
v64UncheckGenUnlimAscSeries(v64Gen *gen)
{
    return v64UncheckGenUnlimAscValue(gen, 1);
}

/**
 * @brief Unchecked unlimited ascending random series generator.
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
v64UncheckGenUnlimAscRnd(v64Gen *gen)
{
    return v64UncheckGenUnlimAscValue(gen, rndint(4) + 1);
}

extern value64                  v64UncheckGenUnlimDescSeries(v64Gen *gen);
extern value64                  v64UncheckGenUnlimDescRnd(v64Gen *gen);
extern value64                  v64UncheckGenUnlimRandom(v64Gen *gen);

// ------------------------------------ ETC. ----------------------------------------

// validation always use DIRECT fprintf (no userraise) to be able to work
// if you want to log using logging system, exec value64GenValidate(logfile, gen);
bool                      v64GenValidate(FILE *restrict out, const v64Gen *restrict gen) {
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
    for (int i = 0; i < V64GENCOUNT; i++) {
        if (!value64_validate(out, gen->data[i].val, gen->data[i].typ) ) {
            if (out)
                fprintf(out, "V64 data[%d] is incorrect\n", i);
            return logsimpleret(false, "V64 data[%d] is incorrect", i);
        }
    }
    return true;
}

// ---------------------------------------- Testing ------------------------------------------
#ifdef V64GEN_TESTING

#include "test.h"

//types for testing

// ------------------------- TEST init_free ---------------------------------

// ------------------------- TEST v64GenInit / v64GenFree -------------------------
static TestStatus
tf1_gen_init_free(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: init and free with Zero generator", ++subnum);
    {
        v64Gen gen = v64GenInit(v64GenUnlimZero, VALUE64_INT,
                                        V64TYPEDZERO(), V64TYPEDZERO());
        test_validate(gen.fnext != NULL, "generator function must be set");
        test_validate(gen.type == VALUE64_INT, "type must be INT");
        test_validate(gen.counter == 0, "initial counter must be 0");
        test_validate(gen.data[0].typ == VALUE64_UNKNOWN, "data[0] should start UNKNOWN");
        test_validate(gen.data[1].typ == VALUE64_UNKNOWN, "data[1] should start UNKNOWN");

        v64GenFree(&gen);   // must not crash / leak
        fs_alloc_check(true);
    }

    test_sub("subtest %d: init and free with STR type", ++subnum);
    {
        v64Gen gen = v64GenInit(v64GenUnlimZero, VALUE64_STR,
                                        V64TYPEDZERO(), V64TYPEDZERO());
        test_validate(gen.type == VALUE64_STR, "type must be STR");
        test_validate(gen.fnext != NULL, "fnext must be set");
        test_validate(gen.counter == 0, "initial counter must be 0");
        test_validate(gen.data[0].typ == VALUE64_UNKNOWN, "data[0] should start UNKNOWN");
        test_validate(gen.data[1].typ == VALUE64_UNKNOWN, "data[1] should start UNKNOWN");
        

        v64GenFree(&gen);   // data[] were zero, no dynamic memory to free
        fs_alloc_check(true);
    }
    test_sub("subtest %d: init and free with STR type using simplifire creator", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimZero(VALUE64_STR);
        test_validate(gen.type == VALUE64_STR, "type must be STR");
        test_validate(gen.fnext == v64GenUnlimZero, "fnext (%p) must be set to v64GenUnlimZero (%p)", gen.fnext, v64GenUnlimZero);
        test_validate(gen.counter == 0, "initial counter must be 0");
        test_validate(gen.data[0].typ == VALUE64_UNKNOWN, "data[0] should start UNKNOWN");
        test_validate(gen.data[1].typ == VALUE64_UNKNOWN, "data[1] should start UNKNOWN");
    }

    return TEST_PASSED;
}

// ------------------------- TEST v64GenNext (zero) -------------------------
static TestStatus
tf2_gen_next_zero(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: Next with Zero generator (INT)", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimZero(VALUE64_INT);   //v64GenInit0(v64GenUnlimZero, VALUE64_INT);

        // Zero generator ignores counter and always returns zero
        value64 v1 = v64GenNext(&gen);
        test_validate(value64_int(v1) == 0, "first call must return 0");
        test_validate(gen.counter == 1, "counter must be 1");

        value64 v2 = v64GenNext(&gen);
        test_validate(value64_int(v2) == 0, "second call must return 0");
        test_validate(gen.counter == 2, "counter must be 2");

        value64 v3 = v64GenNext(&gen);
        test_validate(value64_int(v3) == 0, "third call must return 0");
        test_validate(gen.counter == 3, "counter must be 3");

        v64GenFree(&gen);
    }

    test_sub("subtest %d: Next with Zero generator (STR)", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimZero(VALUE64_STR);

        value64 v = v64GenNext(&gen);
        test_validatefree(
            value64_str(v) && strcmp(value64_str(v), "") == 0,
            value64free(v, VALUE64_STR),
            "Zero for STR must return empty string"
        );
        test_validate(gen.counter == 1, "counter must be 1");
        value64free(v, VALUE64_STR);
        
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 3. Zero generator for FS – stress test (many allocations) */
    test_sub("subtest %d: Next with Zero generator (FS), 100 elements", ++subnum);
    {
        v64Gen gen = v64GenInit0(v64GenUnlimZero, VALUE64_FS);

        enum { N = 100 };
        value64 arr[N];

        for (int i = 0; i < N; i++) {
            arr[i] = v64GenNext(&gen);
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

        v64GenFree(&gen);
        fs_alloc_check(true);   // must not detect any leak
    }
    /* Zero generator for STR – 300 empty strings */
    test_sub("subtest %d: Next with Zero generator (STR), 300 elements", ++subnum);
    {
        v64Gen gen = v64GenInit0(v64GenUnlimZero, VALUE64_STR);

        enum { N = 300 };
        value64 arr[N];
        for (int i = 0; i < N; i++) {
            arr[i] = v64GenNext(&gen);
        }

        for (int i = 0; i < N; i++) {
            test_validate(value64_str(arr[i]) != NULL && strlen(value64_str(arr[i])) == 0,
                          "STR[%d] must be empty", i);
        }
        test_validate(gen.counter == N, "Zero generator counter must be %d", N);

        for (int i = 0; i < N; i++) {
            value64free(arr[i], VALUE64_STR);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }
    test_sub("subtest %d: wrapper for INT", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimZero(VALUE64_INT);
        for (int i = 0; i < 400; i++) {
            int res;
            test_validate(
                (res = v64GenNext(&gen).ival) == 0,
                "Iteration %d got %d", i, res
            );
        }
        v64GenFree(&gen);
    }
    test_sub("subtest %d: wrapper for FS", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimZero(VALUE64_FS);
        for (int i = 0; i < 400; i++) {
            fs *res = v64GenNext(&gen).fsval;
            test_validatefree(
                strcmp(fs_str(res), "") == 0,
                (fs_free(res), v64GenFree(&gen) ),
                "Iteration %d got %s instead of \"\"", i, fs_str(res)
            );
            fs_free(res);
        }
        v64GenFree(&gen);
    }

    return TEST_PASSED;
}

// ------------------------- TEST value64UncheckGenUnlimAscSeries -------------------------
static TestStatus
tf3_gen_asc_series(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. INT ascending from 10 */
    test_sub("subtest %d: AscSeries INT from 10", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscSeries(VALUE64_INT, 10, NULL);
        value64 v1 = v64GenNext(&gen);
        test_validate(value64_int(v1) == 10, "first must be 10");
        test_validate(gen.counter == 1, "counter must be 1");

        value64 v2 = v64GenNext(&gen);
        test_validate(value64_int(v2) == 11, "second must be 11");
        test_validate(gen.counter == 2, "counter must be 2");

        value64 v3 = v64GenNext(&gen);
        test_validate(value64_int(v3) == 12, "third must be 12");
        test_validate(gen.counter == 3, "counter must be 3");

        value64_free(&v1, VALUE64_INT);
        value64_free(&v2, VALUE64_INT);
        value64_free(&v3, VALUE64_INT);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 2. LONG ascending from 100 */
    test_sub("subtest %d: AscSeries LONG from 100", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscSeries(VALUE64_LONG, 100L, NULL);
        value64 v1 = v64GenNext(&gen);
        test_validate(value64_long(v1) == 100L, "first must be 100");
        value64 v2 = v64GenNext(&gen);
        test_validate(value64_long(v2) == 101L, "second must be 101");
        value64 v3 = v64GenNext(&gen);
        test_validate(value64_long(v3) == 102L, "third must be 102");
        test_validate(gen.counter == 3, "counter must be 3");

        value64_free(&v1, VALUE64_LONG);
        value64_free(&v2, VALUE64_LONG);
        value64_free(&v3, VALUE64_LONG);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 3. DBL ascending from 10.5 */
    test_sub("subtest %d: AscSeries DBL from 10.5", ++subnum);
    {
        // only via v64GenInit1 for now, no simplifier for that
        v64Gen gen = v64GenInit1(v64UncheckGenUnlimAscSeries, VALUE64_DBL,
                                        v64typedCreateDbl(10.5));
        value64 v1 = v64GenNext(&gen);
        test_validate(fabs(value64_dbl(v1) - 10.5) < 1e-9, "first must be 10.5");
        value64 v2 = v64GenNext(&gen);
        test_validate(fabs(value64_dbl(v2) - 11.5) < 1e-9, "second must be 11.5");
        value64 v3 = v64GenNext(&gen);
        test_validate(fabs(value64_dbl(v3) - 12.5) < 1e-9, "third must be 12.5");
        test_validate(gen.counter == 3, "counter must be 3");

        value64_free(&v1, VALUE64_DBL);
        value64_free(&v2, VALUE64_DBL);
        value64_free(&v3, VALUE64_DBL);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 4. ULONG ascending from 200 */
    test_sub("subtest %d: AscSeries ULONG from 200", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscSeries(VALUE64_ULONG,
                                        200UL, NULL);
        value64 v1 = v64GenNext(&gen);
        test_validate(value64_ulong(v1) == 200UL, "first must be 200");
        value64 v2 = v64GenNext(&gen);
        test_validate(value64_ulong(v2) == 201UL, "second must be 201");
        value64 v3 = v64GenNext(&gen);
        test_validate(value64_ulong(v3) == 202UL, "third must be 202");
        test_validate(gen.counter == 3, "counter must be 3");

        value64_free(&v1, VALUE64_ULONG);
        value64_free(&v2, VALUE64_ULONG);
        value64_free(&v3, VALUE64_ULONG);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 5. BOOL toggles */
    test_sub("subtest %d: AscSeries BOOL toggles", ++subnum);
    {
        v64Gen gen = v64GenInit1(v64UncheckGenUnlimAscSeries, VALUE64_BOOL,
                                        v64typedCreateBool(false));
        value64 v1 = v64GenNext(&gen);
        test_validate(value64_bool(v1) == false, "first must be false");
        value64 v2 = v64GenNext(&gen);
        test_validate(value64_bool(v2) == true, "second must be true");
        value64 v3 = v64GenNext(&gen);
        test_validate(value64_bool(v3) == false, "third must be false");
        test_validate(gen.counter == 3, "counter must be 3");

        value64_free(&v1, VALUE64_BOOL);
        value64_free(&v2, VALUE64_BOOL);
        value64_free(&v3, VALUE64_BOOL);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 6. CHAR ascending from 'A' (now supported via RegAdd) */
    test_sub("subtest %d: AscSeries CHAR from 'A'", ++subnum);
    {
        v64Gen gen = v64GenInit1(v64UncheckGenUnlimAscSeries, VALUE64_CHR,
                                        v64typedCreateChar('A'));
        value64 v1 = v64GenNext(&gen);
        test_validate(value64_char(v1) == 'A', "first must be 'A'");
        value64 v2 = v64GenNext(&gen);
        test_validate(value64_char(v2) == 'B', "second must be 'B'");
        value64 v3 = v64GenNext(&gen);
        test_validate(value64_char(v3) == 'C', "third must be 'C'");
        test_validate(gen.counter == 3, "counter must be 3");

        value64_free(&v1, VALUE64_CHR);
        value64_free(&v2, VALUE64_CHR);
        value64_free(&v3, VALUE64_CHR);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 7. STR without template (data[0]=int, data[1]=zero) */
    test_sub("subtest %d: AscSeries STR default template", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscSeries(VALUE64_STR, 0, NULL);

        value64 v1 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v1), "0") == 0, "first must be '0'");
        value64_free(&v1, VALUE64_STR);

        value64 v2 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v2), "1") == 0, "second must be '1'");
        value64_free(&v2, VALUE64_STR);

        value64 v3 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v3), "2") == 0, "third must be '2'");
        value64_free(&v3, VALUE64_STR);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 8. STR with template "item %d" starting at 3 (template is FS) */
    test_sub("subtest %d: AscSeries STR with template", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscSeries(VALUE64_STR, 3, "item %d");
        value64 v1 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v1), "item 3") == 0, "first must be 'item 3'");
        value64_free(&v1, VALUE64_STR);

        value64 v2 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v2), "item 4") == 0, "second must be 'item 4'");
        value64_free(&v2, VALUE64_STR);

        value64 v3 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v3), "item 5") == 0, "third must be 'item 5'");
        value64_free(&v3, VALUE64_STR);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 9. FS without template (data[0]=int, data[1]=zero) */
    test_sub("subtest %d: AscSeries FS default template", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscSeries(VALUE64_FS, 0, NULL);

        value64 v1 = v64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v1)), "0") == 0, "first must be '0'");
        value64_free(&v1, VALUE64_FS);

        value64 v2 = v64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v2)), "1") == 0, "second must be '1'");
        value64_free(&v2, VALUE64_FS);

        value64 v3 = v64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v3)), "2") == 0, "third must be '2'");
        value64_free(&v3, VALUE64_FS);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 10. FS with template "val_%d" starting at 7 */
    test_sub("subtest %d: AscSeries FS with template", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscSeries(VALUE64_FS, 7, "val_%d");
        value64 v1 = v64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v1)), "val_7") == 0, "first must be 'val_7'");
        value64_free(&v1, VALUE64_FS);

        value64 v2 = v64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v2)), "val_8") == 0, "second must be 'val_8'");
        value64_free(&v2, VALUE64_FS);

        value64 v3 = v64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v3)), "val_9") == 0, "third must be 'val_9'");
        value64_free(&v3, VALUE64_FS);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST v64gen creators (simple wrappers) -------------------------
static TestStatus
tf4_gen_creators(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. v64GenCreatorUnlimStrValue: cоздаёт генератор типа STR c региcтром data[0] = cтрока,
       а v64GenNext возвращает пуcтую cтроку (zero) */
    test_sub("subtest %d: v64GenCreatorUnlimStrValue (STR)", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimStrValue("hello");

        test_validate(gen.type == VALUE64_STR, "type must be STR");
        test_validate(gen.data[0].typ == VALUE64_STR, "data[0] must be STR");
        test_validate(strcmp(value64_str(gen.data[0].val), "hello") == 0,
                      "data[0] value mismatch");

        value64 v = v64GenNext(&gen);
        test_validatefree(
            value64_str(v) != NULL && strcmp(value64_str(v), "") == 0,
            v64typedFree(&(v64typed){.val = v, .typ = VALUE64_STR}),
            "Zero for STR must be empty string, got '%s'", value64_str(v)
        );
        v64typedFree(&(v64typed){.val = v, .typ = VALUE64_STR});
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 2. v64GenCreatorUnlimValue (LONG): генератор типа LONG, data[0]=long, zero = 0L */
    test_sub("subtest %d: v64GenCreatorUnlimValue (LONG)", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimValue(12345L);

        test_validate(gen.type == VALUE64_LONG, "type must be LONG");
        test_validate(gen.data[0].typ == VALUE64_LONG, "data[0] must be LONG");
        test_validate(value64_long(gen.data[0].val) == 12345L,
                      "data[0] value mismatch");

        value64 v = v64GenNext(&gen);
        test_validate(value64_long(v) == 0L, "Zero for LONG must be 0, got %ld", value64_long(v));
        // no ownership to free for LONG
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: v64GenCreatorUnlimDouble (DBL)", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDouble(3.14);

        // Эти проверки должны пройти поcле иcправления типа на VALUE64_DBL
        test_validate(gen.type == VALUE64_DBL, "type must be DBL (bug: got %d %s)",
                      gen.type, value64_typename(gen.type));
        test_validate(gen.data[0].typ == VALUE64_DBL, "data[0] must be DBL");
        test_validate(fabs(value64_dbl(gen.data[0].val) - 3.14) < 1e-9,
                      "data[0] value mismatch");

        value64 v = v64GenNext(&gen);
        test_validate(fabs(value64_dbl(v) - 0.0) < 1e-9,
                      "Zero for DBL must be 0.0, got %f", value64_dbl(v));
        v64GenFree(&gen);
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
      , TESTADD(tf2_gen_next_zero,           "v64GenNext (zero) simple test")
      , TESTADD(tf3_gen_asc_series,          "value64UncheckGenUnlimAscSeries() simple test")
      , TESTADD(tf4_gen_creators,         "v64gen creators (simple wrappers) simple test")
     //, TESTADD(tf4_v64gen_creators,         "v64gen creators (simple wrappers) simple test")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* V64GEN_TESTING */

