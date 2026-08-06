
#include "ds_adapter.h"

/********************************************************************
                 Ds - fs adapter MODULE IMPLEMENTATION
********************************************************************/

// ------------------------------ Utilities ------------------------

/**
 * @brief Internal helper to print formatted string into a fixed-size buffer.
 * 
 * This function calculates the required size first, then writes the content.
 * It ensures that the buffer does not overflow and handles null-termination.
 *
 * @param[in] ptr      Pointer to the start of the destination buffer.
 * @param[in] pos      Current write position (offset) in the buffer.
 * @param[in] cap      Total capacity of the buffer (limit).
 * @param[in] fmt      Format control string.
 * @param[in] ap       Variable argument list.
 * 
 * @return The number of characters written (excluding null terminator), 
 *         or -1 if the buffer is too small or an error occurred.
 */
int                         dsVPrintStr(char *ptr, size_t pos, size_t cap, const char *fmt, va_list ap) {
    int needed = vsnprintf(NULL, 0, fmt, ap);
    if (needed < 0)
        return userraise(-1, ERR_STREAM_ERROR, "Unable to vsnprintf NULL");
    if ((size_t) needed + pos + 1 > cap)
        return -1;

    int written = vsnprintf(ptr + pos, cap - pos, fmt, ap);

    if (written < 0) {
        return userraise(-1, ERR_STREAM_ERROR, "Unable to vsnprintf");
    }
    return written;
}

/**
 * @brief Helper that performs sscanf with automatic position advance.
 *
 * The format string is extended with a trailing " %n" to capture the
 * number of consumed characters.  The position `*ppos` is updated
 * accordingly.  This requires a second pass, therefore a copy of the
 * variadic argument list is used.
 *
 * @param buf   null‑terminated input buffer
 * @param ppos  pointer to the current read offset (will be updated)
 * @param fmt   scanf‑style format string
 * @param ap    variadic argument list (as passed to vfscanf)
 * @return      number of successfully matched items, or a negative value on error
 */
static int dsVScanf(const char *buf, size_t cap, size_t *ppos, const char *fmt, va_list ap) {
    size_t remaining = cap - *ppos;
    if (remaining == 0)
        return userraise(-1, ERR_OUT_OF_BUFFER, "Buffer exhausted");

    FILE *mem = fmemopen((void *) (buf + *ppos), remaining, "r");
    if (!mem)
        return userraise(-1, ERR_UNABLE_OPEN_FILE_READ, "Unable to fmemopen");

    int ret = vfscanf(mem, fmt, ap);
    if (ret < 0) {
        fclose(mem);
        return userraise(-1, ERR_STREAM_ERROR, "vfscanf error");
    }
    long offset = ftell(mem);
    fclose(mem);

    if (offset > 0)
        *ppos += (size_t) offset;
    return ret;
}

// --------------------------- API ---------------------------------

int                         dsPrintf(Ds *restrict pds, const char *restrict msg, ...) {
    invraisecode(pds != NULL && msg != NULL, ERR_NULLABLE_PTR, 
        "Null input %p %p", pds, msg);

    va_list     ap;
    int         total = 0;
    va_start(ap, msg);
    switch (pds->type) {
        case DS_FILE:
            IOCHECKER(wr, vfprintf(pds->fp, msg, ap), -1)
                total += wr;
            break;
        case DS_STR: // this is NOT autoextendable, till end of pds->ptr only
            IOCHECKER(wr, dsVPrintStr(pds->ptr, pds->pos, pds->cap, msg, ap), -1) {
                total += wr;
                pds->pos += wr;
            }
            break;
        case DS_FS:     // this is autoextendable
            IOCHECKER(wr, fs_sprintf_position(&pds->s, pds->pos, msg, ap), -1) {
                total += wr;
                pds->pos += wr; // iterator over fs pds->s
            }
            break;
        default:
            va_end(ap); // for lulz
            return userraise(-1, ERR_ACTION_NOT_APPLICABLE, 
                "Can't write to %s", DSTypeName(pds->type) );
    }
    va_end(ap);
    return total;
}

int                         dsScanf(Ds *restrict pds, const char *restrict msg, ...) {
    invraisecode(pds != NULL && msg != NULL, ERR_NULLABLE_PTR, 
        "Null input %p %p", pds, msg);
    
    va_list     ap;
    int         ret;
    va_start(ap, msg);

    switch (pds->type) {
        case DS_FILE: {
            ret = vfscanf(pds->fp, msg, ap);
            break;
        }
        case DS_STR:
        case DS_FS:
        case DS_CONSTSTR: {
            const char *buf = dsStrbuf(pds);
            ret = dsVScanf(buf, pds->type == DS_FS ? pds->s.len : pds->cap, &pds->pos, msg, ap);
            }
            break;
        default:
            ret = -1;
    }

    va_end(ap);
    return ret;
}

// -------------------- CONSTRUCTOTS/DESTRUCTORS -------------------

