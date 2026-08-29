
#include "ds_adapter.h"

/********************************************************************
                 DS - fs adapter MODULE IMPLEMENTATION
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
static int                      dsHelperVPrintStr(char *ptr, size_t pos, size_t cap, const char *fmt, va_list ap) {
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
static int                      dsHelperVScanf(const char *buf, size_t cap, size_t *ppos, const char *fmt, va_list ap) {
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

/**
 * @brief Internal helper to parse an integer from a c-string.
 *
 * This function uses @c strtol to convert a string to a long, then verifies 
 * that the result fits within the bounds of a standard integer (@c INT_MIN to @c INT_MAX).
 *
 * @param[in]  str  The source null-terminated string.
 * @param[out] plval Pointer to the long where the parsed value will be stored.
 *
 * @return true if parsing was successful and the value is within integer bounds, 
 *         false otherwise (raises error via @c userraise).
 */
static bool                     dsHelperParseLong(const char *restrict str, long *restrict plval, size_t *restrict pos) {
    char    *endptr;
    errno = 0;
    long    val = strtol(str, &endptr, 10);

    if (str == endptr)
        return userraise(false, ERR_UNABLE_PARSE_DATA, "err parse int/long %ld, errno %s", val, strerror(errno));
    
    *pos = endptr - str;
    if (plval)
        *plval = val;
    return true;
}

/**
 * @brief Wrapper for parsing int, including bounds checking.
 */
static bool                     dsHelperParseInt(const char *restrict str, int *restrict pival, size_t *restrict pos) {
    long temp_val;
    if (!dsHelperParseLong(str, &temp_val, pos)) {
        return false; 
    }
    // check int borders
    if (temp_val > INT_MAX || temp_val < INT_MIN)
        return userraise(false, ERR_UNABLE_PARSE_DATA, "value %ld out of int range", temp_val);
    
    if (pival)
        *pival = (int)temp_val;
    return true;
}

/**
 * @brief Internal helper to parse a double from a null-terminated string.
 *
 * @param[in]  str    The source string.
 * @param[out] pdval  Pointer to store the double value.
 *
 * @return true if parsing was successful, false otherwise.
 */
static bool                     dsHelperParseDouble(const char *restrict str, double *restrict pdval, size_t *restrict pos) {
    char *endptr;
    errno = 0;
    double val = strtod(str, &endptr);

    // Проверяем, что:
    // 1. endptr не равен str (значит, хотя бы одна цифра была прочитана)
    // 2. errno не содержит ошибок (например, переполнение RANGE)
    if (str == endptr || errno != 0) 
        return userraise(false, ERR_UNABLE_PARSE_DATA, "err parse double, errno %s", strerror(errno));

    *pos += endptr - str;
    if (pdval)
        *pdval = val;
    return true;
}

/**
 * @brief Internal helper to parse a char from a string.
 */
static bool                     dsHelperParseChar(const char *restrict str, char *restrict pval, size_t *restrict pos) {
    // Skip leading whitespace to find the first character
    size_t skip = 0;
    while (str[skip] && isspace((unsigned char)str[skip]))
        skip++;
    if (str[skip] == '\0')
        return userraise(false, ERR_UNABLE_PARSE_DATA, "Empty or whitespace string for char");
    
    *pos += skip;          // spaces
    if (pval)
        *pval = str[skip];
    *pos += 1;             // sym
    return true;
}


/**
 * @brief Internal helper to parse an unsigned long from a string.
 */
static bool                     dsHelperParseUnsignedLong(const char *restrict str, unsigned long *restrict plval, size_t *restrict pos) {
    char *endptr;
    errno = 0;
    unsigned long val = strtoul(str, &endptr, 10);

    if (str == endptr || errno != 0) {
        return userraise(false, ERR_UNABLE_PARSE_DATA, "err parse unsigned long, errno %s", strerror(errno));
    }

    *pos += endptr - str;
    if (plval)
        *plval = val;
    return true;
}

/**
 * @brief Wrapper for parsing unsigned int, including bounds checking.
 */
static bool                     dsHelperParseUnsigned(const char *restrict str, unsigned *restrict pival, size_t *restrict pos) {
    unsigned long temp_val;
    if (!dsHelperParseUnsignedLong(str, &temp_val, pos) )
        return false; 

    // check int borders
    if (temp_val > UINT_MAX)
        return userraise(false, ERR_UNABLE_PARSE_DATA, "value %ld out of int range", temp_val);
    
    if (pival)
        *pival = (int)temp_val;
    return true;
}


