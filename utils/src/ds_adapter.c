
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
 * @param buf   input string (null‑terminated)
 * @param ppos  pointer to the current read position, will be advanced
 * @param fmt   scanf‑style format string
 * @param ap    variadic argument list (as passed to dsScanf)
 * @return      number of successfully matched items, or EOF on error
 */
static int                  dsVScanf(const char *buf, size_t *ppos, const char *fmt, va_list ap) {
    int ret = 0;
    size_t pos = *ppos;

    // Первый проход: получаем количество успешных присваиваний
    va_list ap1;
    va_copy(ap1, ap);
    ret = vsscanf(buf + pos, fmt, ap1);
    va_end(ap1);

    if (ret > 0) {
        // Выделяем память под новый формат с " %n" в конце
        size_t fmt_len = strlen(fmt);
        char *new_fmt = malloc(fmt_len + 4); // " %n" + '\0'
        if (new_fmt) {
            memcpy(new_fmt, fmt, fmt_len);
            memcpy(new_fmt + fmt_len, " %n", 4);

            // Второй проход с обновлением consumed
            va_list ap2;
            va_copy(ap2, ap);
            int consumed = 0;
            vsscanf(buf + pos, new_fmt, ap2, &consumed);
            va_end(ap2);

            pos += consumed;
            *ppos = pos;
            free(new_fmt);
        }
    }
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
            ret = dsVScanf(buf, &pds->pos, msg, ap);
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

// ------------------------- TEST Ds (DataSource) -------------------------
static TestStatus
tf_ds(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    return logret(TEST_PASSED, "done");
}

// -------------------------------------------------------------------
int
main( /*int argc, char *argv[] */ )
{
    logsimpleinit("Start");

    testenginestd(
        TESTADD(tf_ds,           "Ds (DataSource) simple tests")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* DS_FS_ADAPTER_TESTING */