// -------------------------- (API) printers -----------------------

// -------------------------------Testing --------------------------

#ifdef DS_ADAPTER_TESTING

#include "test.h"

// ------------------------- TEST dsPrintf -------------------------
static TestStatus
tf_ds_printf(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. DS_FILE: простая запись */
    test_sub("subtest %d: dsPrintf to file", ++subnum);
    {
        const char *fname = "res/ds/test_printf_file.dsadp";
        FILE *fp = fopen(fname, "w");
        Ds ds = dsCreatef(fp);

        int written = dsPrintf(&ds, "Hello %d", 42);
        fclose(fp);
        test_validate(written > 0, "dsPrintf must return > 0, got %d", written);

        fp = fopen(fname, "r");
        char buf[32] = {0};
        fread(buf, 1, sizeof(buf)-1, fp);
        fclose(fp);
        test_validate(strcmp(buf, "Hello 42") == 0,
                      "File must contain 'Hello 42', got '%s'", buf);
    }

    /* 2. DS_FILE: запись с форматированием */
    test_sub("subtest %d: dsPrintf to file with multiple args", ++subnum);
    {
        const char *fname = "res/ds/test_printf_multi.dsadp";
        FILE *fp = fopen(fname, "w");
        Ds ds = dsCreatef(fp);

        int written = dsPrintf(&ds, "%d + %d = %d", 2, 3, 5);
        fclose(fp);
        test_validate(written > 0, "dsPrintf must return > 0");

        fp = fopen(fname, "r");
        char buf[32] = {0};
        fread(buf, 1, sizeof(buf)-1, fp);
        fclose(fp);
        test_validate(strcmp(buf, "2 + 3 = 5") == 0,
                      "File must contain '2 + 3 = 5', got '%s'", buf);
    }

#ifndef NO_FSDS
    /* 3. DS_FS: запись в fs */
    test_sub("subtest %d: dsPrintf to FS", ++subnum);
    {
        fs s = FS();
        Ds ds = dsCreatefs(&s);
        int written = dsPrintf(&ds, "Value=%d", 99);
        test_validate(written > 0, "dsPrintf must return > 0");

        // после dsCreatefs переменная s перемещена, читаем из ds.s
        test_validate(strcmp(ds.s.v, "Value=99") == 0,
                      "FS must contain 'Value=99', got '%s'", ds.s.v);
        dsFree(&ds);
        fs_alloc_check(true);
    }

    /* 4. DS_FS: множественная запись */
    test_sub("subtest %d: dsPrintf to FS multiple calls", ++subnum);
    {
        fs s = FS();
        Ds ds = dsCreatefs(&s);
        dsPrintf(&ds, "Line1\n");
        dsPrintf(&ds, "Line2");
        test_validate(strcmp(ds.s.v, "Line1\nLine2") == 0,
                      "FS must contain 'Line1\\nLine2', got '%s'", ds.s.v);
        dsFree(&ds);
        fs_alloc_check(true);
    }
#endif /* !NO_FSDS */

    /* 5. NULL Ds должен вызвать исключение */
    test_sub("subtest %d: dsPrintf with NULL Ds raises SIGINT", ++subnum);
    {
        if (!try()) {
            dsPrintf(NULL, "test");
            test_validate(false, "Should have raised SIGINT for NULL Ds");
        } else {
            logsimple("Exception correctly raised for NULL Ds");
        }
    }
    /* 4. DS_STR: запись ровно на границе буфера (без переполнения) */
    test_sub("subtest %d: dsPrintf string boundary (no overflow)", ++subnum);
    {
        char buf[11] = "..........";   // strlen = 10 → cap = 10 (достаточно для "123456789" + '\0')
        Ds ds = dsCreatestr(buf);
        int written = dsPrintf(&ds, "123456789");   // нужно 9 символов + '\0' → 10
        test_validate(written == 9,
                    "dsPrintf must return 9, got %d", written);
        test_validate(strcmp(buf, "123456789") == 0,
                    "Buffer must contain '123456789', got '%s'", buf);
        test_validate(ds.pos == 9,
                    "pos must be 9, got %zu", ds.pos);
    }

    /* 5. DS_STR: переполнение буфера */
    test_sub("subtest %d: dsPrintf string overflow", ++subnum);
    {
        char buf[5] = "12";               // strlen = 2 → cap = 2
        Ds ds = dsCreatestr(buf);
        int written = dsPrintf(&ds, "Hello World");   // нужно 11 символов
        test_validate(written == -1,
                    "dsPrintf overflow must return -1, got %d", written);
        test_validate(buf[0] == '1',
                    "Buffer must remain unchanged, got '%s'", buf);
        test_validate(buf[1] == '2',
                    "Buffer must remain unchanged, got '%s'", buf);
        test_validate(buf[2] == '\0',
                    "Buffer must remain unchanged, got '%s'", buf);
        test_validate(ds.pos == 0,
                    "pos must stay 0, got %zu", ds.pos);
    }
    /* 6. DS_STR: множественная запись */
    test_sub("subtest %d: dsPrintf to string multiple calls", ++subnum);
    {
        char buf[] = "1111111111111111111111111111111111";
        Ds ds = dsCreatestr(buf);
        dsPrintf(&ds, "Line1\n");
        dsPrintf(&ds, "Line2");
        test_validate(strcmp(buf, "Line1\nLine2") == 0,
                      "Buffer must contain 'Line1\\nLine2', got '%s'", buf);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST dsScanf -------------------------
static TestStatus
tf_ds_scanf(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. DS_FILE: чтение из файла */
    test_sub("subtest %d: dsScanf from file", ++subnum);
    {
        const char *fname = "res/ds/test_scanf_file.dsadp";
        FILE *fp = fopen(fname, "w");
        fprintf(fp, "42 3.14 hello");
        fclose(fp);

        fp = fopen(fname, "r");
        Ds ds = dsCreatef(fp);
        int i;
        double d;
        char s[10];
        int ret = dsScanf(&ds, "%d %lf %s", &i, &d, s);
        fclose(fp);
        test_validate(ret == 3, "dsScanf must return 3, got %d", ret);
        test_validate(i == 42 && d == 3.14 && strcmp(s, "hello") == 0,
                      "File values: i=%d, d=%.2f, s='%s' (expected 42, 3.14, 'hello')",
                      i, d, s);
    }

    /* 2. DS_STR: чтение из строки с авто‑сдвигом */
    test_sub("subtest %d: dsScanf from mutable string", ++subnum);
    {
        char buf[32] = "10 20 30";
        Ds ds = dsCreatestr(buf);
        int a, b, c;
        dsScanf(&ds, "%d", &a);
        dsScanf(&ds, "%d", &b);
        dsScanf(&ds, "%d", &c);
        test_validate(a == 10 && b == 20 && c == 30,
                      "STR values: a=%d, b=%d, c=%d (expected 10,20,30)", a, b, c);
        test_validate(ds.pos == 8, "After three scans pos must be 8, got %zu", ds.pos);
    }

    /* 3. DS_CONSTSTR: чтение из константной строки */
    test_sub("subtest %d: dsScanf from const string", ++subnum);
    {
        const char *text = "3.14";
        Ds ds = dsCreateconst(text);
        double d;
        int ret = dsScanf(&ds, "%lf", &d);
        test_validate(ret == 1, "dsScanf must return 1, got %d", ret);
        test_validate(d == 3.14, "CONSTSTR: d=%.2f (expected 3.14)", d);
        test_validate(ds.pos == 4, "pos must be 4, got %zu", ds.pos);
    }

#ifndef NO_FSDS
    /* 4. DS_FS: чтение из fs */
    test_sub("subtest %d: dsScanf from FS", ++subnum);
    {
        fs s = FS();
        Ds ds = dsCreatefs(&s);
        dsputc('A', &ds);
        dsputc('B', &ds);
        ds.pos = 0;
        char ch1, ch2;
        int ret = dsScanf(&ds, "%c%c", &ch1, &ch2);
        test_validate(ret == 2, "dsScanf must return 2, got %d", ret);
        test_validate(ch1 == 'A' && ch2 == 'B',
                      "FS chars: ch1='%c', ch2='%c' (expected 'A','B')", ch1, ch2);
        dsFree(&ds);
        fs_alloc_check(true);
    }
#endif /* !NO_FSDS */

    /* 5. Ошибка: пустая строка */
    test_sub("subtest %d: dsScanf on empty string", ++subnum);
    {
        char buf[4] = "";
        Ds ds = dsCreatestr(buf);
        int val;
        int ret = dsScanf(&ds, "%d", &val);
        test_validate(ret <= 0, "dsScanf on empty string must fail, got %d", ret);
    }

    /* 6. Ошибка: неверный формат */
    test_sub("subtest %d: dsScanf with invalid format", ++subnum);
    {
        char buf[8] = "abc";
        Ds ds = dsCreatestr(buf);
        int val;
        int ret = dsScanf(&ds, "%d", &val);
        test_validate(ret <= 0, "dsScanf on non‑numeric string must fail, got %d", ret);
    }

    test_sub("subtest %d: dsScanf with NULL Ds raises SIGINT", ++subnum);
    {
        if (!try()) {
            int dummy;
            dsScanf(NULL, "%d", &dummy);
            test_validate(false, "Should have raised SIGINT for NULL Ds");
        } else {
            logsimple("Exception correctly raised for NULL Ds");
        }
    }

    return logret(TEST_PASSED, "done");
}



// -------------------------------------------------------------------
int
main( /*int argc, char *argv[] */ )
{
    logsimpleinit("Start");

    testenginestd(
        TESTADD(tf_ds_printf,          "dsPrintf() simple tests")
      , TESTADD(tf_ds_scanf,           "dsScanf() simple tests")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* DS_ADAPTER_TESTING */
