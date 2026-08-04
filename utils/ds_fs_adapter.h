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

#ifndef _DS_FS_H
#define _DS_FS_H

#include "ds.h"
#include "checker.h"

// ---------------------------------------------------------------------------------
// --------------------------- Public Ds - fs adapter API --------------------------
// ---------------------------------------------------------------------------------

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

// ---------------------------------- TYPES -----------------------------------------

// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

// -------------------- ACCESS AND MODIFICATORS -------------------------------------

// Ds - fs adapter NOT SURE
typedef union {
    Ds ds;
    struct {
        DSType type;                /**< Determines which union member is active. */
        fs     s;
    };
} Dsfs;

/**
 * @brief Ds printf wrapper
 * @note DS_FILE, DS_FS, DS_STR are allowed
 * @note This is stream function, always used ds->pos as start point
 */
int                         dsPrintf(Ds *ds, const char *msg, ...) __attribute__ ((format (printf, 2, 3)));

/**
 * @brief Ds ысфта wrapper
 * @note DS_FILE, DS_FS, DS_CONSTSTR, DS_STR are allowed
 * @note This is stream function, always used ds->pos as start point
 */
int                         dsScanf(Ds *ds, const char *msg, ...) __attribute__ ((format (scanf, 2, 3)));

/**
 * @brief Stream reset function
 *//*
static inline bool          dsReset(Ds *ds) {
    invraisecode(ds != NULL, ERR_NULLABLE_PTR, "Null ds");
    switch (ds->type) {
        case DS_FILE:
            return ftell(ds->fp,  0L, SEEK_SET) == 0;
        case DS_FS: 

    }
}*/

// ------------------------ PRINTERS/CHECKERS ---------------------------------------

// ------------------------------------ ETC. ----------------------------------------


#endif /* !_DS_FS_H */
