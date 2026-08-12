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

value64                         value64UncheckGenUnlimAsсSeries(value64Gen *gen) {
    
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

// ------------------------------------------------------------------------------------------------------------------------------
int
main(/* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        TESTADD(tf1_gen_init_free,           "Simple init and validate test"),
        TESTADD(tf2_gen_next_zero,           "value64GenNext (zero) sompletest")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* VALUE64GEN_TESTING */

