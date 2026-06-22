/********************************************************************
                    VALUE64(128) SET MODULE IMPLEMENTATION
********************************************************************/

// common include
#include "value64.h"

static const size_t                     value_sizes[] = {
    [VALUE64_INT]  = sizeof(int),
    [VALUE64_LNG]  = sizeof(long),
    [VALUE64_DBL]  = sizeof(double),
    [VALUE64_PTR]  = sizeof(void *),
    [VALUE64_STR]  = sizeof(char *),
    [VALUE64_FS]   = sizeof(fs *)
};


// create value from pointer, value64 constructor ANY type, MOVE semantic
value64                   value64_pcopy_move(void *p, value64_type typ, bool move){
    value64     tmp = VALUE64_ZERO;  // init
    switch (typ){
        case VALUE64_INT:
            tmp.ival = *(const int *) p;
        break;
        case VALUE64_LNG:
            tmp.lval = *(const long *) p;
        break;
        case VALUE64_DBL:
            tmp.dval = *(const double *) p;
        break;
        case VALUE64_PTR:
            tmp.pval = *(void * const *) p;
        break;
        case VALUE64_STR:
            if (move)
                tmp.sval = (char *) p;  //MOVE POINTER
            else
                tmp = value64_createstr(p);
        break;
        // create fs body in head with FS_FLAG_BODYALLOC
        case VALUE64_FS:
            if (move)
                tmp = value64_movefs(p);
            else
                tmp = value64_createfs(p);
        break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "type %d %s isn't suppoted", typ, value64_type_name(typ) );
        break;
    }
    return tmp;
}

// --------------------------------- SERIALIZATION -----------------------------------------

// ---------------------------------------- Testing ------------------------------------------
#ifdef VALUE64TESTING

#include "test.h"
#include "array.h"
#include <time.h>

//types for testing

// ------------------------- TEST init_free ---------------------------------

