/********************************************************************
                    VALUE64GEN MODULE IMPLEMENTATION
********************************************************************/

#include "v64gen.h"

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

// --------------------------- Utilities --------------------------------------------
static bool                     v64GenStringUpdate(v64Gen *gen, /* const char *restrict newbuf, */ long amount) {
    if (amount <= 0)
        return false;
    // only for Stream Buffer
    //if (newbuf)
    //    V64GENREGVAL0(gen).pval = (void *) newbuf;
    if (V64GENREGVAL1(gen).lval != LONG_MAX)
        V64GENREGVAL1(gen).lval += amount;
    return true;
}

/**
 * @brief Returns the number of characters remaining in the source fs.
 *
 * Used only by generators that iterate over an fs (e.g., v64GenFSChar).
 * data[0] holds a non-owning pointer to the source fs,
 * data[1] holds the current read position as a long.
 *
 * @param gen  pointer to the generator
 * @return     remaining characters (0 if exhausted or invalid)
 */
static unsigned long            v64GenGetRemainingCount(v64Gen *gen)
{
    fs *src = (fs *)V64GENREGVAL0(gen).pval;
    if (!src->v)
        return 0L;

    return fs_len(src) - V64GENREGVAL1(gen).ulval;
}

/**
 * @brief Finalizer for newline generator: returns the remaining data as last line.
 */
static value64                  v64GenFSToFsByNewlineFinalize(v64Gen *gen) {
    fs              *src = (fs *) V64GENREGVAL0(gen).pval;
    unsigned long   pos = V64GENREGVAL1(gen).ulval;

    if (!src->v || pos >= src->len)
        return LITERAL64_ZERO;

    size_t          remaining_len = src->len - pos;
    if (remaining_len > 0 && src->v[src->len - 1] == '\r')
        remaining_len--;

    fs              line = fs_newsubstr(src, pos, remaining_len);
    V64GENREGVAL1(gen).ulval = src->len;   // позиция в конец
    return value64_movefs(&line);
}

// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

v64Gen                          v64GenInit(v64GenFunc func, value64_type type, 
                                            v64typed reg0, v64typed reg1, v64typed reg2, v64typed reg3) {
    invraisecode(func != NULL, ERR_NULLABLE_PTR, "Generation function can't be null");
    invraisecode(value64_checktype(type), ERR_UNSUPPORTED_TYPE, "value64 type %d isn't supported", type);

    v64Gen res = (v64Gen) {
        .fnext   = func,
        .type    = type,
        .counter = 0U,
        .updater = NULL,
        .data    = { [0] = reg0, [1] = reg1, [2] = reg2, [3] = reg3 }
    };
    return res;
}

// --------------------------- Creator from Source (c-str, FILE *, Ds *) series ------------

// RETURNS: CHR
// REGITRSY ALLOCATION:
// data[0] STR as SOURCE (no ownership)
// data[1] LONG as lim, if 0 - unlim (LONG_MAX actually)
v64Gen                          v64GenCreatorSourceCstrChar(const char *src, long maxlen) {
    invraisecode(src != NULL, ERR_NULLABLE_PTR, "NUll src c-str");

    if (maxlen <= 0)
        maxlen = LONG_MAX; // unlim
    v64Gen gen =  v64GenInit2(v64GenStringToChar, VALUE64_CHR, 
            v64typedCreateCstrSource(src), v64typedCreateLong(maxlen) );
    gen.updater = v64GenStringUpdate;        // setup updater

    return gen;
}

// RETURNS: CHR
// REGITRSY ALLOCATION:
// data[0] FS as SOURCE (no ownership)
// data[1] ULONG as position
v64Gen                          v64GenCreatorSourceFsToChar(const fs *src) {
    invraisecode(src != NULL, ERR_NULLABLE_PTR, "NUll src fs");

    v64Gen gen =   v64GenInit2(v64GenFSToChar, VALUE64_CHR,
                        v64typedCreateFsSource(src),
                        v64typedCreateULong(0UL));

    gen.remaining = v64GenGetRemainingCount;
    return gen;
}