// --------------------------- API ---------------------------------

int                         dsPrintf(DS *restrict pds, const char *restrict msg, ...) {
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
            IOCHECKER(wr, dsHelperVPrintStr(pds->ptr, pds->pos, pds->cap, msg, ap), -1) {
                total += wr;
                pds->pos += wr;
            }
            break;
        case DS_FS:     // this is autoextendable
            IOCHECKER(wr, fs_vsprintf_position(&pds->s, pds->pos, msg, ap), -1) {
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

int                         dsScanf(DS *restrict pds, const char *restrict msg, ...) {
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
            ret = dsHelperVScanf(buf, pds->type == DS_FS ? pds->s.len : pds->cap, &pds->pos, msg, ap);
            }
            break;
        default:
            ret = userraise(-1, ERR_UNSUPPORTED_TYPE, "Unsupported %s", DSTypeName(pds->type) );
    }

    va_end(ap);
    return ret;
}

bool                        dsParseInt(DS *restrict pds, int *restrict pval) {
    invraisecode(pds != NULL && pval != NULL, ERR_NULLABLE_PTR, 
        "Null input %p %p", pds, pval);
    switch (pds->type) {
        case DS_FILE:
            if (fscanf(pds->fp, "%d", pval) != 1)
                return userraise(false, ERR_UNABLE_PARSE_DATA, "Failed to parse int from file");
            break;
        case DS_STR:
        case DS_FS:
        case DS_CONSTSTR: 
                return dsHelperParseInt(dsStrbuf(pds) + pds->pos, pval, &pds->pos);
            break;
        default:
            return userraise(false, ERR_UNSUPPORTED_TYPE, "Unsupported %s", DSTypeName(pds->type) );
    }
    return true;
}

bool                        dsParseLong(DS *restrict pds, long *restrict pval) {
    invraisecode(pds != NULL && pval != NULL, ERR_NULLABLE_PTR, 
        "Null input %p %p", pds, pval);
    switch (pds->type) {
        case DS_FILE:
            if (fscanf(pds->fp, "%ld", pval) != 1)
                return userraise(false, ERR_UNABLE_PARSE_DATA, "Failed to parse int from file");
            break;
        case DS_STR:
        case DS_FS:
        case DS_CONSTSTR: 
                return dsHelperParseLong(dsStrbuf(pds) + pds->pos, pval, &pds->pos);
            break;
        default:
            return userraise(false, ERR_UNSUPPORTED_TYPE, "Unsupported %s", DSTypeName(pds->type) );
    }
    return true;
}

bool                        dsParseUnsigned(DS *restrict pds, unsigned int *restrict pval) {
    invraisecode(pds != NULL && pval != NULL, ERR_NULLABLE_PTR, "Null input %p %p", pds, pval);
    switch (pds->type) {
        case DS_FILE:
            if (fscanf(pds->fp, "%u", pval) != 1)
                return userraise(false, ERR_UNABLE_PARSE_DATA, "Failed to read unsigned int from file");
            break;
        case DS_STR:
        case DS_FS:
        case DS_CONSTSTR:
            return dsHelperParseUnsigned(dsStrbuf(pds) + pds->pos, pval, &pds->pos);
        default:
            return userraise(false, ERR_UNSUPPORTED_TYPE, "Unsupported %s", DSTypeName(pds->type));
    }
    return true;
}

bool                        dsParseUnsignedLong(DS *restrict pds, unsigned long *restrict pval) {
    invraisecode(pds != NULL && pval != NULL, ERR_NULLABLE_PTR, "Null input %p %p", pds, pval);
    switch (pds->type) {
        case DS_FILE:
            if (fscanf(pds->fp, "%lu", pval) != 1)
                return userraise(false, ERR_UNABLE_PARSE_DATA, "Failed to read unsigned long from file");
            break;
        case DS_STR:
        case DS_FS:
        case DS_CONSTSTR:
            return dsHelperParseUnsignedLong(dsStrbuf(pds) + pds->pos, pval, &pds->pos);
        default:
            return userraise(false, ERR_UNSUPPORTED_TYPE, "Unsupported %s", DSTypeName(pds->type));
    }
    return true;
}

