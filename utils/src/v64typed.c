/********************************************************************
                    VALUE64 TYPED MODULE IMPLEMENTATION
********************************************************************/

#include "v64typed.h"

// ---------------------------------- CONSTANTS AND GLOBALS --------------------------

// ---------------------------- CONSTRUCTORS / DESTRUCTORS ---------------------------

// ------------------------ PRINTERS/CHECKERS ---------------------------------------

int                      v64typedTechfprint(FILE *restrict out, v64typed tval, const char *restrict name) {
    // now it's just a wrapper, but in general logic must be here
    return value64_techfprint(out, tval.val, tval.typ, name);
}

// ------------------------------------ ETC. ----------------------------------------

bool                     v64typedValidate(FILE *out, v64typed tval) {
    if (!value64_checktype(tval.typ)) {
        if (out)
            fprintf(out, "Type validation valied %d %s", tval.typ, value64_typename(tval.typ) );
        return logsimpleret(false, "Type validation valied %d %s", tval.typ, value64_typename(tval.typ) );
    }
    if (!value64_validate(out, tval.val, tval.typ) ) {
        if (out)
            fprintf(out, "V64 data is incorrect\n");
        return logsimpleret(false, "V64 data is incorrect");
    }
    return true;
}

// ---------------------------------------- Testing ------------------------------------------
#ifdef V64TYPED_TESTING

#include "test.h"

//types for testing

// ------------------------- TEST v64typed: init/free/getters (basic) -------------------------
static TestStatus
tf1_v64typed_init_free_getters(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. INT */
    test_sub("subtest %d: v64typed INT constructor and getter", ++subnum);
    {
        v64typed tv = v64typedInt(42);
        test_validate(tv.typ == VALUE64_INT, "type must be INT");
        test_validate(value64_int(tv.val) == 42, "value must be 42");
        test_validate(v64typeGetInt(tv) == 42, "getter returned wrong int");
        test_validatefree(
            v64typedValidate(stderr, tv), 
            v64typedFree(&tv),
            "Validation failed"
        );
        v64typedFree(&tv);
    }

    /* 2. LONG */
    test_sub("subtest %d: v64typed LONG constructor and getter", ++subnum);
    {
        v64typed tv = v64typedLong(123456789L);
        test_validate(tv.typ == VALUE64_LONG, "type must be LONG");
        test_validate(value64_long(tv.val) == 123456789L, "value must be 123456789");
        test_validate(v64typeGetLong(tv) == 123456789L, "getter returned wrong long");
        test_validatefree(
            v64typedValidate(stderr, tv), 
            v64typedFree(&tv),
            "Validation failed"
        );
        v64typedFree(&tv);
    }

    /* 3. ULONG */
    test_sub("subtest %d: v64typed ULONG constructor and getter", ++subnum);
    {
        v64typed tv = v64typedULong(4000000000UL);
        test_validate(tv.typ == VALUE64_ULONG, "type must be ULONG");
        test_validate(value64_ulong(tv.val) == 4000000000UL, "value must be 4000000000");
        test_validate(v64typeGetULong(tv) == 4000000000UL, "getter returned wrong ulong");
        test_validatefree(
            v64typedValidate(stderr, tv), 
            v64typedFree(&tv),
            "Validation failed"
        );
        v64typedFree(&tv);
    }

    /* 4. DBL */
    test_sub("subtest %d: v64typed DBL constructor and getter", ++subnum);
    {
        v64typed tv = v64typedDbl(3.14159);
        test_validate(tv.typ == VALUE64_DBL, "type must be DBL");
        test_validate(fabs(value64_dbl(tv.val) - 3.14159) < 1e-9, "value mismatch");
        test_validate(fabs(v64typeGetDouble(tv) - 3.14159) < 1e-9, "getter mismatch");
        test_validatefree(
            v64typedValidate(stderr, tv), 
            v64typedFree(&tv),
            "Validation failed"
        );
        v64typedFree(&tv);
    }

    /* 5. CHAR (теперь через v64typedChar) */
    test_sub("subtest %d: v64typed CHAR constructor and getter", ++subnum);
    {
        v64typed tv = v64typedChar('Z');
        test_validate(tv.typ == VALUE64_CHR, "type must be CHR");
        test_validate(value64_char(tv.val) == 'Z', "value must be 'Z'");
        test_validate(v64typeGetChar(tv) == 'Z', "getter returned wrong char");
        test_validatefree(
            v64typedValidate(stderr, tv), 
            v64typedFree(&tv),
            "Validation failed"
        );
        v64typedFree(&tv);
    }

    /* 6. BOOL (теперь через v64typedBool) */
    test_sub("subtest %d: v64typed BOOL constructor and getter", ++subnum);
    {
        v64typed tv = v64typedBool(true);
        test_validate(tv.typ == VALUE64_BOOL, "type must be BOOL");
        test_validate(value64_bool(tv.val) == true, "value must be true");
        test_validate(v64typeGetBool(tv) == true, "getter returned wrong bool");
        test_validatefree(
            v64typedValidate(stderr, tv), 
            v64typedFree(&tv),
            "Validation failed"
        );
        v64typedFree(&tv);
    }

    /* 7. STR (owning copy) */
    test_sub("subtest %d: v64typed STR constructor and getter", ++subnum);
    {
        v64typed tv = v64typedStr("hello");
        test_validatefree(
            tv.typ == VALUE64_STR, 
            v64typedFree(&tv),
            "type must be STR"
        );
        test_validatefree(
            strcmp(value64_str(tv.val), "hello") == 0, 
            v64typedFree(&tv),
            "value mismatch"
        );
        test_validatefree(
            strcmp(v64typeGetStr(tv), "hello") == 0, 
            v64typedFree(&tv),
            "getter mismatch"
        );
        test_validatefree(
            v64typedValidate(stderr, tv), 
            v64typedFree(&tv),
            "Validation failed"
        );

        v64typedFree(&tv);
        fs_alloc_check(true);
    }

    /* 8. FS (owning copy) */
    test_sub("subtest %d: v64typed FS constructor and getter", ++subnum);
    {
        fs f = fscopy("world");
        v64typed tv = v64typedFs(&f);
        fsfree(f);   // исходный fs больше не нужен, tv владеет копией

        test_validatefree(
            tv.typ == VALUE64_FS,
            v64typedFree(&tv), 
            "type must be FS"
        );
        test_validate(strcmp(fs_str(value64_fs(tv.val)), "world") == 0, "value mismatch");
        test_validate(strcmp(fs_str(v64typeGetFs(tv)), "world") == 0, "getter mismatch");
        test_validatefree(
            v64typedValidate(stderr, tv), 
            v64typedFree(&tv),
            "Validation failed"
        );

        v64typedFree(&tv);
        fs_alloc_check(true);
    }

    /* 8. FS (v64typedFsAsStr) */
    test_sub("subtest %d: v64typed FS constructor and getter", ++subnum);
    {
        v64typed tv = v64typedFsAsStr("world");

        test_validatefree(
            tv.typ == VALUE64_FS, 
            v64typedFree(&tv),
            "type must be FS"
        );
        test_validate(strcmp(fs_str(value64_fs(tv.val)), "world") == 0, "value mismatch");
        test_validate(strcmp(fs_str(v64typeGetFs(tv)), "world") == 0, "getter mismatch");
        test_validatefree(
            v64typedValidate(stderr, tv), 
            v64typedFree(&tv),
            "Validation failed"
        );

        v64typedFree(&tv);
        fs_alloc_check(true);
    }

    /* 9. UNKNOWN */
    test_sub("subtest %d: v64typed UNKNOWN constructor", ++subnum);
    {
        v64typed tv = v64typedUnk();
        // v64typedValidate NOT passed
        test_validate(tv.typ == VALUE64_UNKNOWN, "type must be UNKNOWN");
        test_validate(tv.val.u64 == 0, "raw value must be zero");
        v64typedFree(&tv);   // безопасно, ничего не делает для UNKNOWN
    }

    return TEST_PASSED;
}