// RETURNS: FS
// REGITRSY ALLOCATION:
// data[0] FS as SOURCE (no ownership)
// data[1] ULONG as position
v64Gen                          v64GenCreatorSourceFsToFsByNewline(const fs *src){
    invraisecode(src != NULL, ERR_NULLABLE_PTR, "NUll src fs");

    v64Gen gen = v64GenInit2(v64GenFSToFsByNewline, VALUE64_FS,
                             v64typedCreateFsSource(src),    // data[0] PTR
                             v64typedCreateULong(0UL));         // data[1] POSITION

    gen.finalizer = v64GenFSToFsByNewlineFinalize;
    gen.remaining = v64GenGetRemainingCount;
    return gen;
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
    gen->counter++;     // just for stats and LIMITS (not impl yet)
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
v64UncheckGenUnlimAscValue(v64Gen *gen, int value) { // TODO: need to be refactored via Table!
    value64 result = V64GENREGVAL0(gen);
    switch (gen->type) {        // type of output generation
        case VALUE64_INT:
        case VALUE64_LONG:
        case VALUE64_DBL:
        case VALUE64_ULONG:
        case VALUE64_CHR:
            break;

        case VALUE64_BOOL:
            v64typedBoolNegative(&V64GENREG0(gen));
            return result;  // origin

        case VALUE64_STR:
        case VALUE64_FS: {
            int         val = value64_long(V64GENREGVAL0(gen));       

            const char *fmt = v64typedNvlStr(V64GENREG1(gen), "%d");
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
    int r = value64_int(V64GENREGVAL2(gen) );
    return v64UncheckGenUnlimAscValue(gen, rndint(r) + 1);
}
/**
 * @brief Unchecked unlimited descending series generator.
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
v64UncheckGenUnlimDescSeries(v64Gen *gen) {
    return v64UncheckGenUnlimAscValue(gen, -1);
}
/**
 * @brief Unchecked unlimited descending random series generator.
 *
 * Numeric types: data[0] holds the current value and is incremented.
 * Boolean:        data[0] toggles.
 * String types:   data[0] holds the current integer, data[1] holds an
 *                optional printf-template (STR or FS). If absent, "%d".
 *
 * @param gen  pointer to the generator
 * @return     the next value64 of the generator's type (ownership passed to caller)
 */
value64                         v64UncheckGenUnlimDescRnd(v64Gen *gen) {
    int r = value64_int(V64GENREGVAL2(gen) );

    return v64UncheckGenUnlimAscValue(gen, -(rndint(r) + 1) );
}

value64                         v64GenUnlimRandom(v64Gen *gen) {
    int         r = value64_int(V64GENREGVAL0(gen) );
    r = rndint(r);
    switch (gen->type) {        // type of output generation
        case VALUE64_INT:
            return value64_createint(r);
        case VALUE64_LONG:
            return value64_createlong(r);
        case VALUE64_ULONG:
            return value64_createulong(r);
        case VALUE64_DBL:
            return value64_createdbl(r);
        case VALUE64_CHR:
            return value64_createchar(r);
        case VALUE64_BOOL:
            return value64_createbool(r);
        case VALUE64_FS: case VALUE64_STR: {
            const char *fmt = v64typedNvlStr(V64GENREG1(gen), "%d");
            fs          tmp = fscopyf(fmt, r);
            value64     result;
            if (gen->type == VALUE64_FS) 
                result = value64_movefs(&tmp); // VALUE64_FS
            else {
                result = LITERAL64_STR(fs_movetostr(&tmp) );
            }
            return result;      // FS or STR
        }
        default:
            return LITERAL64_ZERO;
    }
}

// ------------------------- Source generators (Ds or fs or c-str or FILE *) ------------------------

// RETURNS: CHR
// REGITRSY ALLOCATION:
// data[0] STR as SOURCE (no ownership)
// data[1] LONG as lim, if 0 - unlim (LONG_MAX actually)
value64                         v64GenStringToChar(v64Gen *gen) {
    invraisecode(gen != NULL, ERR_NULLABLE_PTR, "Null generator");

    const char *str = (const char *)V64GENREGVAL0(gen).pval;
    if (!str || *str == '\0')
        return value64_createchar('\0');

    long        remaining = V64GENREGVAL1(gen).lval;
    if (remaining <= 0)
        return value64_createchar('\0');

    // уменьшаем остаток if != LONG_MAX
    if (V64GENREGVAL1(gen).lval != LONG_MAX)
        V64GENREGVAL1(gen).lval = remaining - 1;
    // сдвигаем невладеющий указатель
    V64GENREGVAL0(gen).pval = (void*) (str + 1);

    return value64_createchar(*str);
}

// RETURNS: CHR
// REGITRSY ALLOCATION:
// data[0] – PTR to fs (non-owning)
// data[1] – LONG current position
value64                         v64GenFSToChar(v64Gen *gen){
    invraisecode(gen != NULL, ERR_NULLABLE_PTR, "Null generator");

    fs              *src = (fs *) V64GENREGVAL0(gen).pval;
    unsigned long    pos = V64GENREGVAL1(gen).ulval;

    if (!src->v || pos >= src->len)
        return value64_createchar('\0');

    V64GENREGVAL1(gen).ulval = pos + 1;
    return value64_createchar( fs_str(src)[pos] );
}

/**
 * @brief Parse fs from fs, dividev by newline
 * @note
 * @return: value64/fs
 * @note
 * data[0] – PTR to source fs (non-owning)
 * data[1] – ULONG current read position
 */
value64                         v64GenFSToFsByNewline(v64Gen *gen){
    invraisecode(gen != NULL, ERR_NULLABLE_PTR, "Null generator");

    fs           *src = (fs *)V64GENREGVAL0(gen).pval;
    unsigned long pos = V64GENREGVAL1(gen).ulval;

    /* Нет данных или позиция вышла за пределы — возвращаем null fs (ожидание/конец) */
    if (!src->v || pos >= src->len)
        return LITERAL64_ZERO;

    const char   *start = src->v + pos;
    const char   *newline = strchr(start, '\n');

    if (newline) {
        size_t line_len = newline - start;
        if (line_len > 0 && start[line_len - 1] == '\r')
            line_len--;                       /* убираем \r */

        /* Создаём копию подстроки; длина может быть 0 (пустая строка) */
        fs  line = fs_newsubstr(src, pos, line_len);     // must be freed!!!!!
        V64GENREGVAL1(gen).ulval = newline - src->v + 1;   /* за '\n' */
        return value64_movefs(&line);
    } else 
        return LITERAL64_ZERO;
}


// ------------------------ PRINTERS/CHECKERS ---------------------------------------

int                             v64Techfprint(FILE *restrict out, const v64Gen *restrict gen, const char *restrict name) {
    int cnt = 0;
    if (out) {
        IOCHECKER(w, fprintf(out, "V64GEN:%s [", name), -1)  // name cab be NULL
            cnt += w;
        if (!gen) {
            IOCHECKER(w, fprintf(out, "<NULL>"), -1)
                cnt += w;
        } else {
            IOCHECKER(w, fprintf(out, "fnext=%p, updater=%p, counter=%u, type=%d/%s\t",
                        gen->fnext, gen->updater, gen->counter, gen->type, value64_typename(gen->type) ), -1)
                cnt += w;
            for (int i = 0; i < V64GENCOUNT; i++) {
                char    buf[50];
                snprintf(buf, sizeof(buf) - 1, "REG%d", i);
                v64typedTechfprint(out, gen->data[i], buf);
            }
        }
        IOCHECKER(w, fprintf(out, "]\n"), -1)
            cnt += w;
    }
    return cnt;
}


// ------------------------------------ ETC. ----------------------------------------

// validation always use DIRECT fprintf (no userraise) to be able to work
// if you want to log using logging system, exec value64GenValidate(logfile, gen);
bool                            v64GenValidate(FILE *restrict out, const v64Gen *restrict gen) {
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
        v64Gen gen = v64GenInit2(v64GenUnlimZero, VALUE64_INT,
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
        v64Gen gen = v64GenInit2(v64GenUnlimZero, VALUE64_STR,
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
        v64Gen gen = v64GenCreatorUnlimAscSeries(v64typedCreateInt(10) );
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
        v64Gen gen = v64GenCreatorUnlimAscSeries(v64typedCreateLong(100L) );
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
    test_sub("subtest %d: AscSeries DBL from 10.5 directly via v64GenInit1", ++subnum);
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
    test_sub("subtest %d: AscSeries DBL from 10.5 via creator", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscSeries(v64typedCreateDbl(10.5) );

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
        v64Gen gen = v64GenCreatorUnlimAscSeries(v64typedCreateULong(200UL) );

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
    test_sub("subtest %d: AscSeries BOOL toggles directly", ++subnum);
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

    test_sub("subtest %d: AscSeries BOOL toggles, via creator", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscSeries(v64typedCreateBool(false) );

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
    test_sub("subtest %d: AscSeries CHAR from 'A' directly", ++subnum);
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

    test_sub("subtest %d: AscSeries CHAR from 'A' via creator", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscSeries(v64typedCreateChar('A') );

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
        v64Gen gen = v64GenCreatorUnlimAscStrSeries(0, NULL);

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
        v64Gen gen = v64GenCreatorUnlimAscStrSeries(3, "item %d");
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
        v64Gen gen = v64GenCreatorUnlimAscFsSeries(0, NULL);

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
        v64Gen gen = v64GenCreatorUnlimAscFsSeries(7, "val_%d");
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

// ------------------------- TEST : asc random generator -------------------------
static TestStatus
tf5_gen_asc_rnd(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: AscRnd INT increments by 1..6", ++subnum);
    {
       v64Gen gen = v64GenCreatorUnlimAscRnd(v64typedCreateInt(10), 5);
 
        value64 v1 = v64GenNext(&gen);
        test_validate(value64_int(v1) == 10, "first must be 10");

        value64 v2 = v64GenNext(&gen);
        int diff = value64_int(v2) - value64_int(v1);
        test_validate(diff >= 1 && diff <= 6, "increment must be in [1,6], got %d", diff);

        value64 v3 = v64GenNext(&gen);
        int diff2 = value64_int(v3) - value64_int(v2);
        test_validate(diff2 >= 1 && diff2 <= 6, "second increment must be in [1,6], got %d", diff2);

        value64_free(&v1, gen.type);
        value64_free(&v2, gen.type);
        value64_free(&v3, gen.type);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: AscRnd LONG increments by 1..6", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscRnd(v64typedCreateInt(100L), 5);

        value64 v1 = v64GenNext(&gen);
        test_validate(value64_long(v1) == 100L, "first must be 100");

        value64 v2 = v64GenNext(&gen);
        long diff = value64_long(v2) - value64_long(v1);
        test_validate(diff >= 1 && diff <= 6, "increment must be in [1,6], got %ld", diff);

        value64 v3 = v64GenNext(&gen);
        long diff2 = value64_long(v3) - value64_long(v2);
        test_validate(diff2 >= 1 && diff2 <= 6, "second increment must be in [1,6], got %ld", diff2);

        value64_free(&v1, gen.type);
        value64_free(&v2, gen.type);
        value64_free(&v3, gen.type);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: AscRnd DBL increments by 1.0..6.0 direct call", ++subnum);
    {
        v64Gen gen = v64GenInit3(v64UncheckGenUnlimAscRnd, VALUE64_DBL,
                                v64typedCreateDbl(10.0), V64TYPEDZERO(), v64typedCreateInt(5) );

        value64 v1 = v64GenNext(&gen);
        test_validate(fabs(value64_dbl(v1) - 10.0) < 1e-9, "first must be 10.0");

        value64 v2 = v64GenNext(&gen);
        double diff = value64_dbl(v2) - value64_dbl(v1);
        test_validate(diff >= 1.0 && diff <= 6.0, "increment must be in [1.0,6.0], got %f", diff);

        value64 v3 = v64GenNext(&gen);
        double diff2 = value64_dbl(v3) - value64_dbl(v2);
        test_validate(diff2 >= 1.0 && diff2 <= 6.0, "second increment must be in [1.0,6.0], got %f", diff2);

        value64_free(&v1, gen.type);
        value64_free(&v2, gen.type);
        value64_free(&v3, gen.type);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: AscRnd DBL increments by 1.0..6.0 via creator", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscRnd(v64typedCreateDbl(10.0), 5);

        value64 v1 = v64GenNext(&gen);
        test_validate(fabs(value64_dbl(v1) - 10.0) < 1e-9, "first must be 10.0");

        value64 v2 = v64GenNext(&gen);
        double diff = value64_dbl(v2) - value64_dbl(v1);
        test_validate(diff >= 1.0 && diff <= 6.0, "increment must be in [1.0,6.0], got %f", diff);

        value64 v3 = v64GenNext(&gen);
        double diff2 = value64_dbl(v3) - value64_dbl(v2);
        test_validate(diff2 >= 1.0 && diff2 <= 6.0, "second increment must be in [1.0,6.0], got %f", diff2);

        value64_free(&v1, gen.type);
        value64_free(&v2, gen.type);
        value64_free(&v3, gen.type);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: AscRnd ULONG increments by 1..6", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscRnd(v64typedCreateULong(200UL), 5);

        value64 v1 = v64GenNext(&gen);
        test_validate(value64_ulong(v1) == 200UL, "first must be 200");

        value64 v2 = v64GenNext(&gen);
        unsigned long diff = value64_ulong(v2) - value64_ulong(v1);
        test_validate(diff >= 1 && diff <= 6, "increment must be in [1,6], got %lu", diff);

        value64 v3 = v64GenNext(&gen);
        unsigned long diff2 = value64_ulong(v3) - value64_ulong(v2);
        test_validate(diff2 >= 1 && diff2 <= 6, "second increment must be in [1,6], got %lu", diff2);

        value64_free(&v1, gen.type);
        value64_free(&v2, gen.type);
        value64_free(&v3, gen.type);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: AscRnd CHAR increments by 1..6", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscRnd(v64typedCreateChar('A'), 5);

        value64 v1 = v64GenNext(&gen);
        test_validate(value64_char(v1) == 'A', "first must be 'A'");

        value64 v2 = v64GenNext(&gen);
        int diff = (unsigned char)value64_char(v2) - (unsigned char)value64_char(v1);
        test_validate(diff >= 1 && diff <= 6, "increment must be in [1,6], got %d", diff);

        value64 v3 = v64GenNext(&gen);
        int diff2 = (unsigned char)value64_char(v3) - (unsigned char)value64_char(v2);
        test_validate(diff2 >= 1 && diff2 <= 6, "second increment must be in [1,6], got %d", diff2);

        value64_free(&v1, gen.type);
        value64_free(&v2, gen.type);
        value64_free(&v3, gen.type);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: AscRnd BOOL toggles", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscRnd(v64typedCreateBool(false), 0);

        value64 v1 = v64GenNext(&gen);
        test_validate(value64_bool(v1) == false, "first must be false");

        value64 v2 = v64GenNext(&gen);
        test_validate(value64_bool(v2) == true, "second must be true");

        value64 v3 = v64GenNext(&gen);
        test_validate(value64_bool(v3) == false, "third must be false");

        value64_free(&v1, gen.type);
        value64_free(&v2, gen.type);
        value64_free(&v3, gen.type);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: AscRnd STR with template, data[0] increments", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscStrRnd(0, "item %d", 5);

        value64 v1 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v1), "item 0") == 0,
                      "first must be 'item 0', got '%s'", value64_str(v1));
        value64_free(&v1, gen.type);

        int before = v64typedCastToInt(gen.data[0]);
        value64 v2 = v64GenNext(&gen);
        int after = v64typedCastToInt(gen.data[0]);
        int diff = after - before;
        test_validate(diff >= 1 && diff <= 6,
                      "data[0] must increase by 1..6, got %d", diff);

        value64_free(&v2, gen.type);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }
    test_sub("subtest %d: AscRnd FS with template, data[0] increments", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscFsRnd(0, "item %d", 5);

        value64 v1 = v64GenNext(&gen);
        test_validate(fs_cmpstr(value64_fs(v1), "item 0") == 0,
                      "first must be 'item 0', got '%s'", value64_fsstr(v1) );
        value64_free(&v1, gen.type);    // WOW THINK:

        int before = v64typedCastToInt(gen.data[0]);
        value64 v2 = v64GenNext(&gen);
        int after = v64typedCastToInt(gen.data[0]);
        int diff = after - before;
        test_validate(diff >= 1 && diff <= 6,
                      "data[0] must increase by 1..6, got %d", diff);

        value64_free(&v2, gen.type);
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST: AscRnd with custom rndinc -------------------------
static TestStatus
tf6_gen_asc_rnd_custom(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    srand((unsigned)time(NULL)); 

    /* 1. INT with rndinc=1: increments should be 1 or 2, and both must appear */
    test_sub("subtest %d: AscRnd INT with rndinc=1 (increments 1..2)", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscRnd(v64typedCreateInt(100), 1);

        value64 v0 = v64GenNext(&gen);
        test_validate(value64_int(v0) == 100, "first must be 100");
        value64_free(&v0, gen.type);

        bool saw_1 = false, saw_2 = false;
        value64 prev = value64_createint(100);  // не используется напрямую, можно просто отслеживать предыдущее число
        (void) prev;
        int prev_val = 100;
        for (int i = 0; i < 20; i++) {
            value64 v = v64GenNext(&gen);
            int diff = value64_int(v) - prev_val;
            test_validate(diff >= 1 && diff <= 2,
                          "increment must be in [1,2], got %d", diff);
            if (diff == 1)
                saw_1 = true;
            if (diff == 2)
                saw_2 = true;
            prev_val = value64_int(v);
            value64_free(&v, gen.type);
        }
        test_validate(saw_1 && saw_2,
                      "both increments 1 and 2 should appear with rndinc=1");

        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 2. LONG with rndinc=5: increments should be 1..6 (по текущей реализации) */
    test_sub("subtest %d: AscRnd LONG with rndinc=5 (increments 1..6)", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscRnd(v64typedCreateInt(500L), 5);

        value64 v0 = v64GenNext(&gen);
        test_validate(value64_long(v0) == 500L, "first must be 500");
        value64_free(&v0, gen.type);

        long prev_val = 500L;
        for (int i = 0; i < 30; i++) {
            value64 v = v64GenNext(&gen);
            long diff = value64_long(v) - prev_val;
            test_validate(diff >= 1 && diff <= 6,
                          "increment must be in [1,6], got %ld", diff);
            prev_val = value64_long(v);
            value64_free(&v, gen.type);
        }

        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: AscRnd STR with rndinc=3 and template", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscStrRnd(0, "val %d", 3);

        value64 v0 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v0), "val 0") == 0,
                    "first must be 'val 0', got '%s'", value64_str(v0));
        value64_free(&v0, gen.type);

        for (int i = 0; i < 15; i++) {
            int before = v64typedCastToInt(gen.data[0]);   // счётчик до вызова
            value64 v = v64GenNext(&gen);
            int after = v64typedCastToInt(gen.data[0]);    // после вызова
            int diff = after - before;
            test_validate(diff >= 1 && diff <= 4,
                        "data[0] must increase by 1..4, got %d", diff);
            value64_free(&v, gen.type);
        }

        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: AscRnd FS with rndinc=3 and template", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimAscFsRnd(0, "val %d", 3);

        value64 v0 = v64GenNext(&gen);
        test_validate(fs_cmpstr(value64_fs(v0), "val 0") == 0,
                    "first must be 'val 0', got '%s'", fs_str(value64_fs(v0) ) );
        value64_free(&v0, gen.type);

        for (int i = 0; i < 15; i++) {
            int before = v64typedCastToInt(gen.data[0]);   // счётчик до вызова
            value64 v = v64GenNext(&gen);
            int after = v64typedCastToInt(gen.data[0]);    // после вызова
            int diff = after - before;
            test_validate(diff >= 1 && diff <= 4,
                        "data[0] must increase by 1..4, got %d", diff);
            value64_free(&v, gen.type);
        }

        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST 6: DescSeries -------------------------
static TestStatus
tf7_gen_desc_series(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. INT descending from 10 */
    test_sub("subtest %d: DescSeries INT from 10", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescSeries(v64typedCreateInt(10) );

        value64 v1 = v64GenNext(&gen);
        test_validate(value64_int(v1) == 10, "first must be 10");
        value64_free(&v1, gen.type);

        value64 v2 = v64GenNext(&gen);
        test_validate(value64_int(v2) == 9, "second must be 9");
        value64_free(&v2, gen.type);

        value64 v3 = v64GenNext(&gen);
        test_validate(value64_int(v3) == 8, "third must be 8");
        value64_free(&v3, gen.type);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 2. LONG descending from 100 */
    test_sub("subtest %d: DescSeries LONG from 100", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescSeries(v64typedCreateLong(100L) );

        value64 v1 = v64GenNext(&gen);
        test_validate(value64_long(v1) == 100L, "first must be 100");
        value64_free(&v1, gen.type);

        value64 v2 = v64GenNext(&gen);
        test_validate(value64_long(v2) == 99L, "second must be 99");
        value64_free(&v2, gen.type);

        value64 v3 = v64GenNext(&gen);
        test_validate(value64_long(v3) == 98L, "third must be 98");
        value64_free(&v3, gen.type);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 3. DBL descending from 10.5 */
    test_sub("subtest %d: DescSeries DBL from 10.5", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescSeries(v64typedCreateDbl(10.5) );

        value64 v1 = v64GenNext(&gen);
        test_validate(fabs(value64_dbl(v1) - 10.5) < 1e-9, "first must be 10.5");
        value64_free(&v1, gen.type);

        value64 v2 = v64GenNext(&gen);
        test_validate(fabs(value64_dbl(v2) - 9.5) < 1e-9, "second must be 9.5");
        value64_free(&v2, gen.type);

        value64 v3 = v64GenNext(&gen);
        test_validate(fabs(value64_dbl(v3) - 8.5) < 1e-9, "third must be 8.5");
        value64_free(&v3, gen.type);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 4. ULONG descending from 200 */
    test_sub("subtest %d: DescSeries ULONG from 200", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescSeries(v64typedCreateULong(200UL) );

        value64 v1 = v64GenNext(&gen);
        test_validate(value64_ulong(v1) == 200UL, "first must be 200");
        value64_free(&v1, gen.type);

        value64 v2 = v64GenNext(&gen);
        test_validate(value64_ulong(v2) == 199UL, "second must be 199");
        value64_free(&v2, gen.type);

        value64 v3 = v64GenNext(&gen);
        test_validate(value64_ulong(v3) == 198UL, "third must be 198");
        value64_free(&v3, gen.type);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 5. BOOL toggles */
    test_sub("subtest %d: DescSeries BOOL toggles", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescSeries(v64typedCreateBool(false) );

        value64 v1 = v64GenNext(&gen);
        test_validate(value64_bool(v1) == false, "first must be false");
        value64_free(&v1, gen.type);

        value64 v2 = v64GenNext(&gen);
        test_validate(value64_bool(v2) == true, "second must be true");
        value64_free(&v2, gen.type);

        value64 v3 = v64GenNext(&gen);
        test_validate(value64_bool(v3) == false, "third must be false");
        value64_free(&v3, gen.type);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 6. CHAR descending from 'C' */
    test_sub("subtest %d: DescSeries CHAR from 'C'", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescSeries(v64typedCreateChar('C') );

        value64 v1 = v64GenNext(&gen);
        test_validate(value64_char(v1) == 'C', "first must be 'C'");
        value64_free(&v1, gen.type);

        value64 v2 = v64GenNext(&gen);
        test_validate(value64_char(v2) == 'B', "second must be 'B'");
        value64_free(&v2, gen.type);

        value64 v3 = v64GenNext(&gen);
        test_validate(value64_char(v3) == 'A', "third must be 'A'");
        value64_free(&v3, gen.type);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 7. STR default template starting at 3 */
    test_sub("subtest %d: DescSeries STR default template", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescStrSeries(3L, NULL);

        value64 v1 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v1), "3") == 0, "first must be '3'");
        value64_free(&v1, gen.type);

        value64 v2 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v2), "2") == 0, "second must be '2'");
        value64_free(&v2, gen.type);

        value64 v3 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v3), "1") == 0, "third must be '1'");
        value64_free(&v3, gen.type);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 8. STR with template "item %d" */
    test_sub("subtest %d: DescSeries STR with template 'item %%d'", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescStrSeries(3L, "item %d");

        value64 v1 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v1), "item 3") == 0, "first must be 'item 3'");
        value64_free(&v1, gen.type);

        value64 v2 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v2), "item 2") == 0, "second must be 'item 2'");
        value64_free(&v2, gen.type);

        value64 v3 = v64GenNext(&gen);
        test_validate(strcmp(value64_str(v3), "item 1") == 0, "third must be 'item 1'");
        value64_free(&v3, gen.type);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 9. FS default template starting at 2 */
    test_sub("subtest %d: DescSeries FS default template", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescFsSeries(2L, NULL);

        value64 v1 = v64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v1)), "2") == 0, "first must be '2'");
        value64_free(&v1, gen.type);

        value64 v2 = v64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v2)), "1") == 0, "second must be '1'");
        value64_free(&v2, gen.type);

        value64 v3 = v64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v3)), "0") == 0, "third must be '0'");
        value64_free(&v3, gen.type);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 10. FS with template "val_%d" */
    test_sub("subtest %d: DescSeries FS with template 'val_%%d'", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescFsSeries(7L, "val_%d");

        value64 v1 = v64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v1)), "val_7") == 0, "first must be 'val_7'");
        value64_free(&v1, gen.type);

        value64 v2 = v64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v2)), "val_6") == 0, "second must be 'val_6'");
        value64_free(&v2, gen.type);

        value64 v3 = v64GenNext(&gen);
        test_validate(strcmp(fs_str(value64_fs(v3)), "val_5") == 0, "third must be 'val_5'");
        value64_free(&v3, gen.type);

        test_validate(gen.counter == 3, "counter must be 3");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST 7: DescRnd (descending random) -------------------------
