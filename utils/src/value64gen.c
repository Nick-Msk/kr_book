/********************************************************************
                    VALUE64GEN MODULE IMPLEMENTATION
********************************************************************/

#include "value64gen.h"

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

value64Gen                      value64GenInit(value64GenFunc func, value64_type type, 
                                            int initcnt, value64 initdata1, value64 initdata2) {
    invraisecode(func != NULL, ERR_NULLABLE_PTR, "Generation function can't be null");
    invraisecode(value64_checktype(type), ERR_UNSUPPORTED_TYPE, "vale64 type %d isn't suppoted", type);

    value64Gen res = (value64Gen) {.fnext = func, .type = type, .counter = initcnt, 
                                .data[0] = initdata1, .data[1] = initdata2 };
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
value64 value64GenZero(value64Gen *gen)
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

extern value64                  value64GenAsсSeries(value64Gen *gen);
extern value64                  value64GenAscRnd(value64Gen *gen);
extern value64                  value64GenDescSeries(value64Gen *gen);
extern value64                  value64GenDescRnd(value64Gen *gen);
extern value64                  value64GenRandom(value64Gen *gen);

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
tf_gen_init_free(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: init and free with Zero generator", ++subnum);
    {
        value64Gen gen = value64GenInit(value64GenZero, VALUE64_INT, 0,
                                        LITERAL64_ZERO, LITERAL64_ZERO);
        test_validate(gen.fnext != NULL, "generator function must be set");
        test_validate(gen.type == VALUE64_INT, "type must be INT");
        test_validate(gen.counter == 0, "initial counter must be 0");

        value64GenFree(&gen);   // must not crash / leak
        fs_alloc_check(true);
    }

    test_sub("subtest %d: init and free with STR type", ++subnum);
    {
        value64Gen gen = value64GenInit(value64GenZero, VALUE64_STR, 5,
                                        LITERAL64_ZERO, LITERAL64_ZERO);
        test_validate(gen.type == VALUE64_STR, "type must be STR");
        test_validate(gen.counter == 5, "initial counter must be 5");

        value64GenFree(&gen);   // data[] were zero, no dynamic memory to free
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST value64GenNext (zero) -------------------------
static TestStatus
tf_gen_next_zero(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: Next with Zero generator (INT)", ++subnum);
    {
        value64Gen gen = value64GenInit0(value64GenZero, VALUE64_INT, 100);

        // Zero generator ignores counter and always returns zero
        value64 v1 = value64GenNext(&gen);
        test_validate(value64_int(v1) == 0, "first call must return 0");
        test_validate(gen.counter == 100, "Zero must not change counter");

        value64 v2 = value64GenNext(&gen);
        test_validate(value64_int(v2) == 0, "second call must return 0");
        test_validate(gen.counter == 100, "counter still unchanged");

        value64 v3 = value64GenNext(&gen);
        test_validate(value64_int(v3) == 0, "third call must return 0");
        test_validate(gen.counter == 100, "counter still unchanged");

        value64GenFree(&gen);
    }

    test_sub("subtest %d: Next with Zero generator (STR)", ++subnum);
    {
        value64Gen gen = value64GenInit0(value64GenZero, VALUE64_STR, 7);

        value64 v = value64GenNext(&gen);
        test_validatefree(
            value64_str(v) && strcmp(value64_str(v), "") == 0,
            value64free(v, VALUE64_STR),
            "Zero for STR must return empty string"
        );
        test_validate(gen.counter == 7, "counter must not change");
        value64free(v, VALUE64_STR);

        value64free(v, VALUE64_STR);
        
        value64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 3. Zero generator for FS – stress test (many allocations) */
    test_sub("subtest %d: Next with Zero generator (FS), 100 elements", ++subnum);
    {
        value64Gen gen = value64GenInit00(value64GenZero, VALUE64_FS);

        enum { N = 100 };
        value64 arr[N];

        for (int i = 0; i < N; i++) {
            arr[i] = value64GenNext(&gen);
        }

        /* Verify all elements are empty strings and no corruption occurred */
        for (int i = 0; i < N; i++) {
            test_validate(arr[i].fsval != NULL && fs_len(arr[i].fsval) == 0,
                        "FS[%d] must be empty", i);
            test_validate(gen.counter == 0, "Zero generator must keep counter unchanged");
        }

        /* Free all generated FS values */
        for (int i = 0; i < N; i++) {
            value64free(arr[i], VALUE64_FS);
        }

        value64GenFree(&gen);
        fs_alloc_check(true);   // must not detect any leak
    }

    return TEST_PASSED;
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main(/* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        TESTADD(tf_gen_init_free,           "Simple init and validate test"),
        TESTADD(tf_gen_next_zero,           "value64GenNext (zero) sompletest")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* VALUE64GEN_TESTING */

