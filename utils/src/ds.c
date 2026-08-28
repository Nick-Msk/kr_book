#include "ds.h"

/********************************************************************
                 Datasource MODULE IMPLEMENTATION
********************************************************************/

// ------------------------------ Utilities ------------------------

/**
 * @brief Internal helper for reading from a memory buffer.
 */
static inline int           dsgetc_buffer(const char *restrict ptr, size_t *restrict pos) {
    if (ptr[*pos] == '\0') {
        return EOF;
    }
    return (unsigned char) ptr[(*pos)++];
}

/**
 * @brief Internal helper for mutable string unget.
 */
static inline int           dsreplace_str(char *restrict ptr, size_t *restrict pos, int c) {
    if (*pos > 0)
        return ptr[--(*pos)] = (unsigned char) c;
    else
        return EOF;
}

/**
 * @brief Internal helper for mutable string put.
 */
static inline int           dsputc_strbuf(char *restrict ptr, size_t *restrict pos, int c) {
    if (ptr[*pos] != '\0')
        return ptr[(*pos)++] = (unsigned char) c;
    else
        return EOF;
}

/**
 * @brief Internal helper for constant string unget (conditional rollback).
 */
static inline int           dsungetc_conststr(const char *restrict ptr, size_t *restrict pos, int c) {
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
static int                  ds_print_buffer_content(FILE *restrict out, const char *restrict ptr, size_t start, size_t end) {
    int total = 0;
    for (size_t i = start; ptr[i] != '\0' && (end ==0 || i < end); i++) {
        total += ds_escape_print(out, (unsigned char) ptr[i]);
    }
    return total;
}

// --------------------------- API ---------------------------------

// -------------------- CONSTRUCTOTS/DESTRUCTORS -------------------

bool                        dsInitf(DS *restrict pds, FILE *restrict fp) {
    if (pds == NULL || fp == NULL)
        return false;
    pds->type = DS_FILE;
    pds->fp = fp;
    return true;
}

bool                        dsInitstrCap(DS *restrict pds, char *restrict buf, size_t cap) {
    if (pds == NULL || buf == NULL)
        return false;
    pds->type = DS_STR;
    pds->ptr = buf;
    if (cap != 0L) { // write or read/write => need to empty buffer
        pds->cap = cap;
        // memset(pds->ptr, '\0', cap);
    } else
        pds->cap = strlen(buf);
    pds->pos = 0;
    return true;
}

bool                        dsInitconst(DS *restrict pds, const char *restrict buf) {
    if (pds == NULL || buf == NULL)
        return false;
    pds->type = DS_CONSTSTR;
    pds->constptr = buf;
    pds->cap = strlen(buf); // MUST BE '\0' at the EOL
    pds->pos = 0;
    return true;
}

#ifndef NO_FSDS
    bool                    dsInitfs(DS *restrict pds, fs *restrict s) {
        if (pds == NULL || s == NULL)
            return false;
        pds->type = DS_FS;
        pds->pos = 0;        // iterator
        pds->s = fs_move(s);    // clear s
        return true;
    }
#endif  /* !NO_FSDS */   

// --------------------- ACCESS AND MODIFICATION --------------------

int                         dsgetc(DS *pds) {
    switch (pds->type) {
        case DS_FILE:
            return fgetc(pds->fp);
        case DS_STR:
            return dsgetc_buffer(pds->ptr, &pds->pos);
        case DS_CONSTSTR:
            return dsgetc_buffer(pds->constptr, &pds->pos);
        case DS_FS:
#ifndef NO_FSDS
            if (fsstr(pds->s))
                return dsgetc_buffer(pds->s.v, &pds->pos);
            else
                return EOF;
#else
            return EOF;
#endif  /* !NO_FSDS */ 
    }
}

int                             dsungetc(int c, DS *pds) {
    if (c == EOF)   // NOT SURE, LET IT BE FOR NOW
        return EOF;
    switch (pds->type) {
        case DS_FILE:
            return ungetc(c, pds->fp);
        case DS_STR:
#ifndef NO_FSDS
        case DS_FS:
#endif  /* !NO_FSDS */ 
        case DS_CONSTSTR: {
            const char *ptr = dsStrbuf(pds);
            // the same logic for FS, STR and CONSTSTR
            return dsungetc_conststr(ptr, &pds->pos, c);
        }
        default:      
            //*elemnull(pds->s, pds->pos++) = (unsigned char) c;
            return EOF;
    }
}

// only for DS_STR and DS_FS
int                         dsreplacec(int c, DS *pds) {
    if (c == EOF)
        return EOF;
    switch (pds->type) {
        case DS_STR:
#ifndef NO_FSDS
        case DS_FS:
#endif  /* !NO_FSDS */      
            char *ptr = (char *) dsStrbuf(pds);
            // the same logic for FS and STR
            return dsreplace_str(ptr, &pds->pos, c);
        default:
            return EOF;
    }
}

int                         dsputc(int c, DS *pds) {
    if (c == EOF)
        return EOF;
    switch (pds->type) {
        case DS_CONSTSTR:
            return EOF;     // N/A
        case DS_STR:
            return dsputc_strbuf(pds->ptr, &pds->pos, c);
        case DS_FILE:
            return fputc(c, pds->fp);
        case DS_FS:
#ifndef NO_FSDS
            return *(fs_elem0(&pds->s, pds->pos++) ) = c;   // autoextend with final '\0'
#else
        default:
            return EOF;
#endif  /* !NO_FSDS */  
    }
}

int                         dsTechFPrint(FILE *restrict out, const DS *restrict pds, const char *restrict name) {
    if (!pds || !out) 
        return -1;

    int total = 0;

    switch (pds->type) {
        case DS_FILE:
            IOCHECKERSIMPLE(written, fprintf(out, "[DS_FILE %s] %p\n", name ? name : "", pds->fp), -1)
                total += written;
            break;
        case DS_STR:
        case DS_CONSTSTR: {
            const char *ptr = dsStrbuf(pds);
            IOCHECKERSIMPLE(written, fprintf(out, "[DS_%s %s] pos=%zu, cap=%zu, data=\"", 
                    (pds->type == DS_STR) ? "STR" : "CONSTSTR", 
                        name ? name : "", pds->pos, pds->type == DS_STR ? pds->cap : 0), -1)
                    total += written;
            //
            IOCHECKERSIMPLE(written, ds_print_buffer_content(out, ptr, 0, pds->pos), -1)
                total += written;
            IOCHECKERSIMPLE(written, fprintf(out, "\" => \""), -1)
                total += written;
            IOCHECKERSIMPLE(written, ds_print_buffer_content(out, ptr, pds->pos, 0), -1)
                total += written;
            IOCHECKERSIMPLE(written, fprintf(out, "\"\n"), -1)
                total += written;
            break;
        }
        case DS_FS:
#ifndef NO_FSDS
            IOCHECKERSIMPLE(written, fprintf(out, "[DS_FS %s] pos=%zu ", name ? name : "", pds->pos), -1)
                total += written;
            IOCHECKERSIMPLE(written, fs_techfprint(out, &pds->s, NULL), -1)
                total += written;
#else
            IOCHECKERSIMPLE(written, fprintf(out, "[DS_FS %s (Not supported)]\n", name ? name : ""), -1)
                total += written;
#endif  /* !NO_FSDS */ 
            break;
        default:
            IOCHECKERSIMPLE(written, fprintf(out, "[DS_UNKNOWN]\n"), -1)
                total += written;
            break;
    }
    return total;
}

// -------------------------- (API) printers -----------------------

// -------------------------------Testing --------------------------

#ifdef DS_TESTING

#include "test.h"
#include "checker.h"

// ------------------------- TEST DS (DataSource) -------------------------
static TestStatus
tf_ds(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== 1. Строковый источник: чтение ========== */
    test_sub("subtest %d: dsgetc from string", ++subnum);
    {
        DS ds;
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
        DS ds;
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
        DS ds;
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
        DS ds;
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
        DS ds;
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
        DS ds;
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
        DS ds;
        char text[] = "Data";
        dsInitstr(&ds, text);
        dsgetc(&ds);  // 'D'
        dsgetc(&ds);  // 'a'

        const char *fname = "res/ds/test_techprint.ds";
        FILE *out = fopen(fname, "w");
        DSTECHFPRINT(out, ds);
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
        DS ds;
        char text[] = "";
        dsInitstr(&ds, text);
        test_validate(dsgetc(&ds) == EOF, "Empty string must return EOF immediately");
    }

    test_sub("subtest %d: dsungetc on empty string (buffer underflow)", ++subnum);
    {
        DS ds;
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
        DS ds;
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
        DS ds;
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
        DS ds;
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
        DS ds;
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

    /* ========== dsReset ========== */
    test_sub("subtest %d: dsReset on DS_STR", ++subnum);
    {
        char text[] = "XYZ";
        DS ds = dsCreatestr(text);
        dsgetc(&ds); // 'X'
        dsgetc(&ds); // 'Y'
        dsReset(&ds);
        int c = dsgetc(&ds); // должно быть 'X'
        test_validate(c == 'X', "After reset, next char must be 'X', got '%c'", c);
    }

    test_sub("subtest %d: dsReset on DS_FILE", ++subnum);
    {
        const char *fname = "res/ds/tmp_reset.ds";
        FILE *fp = fopen(fname, "w");
        fprintf(fp, "ABC");
        fclose(fp);

        fp = fopen(fname, "r");
        DS ds = dsCreatef(fp);
        dsgetc(&ds); // 'A'
        dsgetc(&ds); // 'B'
        dsReset(&ds);
        int c = dsgetc(&ds); // должно быть 'A'
        test_validate(c == 'A', "After reset on file, next char must be 'A', got '%c'", c);
        fclose(fp);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST DS additional functions -------------------------
static TestStatus
tf_ds_extra(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. dsStrbuf – строка */
    test_sub("subtest %d: dsStrbuf on DS_STR", ++subnum);
    {
        char text[] = "Hello";
        DS ds = dsCreatestr(text);
        const char *p = dsStrbuf(&ds);
        test_validate(p == text, "dsStrbuf must return original buffer pointer");
    }

    /* 2. dsStrbuf – константная строка */
    test_sub("subtest %d: dsStrbuf on DS_CONSTSTR", ++subnum);
    {
        const char *text = "World";
        DS ds = dsCreateconst(text);
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
        DS ds = dsCreatef(fp);
        const char *p = dsStrbuf(&ds);
        test_validate(p == NULL, "dsStrbuf on file must return NULL");
        fclose(fp);
    }

    /* 4. dsIsstr – строковые источники */
    test_sub("subtest %d: dsIsstr true for DS_STR and DS_CONSTSTR", ++subnum);
    {
        DS ds1 = dsCreatestr("a");
        DS ds2 = dsCreateconst("b");
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
        DS ds = dsCreatef(fp);
        test_validate(!dsIsstr(&ds), "File source must not be a string");
        fclose(fp);
    }

    /* 6. dsSavepos / dsRestorepos – строковый источник */
    test_sub("subtest %d: dsSavepos/dsRestorepos on DS_STR", ++subnum);
    {
        char text[] = "ABCDEF";
        DS ds = dsCreatestr(text);
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
        DS ds = dsCreatef(fp);
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
        DS ds = dsCreatestr(text);
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
        DS ds = dsCreatef(fp);
        dsgetc(&ds); // 'A'
        dsgetc(&ds); // 'B'
        dsRestorepos(&ds); // без Save – filesavepos == 0
        int c = dsgetc(&ds); // должно быть 'A'
        test_validate(c == 'A', "Restore without save on file must reset to beginning, got '%c'", c);
        fclose(fp);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST dsputc (output) -------------------------
static TestStatus
tf_ds_putc(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. DS_FILE: запись нескольких символов */
    test_sub("subtest %d: dsputc to file", ++subnum);
    {
        const char  *fname = "res/ds/test_putc_file.ds";
        FILE        *fp = fopen(fname, "w");
        DS           ds = dsCreatef(fp);
        int          c;

        c = dsputc('A', &ds);
        test_validate(c == 'A', "dsputc 'A' must return 'A', got '%c'", c);
        c = dsputc('B', &ds);
        test_validate(c == 'B', "dsputc 'B' must return 'B', got '%c'", c);
        c = dsputc('C', &ds);
        test_validate(c == 'C', "dsputc 'C' must return 'C', got '%c'", c);
        fclose(fp);

        // проверяем содержимое файла
        fp = fopen(fname, "r");
        char buf[8] = {0};
        fread(buf, 1, 3, fp);
        fclose(fp);
        test_validate(strcmp(buf, "ABC") == 0,
                      "File must contain 'ABC', got '%s'", buf);
    }

    /* 2. DS_STR: запись в mutable строку */
    test_sub("subtest %d: dsputc to mutable string", ++subnum);
    {
        char text[10] = ".........";
        DS ds = dsCreatestr(text);
        int c;

        c = dsputc('X', &ds);
        test_validate(c == 'X', "dsputc 'X' must return 'X', got '%c'", c);
        c = dsputc('Y', &ds);
        test_validate(c == 'Y', "dsputc 'Y' must return 'Y', got '%c'", c);
        c = dsputc('Z', &ds);
        test_validate(c == 'Z', "dsputc 'Z' must return 'Z', got '%c'", c);
        test_validate(text[0] == 'X' && text[1] == 'Y' && text[2] == 'Z',
                      "String must contain 'XYZ', got '%c%c%c'",
                      text[0], text[1], text[2]);
        test_validate(ds.pos == 3, "pos must be 3, got %zu", ds.pos);
    }

    /* 3. DS_STR: попытка записи за нуль-терминатор */
    test_sub("subtest %d: dsputc past null terminator returns EOF", ++subnum);
    {
        char text[5] = "AB";
        DS ds = dsCreatestr(text);
        ds.pos = 2;                   // встали на нуль-терминатор
        int c = dsputc('X', &ds);
        test_validate(c == EOF,
                      "dsputc past null terminator must return EOF, got '%c'", c);
    }

    /* 4. DS_CONSTSTR: запись запрещена */
    test_sub("subtest %d: dsputc to const string returns EOF", ++subnum);
    {
        const char *text = "const";
        DS ds = dsCreateconst(text);
        int c = dsputc('A', &ds);
        test_validate(c == EOF,
                      "dsputc on const string must return EOF, got '%c'", c);
    }

    /* 5. Передача EOF возвращает EOF */
    test_sub("subtest %d: dsputc(EOF) returns EOF", ++subnum);
    {
        DS ds = dsCreatef(stdout);   // любой валидный DS
        int c = dsputc(EOF, &ds);
        test_validate(c == EOF,
                      "dsputc(EOF) must return EOF, got '%c'", c);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST dsputc / dsgetc for DS_FS -------------------------
static TestStatus
tf_ds_fs(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

#ifndef NO_FSDS
    /* 1. DS_FS: запись нескольких символов и чтение (round-trip) */
    test_sub("subtest %d: dsputc/dsgetc round‑trip on FS", ++subnum);
    {
        fs s = FS();
        DS ds = dsCreatefs(&s);
        int c;

        // записываем три символа
        c = dsputc('A', &ds);
        test_validate(c == 'A', "dsputc 'A' must return 'A', got '%c'", c);
        c = dsputc('B', &ds);
        test_validate(c == 'B', "dsputc 'B' must return 'B', got '%c'", c);
        c = dsputc('C', &ds);
        test_validate(c == 'C', "dsputc 'C' must return 'C', got '%c'", c);

        // сбрасываем позицию на начало для чтения
        // ds.pos = 0;
        dsReset(&ds);

        c = dsgetc(&ds);
        test_validate(c == 'A', "dsgetc must read 'A', got '%c'", c);
        c = dsgetc(&ds);
        test_validate(c == 'B', "dsgetc must read 'B', got '%c'", c);
        c = dsgetc(&ds);
        test_validate(c == 'C', "dsgetc must read 'C', got '%c'", c);
        c = dsgetc(&ds);
        test_validate(c == EOF, "dsgetc after 'C' must return EOF, got '%c'", c);

        dsFree(&ds);
    }
    fs_alloc_check(true);

    /* 2. DS_FS: запись с авто‑расширением (fs_elem0) */
    test_sub("subtest %d: dsputc auto‑extend on FS", ++subnum);
    {
        fs s = FS();
        DS ds = dsCreatefs(&s);
        ds.pos = 5;
        int c = dsputc('X', &ds);
        test_validate(c == 'X', "dsputc at pos 5 must return 'X', got '%c'", c);

        // проверяем внутреннее состояние DS
        test_validate(ds.s.len == 6 && ds.s.v[5] == 'X' && ds.s.v[6] == '\0',
                    "FS len must be 6, str[5]='X', str[6]='\\0', got len=%zu, str='%s'",
                    ds.s.len, ds.s.v);
        dsFree(&ds);
    }
    fs_alloc_check(true);

    /* 3. DS_FS: попытка чтения за пределами len возвращает EOF */
    test_sub("subtest %d: dsgetc beyond len on FS returns EOF", ++subnum);
    {
        const char  pattern[] = "Test1";
        int         c; 
        fs          s = fscopy(pattern);
        DS          ds = dsCreatefs(&s);
        //ds.pos = 0;
        for (int i = 0; i < (int) strlen(pattern); i++) {
            c = dsgetc(&ds);
            test_validatefree(
                (unsigned char) c == pattern[i],
                dsFree(&ds),
                "%d elem must be '%c', got '%c", i, pattern[i], c
            );
        }
        c = dsgetc(&ds);
        test_validatefree(
            c == EOF, 
            dsFree(&ds),
            "dsgetc on empty FS must return EOF, got '%c'", c
        );
        dsFree(&ds);
    }
    fs_alloc_check(true);

    /* 4. DS_FS: попытка чтения за пределами len возвращает EOF */
    test_sub("subtest %d: dsgetc beyond len on EMPTY FS returns EOF", ++subnum);
    {
        fs s = FS();
        DS ds = dsCreatefs(&s);
        //ds.pos = 0;
        int c = dsgetc(&ds);
        test_validate(c == EOF, "dsgetc on empty FS must return EOF, got '%c'", c);
        dsFree(&ds);
    }
    fs_alloc_check(true);
#endif /* !NO_FSDS */

    return logret(TEST_PASSED, "done");
}

// -------------------------------------------------------------------
int
main( /*int argc, char *argv[] */ )
{
    logsimpleinit("Start");

    testenginestd(
        TESTADD(tf_ds,           "DS (DataSource) simple tests"),
        TESTADD(tf_ds_extra,     "DS additional functions"),
        TESTADD(tf_ds_putc,      "dsputc simple test"),
        TESTADD(tf_ds_fs,        "dsputc / dsgetc for DS_FS test")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* DS_TESTING */