static TestStatus
tf8_gen_desc_rnd_custom(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. INT with rndinc=1 (decrements 1..2) */
    test_sub("subtest %d: DescRnd INT with rndinc=1 (decrements 1..2)", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescRnd(v64typedCreateInt(100), 1);

        value64 v0 = v64GenNext(&gen);
        test_validate(value64_int(v0) == 100, "first must be 100");
        value64_free(&v0, gen.type);

        bool saw_1 = false, saw_2 = false;
        int prev_val = 100;
        for (int i = 0; i < 30; i++) {
            value64 v = v64GenNext(&gen);
            int diff = prev_val - value64_int(v);
            test_validate(diff >= 1 && diff <= 2,
                          "decrement must be in [1,2], got %d", diff);
            if (diff == 1) saw_1 = true;
            if (diff == 2) saw_2 = true;
            prev_val = value64_int(v);
            value64_free(&v, gen.type);
        }
        test_validate(saw_1 && saw_2,
                      "both decrements 1 and 2 should appear with rndinc=1");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 2. LONG with rndinc=5 (decrements 1..6) */
    test_sub("subtest %d: DescRnd LONG with rndinc=5 (decrements 1..6)", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescRnd(v64typedCreateLong(500L), 5);

        value64 v0 = v64GenNext(&gen);
        test_validate(value64_long(v0) == 500L, "first must be 500");
        value64_free(&v0, gen.type);

        long prev_val = 500L;
        for (int i = 0; i < 30; i++) {
            value64 v = v64GenNext(&gen);
            long diff = prev_val - value64_long(v);
            test_validate(diff >= 1 && diff <= 6,
                          "decrement must be in [1,6], got %ld", diff);
            prev_val = value64_long(v);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 3. DBL with rndinc=3 (decrements 1.0..4.0) */
    test_sub("subtest %d: DescRnd DBL with rndinc=3 (decrements 1.0..4.0)", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescRnd(v64typedCreateDbl(20.0), 3);

        value64 v0 = v64GenNext(&gen);
        test_validate(fabs(value64_dbl(v0) - 20.0) < 1e-9, "first must be 20.0");
        value64_free(&v0, gen.type);

        double prev_val = 20.0;
        for (int i = 0; i < 20; i++) {
            value64 v = v64GenNext(&gen);
            double diff = prev_val - value64_dbl(v);
            test_validate(diff >= 1.0 && diff <= 4.0,
                          "decrement must be in [1.0,4.0], got %f", diff);
            prev_val = value64_dbl(v);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 4. ULONG with rndinc=2 (decrements 1..3) */
    test_sub("subtest %d: DescRnd ULONG with rndinc=2 (decrements 1..3)", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescRnd(v64typedCreateULong(1000UL), 2);

        value64 v0 = v64GenNext(&gen);
        test_validate(value64_ulong(v0) == 1000UL, "first must be 1000");
        value64_free(&v0, gen.type);

        unsigned long prev_val = 1000UL;
        for (int i = 0; i < 20; i++) {
            value64 v = v64GenNext(&gen);
            unsigned long diff = prev_val - value64_ulong(v);
            test_validate(diff >= 1 && diff <= 3,
                          "decrement must be in [1,3], got %lu", diff);
            prev_val = value64_ulong(v);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 5. CHR with rndinc=1 (decrements 1..2) */
    test_sub("subtest %d: DescRnd CHAR with rndinc=1 (decrements 1..2)", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescRnd(v64typedCreateChar('Z'), 1);

        value64 v0 = v64GenNext(&gen);
        test_validate(value64_char(v0) == 'Z', "first must be 'Z'");
        value64_free(&v0, gen.type);

        int prev_val = (unsigned char)'Z';
        for (int i = 0; i < 10; i++) {
            value64 v = v64GenNext(&gen);
            int diff = prev_val - (unsigned char)value64_char(v);
            test_validate(diff >= 1 && diff <= 2,
                          "decrement must be in [1,2], got %d", diff);
            prev_val = (unsigned char)value64_char(v);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 6. BOOL toggles (value ignored) */
    test_sub("subtest %d: DescRnd BOOL toggles", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescRnd(v64typedCreateBool(false), 3);

        value64 v1 = v64GenNext(&gen);
        test_validate(value64_bool(v1) == false, "first must be false");
        value64_free(&v1, gen.type);

        value64 v2 = v64GenNext(&gen);
        test_validate(value64_bool(v2) == true, "second must be true");
        value64_free(&v2, gen.type);

        value64 v3 = v64GenNext(&gen);
        test_validate(value64_bool(v3) == false, "third must be false");
        value64_free(&v3, gen.type);

        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: DescRnd STR with template and rndinc=3", ++subnum);
{
    v64Gen gen = v64GenCreatorUnlimDescStrRnd(10, "item %d", 3);

    value64 v0 = v64GenNext(&gen);
    test_validate(strcmp(value64_str(v0), "item 10") == 0,
                  "first must be 'item 10', got '%s'", value64_str(v0));
    value64_free(&v0, gen.type);

    for (int i = 0; i < 15; i++) {
        int before = v64typedCastToInt(gen.data[0]);   // текущее значение до вызова
        value64 v = v64GenNext(&gen);
        int after = v64typedCastToInt(gen.data[0]);    // после вызова
        int diff = before - after;
        test_validate(diff >= 1 && diff <= 4,
                      "data[0] must decrease by 1..4, got %d", diff);
        value64_free(&v, gen.type);
    }

    v64GenFree(&gen);
    fs_alloc_check(true);
}

    /* 8. FS with template "val %d" and rndinc=2 (decrements 1..3) */
    test_sub("subtest %d: DescRnd FS with template and rndinc=2", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimDescFsRnd(7, "val %d", 2);

        value64 v0 = v64GenNext(&gen);
        test_validate(fs_cmpstr(value64_fs(v0), "val 7") == 0,
                      "first must be 'val 7', got '%s'", fs_str(value64_fs(v0)));
        value64_free(&v0, gen.type);

        for (int i = 0; i < 10; i++) {
            int before = v64typedCastToInt(gen.data[0]);   // текущее значение до вызова
            value64 v = v64GenNext(&gen);
            int after = v64typedCastToInt(gen.data[0]);    // после вызова
            int diff = before - after;
            test_validate(diff >= 1 && diff <= 4,
                        "data[0] must decrease by 1..4, got %d", diff);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST 8: UnlimRandom generators -------------------------
static TestStatus
tf9_gen_unlim_random(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. INT random in [0, rndinc] */
    test_sub("subtest %d: UnlimRandom INT with rndinc=10", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimRnd(VALUE64_INT, 10);
        for (int i = 0; i < 50; i++) {
            value64 v = v64GenNext(&gen);
            int val = value64_int(v);
            test_validate(val >= 0 && val <= 10,
                          "INT random value out of range: %d", val);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 2. LONG random in [0, rndinc] */
    test_sub("subtest %d: UnlimRandom LONG with rndinc=10", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimRnd(VALUE64_LONG, 10);
        for (int i = 0; i < 50; i++) {
            value64 v = v64GenNext(&gen);
            long val = value64_long(v);
            test_validate(val >= 0 && val <= 10,
                          "LONG random value out of range: %ld", val);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 3. ULONG random in [0, rndinc] */
    test_sub("subtest %d: UnlimRandom ULONG with rndinc=10", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimRnd(VALUE64_ULONG, 10);
        for (int i = 0; i < 50; i++) {
            value64 v = v64GenNext(&gen);
            unsigned long val = value64_ulong(v);
            test_validate(val >= 0 && val <= 10,
                          "ULONG random value out of range: %lu", val);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 4. DBL random in [0, rndinc] (integer values as double) */
    test_sub("subtest %d: UnlimRandom DBL with rndinc=10", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimRnd(VALUE64_DBL, 10);
        for (int i = 0; i < 50; i++) {
            value64 v = v64GenNext(&gen);
            double val = value64_dbl(v);
            test_validate(val >= 0.0 && val <= 10.0,
                          "DBL random value out of range: %f", val);
            test_validate(fabs(val - round(val)) < 1e-9,
                          "DBL random value should be integer, got %f", val);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 5. CHAR random in [0, rndinc] */
    test_sub("subtest %d: UnlimRandom CHAR with rndinc=25", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimRnd(VALUE64_CHR, 25);
        for (int i = 0; i < 50; i++) {
            value64 v = v64GenNext(&gen);
            unsigned char c = (unsigned char)value64_char(v);
            test_validate(c >= 0 && c <= 25,
                          "CHAR random value out of range: %d", c);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 6. BOOL random (rndinc=1) should produce both true and false */
    test_sub("subtest %d: UnlimRandom BOOL with rndinc=1", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimRnd(VALUE64_BOOL, 1);
        bool saw_true = false, saw_false = false;
        for (int i = 0; i < 50; i++) {
            value64 v = v64GenNext(&gen);
            bool b = value64_bool(v);
            if (b) saw_true = true;
            else saw_false = true;
            value64_free(&v, gen.type);
        }
        test_validate(saw_true && saw_false,
                      "BOOL random should produce both true and false");
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 7. STR random with template "val %d" and rndinc=5 */
    test_sub("subtest %d: UnlimRandom STR with template and rndinc=5", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimStrRnd("val %d", 5);
        for (int i = 0; i < 30; i++) {
            value64 v = v64GenNext(&gen);
            const char *str = value64_str(v);
            int num;
            bool ok = (sscanf(str, "val %d", &num) == 1) && num >= 0 && num <= 5;
            test_validate(ok,
                          "STR random pattern mismatch: '%s'", str);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 8. STR random without template (default "%d") and rndinc=7 */
    test_sub("subtest %d: UnlimRandom STR without template, rndinc=7", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimStrRnd(NULL, 7);
        for (int i = 0; i < 30; i++) {
            value64 v = v64GenNext(&gen);
            const char *str = value64_str(v);
            int num;
            bool ok = (sscanf(str, "%d", &num) == 1) && num >= 0 && num <= 7;
            test_validate(ok,
                          "STR random default pattern mismatch: '%s'", str);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 9. FS random with template "item %d" and rndinc=3 */
    test_sub("subtest %d: UnlimRandom FS with template and rndinc=3", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimFsRnd("item %d", 3);
        for (int i = 0; i < 30; i++) {
            value64 v = v64GenNext(&gen);
            const char *str = fs_str(value64_fs(v));
            int num;
            bool ok = (sscanf(str, "item %d", &num) == 1) && num >= 0 && num <= 3;
            test_validate(ok,
                          "FS random pattern mismatch: '%s'", str);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 10. FS random without template (default "%d") and rndinc=4 */
    test_sub("subtest %d: UnlimRandom FS without template, rndinc=4", ++subnum);
    {
        v64Gen gen = v64GenCreatorUnlimFsRnd(NULL, 4);
        for (int i = 0; i < 30; i++) {
            value64 v = v64GenNext(&gen);
            const char *str = fs_str(value64_fs(v));
            int num;
            bool ok = (sscanf(str, "%d", &num) == 1) && num >= 0 && num <= 4;
            test_validate(ok,
                          "FS random default pattern mismatch: '%s'", str);
            value64_free(&v, gen.type);
        }
        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST 10: Source C-string to char generator (LONG lim) -------------------------
static TestStatus
tf10_gen_string_source(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Обычная строка "hello", безлимит (maxlen <= 0) */
    test_sub("subtest %d: v64GenStringToChar basic 'hello' unlimited", ++subnum);
    {
        v64Gen gen = v64GenCreatorSourceCstrChar("hello", 0);

        const char expected[] = "hello";
        for (int i = 0; i < (int)strlen(expected); i++) {
            value64 v = v64GenNext(&gen);
            test_validate(value64_char(v) == expected[i],
                          "pos %d: expected '%c', got '%c'", i, expected[i], value64_char(v));
            value64_free(&v, gen.type);
        }

        value64 v_end = v64GenNext(&gen);
        test_validate(value64_char(v_end) == '\0',
                      "after end expected '\\0', got '%c'", value64_char(v_end));
        value64_free(&v_end, gen.type);

        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 2. Пустая строка */
    test_sub("subtest %d: v64GenStringToChar empty string", ++subnum);
    {
        v64Gen gen = v64GenCreatorSourceCstrChar("", 0);

        value64 v = v64GenNext(&gen);
        test_validate(value64_char(v) == '\0',
                      "empty string must return '\\0', got '%c'", value64_char(v));
        value64_free(&v, gen.type);

        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 3. Ограничение maxlen: строка "hello", maxlen=3 – только "hel" */
    test_sub("subtest %d: v64GenStringToChar with maxlen=3", ++subnum);
    {
        int     cnt = 3;
        v64Gen  gen = v64GenCreatorSourceCstrChar("hello", cnt);

        for (int i = 0; i < cnt; i++) {
            value64 v = v64GenNext(&gen);
            test_validate(value64_char(v) == "hel"[i],
                          "pos %d: expected '%c', got '%c'", i, "hel"[i], value64_char(v));
            value64_free(&v, gen.type);
        }
        for (int i = 0; i < 33; i++)  {
            value64 v = v64GenNext(&gen);
            test_validate(
                value64_char(v) == '\0',
                "after reading all %d iter generator must return only \\0, but got %c", cnt, value64_char(v)
            );
        }

        value64 v_end = v64GenNext(&gen);
        test_validate(value64_char(v_end) == '\0',
                      "after maxlen expected '\\0', got '%c'", value64_char(v_end));
        value64_free(&v_end, gen.type);

        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 4. maxlen больше длины строки: строка "abc", maxlen=10 */
    test_sub("subtest %d: v64GenStringToChar maxlen > strlen", ++subnum);
    {
        v64Gen gen = v64GenCreatorSourceCstrChar("abc", 10);

        for (int i = 0; i < 3; i++) {
            value64 v = v64GenNext(&gen);
            test_validate(value64_char(v) == "abc"[i],
                          "pos %d: expected '%c', got '%c'", i, "abc"[i], value64_char(v));
            value64_free(&v, gen.type);
        }

        value64 v_end = v64GenNext(&gen);
        test_validate(value64_char(v_end) == '\0',
                      "after string expected '\\0', got '%c'", value64_char(v_end));
        value64_free(&v_end, gen.type);

        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    /* 5. NULL источник должен безопасно возвращать '\0' */
    /*test_sub("subtest %d: v64GenStringToChar NULL source", ++subnum);
    {
        v64Gen gen = v64GenCreatorSourceCstrChar(NULL, 0);

        value64 v = v64GenNext(&gen);
        test_validate(value64_char(v) == '\0',
                      "NULL source must return '\\0', got '%c'", value64_char(v));
        value64_free(&v, gen.type);

        v64GenFree(&gen);
        fs_alloc_check(true);
    }*/

    /* 6. Проверка отсутствия утечек (многократные вызовы) */
    test_sub("subtest %d: v64GenStringToChar no leaks (multiple reads)", ++subnum);
    {
        const char  pt[] = "abcdefghij";
        v64Gen gen = v64GenCreatorSourceCstrChar(pt, 0);

        for (int i = 0; i < 20; i++) {
            value64 v = v64GenNext(&gen);
            if (i > (int) strlen(pt))
                test_validatefree(
                    value64_char(v) == '\0',
                    value64_free(&v, gen.type),
                    "%d iter must be \\0, but got %c", i, value64_char(v)
                );
            value64_free(&v, gen.type);
        }

        v64GenFree(&gen);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST 11: Sequential chunks from growing fs -------------------------
static TestStatus
tf11_gen_string_chunks(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: sequential chunks reading from growing fs", ++subnum);
    {
        fs          buf = FS();
        int         total_len = 0;
        const int   iterations = 10;
        char        pt[] = "abcdefghijklmnopqrstuvwxyz";

        srand(42); // детерминированный seed для воспроизводимости

        for (int iter = 0; iter < iterations; iter++) {
            int chunk_len = rndint(sizeof(pt) - 2) + 1;
            
            {
                char c = pt[chunk_len];
                pt[chunk_len] = '\0';
                fs_catstr(&buf, pt);
                pt[chunk_len] = c;  // restore
            }
            // Генератор читает только эту порцию
            v64Gen gen = v64GenCreatorSourceCstrChar(fs_str(&buf) + total_len, chunk_len);

            for (int j = 0; j < chunk_len; j++) {
                value64 v = v64GenNext(&gen);
                test_validate(
                    value64_char(v) == 'a' + j,
                    "iter %d chunk %d: expected '%c', got '%c'",
                    iter, j, 'a' + j, value64_char(v)
                );
            }

            // После прочтения лимита генератор должен вернуть '\0'
            value64 v_end = v64GenNext(&gen);
            test_validate(
                value64_char(v_end) == '\0',
                "iter %d: expected null after chunk, got '%c'",
                iter, value64_char(v_end)
            );
            value64_free(&v_end, VALUE64_CHR);

            total_len += chunk_len;
            v64GenFree(&gen);   // for the next
        }

        // Проверяем итоговую длину
        test_validate(total_len == (int)fs_len(&buf),
                      "total_len %d mismatch fs_len %zu", total_len, fs_len(&buf));

        fsfree(buf);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST 12: Append to source generator -------------------------
static TestStatus
tf12_gen_string_append(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: append chunks to source generator", ++subnum);
    {
        fs          buf = fsinit(10000); // to avoid realloc, realoc isn't suported by v64GenCreatorSourceCstrChar
        int         total_len = 0;
        const int   iterations = 10;
        char        pt[] = "abcdefghijklmnopqrstuvwxyz";

        // Генератор создаём один раз, источник пустой
        v64Gen gen = v64GenCreatorSourceCstrChar(buf.v, 0);

        srand(42);

        for (int iter = 0; iter < iterations; iter++) {
            int chunk_len = rndint(sizeof(pt) - 2) + 1;   // 1..25

            // Добавляем порцию символов в буфер
            {
                char c = pt[chunk_len];
                pt[chunk_len] = '\0';
                fs_catstr(&buf, pt);
                pt[chunk_len] = c;
            }

            // Вычисляем актуальный адрес начала порции ПОСЛЕ fs_catstr
            //const char *chunk_start = fs_str(&buf) + total_len;

            // Перенастраиваем генератор на новую порцию
            v64GenStringAppend(&gen, /*chunk_start, */ chunk_len);

            // Читаем и проверяем символы
            for (int j = 0; j < chunk_len; j++) {
                value64 v = v64GenNext(&gen);
                test_validate(
                    value64_char(v) == 'a' + j,
                    "iter %d pos %d: expected '%c', got '%c'",
                    iter, j, 'a' + j, value64_char(v)
                );
                value64_free(&v, VALUE64_CHR);
            }

            // После порции должен быть '\0'
            value64 v_end = v64GenNext(&gen);
            test_validate(
                value64_char(v_end) == '\0',
                "iter %d: expected null after chunk", iter
            );
            value64_free(&v_end, VALUE64_CHR);

            total_len += chunk_len;
        }

        test_validate(total_len == (int)fs_len(&buf),
                      "total_len %d mismatch fs_len %zu", total_len, fs_len(&buf));

        v64GenFree(&gen);
        fsfree(buf);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST 13: v64GenFSToChar()  simple test -------------------------

static TestStatus
tf13_gen_fs_stream_simple(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: fs stream reads appended chunks automatically", ++subnum);
    {
        fs buf = fsinit(10);          // начальный маленький буфер -> будут realloc'и
        int total_len = 0;
        const int iterations = 10;
        char pt[] = "abcdefghijklmnopqrstuvwxyz";

        v64Gen gen = v64GenCreatorSourceFsToChar(&buf);

        srand(42);

        for (int iter = 0; iter < iterations; iter++) {
            int chunk_len = rndint(sizeof(pt) - 2) + 1;

            // Добавляем порцию
            {
                char c = pt[chunk_len];
                pt[chunk_len] = '\0';
                fs_catstr(&buf, pt);
                pt[chunk_len] = c;
            }

            // Читаем порцию — генератор автоматически видит новые данные
            for (int j = 0; j < chunk_len; j++) {
                value64 v = v64GenNext(&gen);
                test_validate(value64_char(v) == 'a' + j,
                              "iter %d pos %d: expected '%c', got '%c'",
                              iter, j, 'a' + j, value64_char(v));
                value64_free(&v, VALUE64_CHR);
            }

            // После порции должен быть '\0' (генератор ещё не знает о новых данных)
            value64 v_end = v64GenNext(&gen);
            test_validate(value64_char(v_end) == '\0',
                          "iter %d: expected null after chunk", iter);
            value64_free(&v_end, VALUE64_CHR);

            total_len += chunk_len;
        }

        test_validate(total_len == (int) fs_len(&buf),
                      "total_len %d mismatch fs_len %zu", total_len, fs_len(&buf));

        v64GenFree(&gen);
        fsfree(buf);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST 14: v64GenFSToFsByNewline()  simple test-------------------------

static TestStatus
tf14_gen_fs_bynewline(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: lines with empty and waiting", ++subnum);
    {
        // "abc\n\nqwertt" – после первого \n идёт пустая строка, затем qwertt без \n
        fs buf = fscopy("abc\n\nqwertt");
        v64Gen gen = v64GenCreatorSourceFsToFsByNewline(&buf);

        // 1-я строка "abc"
        value64 v1 = v64GenNext(&gen);
        test_validatefree(
            !fs_isnull(value64_fs(v1)) && fs_cmpstr(value64_fs(v1), "abc") == 0,
            value64_free(&v1, VALUE64_FS),
            "line1 mismatch"
        );
        value64_free(&v1, VALUE64_FS);

        // 2-я строка: пустая (из-за \n\n) — должна быть НЕ null и длина 0
        value64 v2 = v64GenNext(&gen);
        test_validatefree(
            !fs_isnull(value64_fs(v2)) && fs_len(value64_fs(v2)) == 0,
            value64_free(&v2, VALUE64_FS),
            "expected empty line (not null)"
        );
        value64_free(&v2, VALUE64_FS);

        // 3-я строка: данных нет — должна быть null fs (ожидание)
        value64 v3 = v64GenNext(&gen);
        test_validatefree(
            fs_isnull(value64_fs(v3)),
            value64_free(&v3, VALUE64_FS),
            "expected null fs (waiting for data)"
        );
        value64_free(&v3, VALUE64_FS);

        // Финализация: остаток "qwertt"
        value64 v_last = v64GenFinalize(&gen);
        test_validatefree(
            !fs_isnull(value64_fs(v_last)) && fs_cmpstr(value64_fs(v_last), "qwertt") == 0,
            value64_free(&v_last, VALUE64_FS),
            "final line mismatch"
        );
        value64_free(&v_last, VALUE64_FS);

        // После финализации — null fs
        value64 v_end = v64GenNext(&gen);
        test_validatefree(
            fs_isnull(value64_fs(v_end)),
            value64_free(&v_end, VALUE64_FS),
            "expected null after finalize"
        );
        value64_free(&v_end, VALUE64_FS);

        v64GenFree(&gen);
        fsfree(buf);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST 15: v64GenGetRemainingCount with fs char generator -------------------------
static TestStatus
tf15_gen_fs_remaining(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: remaining count decreases while reading fs char generator", ++subnum);
    {
        fs buf = fscopy("hello");
        v64Gen gen = v64GenCreatorSourceFsToChar(&buf);

        // В начале оставшихся символов должно быть равно длине строки
        unsigned long rem = v64GenGetRemainingCount(&gen);
        test_validatefree(
            rem == 5, 
            v64GenFree(&gen),
            "expected remaining 5, got %lu", rem
        );

        // Читаем два символа
        value64 v1 = v64GenNext(&gen);
        value64 v2 = v64GenNext(&gen);
        test_validatefree(
            value64_char(v1) == 'h' && value64_char(v2) == 'e',
            v64GenFree(&gen),
            "first two chars mismatch"
        );

        // После двух прочтений остаток должен быть 3
        rem = v64GenGetRemainingCount(&gen);
        test_validatefree(
            rem == 3, 
            v64GenFree(&gen),
            "expected remaining 3 after two reads, got %lu", rem
        );

        value64_free(&v1, VALUE64_CHR);
        value64_free(&v2, VALUE64_CHR);

        // Дочитываем оставшиеся три символа
        for (int i = 0; i < 3; i++) {
            value64 v = v64GenNext(&gen);
            value64_free(&v, VALUE64_CHR);
        }

        // Теперь остаток должен быть 0
        rem = v64GenGetRemainingCount(&gen);
        test_validatefree(
            rem == 0, 
            v64GenFree(&gen),
            "expected remaining 0 after full read, got %lu", rem
        );

        // Дальше генератор должен возвращать '\0' (конец)
        value64 v_end = v64GenNext(&gen);
        test_validatefree(
            value64_char(v_end) == '\0',
            v64GenFree(&gen),
            "expected null character after end"
        );
        value64_free(&v_end, VALUE64_CHR);

        v64GenFree(&gen);
        fsfree(buf);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: remaining count increases after appending data", ++subnum);
    {
        fs buf = fscopy("abc");
        v64Gen gen = v64GenCreatorSourceFsToChar(&buf);

        unsigned long rem = v64GenGetRemainingCount(&gen);
        test_validatefree(
            rem == fslen(buf), 
            v64GenFree(&gen),
            "after full read remaining should be %zu, got %lu", fslen(buf), rem
        );

        // Читаем все три символа
        for (int i = 0; i < 3; i++) {
            value64 v = v64GenNext(&gen);
            value64_free(&v, VALUE64_CHR);
        }

        rem = v64GenGetRemainingCount(&gen);
        test_validate(rem == 0, "after full read remaining should be 0, got %lu", rem);

        // Добавляем новые данные в буфер
        fs_catstr(&buf, "def");

        // Генератор должен увидеть новые данные, позиция осталась на 3
        rem = v64GenGetRemainingCount(&gen);
        test_validatefree(
            rem == 3, 
            v64GenFree(&gen),
            "after appending remaining should be 3, got %lu", rem
        );

        // Читаем новые символы
        value64 v1 = v64GenNext(&gen);
        test_validatefree(
            value64_char(v1) == 'd', 
            v64GenFree(&gen),
            "expected 'd', got '%c'", value64_char(v1)
        );
        value64_free(&v1, VALUE64_CHR);

        rem = v64GenGetRemainingCount(&gen);
        test_validatefree(
            rem == 2, 
            v64GenFree(&gen),
            "after reading one new char remaining should be 2, got %lu", rem
        );

        // Дочитываем остальные
        for (int i = 0; i < 2; i++) {
            value64 v = v64GenNext(&gen);
            value64_free(&v, VALUE64_CHR);
        }

        rem = v64GenGetRemainingCount(&gen);
        test_validatefree(
            rem == 0, 
            v64GenFree(&gen),
            "after reading all new chars remaining should be 0, got %lu", rem
        );

        v64GenFree(&gen);
        fsfree(buf);
        fs_alloc_check(true);
    }

    return TEST_PASSED;
}

// ------------------------- TEST 15: FsToFsByNewline remaining count -------------------------
static TestStatus
tf16_gen_fs_bynewline_remaining(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: remaining count for FsToFsByNewline", ++subnum);
    {
        const char *data = "abc\n\nqwertt";
        fs buf = fscopy(data);
        v64Gen gen = v64GenCreatorSourceFsToFsByNewline(&buf);

        /* Начальный остаток равен длине буфера (11 символов) */
        unsigned long rem = v64GenGetRemaining(&gen);
        test_validatefree(
            rem == 11, 
            (v64GenFree(&gen), fsfree(buf)),
            "initial remaining expected 11, got %lu", rem
        );

        /* Читаем первую строку "abc" */
        value64 v1 = v64GenNext(&gen);
        test_validatefree(
            !fs_isnull(value64_fs(v1)) && fs_cmpstr(value64_fs(v1), "abc") == 0,
            (value64_free(&v1, VALUE64_FS), v64GenFree(&gen), fsfree(buf)),
            "first line mismatch"
        );
        value64_free(&v1, VALUE64_FS);

        rem = v64GenGetRemaining(&gen);
        test_validatefree(
            rem == 7,
            (v64GenFree(&gen), fsfree(buf)),
            "after first line remaining expected 7, got %lu", rem
        );

        /* Читаем вторую строку (пустая из-за \n\n) */
        value64 v2 = v64GenNext(&gen);
        test_validatefree(
            !fs_isnull(value64_fs(v2)) && fs_len(value64_fs(v2)) == 0,
            (value64_free(&v2, VALUE64_FS), v64GenFree(&gen), fsfree(buf)),
            "second line should be empty"
        );
        value64_free(&v2, VALUE64_FS);

        rem = v64GenGetRemaining(&gen);
        test_validatefree(
            rem == 6,
            (v64GenFree(&gen), fsfree(buf)),
            "after second line remaining expected 6, got %lu", rem
        );

        /* Третий вызов: данных нет, возвращается null fs, позиция не меняется */
        value64 v3 = v64GenNext(&gen);
        test_validatefree(
            fs_isnull(value64_fs(v3)),
            (value64_free(&v3, VALUE64_FS), v64GenFree(&gen), fsfree(buf)),
            "third call should return null (waiting)"
        );
        value64_free(&v3, VALUE64_FS);   /* при успешном условии освобождаем вручную */

        rem = v64GenGetRemaining(&gen);
        test_validatefree(
            rem == 6,
            (v64GenFree(&gen), fsfree(buf)),
            "after waiting remaining should still be 6, got %lu", rem
        );

        /* Финализация: возвращает остаток без \n */
        value64 v_last = v64GenFinalize(&gen);
        test_validatefree(
            !fs_isnull(value64_fs(v_last)) && fs_cmpstr(value64_fs(v_last), "qwertt") == 0,
            (value64_free(&v_last, VALUE64_FS), v64GenFree(&gen), fsfree(buf)),
            "final line mismatch"
        );
        value64_free(&v_last, VALUE64_FS);

        rem = v64GenGetRemaining(&gen);
        test_validatefree(
            rem == 0,
            (v64GenFree(&gen), fsfree(buf)),
            "after finalize remaining expected 0, got %lu", rem
        );

        v64GenFree(&gen);
        fsfree(buf);
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
      , TESTADD(tf4_gen_creators,            "v64gen creators (simple wrappers) simple test")
      , TESTADD(tf5_gen_asc_rnd,             "v64UncheckGenUnlimAscRnd() simple test")
      , TESTADD(tf6_gen_asc_rnd_custom,      "AscRnd with custom rndinc simple test")
      , TESTADD(tf7_gen_desc_series,         "v64UncheckGenUnlimDescSeries() simple test")
      , TESTADD(tf8_gen_desc_rnd_custom,     "DescRnd with custom rndinc simple test")
      , TESTADD(tf9_gen_unlim_random,        "v64GenCreatorUnlimRnd...() generators simple test")
      , TESTADD(tf10_gen_string_source,      "Source C-string to char generator (LONG lim)")
      , TESTADD(tf11_gen_string_chunks,      "Sequential chunks from growing fs test")
      , TESTADD(tf12_gen_string_append,      "Append to source generator test")
      , TESTADD(tf13_gen_fs_stream_simple,   "v64GenFSToChar()  simple test")
      , TESTADD(tf14_gen_fs_bynewline,       "v64GenFSToFsByNewline()  simple test")
      , TESTADD(tf15_gen_fs_remaining,       "v64GenGetRemainingCount with fs char generator")
      , TESTADD(tf16_gen_fs_bynewline_remaining, "v64GenGetRemainingCount with FsToFsByNewlin")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* V64GEN_TESTING */