static TestStatus
tf_init_free(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. int */
    test_sub("subtest %d: value64 int", ++subnum);
    {
        value64 v = value64_createint(42);
        test_validatefree(
            v.ival == 42,
            value64_free(v, VALUE64_INT),
            "Int value mismatch: got %d, expected 42", v.ival
        );
        // для int освобождение не требуется
        value64_free(v, VALUE64_INT);
    }

    /* 2. long */
    test_sub("subtest %d: value64 long", ++subnum);
    {
        value64 v = value64_createlong(1234567890L);
        test_validate(
            v.lval == 1234567890L,
            "Long value mismatch: got %ld, expected 1234567890", v.lval
        );
    }

    /* 3. double */
    test_sub("subtest %d: value64 double", ++subnum);
    {
        value64 v = value64_createdbl(3.14159265);
        test_validate(
            fabs(v.dval - 3.14159265) < 0.00000001,
            "Double value mismatch: got %f, expected 3.14159265", v.dval
        );
    }

    /* 4. pointer */
    test_sub("subtest %d: value64 pointer", ++subnum);
    {
        int x = 77;
        value64 v = value64_createptr(&x);
        test_validate(
            v.pval == &x,
            "Pointer mismatch: got %p, expected %p", v.pval, (void*)&x
        );
    }

    /* 5. fs (copy) */
    test_sub("subtest %d: value64 createfs (copy)", ++subnum);
    {
        const char *text = "hello value64";
        fs orig = fscopy(text);
        value64 v = value64_createfs(&orig);

        test_validatefree(
            strcmp(fs_str(v.fsval), text) == 0,
            (fsfree(orig), fs_free(v.fsval)),
            "FS copy mismatch: got '%s', expected '%s'", fs_str(v.fsval), text
        );
        test_validatefree(
            fs_bodyalloc(v.fsval),
            (fsfree(orig), fs_free(v.fsval)),
            "Copied fs must have FS_FLAG_BODYALLOC flag"
        );

        fsfree(orig);
        value64_freefs(v);
        fs_alloc_check(true);
    }

    /* 6. fs (move) */
    test_sub("subtest %d: value64 movefs", ++subnum);
    {
        const char *text = "move me";
        fs orig = fscopy(text);
        value64 v = value64_movefs(&orig);   // orig будет опустошён

        test_validatefree(
            strcmp(fs_str(v.fsval), text) == 0,
            value64_freefs(v),
            "Moved fs value mismatch: got '%s', expected '%s'", fs_str(v.fsval), text
        );
        test_validatefree(
            fs_bodyalloc(v.fsval),
            value64_freefs(v),
            "Moved fs must have FS_FLAG_BODYALLOC flag"
        );
        // Проверяем, что оригинал действительно опустошён
        test_validatefree(
            fslen(orig) == 0 && fsstr(orig) == NULL,
            value64_freefs(v),
            "After move, original fs must be empty (len=%d, str=%p)", fslen(orig), (void*)fsstr(orig)
        );

        value64_freefs(v);
        fsfree(orig);   // orig пуст, но fsfree безопасен
        fs_alloc_check(true);
    }

    /* 7. Множественные вызовы и проверка утечек */
    test_sub("subtest %d: value64 multiple create/free (leak check)", ++subnum);
    {
        const char *words[] = {"one", "two", "three"};
        value64 vals[COUNT(words)];

        for (int i = 0; i < COUNT(words); i++) {
            vals[i] = value64_createfs(
                &(fs){ .v = (char*)words[i], .len = strlen(words[i]), .sz = 0, .flags = FS_FLAG_STATIC }
            );   // временный fs для создания копии
        }

        for (int i = 0; i < COUNT(words); i++) {
            test_validatefree(
                strcmp(fs_str(vals[i].fsval), words[i]) == 0,
                (value64_freefs(vals[0]), value64_freefs(vals[1]), value64_freefs(vals[2])),
                "FS %d mismatch: got '%s', expected '%s'", i, fs_str(vals[i].fsval), words[i]
            );
        }

        for (int i = 0; i < COUNT(words); i++)
            value64_freefs(vals[i]);
        fs_alloc_check(true);
    }
    test_sub("subtest %d: value64 str", ++subnum);
    {
        const char *text = "hello c-string";
        value64 v = value64_createstr(text);

        test_validatefree(
            strcmp(v.sval, text) == 0,
            value64_freestr(v),
            "Str copy mismatch: got '%s', expected '%s'", v.sval, text
        );

        free(v.sval);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_pcopy_move ---------------------------------
static TestStatus
tf_point_init(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ---------- value64_pinit (copy) ---------- */

    /* 1. copy int */
    test_sub("subtest %d: pinit int", ++subnum);
    {
        int ival = 123;
        value64 v = value64_pinit(&ival, VALUE64_INT);
        test_validate(v.ival == 123, "Copy int: got %d, expected 123", v.ival);
    }

    /* 2. copy long */
    test_sub("subtest %d: pinit long", ++subnum);
    {
        long lval = 999999999L;
        value64 v = value64_pinit(&lval, VALUE64_LNG);
        test_validate(v.lval == 999999999L, "Copy long: got %ld, expected 999999999", v.lval);
    }

    /* 3. copy double */
    test_sub("subtest %d: pinit double", ++subnum);
    {
        double dval = 2.7182818;
        value64 v = value64_pinit(&dval, VALUE64_DBL);
        test_validate(fabs(v.dval - 2.7182818) < 0.0000001,
                      "Copy double: got %f, expected 2.7182818", v.dval);
    }

    /* 4. copy pointer */
    test_sub("subtest %d: pinit pointer", ++subnum);
    {
        int x = 5;
        void *ptr = &x;
        value64 v = value64_pinit(&ptr, VALUE64_PTR);
        test_validate(v.pval == ptr,
                      "Copy pointer: got %p, expected %p", v.pval, ptr);
    }

    /* 5. copy C-string */
    test_sub("subtest %d: pinit str", ++subnum);
    {
        const char *text = "copy-me";
        value64 v = value64_pinit(text, VALUE64_STR);
        test_validatefree(
            strcmp(v.sval, text) == 0,
            value64_freestr(v),
            "Copy str: got '%s', expected '%s'", v.sval, text
        );
        test_validatefree(
            v.sval != text,
            value64_freestr(v),
            "Copy str must have different address from original"
        );
        value64_freestr(v);
    }

    /* 6. copy fs */
    test_sub("subtest %d: pinit fs", ++subnum);
    {
        const char *text = "fs-copy";
        fs orig = fscopy(text);
        value64 v = value64_pinit(&orig, VALUE64_FS);

        test_validatefree(
            strcmp(fs_str(v.fsval), text) == 0,
            (fsfree(orig), value64_freefs(v)),
            "Copy fs: got '%s', expected '%s'", fs_str(v.fsval), text
        );
        test_validatefree(
            fs_bodyalloc(v.fsval),
            (fsfree(orig), value64_freefs(v)),
            "Copy fs must have FS_FLAG_BODYALLOC"
        );

        fsfree(orig);
        value64_freefs(v);
        fs_alloc_check(true);
    }

    /* ---------- value64_pmove (move) ---------- */

    /* 7. move int (семантика копирования, т.к. скаляр) */
    test_sub("subtest %d: pmove int", ++subnum);
    {
        int ival = -5;
        value64 v = value64_pmove(&ival, VALUE64_INT);
        test_validate(v.ival == -5, "Move int: got %d, expected -5", v.ival);
    }

    /* 8. move long */
    test_sub("subtest %d: pmove long", ++subnum);
    {
        long lval = -999999999L;
        value64 v = value64_pmove(&lval, VALUE64_LNG);
        test_validate(v.lval == -999999999L, "Move long: got %ld, expected -999999999", v.lval);
    }

    /* 9. move double */
    test_sub("subtest %d: pmove double", ++subnum);
    {
        double dval = -1.4142135;
        value64 v = value64_pmove(&dval, VALUE64_DBL);
        test_validate(fabs(v.dval - (-1.4142135)) < 0.0000001,
                      "Move double: got %f, expected -1.4142135", v.dval);
    }

    /* 10. move pointer */
    test_sub("subtest %d: pmove pointer", ++subnum);
    {
        int x = 99;
        void *ptr = &x;
        value64 v = value64_pmove(&ptr, VALUE64_PTR);
        test_validate(v.pval == &x,
                      "Move pointer: got %p, expected %p", v.pval, (void*)&x);
    }

    /* 11. move C-string (забирает владение) */
    test_sub("subtest %d: pmove str", ++subnum);
    {
        char *text = strdup("move-str");
        value64 v = value64_pmove(text, VALUE64_STR);
        test_validatefree(
            strcmp(v.sval, "move-str") == 0,
            free(v.sval),
            "Move str: got '%s', expected 'move-str'", v.sval
        );
        // text больше не владеет памятью, его нельзя освобождать
        free(v.sval);
    }

    /* 12. move fs (оригинал опустошается) */
    test_sub("subtest %d: pmove fs", ++subnum);
    {
        const char *text = "fs-move";
        fs orig = fscopy(text);
        value64 v = value64_pmove(&orig, VALUE64_FS);

        test_validatefree(
            strcmp(fs_str(v.fsval), text) == 0,
            fs_free(v.fsval),
            "Move fs: got '%s', expected '%s'", fs_str(v.fsval), text
        );
        test_validatefree(
            fs_bodyalloc(v.fsval),
            fs_free(v.fsval),
            "Move fs must have FS_FLAG_BODYALLOC"
        );
        test_validatefree(
            fslen(orig) == 0 && fsstr(orig) == NULL,
            fs_free(v.fsval),
            "After move, original fs must be empty (len=%d, str=%p)", fslen(orig), (void*)fsstr(orig)
        );

        fs_free(v.fsval);
        fsfree(orig);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main(int argc, const char *argv[])
{

 logsimpleinit("Start");
    bool    runall = argc == 1;
    printf("%d\n", argc);

    while (runall || *++argv){
        int     num = INT_MAX;    // INT_MAX for all test
        if (!runall){
            num = atoi(*argv);
            if (num < 0){
                fprintf(stderr,"Invalid test num %d\n", num);
                continue;
            }
        }
        printf("Num %d\n", num);
            testenginestd_run(num,
                testnew(.f2 = tf_init_free,        .num =  1, .name = "Simple init and validate test"              , .desc="", .mandatory=true)
              , testnew(.f2 = tf_point_init,       .num =  2, .name = "Simple value64_pcopy_move() test"           , .desc="", .mandatory=true)
            );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HSETTESTING */


