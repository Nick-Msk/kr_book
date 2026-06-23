/********************************************************************
                    VALUE64(128) SET MODULE IMPLEMENTATION
********************************************************************/

// common include
#include "value64.h"

// -------------------------- TYPE SUPPORT API ------------------------

// Вся информация о типах в одном месте!
static const value64_typeinfo           value64_info[] = {
    [VALUE64_UKNOWN]     = {"INVALID",     0,              false},
    [VALUE64_INT]        = {"INT",         sizeof(int),    true},
    [VALUE64_LNG]        = {"LNG",         sizeof(long),   true},
    [VALUE64_DBL]        = {"DBL",         sizeof(double), true},
    [VALUE64_FS]         = {"FS",          sizeof(fs *),   true},
    [VALUE64_PTR]        = {"PTR",         sizeof(void *), true},
    [VALUE64_STR]        = {"STR",         sizeof(char *), true},
    [VALUE64_TYPE_COUNT] = {"",            0,              false}
};

_Static_assert(COUNT(value64_info) == VALUE64_TYPE_COUNT + 1,
               "Размер массива value65_info не совпадает с количеством типов!");

const                   value64_typeinfo* value64_info_get(value64_type typ) {
    // Проверка границ массива
    if (typ < 0 || typ >= COUNT(value64_info) || !value64_info[typ].is_valid)
        return NULL;
    return &value64_info[typ];
}