// ------------------------- TEST v64typed: nvl functions (Str/Fs) -------------------------
static TestStatus
tf2_v64typed_nvl(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ================= v64typedNvlStr ================= */
    test_sub("subtest %d: v64typedNvlStr non-empty STR returns value", ++subnum);
    {
        v64typed tv = v64typedStr("hello");
        const char *res = v64typedNvlStr(tv, "default");
        test_validate(strcmp(res, "hello") == 0,
                      "expected 'hello', got '%s'", res);
        v64typedFree(&tv);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: v64typedNvlStr empty STR returns default", ++subnum);
    {
        v64typed tv = v64typedStr("");
        const char *res = v64typedNvlStr(tv, "default");
        test_validate(strcmp(res, "default") == 0,
                      "expected default, got '%s'", res);
        v64typedFree(&tv);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: v64typedNvlStr NULL STR returns default", ++subnum);
    {
        v64typed tv = v64typedCommon(LITERAL64_STR(NULL), VALUE64_STR);
        const char *res = v64typedNvlStr(tv, "default");
        test_validate(strcmp(res, "default") == 0,
                      "expected default, got '%s'", res);
        v64typedFree(&tv);
    }

    test_sub("subtest %d: v64typedNvlStr non-STR type returns default", ++subnum);
    {
        v64typed tv = v64typedInt(42);
        const char *res = v64typedNvlStr(tv, "default");
        test_validate(strcmp(res, "default") == 0,
                      "expected default, got '%s'", res);
        v64typedFree(&tv);
    }

    /* ================= v64typedNvlFs ================= */
    test_sub("subtest %d: v64typedNvlFs non-empty FS returns value", ++subnum);
    {
        v64typed tv = v64typedFsAsStr("world");

        const char *res = v64typedNvlFs(tv, "default");
        test_validate(strcmp(res, "world") == 0,
                      "expected 'world', got '%s'", res);
        v64typedFree(&tv);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: v64typedNvlFs empty FS returns default", ++subnum);
    {
        v64typed tv = v64typedFsAsStr("");

        const char *res = v64typedNvlFs(tv, "default");
        // Теперь fs_isempty считает пустую строку пустой
        test_validate(res != NULL && strcmp(res, "default") == 0,
                      "expected default, got '%s'", res ? res : "NULL");
        v64typedFree(&tv);
        fs_alloc_check(true);
    }

    test_sub("subtest %d: v64typedNvlFs NULL FS returns default", ++subnum);
    {
        v64typed tv = v64typedCommon(LITERAL64_PFS(NULL), VALUE64_FS);
        const char *res = v64typedNvlFs(tv, "default");
        test_validate(strcmp(res, "default") == 0,
                      "expected default, got '%s'", res);
        v64typedFree(&tv);
    }

    test_sub("subtest %d: v64typedNvlFs non-FS type returns default", ++subnum);
    {
        v64typed tv = v64typedInt(42);
        const char *res = v64typedNvlFs(tv, "default");
        test_validate(strcmp(res, "default") == 0,
                      "expected default, got '%s'", res);
        v64typedFree(&tv);
    }

    return TEST_PASSED;
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main(/* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        TESTADD(tf1_v64typed_init_free_getters,           "Simple init/free/getters")
      , TESTADD(tf2_v64typed_nvl,                         "v64typed: nvl functions (Str/Fs)")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* V64TYPED_TESTING */


