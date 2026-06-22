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

// ------------------------- TEST 1 ---------------------------------

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
            );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HSETTESTING */


