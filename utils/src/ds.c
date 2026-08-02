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
static inline int           dsungetc_str(char *ptr, size_t *pos, int c) {
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
    for (size_t i = start; i < end; i++) {
        total += ds_escape_print(out, (unsigned char) ptr[i]);
    }
    return total;
}


// --------------------------- API ---------------------------------

// -------------------- CONSTRUCTOTS/DESTRUCTORS -------------------

void                        dsInitf(Ds *ds, FILE *fp) {
    ds->type = DS_FILE;
    ds->source.fp = fp;
}

void                        dsInitstr(Ds *ds, char *buf) {
    ds->type = DS_STR;
    ds->source.buf.ptr = buf;
    ds->source.buf.pos = 0;
}

void                        dsInitconst(Ds *ds, const char *buf) {
    ds->type = DS_CONSTSTR;
    ds->source.constbuf.ptr = buf;
    ds->source.constbuf.pos = 0;
}

// --------------------- ACCESS AND MODIFICATION --------------------

int                         dsgetc(Ds *ds) {
    switch (ds->type) {
        case DS_FILE:
            return fgetc(ds->source.fp);
        case DS_STR:
            return dsgetc_buffer(ds->source.buf.ptr, &ds->source.buf.pos);
        case DS_CONSTSTR:
            return dsgetc_buffer(ds->source.constbuf.ptr, &ds->source.constbuf.pos);
        default:
            return EOF;
    }
}

int                         dsungetc(int c, Ds *ds) {
    if (c == EOF)   // NOT SURE, LET IT BE FOR NOW
        return EOF;
    switch (ds->type) {
        case DS_FILE:
            return ungetc(c, ds->source.fp);
        case DS_STR:
            return dsungetc_str(ds->source.buf.ptr, &ds->source.buf.pos, c);
        case DS_CONSTSTR:
            return dsungetc_conststr(ds->source.constbuf.ptr, &ds->source.constbuf.pos, c);
        default:
            return EOF;
    }
}


/**
 * @brief Implementation of technical debug print.
 * @return Total bytes printed to 'out'.
 */
int dsTechFPrint(FILE *restrict out, const Ds *restrict ds, int printbufcnt) {
    if (!ds || !out) return -1;

    int total = 0;

    switch (ds->type) {
        case DS_FILE:
            return fprintf(out, "[DS_FILE]\n");

        case DS_STR:
        case DS_CONSTSTR: {
            const char *ptr = (ds->type == DS_STR) ? 
                               ds->source.buf.ptr : ds->source.constbuf.ptr;
            size_t start_pos = (ds->type == DS_STR) ? 
                               ds->source.buf.pos : ds->source.constbuf.pos;
            size_t end_pos;

            // Determine print range
            if (printbufcnt == 0) {
                // Print entire buffer from start to null
                end_pos = 0;
                while (ptr[end_pos] != '\0') end_pos++;
            } else {
                // Print up to printbufcnt characters from current position
                end_pos = start_pos + printbufcnt;
                // Safety check: don't read past null or end of string
                while (end_pos > start_pos && ptr[end_pos - 1] == '\0') end_pos--;
            }

            total += fprintf(out, "[DS_%s] pos=%zu, data=\"", 
                    (ds->type == DS_STR) ? "STR" : "CONSTSTR", start_pos);
            total += ds_print_buffer_content(out, ptr, start_pos, end_pos);
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

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d: get values", ++subnum);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// -------------------------------------------------------------------
int
main( /*int argc, char *argv[] */ )
{
    logsimpleinit("Start");

    testenginestd(
        TESTADD(tf1,                            "Int/double creation/descr test")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* DS_TESTING */
