#include "ds.h"

/********************************************************************
                 Datasource MODULE IMPLEMENTATION
********************************************************************/

// ------------------------------ Utilities ------------------------

/**
 * @brief Internal helper for reading from a memory buffer.
 */
static inline int           dsgetc_buffer(const char *ptr, size_t *pos) {
    if (ptr[*pos] == '\0') {
        return EOF;
    }
    return (unsigned char) ptr[(*pos)++];
}

/**
 * @brief Internal helper for mutable string unget.
 */
static inline int           dsreplace_str(char *ptr, size_t *pos, int c) {
    if (*pos > 0)
        return ptr[--(*pos)] = (unsigned char) c;
    else
        return EOF;
}

/**
 * @brief Internal helper for constant string unget (conditional rollback).
 */
static inline int           dsungetc_conststr(const char *ptr, size_t *pos, int c) {
    if (*pos > 0 && (unsigned char) ptr[*pos - 1] == (unsigned char) c) {
        (*pos)--;
        return c;
    }
    return EOF;
}

/**
 * @brief Escapes non-printable characters for debug output.
 * @return Number of characters printed.
 */
static int                  ds_escape_print(FILE *out, unsigned char c) {
    switch (c) {
        case '\n': return fprintf(out, "\\n");
        case '\r': return fprintf(out, "\\r");
        case '\t': return fprintf(out, "\\t");
        case '\0': return fprintf(out, "\\0");
        default:
            if (isprint(c)) {
                fputc(c, out);
                return 1;
            } else {
                return fprintf(out, "[0x%02X]", c);
            }
    }
}

/**
 * @brief Helper to print the buffer content and closing delimiters.
 * @return Total characters written to 'out'.
 */
static int                  ds_print_buffer_content(FILE *out, const char *ptr, size_t start, size_t end) {
    int total = 0;
    for (size_t i = start; ptr[i] != '\0' && (end ==0 || i < end); i++) {
        total += ds_escape_print(out, (unsigned char) ptr[i]);
    }
    return total;
}

// --------------------------- API ---------------------------------

// -------------------- CONSTRUCTOTS/DESTRUCTORS -------------------

void                        dsInitf(Ds *ds, FILE *fp) {
    if (ds == NULL || fp == NULL)
        return;
    ds->type = DS_FILE;
    ds->fp = fp;
}

void                        dsInitstr(Ds *ds, char *buf) {
    if (ds == NULL || buf == NULL)
        return;
    ds->type = DS_STR;
    ds->ptr = buf;
    ds->pos = 0;
}

void                        dsInitconst(Ds *ds, const char *buf) {
    if (ds == NULL || buf == NULL)
        return;
    ds->type = DS_CONSTSTR;
    ds->constptr = buf;
    ds->pos = 0;
}

// --------------------- ACCESS AND MODIFICATION --------------------

int                         dsgetc(Ds *ds) {
    switch (ds->type) {
        case DS_FILE:
            return fgetc(ds->fp);
        case DS_STR:
            return dsgetc_buffer(ds->ptr, &ds->pos);
        case DS_CONSTSTR:
            return dsgetc_buffer(ds->constptr, &ds->pos);
        default:
            return EOF;
    }
}

int                         dsungetc(int c, Ds *ds) {
    if (c == EOF)   // NOT SURE, LET IT BE FOR NOW
        return EOF;
    switch (ds->type) {
        case DS_FILE:
            return ungetc(c, ds->fp);
        case DS_STR:
        case DS_CONSTSTR: {
            const char *ptr = (ds->type == DS_STR) ? 
                               ds->ptr : ds->constptr;
            return dsungetc_conststr(ptr, &ds->pos, c);
        }
        default:
            return EOF;
    }
}

// only for DS_STR
int                        dsreplacec(int c, Ds *ds) {
    if (c == EOF)
        return EOF;
    switch (ds->type) {
        case DS_STR:
            return dsreplace_str(ds->ptr, &ds->pos, c);
        default:
            return EOF;
    }
}