bool                        dsParseDouble(DS *restrict pds, double *restrict pdval) {
    invraisecode(pds != NULL && pdval != NULL, ERR_NULLABLE_PTR, 
        "Null input %p %p", pds, pdval);

    switch (pds->type) {
        case DS_FILE: {
            // %lf - double в fscanf
            if (fscanf(pds->fp, "%lf", pdval) != 1)
                return userraise(false, ERR_UNABLE_PARSE_DATA, "Failed to parse double from file");
            break;
        }
        case DS_STR:
        case DS_FS:
        case DS_CONSTSTR: {
            return dsHelperParseDouble(dsStrbuf(pds) + pds->pos, pdval, &pds->pos);
        }
        default:
            return userraise(false, ERR_UNSUPPORTED_TYPE, "Unsupported %s", DSTypeName(pds->type));
    }
    return true;
}

bool                        dsParseChar(DS *restrict pds, char *restrict pval) {
    invraisecode(pds != NULL && pval != NULL, ERR_NULLABLE_PTR, "Null input %p %p", pds, pval);

    switch (pds->type) {
        case DS_FILE:
            if (fscanf(pds->fp, " %c", pval) != 1) // " %c" skips whitespace
                return userraise(false, ERR_UNABLE_PARSE_DATA, "Failed to read char from file");
            break;
        case DS_STR: case DS_FS: case DS_CONSTSTR:
            return dsHelperParseChar(dsStrbuf(pds) + pds->pos, pval, &pds->pos);
        default:
            return userraise(false, ERR_UNSUPPORTED_TYPE, "Unsupported %s", DSTypeName(pds->type));
    }
    return true;
}

// -------------------------------------- fs adapters ------------------------------------------------

// Helper for techprint fs 
static long                     
dsfsHelperTechprintTofs(fs *restrict out, size_t pos, const fs *restrict s, const char *restrict name) {
    long    initpos = pos;
    if (s) {
        size_t     len = MIN(FS_TECH_PRINT_COUNT, s->len);
        pos += WRITE_OR_RET(fs_sprintf_position(out, pos, 
            "FS: %s: len [%zu], sz [%zu], flags [%d], s [", name, s->len, s->sz, s->flags), -1L);
        
        if (s->v)
            pos += WRITE_OR_RET(fs_sprintf_position(out, pos, "%.*s", (unsigned) len, s->v), -1L);
        else
            pos += WRITE_OR_RET(fs_sprintf_position(out, pos, "<NULL>"), -1);

        if (FS_TECH_PRINT_COUNT < s->len)
            pos += WRITE_OR_RET(fs_sprintf_position(out, pos, "..."), -1L);
        pos += WRITE_OR_RET(fs_sprintf_position(out, pos, "]\n"), -1L);
    } else 
        pos += WRITE_OR_RET(fs_sprintf_position(out, pos, "FS: %s: <NULL>\n", name), -1);
    return pos - initpos; 
}

// print fs data into stream out
long                            fs_dsprintf(DS *restrict out, const fs *restrict s) {
    if (!out)
        return userraise(-1L, ERR_NULL_OUTPUT, "");
    switch (out->type) {
        case DS_FILE:
            fs_fprint(out->fp, s, "");
        default:
            return userraise(-1L, ERR_UNSUPPORTED_TYPE, "Unsupported %s", DSTypeName(out->type));
    }
}

