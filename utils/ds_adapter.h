/**
 * @file ds_fs.h
 * @brief Ds - fs adapter read/write.
 *
 * This module provides a @ref stdio/fs API via standartized Ds function
 * Ds - fs, ds -> FILE *.
 * DS_FILE, DS_FS       allowes to R/W (unlimited)
 * DS_CONSTSTR          alloes only R
 * DS_STR               alloes only R, and W only limited by '\0'. No reallocation.
 */

#ifndef _DS_ADAPTER_H
#define _DS_ADAPTER_H

#include <stdarg.h>

#include "ds.h"
#include "checker.h"
#include "common.h"

// ---------------------------------------------------------------------------------
// --------------------------- Public Ds - fs adapter API --------------------------
// ---------------------------------------------------------------------------------

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

// ---------------------------------- TYPES -----------------------------------------

// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

// -------------------- ACCESS AND MODIFICATORS -------------------------------------

/**
 * @brief Ds printf wrapper
 * @note DS_FILE, DS_FS, DS_STR are allowed
 * @note This is stream function, always used ds->pos as start point
 */
extern int                         dsPrintf(Ds *restrict ds, const char *restrict msg, ...) __attribute__ ((format (printf, 2, 3)));

/**
 * @brief Formatted input from a Ds source (scanf analog).
 *
 * DS_FILE uses vfscanf() directly.
 * DS_STR, DS_CONSTSTR and DS_FS use an internal helper that wraps
 * fmemopen() + vfscanf() + ftell() in order to automatically advance
 * the read position.
 *
 * @param pds pointer to the data source
 * @param msg scanf‑style format string
 * @param ...  pointers to store the parsed values
 * @return     number of successfully matched items, or -1 on error
 */
extern int                         dsScanf(Ds *restrict ds, const char *restrict msg, ...) __attribute__ ((format (scanf, 2, 3)));

// ------------------------ PRINTERS/CHECKERS ---------------------------------------

// ------------------------------------ ETC. ----------------------------------------


#endif /* !_DS_ADAPTER_H */
