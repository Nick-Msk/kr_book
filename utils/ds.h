/**
 * @file ds.h
 * @brief Universal interface for reading data from various sources.
 *
 * This module provides a @ref Ds abstraction that allows the same 
 * code to read characters from both standard I/O streams (FILE*) and 
 * memory buffers (const char*).
 * This is low-level code, NO FS, LOG OR USERRASE allowed!
 */

#ifndef _DS_H
#define _DS_H

#include <stdio.h>
#include <ctype.h>

// ---------------------------------------------------------------------------------
// --------------------------- Public datasource API -------------------------------
// ---------------------------------------------------------------------------------

/**
 * @enum DSType
 * @brief Defines the type of the data source. Now DS_FILE and DS_BUFFER are suppoted
 */
typedef enum {
    DS_FILE,     /**< Data source is a standard file (FILE*). */
    DS_STR,      /**< Data source is a memory buffer (null-terminated string). */
    DS_CONSTSTR, /**< Data source is a memory const buffer (null-terminated string). */
    DS_FS        /**< FS, Not suppoted now */
} DSType;

/**
 * @struct Ds
 * @brief A wrapper for the data source.
 *
 * This structure encapsulates the logic for both file and buffer reading,
 * providing a unified interface for sequential data access.
 */
typedef struct Ds {
    DSType type;                /**< Determines which union member is active. */
    union {         
        FILE *fp;               /**< Pointer to the file (used in @c DS_FILE mode). */
        struct {
            char    *ptr;       /**< Pointer to the start of the string (used in @c DS_STR mode). */
            size_t  pos;        /**< Current read position in the buffer. */
        } buf;                  /**< Buffer details. */
        struct {
            const char    *ptr; /**< Pointer to the start of the constant string (used in @c DS_STR mode). */
            size_t         pos; /**< Current read position in the buffer. */
            // int     ungetchsym; /**< Current read position in the buffer. */
        } constbuf;             /**< Buffer details. */
    } source;
} Ds;

/**
 * @brief Initializes a Ds object for reading from a file.
 * 
 * @param[out] Ds Pointer to the Ds structure to be initialized.
 * @param[in]  fp Pointer to the input file (FILE*).
 */
extern void                    dsInitf(Ds *Dsgetc, FILE *fp);
/**
 * @brief Initializes a Datasource (Ds) object for reading from a c-string.
 * 
 * @param[out] Ds Pointer to the Datasource (Ds) structure to be initialized.
 * @param[in]  buf Pointer to a null-terminated string (const char*).
 */
extern void                    dsInitstr(Ds *ds, char *buf);
/**
 * @brief Initializes a Datasource (Ds) object for reading from a const c-string.
 * 
 * @param[out] Ds Pointer to the Datasource (Ds) structure to be initialized.
 * @param[in]  buf Pointer to a null-terminated string (const char*).
 */
extern void                    dsInitconst(Ds *ds, const char *buf);
/**
 * @brief Reads the next character from the source.
 * 
 * @param[in,out] Ds Pointer to the data source.
 * @return The next character (0-255) or @c EOF if the end of the source is reached.
 * @note For @c DS_BUFFER mode, EOF is returned when the '\0' character is encountered.
 */
extern int                     dsgetc(Ds *ds);
/**
 * @brief Pushes a character back into the stream or rolls back the position.
 * 
 * @param[in]  c The character to push back.
 * @param[in,out] ds Pointer to the data source.
 * @return The character 'c' if successful, or @c EOF if the operation failed.
 */
extern int                     dsungetc(int c, Ds *ds);

/**
 * @brief Debugging print implementation.
 * Returns the number of characters printed.
 */
extern int                     dsTechFPrint(FILE *restrict out, const Ds *restrict ds, int printbufcnt);
static inline int       dsTechPrint(const Ds * ds, int printbufcnt) {
    return dsTechFPrint(stdout, ds, printbufcnt);
}

#endif /* !_DS_H */