// techprint used temporary fs buffer (low performace) in order to have the same logic for all path
long                            fs_dstechprintf(DS *restrict out, const fs *restrict s, const char *restrict name) {
    if (!out)
        return userraise(-1L, ERR_NULL_OUTPUT, "");
    if (int_notin(out->type, DS_FILE, DS_STR, DS_FS) )
        return userraise(-1L, ERR_UNSUPPORTED_TYPE, 
            "Unsupported %d/%s", out->type, DSTypeName(out->type));

    fs           buf = FS();      // empty
    size_t       total_prepared, actual_written = 0;
    // common printer for all types! 
    // That is not very good for perf, but ok for techprint
    off_t        res_helper = WRITE_OR_RET_ACTION(
            dsfsHelperTechprintTofs(&buf, 0L, s, name), -1, 
            fsfree(buf));
    total_prepared = (size_t) res_helper;

    switch (out->type) {
        case DS_FILE: {
            size_t written;
            if ( (written = fwrite(buf.v, sizeof(char), buf.len, out->fp) ) < buf.len) {
                fsfree(buf);
                return userraise(-1L, ERR_STREAM_ERROR,
                    "Unable to fwrite %zu bytes (only %zu)", buf.len, written);
            }
            actual_written = written;
            break;
        }
        case DS_STR: {
            size_t       remaining = out->cap - out->pos;
            actual_written = (remaining < total_prepared) ? remaining : total_prepared;
            memcpy(out->ptr + out->pos, buf.v, actual_written);
            out->pos += actual_written;
            break;
        }
        case DS_FS:
            fs_setlen(&out->s, out->pos);   // just in case
            fs_cat(&out->s, buf);
            out->pos += (actual_written = total_prepared);
            break;
        default:    // just to avoid warning
    }
    fsfree(buf);

    return actual_written;
}

long                            fs_dsserialize(DS *restrict out, const fs *restrict s) {
    if (!out || !s)
        return userraise(-1L, ERR_NULL_OUTPUT, "%p %p", out, scalbln);
    long    total = 0L;
    return total;
}


// -------------------- CONSTRUCTOTS/DESTRUCTORS -------------------

// N/A

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
        DS ds = dsCreatef(fp);

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
        DS ds = dsCreatef(fp);

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
        DS ds = dsCreatefs(&s);
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
        DS ds = dsCreatefs(&s);
        dsPrintf(&ds, "Line1\n");
        dsPrintf(&ds, "Line2");
        test_validate(strcmp(ds.s.v, "Line1\nLine2") == 0,
                      "FS must contain 'Line1\\nLine2', got '%s'", ds.s.v);
        dsFree(&ds);
        fs_alloc_check(true);
    }