int                         dsTechFPrint(FILE *restrict out, const Ds *restrict ds) {
    if (!ds || !out) 
        return -1;

    int total = 0;

    switch (ds->type) {
        case DS_FILE:
            total += fprintf(out, "[DS_FILE] %p\n", ds->fp);
            break;
        case DS_STR:
        case DS_CONSTSTR: {
            const char *ptr = (ds->type == DS_STR) ? 
                               ds->ptr : ds->constptr;
            total += fprintf(out, "[DS_%s] pos=%zu, data=\"", 
                    (ds->type == DS_STR) ? "STR" : "CONSTSTR", ds->pos);
            //
            total += ds_print_buffer_content(out, ptr, 0, ds->pos);
            total += fprintf(out, "\" => \"");
            total += ds_print_buffer_content(out, ptr, ds->pos, 0);
            total += fprintf(out, "\"\n");
            break;
        }
        case DS_FS:
            total += fprintf(out, "[DS_FS (Not supported)]\n");
            break;
        default:
            total += fprintf(out, "[DS_UNKNOWN]\n");
            break;
    }
    return total;
}

// -------------------------- (API) printers -----------------------

// -------------------------------Testing --------------------------

#ifdef DS_TESTING

#include "test.h"
#include "checker.h"

// ------------------------- TEST Ds (DataSource) -------------------------
static TestStatus
tf_ds(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== 1. Строковый источник: чтение ========== */
    test_sub("subtest %d: dsgetc from string", ++subnum);
    {
        Ds ds;
        char text[] = "Hello";
        dsInitstr(&ds, text);

        test_validate(dsgetc(&ds) == 'H', "First char must be 'H'");
        test_validate(dsgetc(&ds) == 'e', "Second char must be 'e'");
        test_validate(dsgetc(&ds) == 'l', "Third char must be 'l'");
        test_validate(dsgetc(&ds) == 'l', "Fourth char must be 'l'");
        test_validate(dsgetc(&ds) == 'o', "Fifth char must be 'o'");
        test_validate(dsgetc(&ds) == EOF, "After end must be EOF");
    }

    /* ========== 2. dsungetc для строки ========== */
    test_sub("subtest %d: dsungetc for string", ++subnum);
    {
        Ds ds;
        char text[] = "AB";
        dsInitstr(&ds, text);

        int c1 = dsgetc(&ds);  // 'A'
        test_validate(c1 == 'A', "First char must be 'A'");
        int ret = dsungetc(c1, &ds);
        test_validate(ret == 'A', "dsungetc must return 'A'");
        int c2 = dsgetc(&ds);  // снова 'A'
        test_validate(c2 == 'A', "After ungetc, char must be 'A' again");
        int c3 = dsgetc(&ds);  // 'B'
        test_validate(c3 == 'B', "Next char must be 'B'");
    }

    /* ========== 3. Константный строковый источник ========== */
    test_sub("subtest %d: const string source", ++subnum);
    {
        Ds ds;
        const char *text = "World";
        dsInitconst(&ds, text);

        test_validate(dsgetc(&ds) == 'W', "First char must be 'W'");
        test_validate(dsgetc(&ds) == 'o', "Second char must be 'o'");
        test_validate(dsgetc(&ds) == 'r', "Third char must be 'r'");
        test_validate(dsgetc(&ds) == 'l', "Fourth char must be 'l'");
        test_validate(dsgetc(&ds) == 'd', "Fifth char must be 'd'");
        test_validate(dsgetc(&ds) == EOF, "After end must be EOF");
    }

    /* ========== 4. dsungetc для константной строки ========== */
    test_sub("subtest %d: dsungetc for const string", ++subnum);
    {
        Ds ds;
        const char *text = "XY";
        dsInitconst(&ds, text);

        int c1 = dsgetc(&ds);  // 'X'
        test_validate(c1 == 'X', "First char must be 'X'");
        dsungetc(c1, &ds);
        int c2 = dsgetc(&ds);
        test_validate(c2 == 'X', "After ungetc, char must be 'X' again");
    }

    /* ========== 5. Файловый источник: чтение ========== */
    test_sub("subtest %d: file source read", ++subnum);
    {
        const char *fname = "res/ds/test_ds_file.ds";
        FILE *fp = fopen(fname, "w");
        test_validate(fp != NULL, "Failed to create temporary file");
        fprintf(fp, "Test");
        fclose(fp);

        fp = fopen(fname, "r");
        Ds ds;
        dsInitf(&ds, fp);
        test_validate(dsgetc(&ds) == 'T', "First char must be 'T'");
        test_validate(dsgetc(&ds) == 'e', "Second char must be 'e'");
        test_validate(dsgetc(&ds) == 's', "Third char must be 's'");
        test_validate(dsgetc(&ds) == 't', "Fourth char must be 't'");
        test_validate(dsgetc(&ds) == EOF, "After end must be EOF");
        fclose(fp);
    }

    /* ========== 6. dsungetc для файла ========== */
    test_sub("subtest %d: dsungetc for file", ++subnum);
    {
        const char *fname = "res/ds/test_ds_ungetc.ds";
        FILE *fp = fopen(fname, "w");
        fprintf(fp, "Z");
        fclose(fp);

        fp = fopen(fname, "r");
        Ds ds;
        dsInitf(&ds, fp);
        int c = dsgetc(&ds);
        test_validate(c == 'Z', "Char must be 'Z'");
        dsungetc(c, &ds);
        int c2 = dsgetc(&ds);
        test_validate(c2 == 'Z', "After ungetc, char must be 'Z' again");
        fclose(fp);
    }

    /* ========== 7. dsTechFPrint для строки (вывод в файл) ========== */
    test_sub("subtest %d: dsTechFPrint for string", ++subnum);
    {
        Ds ds;
        char text[] = "Data";
        dsInitstr(&ds, text);
        dsgetc(&ds);  // 'D'
        dsgetc(&ds);  // 'a'

        const char *fname = "res/ds/test_techprint.ds";
        FILE *out = fopen(fname, "w");
        dsTechFPrint(out, &ds);
        fclose(out);

        char buf[128];
        FILE *in = fopen(fname, "r");
        size_t n = fread(buf, 1, sizeof(buf) - 1, in);
        buf[n] = '\0';
        fclose(in);

        test_validate(strstr(buf, "DS_STR") != NULL, "Techprint must contain 'DS_STR'");
        test_validate(strstr(buf, "\"Da\" => \"ta\"") != NULL, "Techprint must show read part 'Da' and remaining 'ta'");
    }

    /* ========== 8. Граничные случаи ========== */
    test_sub("subtest %d: empty string", ++subnum);
    {
        Ds ds;
        char text[] = "";
        dsInitstr(&ds, text);
        test_validate(dsgetc(&ds) == EOF, "Empty string must return EOF immediately");
    }

    test_sub("subtest %d: dsungetc on empty string (buffer underflow)", ++subnum);
    {
        Ds ds;
        char text[] = "X";
        dsInitstr(&ds, text);
        int c = dsgetc(&ds);  // 'X'
        // пробуем вернуть символ, когда позиция уже в начале (после возврата)
        dsungetc(c, &ds);
        // повторный ungetc должен вернуть EOF или последний символ? По реализации dsungetc_str возвращает EOF если pos==0.
        // после первого ungetc pos стало 0, второй вызов:
        int ret = dsungetc('Y', &ds);
        test_validate(ret == EOF, "Second consecutive dsungetc on string must return EOF");
        // но первый ungetc был успешен, поэтому 'X' снова доступен
        int c2 = dsgetc(&ds);
        test_validate(c2 == 'X', "After first ungetc, char must be 'X' again");
    }

    /* ========== 9. dsreplacec ========== */
    test_sub("subtest %d: dsreplacec basic", ++subnum);
    {
        Ds ds;
        char text[] = "Hello";
        dsInitstr(&ds, text);

        int c1 = dsgetc(&ds);  // 'H'
        test_validate(c1 == 'H', "First char must be 'H'");
        // заменяем 'H' на 'X'
        int ret = dsreplacec('X', &ds);
        test_validate(ret == 'X', "dsreplacec must return 'X'");
        // перечитываем с той же позиции
        int c2 = dsgetc(&ds);
        test_validate(c2 == 'X', "After replace, char must be 'X'");
        // следующий символ должен быть 'e'
        int c3 = dsgetc(&ds);
        test_validate(c3 == 'e', "Next char must be 'e'");
    }

    test_sub("subtest %d: dsreplacec with EOF", ++subnum);
    {
        Ds ds;
        char text[] = "AB";
        dsInitstr(&ds, text);
        dsgetc(&ds);  // 'A'
        // замена на EOF должна вернуть EOF и не менять позицию
        int ret = dsreplacec(EOF, &ds);
        test_validate(ret == EOF, "dsreplacec(EOF) must return EOF");
        // повторное чтение должно вернуть 'B'
        int c = dsgetc(&ds);
        test_validate(c == 'B', "Next char must be 'B'");
    }

    test_sub("subtest %d: dsreplacec at pos 0", ++subnum);
    {
        Ds ds;
        char text[] = "XY";
        dsInitstr(&ds, text);
        // позиция 0, замена невозможна
        int ret = dsreplacec('Z', &ds);
        test_validate(ret == EOF, "dsreplacec at pos 0 must return EOF");
        // первое чтение должно вернуть 'X'
        int c = dsgetc(&ds);
        test_validate(c == 'X', "First char must be 'X'");
    }

    test_sub("subtest %d: dsreplacec on non‑string sources", ++subnum);
    {
        // файловый источник не поддерживает замену
        const char *fname = "res/ds/test_replace.ds";
        FILE *fp = fopen(fname, "w");
        fprintf(fp, "File");
        fclose(fp);

        fp = fopen(fname, "r");
        Ds ds;
        dsInitf(&ds, fp);
        int c = dsgetc(&ds);  // 'F'
        test_validate(c == 'F', "First file char must be 'F'");
        int ret = dsreplacec('G', &ds);
        test_validate(ret == EOF, "dsreplacec on file must return EOF");
        fclose(fp);

        // константная строка также не поддерживает замену
        const char *ctext = "Const";
        dsInitconst(&ds, ctext);
        c = dsgetc(&ds);  // 'C'
        test_validate(c == 'C', "First const char must be 'C'");
        ret = dsreplacec('D', &ds);
        test_validate(ret == EOF, "dsreplacec on const string must return EOF");
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST Ds additional functions -------------------------
static TestStatus
tf_ds_extra(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. dsStrbuf – строка */
    test_sub("subtest %d: dsStrbuf on DS_STR", ++subnum);
    {
        char text[] = "Hello";
        Ds ds = dsCreatestr(text);
        const char *p = dsStrbuf(&ds);
        test_validate(p == text, "dsStrbuf must return original buffer pointer");
    }

    /* 2. dsStrbuf – константная строка */
    test_sub("subtest %d: dsStrbuf on DS_CONSTSTR", ++subnum);
    {
        const char *text = "World";
        Ds ds = dsCreateconst(text);
        const char *p = dsStrbuf(&ds);
        test_validate(p == text, "dsStrbuf must return original const pointer");
    }

    /* 3. dsStrbuf – файловый источник должен вернуть NULL */
    test_sub("subtest %d: dsStrbuf on DS_FILE returns NULL", ++subnum);
    {
        const char *fname = "res/ds/tmp_strbuf.ds";
        FILE *fp = fopen(fname, "w");
        fprintf(fp, "ignored");
        fclose(fp);

        fp = fopen(fname, "r");
        Ds ds = dsCreatef(fp);
        const char *p = dsStrbuf(&ds);
        test_validate(p == NULL, "dsStrbuf on file must return NULL");
        fclose(fp);
    }

    /* 4. dsIsstr – строковые источники */
    test_sub("subtest %d: dsIsstr true for DS_STR and DS_CONSTSTR", ++subnum);
    {
        Ds ds1 = dsCreatestr("a");
        Ds ds2 = dsCreateconst("b");
        test_validate(dsIsstr(&ds1) && dsIsstr(&ds2), "Both DS_STR and DS_CONSTSTR must be recognised as strings");
    }

    /* 5. dsIsstr – файловый источник не строка */
    test_sub("subtest %d: dsIsstr false for DS_FILE", ++subnum);
    {
        const char *fname = "res/ds/tmp_isstr.ds";
        FILE *fp = fopen(fname, "w");
        fprintf(fp, "x");
        fclose(fp);

        fp = fopen(fname, "r");
        Ds ds = dsCreatef(fp);
        test_validate(!dsIsstr(&ds), "File source must not be a string");
        fclose(fp);
    }

    /* 6. dsSavepos / dsRestorepos – строковый источник */
    test_sub("subtest %d: dsSavepos/dsRestorepos on DS_STR", ++subnum);
    {
        char text[] = "ABCDEF";
        Ds ds = dsCreatestr(text);
        // читаем несколько символов
        dsgetc(&ds); // 'A'
        dsgetc(&ds); // 'B'
        dsSavepos(&ds);
        dsgetc(&ds); // 'C'
        dsgetc(&ds); // 'D'
        dsRestorepos(&ds);
        int c = dsgetc(&ds); // должно быть 'C'
        test_validate(c == 'C', "After restore, next char must be 'C', got '%c'", c);
    }

    /* 7. dsSavepos / dsRestorepos – файловый источник */
    test_sub("subtest %d: dsSavepos/dsRestorepos on DS_FILE", ++subnum);
    {
        const char *fname = "res/ds/tmp_savepos.ds";
        FILE *fp = fopen(fname, "w");
        fprintf(fp, "12345");
        fclose(fp);

        fp = fopen(fname, "r");
        Ds ds = dsCreatef(fp);
        dsgetc(&ds); // '1'
        dsgetc(&ds); // '2'
        dsSavepos(&ds);
        dsgetc(&ds); // '3'
        dsgetc(&ds); // '4'
        dsRestorepos(&ds);
        int c = dsgetc(&ds); // должно быть '3'
        test_validate(c == '3', "After restore on file, next char must be '3', got '%c'", c);
        fclose(fp);
    }

    /* 8. dsRestorepos без предварительного Save – не ломается, просто возвращается в начало? */
    test_sub("subtest %d: dsRestorepos without Save (DS_STR)", ++subnum);
    {
        char text[] = "XYZ";
        Ds ds = dsCreatestr(text);
        dsgetc(&ds); // 'X'
        dsgetc(&ds); // 'Y'
        dsRestorepos(&ds); // без Save – поведение не определено, но не должно падать
        // просто проверяем, что не краш
        int c = dsgetc(&ds);
        test_validate(c == 'X', "Restore without save must not crash");
    }
    /* 9. dsRestorepos without Save (DS_FILE) */
    test_sub("subtest %d: dsRestorepos without Save (DS_FILE)", ++subnum);
    {
        const char *fname = "res/ds/tmp_restore_nosave.ds";
        FILE *fp = fopen(fname, "w");
        fprintf(fp, "ABC");
        fclose(fp);

        fp = fopen(fname, "r");
        Ds ds = dsCreatef(fp);
        dsgetc(&ds); // 'A'
        dsgetc(&ds); // 'B'
        dsRestorepos(&ds); // без Save – filesavepos == 0
        int c = dsgetc(&ds); // должно быть 'A'
        test_validate(c == 'A', "Restore without save on file must reset to beginning, got '%c'", c);
        fclose(fp);
    }

    return logret(TEST_PASSED, "done");
}

// -------------------------------------------------------------------
int
main( /*int argc, char *argv[] */ )
{
    logsimpleinit("Start");

    testenginestd(
        TESTADD(tf_ds,           "Ds (DataSource) simple tests"),
        TESTADD(tf_ds_extra,     "Ds additional functions")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* DS_TESTING */