// the part of mass creation API, probably'll be changed
// create value from pointer, value64 constructor ANY type, MOVE semantic
value64                   value64_pcopy_move(void *p, value64_type typ, bool move){
    invraisecode(p != NULL, ERR_NULLABLE_PTR, "Null pointer");
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
            if (move)
                userraiseint(ERR_UNSUPPORTED_TYPE, "VALUE64_PTR can't be moved");
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
        test_validatefree(
            value64_int(v) == 42,
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
        test_validate(
            value64_long(v) == 1234567890L,
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
        test_validate(
            fabs(value64_dbl(v) - 3.14159265) < 0.00000001,
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
        test_validate(
            value64_ptr(v) == &x,
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
            strcmp(fs_str(value64_fs(v) ), text) == 0,
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
            strcmp(fs_str(value64_fs(v) ), text) == 0,
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
        test_validatefree(
            strcmp(value64_str(v), text) == 0,
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

    /* 10. move pointer  DISABLED
    test_sub("subtest %d: pmove pointer", ++subnum);
    {
        int x = 99;
        void *ptr = &x;
        value64 v = value64_pmove(&ptr, VALUE64_PTR);
        test_validate(v.pval == &x,
                      "Move pointer: got %p, expected %p", v.pval, (void*)&x);
    } */

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

// ------------------------- TEST value64_clone ---------------------------------

static TestStatus
tf_clone(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ---------- value64_create* (конструкторы) ---------- */

    /* 1. int */
    test_sub("subtest %d: create int", ++subnum);
    {
        value64 v = value64_createint(42);
        test_validate(v.ival == 42, "Create int: got %d, expected 42", v.ival);
    }

    /* 2. long */
    test_sub("subtest %d: create long", ++subnum);
    {
        value64 v = value64_createlong(1234567890L);
        test_validate(v.lval == 1234567890L, "Create long: got %ld, expected 1234567890", v.lval);
    }

    /* 3. double */
    test_sub("subtest %d: create double", ++subnum);
    {
        value64 v = value64_createdbl(2.718281828);
        test_validate(fabs(v.dval - 2.718281828) < 0.000000001,
                      "Create double: got %f, expected 2.718281828", v.dval);
    }

    /* 4. pointer */
    test_sub("subtest %d: create pointer", ++subnum);
    {
        int x = 77;
        value64 v = value64_createptr(&x);
        test_validate(v.pval == &x,
                      "Create pointer: got %p, expected %p", v.pval, (void*)&x);
    }

    /* 5. C-string (копирование) */
    test_sub("subtest %d: create str", ++subnum);
    {
        const char *text = "hello value64";
        value64 v = value64_createstr(text);

        test_validatefree(
            strcmp(v.sval, text) == 0,
            value64_free(v, VALUE64_STR),
            "Create str: got '%s', expected '%s'", v.sval, text
        );
        test_validatefree(
            v.sval != text,
            value64_free(v, VALUE64_STR),
            "Create str must have its own memory"
        );
        value64_free(v, VALUE64_STR);
    }

    /* 6. fs (копирование) */
    test_sub("subtest %d: create fs", ++subnum);
    {
        const char *text = "hello fs";
        fs orig = fscopy(text);
        value64 v = value64_createfs(&orig);

        test_validatefree(
            strcmp(fs_str(v.fsval), text) == 0,
            (fsfree(orig), value64_free(v, VALUE64_FS)),
            "Create fs: got '%s', expected '%s'", fs_str(v.fsval), text
        );
        test_validatefree(
            fs_bodyalloc(v.fsval),
            (fsfree(orig), value64_free(v, VALUE64_FS)),
            "Create fs must have FS_FLAG_BODYALLOC"
        );

        fsfree(orig);
        value64_free(v, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* ---------- value64_clone ---------- */

    /* 7. clone int */
    test_sub("subtest %d: clone int", ++subnum);
    {
        value64 orig = value64_createint(100);
        value64 copy = value64_clone(orig, VALUE64_INT);
        test_validate(copy.ival == 100, "Clone int: got %d, expected 100", copy.ival);
    }

    /* 8. clone long */
    test_sub("subtest %d: clone long", ++subnum);
    {
        value64 orig = value64_createlong(999999999L);
        value64 copy = value64_clone(orig, VALUE64_LNG);
        test_validate(copy.lval == 999999999L, "Clone long: got %ld, expected 999999999", copy.lval);
    }

    /* 9. clone double */
    test_sub("subtest %d: clone double", ++subnum);
    {
        value64 orig = value64_createdbl(1.6180339);
        value64 copy = value64_clone(orig, VALUE64_DBL);
        test_validate(fabs(copy.dval - 1.6180339) < 0.0000001,
                      "Clone double: got %f, expected 1.6180339", copy.dval);
    }

    /* 10. clone pointer */
    test_sub("subtest %d: clone pointer", ++subnum);
    {
        int x = 123;
        value64 orig = value64_createptr(&x);
        value64 copy = value64_clone(orig, VALUE64_PTR);
        test_validate(copy.pval == &x,
                      "Clone pointer: got %p, expected %p", copy.pval, (void*)&x);
    }

    /* 11. clone C-string */
    test_sub("subtest %d: clone str", ++subnum);
    {
        const char *text = "clone-string";
        value64 orig = value64_createstr(text);
        value64 copy = value64_clone(orig, VALUE64_STR);

        test_validatefree(
            strcmp(copy.sval, text) == 0,
            (value64_free(orig, VALUE64_STR), value64_free(copy, VALUE64_STR)),
            "Clone str: got '%s', expected '%s'", copy.sval, text
        );
        test_validatefree(
            copy.sval != orig.sval,
            (value64_free(orig, VALUE64_STR), value64_free(copy, VALUE64_STR)),
            "Clone str must have different address"
        );

        value64_free(orig, VALUE64_STR);
        value64_free(copy, VALUE64_STR);
    }

    /* 12. clone fs */
    test_sub("subtest %d: clone fs", ++subnum);
    {
        const char *text = "clone-fs";
        fs orig_fs = fscopy(text);
        value64 orig = value64_createfs(&orig_fs);
        value64 copy = value64_clone(orig, VALUE64_FS);

        test_validatefree(
            strcmp(fs_str(copy.fsval), text) == 0,
            (fsfree(orig_fs), value64_free(orig, VALUE64_FS), value64_free(copy, VALUE64_FS)),
            "Clone fs: got '%s', expected '%s'", fs_str(copy.fsval), text
        );
        test_validatefree(
            fs_bodyalloc(copy.fsval),
            (fsfree(orig_fs), value64_free(orig, VALUE64_FS), value64_free(copy, VALUE64_FS)),
            "Clone fs must have FS_FLAG_BODYALLOC"
        );
        test_validatefree(
            copy.fsval != orig.fsval,
            (fsfree(orig_fs), value64_free(orig, VALUE64_FS), value64_free(copy, VALUE64_FS)),
            "Clone fs must have different pointer"
        );

        fsfree(orig_fs);
        value64_free(orig, VALUE64_FS);
        value64_free(copy, VALUE64_FS);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST value64_move ---------------------------------

static TestStatus
tf_move(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. move int */
    test_sub("subtest %d: move int", ++subnum);
    {
        value64 src = value64_createint(42);
        value64 dst = VALUE64_ZERO;
        value64 *ret = value64_move_int(&dst, &src);

        test_validate(ret == &dst, "move_int must return &dst");
        test_validate(value64_int(dst) == 42, "dst must be 42, got %d", value64_int(dst));
        test_validate(value64_int(src) == 0, "src must be 0 after move, got %d", value64_int(src));
    }

    /* 2. move long */
    test_sub("subtest %d: move long", ++subnum);
    {
        value64 src = value64_createlong(123456789L);
        value64 dst = VALUE64_ZERO;
        value64_move_long(&dst, &src);

        test_validate(value64_long(dst) == 123456789L, "dst mismatch, got %ld", value64_long(dst));
        test_validate(value64_long(src) == 0L, "src must be 0 after move, got %ld", value64_long(src));
    }

    /* 3. move double */
    test_sub("subtest %d: move double", ++subnum);
    {
        value64 src = value64_createdbl(3.1415);
        value64 dst = VALUE64_ZERO;
        value64_move_dbl(&dst, &src);

        test_validate(fabs(value64_dbl(dst) - 3.1415) < 0.0001, "dst mismatch, got %f", value64_dbl(dst));
        test_validate(fabs(value64_dbl(src) - 0.0) < 1e-12, "src must be 0.0 after move, got %f", value64_dbl(src));
    }

    /* 4. move pointer */
    test_sub("subtest %d: move pointer", ++subnum);
    {
        int x = 7;
        value64 src = value64_createptr(&x);
        value64 dst = VALUE64_ZERO;
        value64_move_ptr(&dst, &src);

        test_validate(value64_ptr(dst) == &x, "dst must point to x, got %p", value64_ptr(dst));
        test_validate(value64_ptr(src) == NULL, "src must be NULL after move, got %p", value64_ptr(src));
    }

    /* 5. move str */
    test_sub("subtest %d: move str", ++subnum);
    {
        const char *text = "movable string";
        value64 src = value64_createstr(text);
        value64 dst = VALUE64_ZERO;
        value64_move_str(&dst, &src);

        test_validatefree(
            value64_str(dst) != NULL && strcmp(value64_str(dst), text) == 0,
            value64_free(dst, VALUE64_STR),
            "dst must be '%s', got '%s'", text, value64_str(dst)
        );
        test_validatefree(
            value64_str(src) == NULL,
            value64_free(dst, VALUE64_STR),
            "src must be NULL after move, got %p", value64_str(src)
        );
        value64_free(dst, VALUE64_STR);
    }

    /* 6. move fs */
    test_sub("subtest %d: move fs", ++subnum);
    {
        const char *text = "fs-move-target";
        fs orig = fscopy(text);
        value64 src = value64_createfs(&orig);
        fsfree(orig);

        fs *src_fs_before = value64_fs(src);   // запоминаем указатель до move

        value64 dst = VALUE64_ZERO;
        value64_move_fs(&dst, &src);

        fs *dst_fs = value64_fs(dst);

        test_validatefree(
            dst_fs != NULL && strcmp(fs_str(dst_fs), text) == 0,
            value64_free(dst, VALUE64_FS),
            "dst must be '%s', got '%s'", text, fs_str(dst_fs)
        );
        test_validatefree(
            fs_bodyalloc(dst_fs),
            value64_free(dst, VALUE64_FS),
            "dst fs must have FS_FLAG_BODYALLOC"
        );
       // Убеждаемся, что dst получил новую память (не ту, что была у src)
        test_validatefree(
            dst_fs != src_fs_before,
            value64_free(dst, VALUE64_FS),
            "dst fs pointer must differ from original src pointer"
        );
        value64_free(dst, VALUE64_FS);
        fs_alloc_check(true);
    }

    /* 7. multiple fs moves (leak check) */
    test_sub("subtest %d: multiple fs moves (leak check)", ++subnum);
    {
        const char *words[] = {"first", "second", "third"};
        value64 dst[3] = { VALUE64_ZERO, VALUE64_ZERO, VALUE64_ZERO };

        for (int i = 0; i < COUNT(words); i++) {
            fs orig = fscopy(words[i]);
            value64 src = value64_createfs(&orig);
            fsfree(orig);
            value64_move_fs(&dst[i], &src);
        }

        for (int i = 0; i < COUNT(words); i++) {
            fs *dst_fs = value64_fs(dst[i]);
            test_validatefree(
                strcmp(fs_str(dst_fs), words[i]) == 0,
                (value64_free(dst[0], VALUE64_FS), value64_free(dst[1], VALUE64_FS), value64_free(dst[2], VALUE64_FS)),
                "dst[%d] must be '%s', got '%s'", i, words[i], fs_str(dst_fs)
            );
        }
        for (int i = 0; i < COUNT(words); i++) {
            value64_free(dst[i], VALUE64_FS);
        }
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
              , testnew(.f2 = tf_clone,            .num =  3, .name = "Simple value64_clone() test"                , .desc="", .mandatory=true)
              , testnew(.f2 = tf_move,             .num =  4, .name = "Simple value64_move() test"                 , .desc="", .mandatory=true)
            );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HSETTESTING */