#endif /* !NO_FSDS */

    /* 5. NULL DS должен вызвать исключение */
    test_sub("subtest %d: dsPrintf with NULL DS raises SIGINT", ++subnum);
    {
        if (!try()) {
            dsPrintf(NULL, "test");
            test_validate(false, "Should have raised SIGINT for NULL DS");
        } else {
            logsimple("Exception correctly raised for NULL DS");
        }
    }
    /* 4. DS_STR: запись ровно на границе буфера (без переполнения) */
    test_sub("subtest %d: dsPrintf string boundary (no overflow)", ++subnum);
    {
        char buf[] = "..........";   // strlen = 10 → cap = 10 (достаточно для "123456789" + '\0')
        DS ds = dsCreatestr(buf);

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
        DS ds = dsCreatestr(buf);
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
        const int cnt = 50;
        char buf[cnt];
        DS ds = dsCreatestrCap(buf, cnt);
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
        DS ds = dsCreatef(fp);
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
        DS ds = dsCreatestr(buf);
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
        DS ds = dsCreateconst(text);
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
        DS ds = dsCreatefs(&s);
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
        DS ds = dsCreatestr(buf);
        int val;
        int ret = dsScanf(&ds, "%d", &val);
        test_validate(ret <= 0, "dsScanf on empty string must fail, got %d", ret);
    }

    /* 6. Ошибка: неверный формат */
    test_sub("subtest %d: dsScanf with invalid format", ++subnum);
    {
        char buf[8] = "abc";
        DS ds = dsCreatestr(buf);
        int val;
        int ret = dsScanf(&ds, "%d", &val);
        test_validate(ret <= 0, "dsScanf on non‑numeric string must fail, got %d", ret);
    }

    test_sub("subtest %d: dsScanf with NULL DS raises SIGINT", ++subnum);
    {
        if (!try()) {
            int dummy;
            dsScanf(NULL, "%d", &dummy);
            test_validate(false, "Should have raised SIGINT for NULL DS");
        } else {
            logsimple("Exception correctly raised for NULL DS");
        }
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST dsScanf() and dsPrintf() combined tests -------------------------
static TestStatus
tf_ds_scanf_printf(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: dsPrintf + dsScanf round-trip (position)", ++subnum);
    {
        char buf[128];
        DS ds = dsCreatestrCap(buf, sizeof(buf));   // cap = 128

        // Записываем несколько значений
        size_t written_pos = dsPrintf(&ds, "%d %s %c", 42, "test", 'X');

        // Сбрасываем позицию для чтения
        dsReset(&ds);

        // Читаем обратно
        int i;
        char s[10];
        char c;
        int ret = dsScanf(&ds, "%d %s %c", &i, s, &c);
        size_t read_pos = ds.pos;       // позиция после чтения

        test_validate(ret == 3, "Scanf must read 3 items, got %d", ret);
        test_validate(i == 42 && strcmp(s, "test") == 0 && c == 'X',
                    "Values: i=%d, s='%s', c='%c' (expected 42, 'test', 'X')",
                    i, s, c);
        test_validate(written_pos == read_pos,
                    "Position after read (%zu) must equal position after write (%zu)",
                    read_pos, written_pos);
    }

    test_sub("subtest %d: multiple printf / scanf round‑trip", ++subnum);
    {
        char buf[256];
        DS ds = dsCreatestrCap(buf, sizeof(buf));

        // ---------- запись ----------
        int pt1 = dsPrintf(&ds, "%d ", 10);      // "10 "
        int pt2 = dsPrintf(&ds, "%s ", "hello"); // "10 hello "
        int pt3 = dsPrintf(&ds, "%c", '!');      // "10 hello !"
        int pt4 = dsPrintf(&ds, "%s ", " ?");
        //int pt4 = dsPrintf(&ds, "%c", '?');
        int total_written = pt1 + pt2 + pt3 + pt4;                 // позиция после третьей записи
        DSTECHPRINT(ds);

        // ---------- чтение ----------
        dsReset(&ds);

        int i;
        char s[10];
        char c, c1, c2, c3;

        int r1 = dsScanf(&ds, "%d", &i);         // "10"
        test_validate(
            r1 == 1,
            "r1 must be 1, got '%d'", r1
        );

        int r2 = dsScanf(&ds, "%s", s);          // "hello"
        test_validate(
            r2 == 1,
            "r2 must be 1, got '%d'", r2
        );

        int r3 = dsScanf(&ds, "%c%c", &c1, &c);         // "!"
        test_validate(
            r3 == 2,
            "r3 must be 2, got '%d'", r3
        );

        int r4 = dsScanf(&ds, " %c%c", &c2, &c3);         // "? "
        test_validate(
            r4 == 2,
            "r3 must be 1, got '%d'", r4
        );

        size_t total_read = ds.pos;

        test_validate(i == 10, "First scanf: i=%d (expected 10)", i);
        test_validate(strcmp(s, "hello") == 0, "Second scanf: s='%s' (expected 'hello')", s);
        test_validate(c == '!', "Third scanf: c='%c' (expected '!')", c);
        test_validate(c2 == '?', "Forth scanf: c2='%c' (expected '?')", c2);
        test_validate(c3 == ' ', "Forth scanf: c3='%c' (expected ' ')", c3);
        test_validate( (int) total_read == total_written,
                    "Total positions must match: written=%d, read=%zu",
                    total_written, total_read);
    }

    return logret(TEST_PASSED, "done");
}

// --------------------------- TEST dsParse*  ---------------------------------
static TestStatus
tf_ds_parsers(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== dsParseInt ========== */
    test_sub("subtest %d: dsParseInt from mutable string", ++subnum);
    {
        char buf[64] = "42 123";
        DS ds = dsCreatestrCap(buf, sizeof(buf));
        int val;
        test_validate(dsParseInt(&ds, &val) && val == 42,
                      "dsParseInt: expected 42, got %d", val);
        test_validate(ds.pos == 2, "pos must be 2, got %zu", ds.pos);
    }

    test_sub("subtest %d: dsParseInt from const string", ++subnum);
    {
        const char *text = "-10 999";
        DS ds = dsCreateconst(text);
        int val;
        test_validate(dsParseInt(&ds, &val) && val == -10,
                      "dsParseInt const: expected -10, got %d", val);
        test_validate(ds.pos == 3, "pos must be 3, got %zu", ds.pos);
    }

    test_sub("subtest %d: dsParseInt from file", ++subnum);
    {
        const char *fname = "res/ds/test_parse_int.dsadp";
        FILE *fp = fopen(fname, "w");
        fprintf(fp, "77");
        fclose(fp);
        fp = fopen(fname, "r");
        DS ds = dsCreatef(fp);
        int val;
        test_validatefree(dsParseInt(&ds, &val) && val == 77,
                          fclose(fp),
                          "dsParseInt file: expected 77, got %d", val);
        fclose(fp);
    }

    test_sub("subtest %d: dsParseInt fails on non‑numeric", ++subnum);
    {
        char buf[32] = "abc";
        DS ds = dsCreatestrCap(buf, sizeof(buf));
        int val;
        test_validate(!dsParseInt(&ds, &val),
                      "dsParseInt on 'abc' must fail, got val=%d", val);
        test_validate(ds.pos == 0, "pos must stay 0, got %zu", ds.pos);
    }

    test_sub("subtest %d: dsParseInt NULL DS raises", ++subnum);
    {
        if (!try()) {
            int v;
            dsParseInt(NULL, &v);
            test_validate(false, "Should have raised SIGINT");
        } else {
            logsimple("Exception correctly raised");
        }
    }

    /* ========== dsParseLong ========== */
    test_sub("subtest %d: dsParseLong basic", ++subnum);
    {
        char buf[64] = "123456789012";
        DS ds = dsCreatestrCap(buf, sizeof(buf));
        long val;
        test_validate(dsParseLong(&ds, &val) && val == 123456789012L,
                      "dsParseLong: expected 123456789012, got %ld", val);
    }

    test_sub("subtest %d: dsParseLong fails on empty", ++subnum);
    {
        char buf[4] = "";
        DS ds = dsCreatestrCap(buf, sizeof(buf));
        long val;
        test_validate(!dsParseLong(&ds, &val),
                      "dsParseLong on empty must fail");
    }

    /* ========== dsParseUnsigned ========== */
    test_sub("subtest %d: dsParseUnsigned basic", ++subnum);
    {
        char buf[64] = "3000000000";
        DS ds = dsCreatestrCap(buf, sizeof(buf));
        unsigned int val;
        test_validate(dsParseUnsigned(&ds, &val) && val == 3000000000u,
                      "dsParseUnsigned: expected 3000000000u, got %u", val);
    }

    /* ========== dsParseUnsignedLong ========== */
    test_sub("subtest %d: dsParseUnsignedLong basic", ++subnum);
    {
        char buf[64] = "18446744073709551615";
        DS ds = dsCreatestrCap(buf, sizeof(buf));
        unsigned long val;
        test_validate(dsParseUnsignedLong(&ds, &val) && val == 18446744073709551615UL,
                      "dsParseUnsignedLong: expected max, got %lu", val);
    }

    /* ========== dsParseDouble ========== */
    test_sub("subtest %d: dsParseDouble basic", ++subnum);
    {
        char buf[64] = "3.1415 -2.5e1";
        DS ds = dsCreatestrCap(buf, sizeof(buf));
        double val;
        test_validate(dsParseDouble(&ds, &val) && val == 3.1415,
                      "dsParseDouble: expected 3.1415, got %lf", val);
        test_validate(dsParseDouble(&ds, &val) && val == -25.0,
                      "dsParseDouble second: expected -25.0, got %lf", val);
    }

    test_sub("subtest %d: dsParseDouble fails on text", ++subnum);
    {
        char buf[16] = "hello";
        DS ds = dsCreatestrCap(buf, sizeof(buf));
        double val;
        test_validate(!dsParseDouble(&ds, &val),
                      "dsParseDouble on text must fail");
    }

    /* ========== dsParseChar ========== */
    test_sub("subtest %d: dsParseChar basic", ++subnum);
    {
        char buf[64] = "A B";
        DS ds = dsCreatestrCap(buf, sizeof(buf));
        char c;
        test_validate(dsParseChar(&ds, &c) && c == 'A',
                      "dsParseChar: expected 'A', got '%c'", c);
        test_validate(ds.pos == 1, "pos must be 1, got %zu", ds.pos);
        test_validate(dsParseChar(&ds, &c) && c == 'B',
                      "dsParseChar second: expected 'B', got '%c'", c);
        test_validate(ds.pos == 3, "pos must be 3, got %zu", ds.pos);
    }

    test_sub("subtest %d: dsParseChar fails on empty", ++subnum);
    {
        char buf[4] = "";
        DS ds = dsCreatestrCap(buf, sizeof(buf));
        char c;
        test_validate(!dsParseChar(&ds, &c),
                      "dsParseChar on empty must fail");
    }

    /* ========== DS_FS (если доступно) ========== */
#ifndef NO_FSDS
    test_sub("subtest %d: dsParseInt from FS", ++subnum);
    {
        fs s = FS();
        DS ds = dsCreatefs(&s);
        dsPrintf(&ds, "%d", 202);
        dsReset(&ds);
        int val;
        test_validatefree(dsParseInt(&ds, &val) && val == 202,
                          dsFree(&ds),
                          "dsParseInt FS: expected 202, got %d", val);
        dsFree(&ds);
        fs_alloc_check(true);
    }
#endif

    /* ========== Переполнение буфера при записи + чтение ========== */
    test_sub("subtest %d: dsParseInt after overflow", ++subnum);
    {
        char buf[8] = "";   // cap = 8
        DS ds = dsCreatestrCap(buf, sizeof(buf));
        dsPrintf(&ds, "12345678");   // попытка записи не влезет
        dsReset(&ds);
        int val;
        test_validate(!dsParseInt(&ds, &val),
                      "dsParseInt on overflowed buffer must fail");
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST fs_dstechprintf -------------------------
static TestStatus
tf5_fs_dstechprintf(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Вывод в DS_FILE */
    test_sub("subtest %d: DS_FILE output", ++subnum);
    {
        const char *fname = "res/fs/dstechprintf_file.tmp";
        FILE *fp = fopen(fname, "w+");
        test_validatefree(fp != NULL, remove(fname), "fopen failed");

        DS ds = { .type = DS_FILE, .fp = fp, .pos = 0 };
        fs sample = fscopy("hello");

        long written = fs_dstechprintf(&ds, &sample, "sample_fs");
        test_validatefree(written > 0, (fsfree(sample), fclose(fp), remove(fname)),
                          "expected positive written count");

        // перематываем и читаем файл
        rewind(fp);
        char buf[256];
        size_t n = fread(buf, 1, sizeof(buf)-1, fp);
        buf[n] = '\0';

        // проверяем, что в файле есть основные части
        test_validatefree(strstr(buf, "sample_fs") != NULL,
                          (fsfree(sample), fclose(fp), remove(fname)),
                          "file content must contain name");
        test_validatefree(strstr(buf, "hello") != NULL,
                          (fsfree(sample), fclose(fp), remove(fname)),
                          "file content must contain fs data");

        fsfree(sample);
        fclose(fp);
        remove(fname);
        fs_alloc_check(true);
    }

    /* 2. Вывод в DS_STR */
    test_sub("subtest %d: DS_STR output", ++subnum);
    {
        char buffer[256];
        DS ds = { .type = DS_STR, .ptr = buffer, .cap = sizeof(buffer), .pos = 0 };
        fs sample = fscopy("test");

        long written = fs_dstechprintf(&ds, &sample, "str_fs");
        test_validatefree(written > 0, (fsfree(sample)), "expected positive count");

        buffer[ds.pos] = '\0';
        test_validatefree(strstr(buffer, "str_fs") != NULL,
                          (fsfree(sample)),
                          "DS_STR content must contain name");
        test_validatefree(strstr(buffer, "test") != NULL,
                          (fsfree(sample)),
                          "DS_STR content must contain fs data");
        test_validatefree(ds.pos == (size_t)written,
                          (fsfree(sample)),
                          "DS_STR pos must equal written");

        fsfree(sample);
        fs_alloc_check(true);
    }

    /* 3. Вывод в DS_FS */
    test_sub("subtest %d: DS_FS output", ++subnum);
    {
        fs out = FS();
        DS ds = { .type = DS_FS, .s = out, .pos = 0 };
        fs sample = fscopy("world");

        long written = fs_dstechprintf(&ds, &sample, "fs_fs");
        test_validatefree(written > 0, (fsfree(sample), fsfree(out)), "expected positive count");

        test_validatefree(strstr(fs_str(&out), "fs_fs") != NULL,
                          (fsfree(sample), fsfree(out)),
                          "DS_FS content must contain name");
        test_validatefree(strstr(fs_str(&out), "world") != NULL,
                          (fsfree(sample), fsfree(out)),
                          "DS_FS content must contain fs data");
        test_validatefree(ds.pos == (size_t)written,
                          (fsfree(sample), fsfree(out)),
                          "DS_FS pos must equal written");

        fsfree(sample);
        fsfree(out);
        fs_alloc_check(true);
    }

    /* 4. Ошибка при неподдерживаемом типе DS */
    test_sub("subtest %d: unsupported DS type", ++subnum);
    {
        DS ds = { .type = 100500 /* some value */, .fp = NULL, .ptr = NULL, .cap = 0, .pos = 0 };
        fs sample = FS();

        if (!try()) {
            long res = fs_dstechprintf(&ds, &sample, "bad");
            test_validatefree(false, (fsfree(sample)), "must raise error");
            (void)res;
        } else {
            test_validatefree(true, (fsfree(sample)), "correctly raised error");
        }

        fsfree(sample);
        fs_alloc_check(true);
    }

    /* 5. NULL выходной параметр */
    test_sub("subtest %d: NULL output", ++subnum);
    {
        fs sample = FS();

        if (!try()) {
            fs_dstechprintf(NULL, &sample, "null");
            test_validatefree(false, (fsfree(sample)), "must raise error");
        } else {
            test_validatefree(true, (fsfree(sample)), "correctly raised error");
        }

        fsfree(sample);
        fs_alloc_check(true);
    }

        /* 6. Пустая строка с выделенной памятью (fscopy("")) */
    test_sub("subtest %d: empty fs with allocated buffer", ++subnum);
    {
        char buffer[256];
        DS ds = { .type = DS_STR, .ptr = buffer, .cap = sizeof(buffer), .pos = 0 };
        fs sample = fscopy("");

        long written = fs_dstechprintf(&ds, &sample, "empty_fs");
        test_validatefree(written > 0, (fsfree(sample)), "expected positive count");

        buffer[ds.pos] = '\0';
        test_validatefree(strstr(buffer, "empty_fs") != NULL,
                          (fsfree(sample)),
                          "buffer must contain name");
        test_validatefree(strstr(buffer, "len [0]") != NULL,
                          (fsfree(sample)),
                          "buffer must indicate len 0");

        fsfree(sample);
        fs_alloc_check(true);
    }

    /* 7. Пустой fs без памяти (FS()) */
    test_sub("subtest %d: empty fs without memory (FS())", ++subnum);
    {
        char buffer[256];
        DS ds = { .type = DS_STR, .ptr = buffer, .cap = sizeof(buffer), .pos = 0 };
        fs sample = FS();

        long written = fs_dstechprintf(&ds, &sample, "null_fs");
        test_validatefree(written > 0, (fsfree(sample)), "expected positive count");

        buffer[ds.pos] = '\0';
        test_validatefree(strstr(buffer, "null_fs") != NULL,
                          (fsfree(sample)),
                          "buffer must contain name");
        test_validatefree(strstr(buffer, "<NULL>") != NULL,
                          (fsfree(sample)),
                          "buffer must indicate NULL string");

        fsfree(sample);
        fs_alloc_check(true);
    }

    /* 8. NULL fs (s == NULL) */
    test_sub("subtest %d: NULL fs (s == NULL)", ++subnum);
    {
        char buffer[256];
        DS ds = { .type = DS_STR, .ptr = buffer, .cap = sizeof(buffer), .pos = 0 };

        long written = fs_dstechprintf(&ds, NULL, "null_ptr");
        test_validate(written > 0, "expected positive count");

        buffer[ds.pos] = '\0';
        test_validate(strstr(buffer, "null_ptr") != NULL,
                      "buffer must contain name");
        test_validate(strstr(buffer, "<NULL>") != NULL,
                      "buffer must indicate NULL");
    }

    return TEST_PASSED;
}

// -------------------------------------------------------------------
int
main( /*int argc, char *argv[] */ )
{
    logsimpleinit("Start");

    testenginestd(
        TESTADD(tf_ds_printf,          "dsPrintf() simple tests")
      , TESTADD(tf_ds_scanf,           "dsScanf() simple tests")
      , TESTADD(tf_ds_scanf_printf,    "dsScanf() and dsPrintf() combined tests")
      , TESTADD(tf_ds_parsers,         "dsParse<type> simple tests")
      , TESTADD(tf5_fs_dstechprintf,   "fs_dstechprintf simple test")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* DS_ADAPTER_TESTING */
