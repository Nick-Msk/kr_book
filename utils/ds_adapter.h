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
extern int                         dsScanf(Ds *restrict pds, const char *restrict msg, ...) __attribute__ ((format (scanf, 2, 3)));

// ------------------------------- SCANNERS ----------------------------------------

/**
 * @brief Parses an integer from the provided data source.
 *
 * This function acts as an adapter that handles different data source types 
 * defined in the @c Ds structure. It supports:
 * - @c DS_FILE: Reads an integer from a file stream using @c fscanf.
 * - @c DS_STR, @c DS_FS, @c DS_CONSTSTR: Parses an integer from a memory buffer.
 *
 * @param[in,out] pds  Pointer to the @c Ds structure containing the data source.
 * @param[out] pval    Pointer to the integer where the parsed value will be stored.
 *
 * @return true if the integer was successfully parsed and stored in @p pval, 
 *         false otherwise.
 * 
 * @note For @c DS_FILE, the file position is advanced after a successful read.
 * @retval false if input pointers are NULL, the source type is unsupported, or parsing fails.
 */
extern bool                        dsParseInt(Ds *restrict pds, int *restrict val);

/**
 * @brief Parses an long from the provided data source.
 *
 * This function acts as an adapter that handles different data source types 
 * defined in the @c Ds structure. It supports:
 * - @c DS_FILE: Reads an integer from a file stream using @c fscanf.
 * - @c DS_STR, @c DS_FS, @c DS_CONSTSTR: Parses an integer from a memory buffer.
 *
 * @param[in,out] pds  Pointer to the @c Ds structure containing the data source.
 * @param[out] pval    Pointer to the long where the parsed value will be stored.
 *
 * @return true if the integer was successfully parsed and stored in @p pval, 
 *         false otherwise.
 * 
 * @note the position is advanced after a successful read.
 * @retval false if input pointers are NULL, the source type is unsupported, or parsing fails.
 */
extern bool                        dsParseLong(Ds *restrict pds, long *restrict pval);

/**
 * @brief Parses a unsigned int from the provided data source.
 *
 * This function acts as an adapter that handles different data source types 
 * defined in the @c Ds structure. It supports:
 * - @c DS_FILE: Reads an integer from a file stream using @c fscanf.
 * - @c DS_STR, @c DS_FS, @c DS_CONSTSTR: Parses an integer from a memory buffer.
 *
 * @param[in,out] pds  Pointer to the @c Ds structure containing the data source.
 * @param[out] pval    Pointer to the unsigned int where the parsed value will be stored.
 *
 * @return true if the integer was successfully parsed and stored in @p pval, 
 *         false otherwise.
 * 
 * @note the position is advanced after a successful read.
 * @retval false if input pointers are NULL, the source type is unsupported, or parsing fails.
 */
extern bool                        dsParseUnsigned(Ds *restrict pds, unsigned int *restrict pval);

/**
 * @brief Parses a unsigned long from the provided data source.
 *
 * This function acts as an adapter that handles different data source types 
 * defined in the @c Ds structure. It supports:
 * - @c DS_FILE: Reads an integer from a file stream using @c fscanf.
 * - @c DS_STR, @c DS_FS, @c DS_CONSTSTR: Parses an integer from a memory buffer.
 *
 * @param[in,out] pds  Pointer to the @c Ds structure containing the data source.
 * @param[out] pval    Pointer to the unsigned long where the parsed value will be stored.
 *
 * @return true if the integer was successfully parsed and stored in @p pval, 
 *         false otherwise.
 * 
 * @note the position is advanced after a successful read.
 * @retval false if input pointers are NULL, the source type is unsupported, or parsing fails.
 */
extern bool                        dsParseUnsignedLong(Ds *restrict pds, unsigned long *restrict pval);

/**
 * @brief Parses a double from the provided data source.
 *
 * This function acts as an adapter that handles different data source types 
 * defined in the @c Ds structure. It supports:
 * - @c DS_FILE: Reads an integer from a file stream using @c fscanf.
 * - @c DS_STR, @c DS_FS, @c DS_CONSTSTR: Parses an integer from a memory buffer.
 *
 * @param[in,out] pds  Pointer to the @c Ds structure containing the data source.
 * @param[out] pval    Pointer to the double int where the parsed value will be stored.
 *
 * @return true if the integer was successfully parsed and stored in @p pval, 
 *         false otherwise.
 * 
 * @note the position is advanced after a successful read.
 * @retval false if input pointers are NULL, the source type is unsupported, or parsing fails.
 */
extern bool                         dsParseDouble(Ds *restrict pds, double *restrict pdval);

/**
 * @brief Parses a char from the provided data source.
 *
 * This function acts as an adapter that handles different data source types 
 * defined in the @c Ds structure. It supports:
 * - @c DS_FILE: Reads an integer from a file stream using @c fscanf.
 * - @c DS_STR, @c DS_FS, @c DS_CONSTSTR: Parses an integer from a memory buffer.
 *
 * @param[in,out] pds  Pointer to the @c Ds structure containing the data source.
 * @param[out] pval    Pointer to the char where the parsed value will be stored.
 *
 * @return true if the integer was successfully parsed and stored in @p pval, 
 *         false otherwise.
 * 
 * @note the position is advanced after a successful read.
 * @retval false if input pointers are NULL, the source type is unsupported, or parsing fails.
 */
extern bool                         dsParseChar(Ds *restrict pds, char *restrict pval);

// ------------------------ PRINTERS/CHECKERS ---------------------------------------

// ------------------------------------ ETC. ----------------------------------------


#endif /* !_DS_ADAPTER_H */
