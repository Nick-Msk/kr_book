/**
 * @file ds.h
 * @brief Universal interface for reading/writing data from various sources.
 *
 * This module provides a @ref Ds abstraction that allows the same 
 * code to read characters from both standard I/O streams (FILE*) and 
 * memory buffers (const char*).
 * This is low-level code, NO FS (#ifndef), LOG OR USERRASE allowed!
 */

#ifndef _DS_H
#define _DS_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifndef NO_FSDS
    #include "fs.h"
#endif /* !NO_FSDS */

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
    DS_FS        /**< FS, suppoerted via ifndef NO_FSDS */
} DSType;

static inline const char               *DSTypeName(DSType typ) {
    switch (typ) {
        CASE_RETURN(DS_FILE);
        CASE_RETURN(DS_STR);
        CASE_RETURN(DS_CONSTSTR);
        CASE_RETURN(DS_FS);
        default: return "Unknown DS type";
    }
}

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
        struct {    
            FILE *fp;               /**< Pointer to the file (used in @c DS_FILE mode). */
            off_t   filesavepos;       /**< For save FILE position */
        };
        struct {
            union {
                struct {
                    union {
                        char          *ptr; /**< Pointer to the start of the string (used in @c DS_STR mode). */
                        const char    *constptr; /**< Pointer to the start of the constant string (used in @c DS_STR mode). */
                    }; 
                    size_t  cap;        /**< Capacity of STR and CONSTSTR */
                };
#ifndef NO_FSDS
                fs    s;                /**< fs autoextendable string */
#endif  /* !NO_FSDS */             
            };
            size_t  pos;        /**< Current read position in the buffer. */
            size_t  strssavepos;      /**< For save/resrote position  */
        };                  /**< Buffer details. */
    };
} Ds;

#define DS(...) (Ds) {.type = DS_STR, .pos = 0L, .ptr = NULL,\
     .strssavepos = 0L, __VA_ARGS__}
#define DSFILE(...) (Ds) {.type = DS_FILE, .fp = NULL,\
     .filesavepos = 0L, __VA_ARGS__}
#define DSSTR(...) (Ds) {.type = DS_STR, .pos = 0L, .ptr = NULL,\
     .strssavepos = 0L, .cap = 0L, __VA_ARGS__}
#define DSCONST(...) (Ds) {.type = DS_CONSTSTR, .pos = 0L, .constptr = NULL, \
     .strssavepos = 0L, __VA_ARGS__}

#ifndef NO_FSDS
    #define DSFS(...) (Ds) {.type = DS_FS, .pos = 0L, .s = FS(), .strssavepos = 0L, __VA_ARGS__}
#endif  /* !NO_FSDS */    


/**
 * @brief Initializes a Ds object for reading from a file.
 * 
 * @param[out] Ds Pointer to the Ds structure to be initialized.
 * @param[in]  fp Pointer to the input file (FILE*).
 */
extern bool                    dsInitf(Ds *restrict pds, FILE *restrict fp);
/**
 * @brief Initializes a Ds object for reading from a file. wrapper for dsInitf
 * 
 * @param[in]  fp Pointer to the input file (FILE*).
 * @return     Ds (initialied or not)
 */
static inline Ds               dsCreatef(FILE *fp) {
    Ds      tmp = DSFILE();
    if (dsInitf(&tmp, fp) )
        return tmp;
    Ds      empty = {0};
    return empty;
}
/**
 * @brief Initializes a Datasource (Ds) object for reading from a c-string.
 * 
 * @param[out] Ds Pointer to the Datasource (Ds) structure to be initialized.
 * @param[in]  buf Pointer to a null-terminated string (const char*).
 */
extern bool                    dsInitstr(Ds *restrict ds, char *restrict buf);
/**
 * @brief Initializes a Datasource (Ds) object for reading from a c-string.
 * 
 * @param[in]  buf Pointer to a null-terminated string (const char*).
 * @return     Ds (initialied or not)
 */
static inline Ds               dsCreatestr(char *buf) {
    Ds      tmp = DSSTR();
    if (dsInitstr(&tmp, buf) )
        return tmp;
    Ds      empty = {0};
    return empty;
}
/**
 * @brief Initializes a Datasource (Ds) object for reading from a const c-string.
 * 
 * @param[out] Ds Pointer to the Datasource (Ds) structure to be initialized.
 * @param[in]  buf Pointer to a null-terminated string (const char*).
 */
extern bool                    dsInitconst(Ds *restrict pds, const char *restrict buf);
/**
 * @brief Initializes a Datasource (Ds) object for reading from a const c-string.
 * 
 * @param[in]  buf Pointer to a null-terminated string (const char*).
 * @return     Ds (initialied or not)
 */
static inline Ds               dsCreateconst(const char *buf) {
    Ds      tmp = DSCONST();
    if (dsInitconst(&tmp, buf) )
        return tmp;
    Ds      empty = {0};
    return empty;
}
// ----------------------------------------------------------------------
#ifndef NO_FSDS
/**
 * @brief Initializes a Ds object for reading from a fs.
 * 
 * @param[out] Ds Pointer to the Ds structure to be initialized.
 * @param[in]  s Pointer to the fs.
 * @note       fs s is MOVED into Ds structure!
 */
extern bool                     dsInitfs(Ds *restrict pds, fs *restrict s);
/**
 * @brief Initializes a Ds object for reading from a file. wrapper for dsInitf
 * 
 * @param[in]  s Pointer to the fs.
 * @return     Ds (initialied or not).
 * @note       fs s is MOVED into Ds structure!
 */
static inline Ds                dsCreatefs(fs *s) {
    Ds      tmp = DSFS();
    if (dsInitfs(&tmp, s) )
        return tmp;
    Ds      empty = {0};
    return empty;
}
// for wrire mostly
static inline Ds                dsCreatefsempty(void) {
    fs  tmp = FS();
    return dsCreatefs(&tmp);
}

// DS_FS only
static inline void              dsFree(Ds *pds) {
    if (pds && pds->type == DS_FS)
        fsfree(pds->s);
}

#endif  /* !NO_FSDS */   

// ----------------------------------------------------------------------

/**
 * @brief Reads the next character from the source.
 * 
 * @param[in,out] Ds Pointer to the data source.
 * @return The next character (0-255) or @c EOF if the end of the source is reached.
 * @note For @c DS_BUFFER mode, EOF is returned when the '\0' character is encountered.
 */
extern int                     dsgetc(Ds *pds);
/**
 * @brief Pushes a character back into the stream or rolls back the position.
 * 
 * @param[in]  c The character to push back.
 * @param[in,out] ds Pointer to the data source.
 * @return The character 'c' if successful, or @c EOF if the operation failed.
 */
extern int                     dsungetc(int c, Ds *pds);

/**
 * @brief Pushes ANY character back into the stream (DS_STR, DS_FS)
 *  
 * @param[in]  c The character to push back.
 * @param[in,out] ds Pointer to the data source.
 * @return The character 'c' if successful, or @c EOF if the operation failed.
 */
extern int                      dsreplacec(int c, Ds *pds);

/**
 * @brief Pushes ANY character into the stream (DS_STR, DS_FS)
 *  
 * @param[in]  c The character to push back.
 * @param[in,out] ds Pointer to the data source.
 * @return The character 'c' if successful, or @c EOF if the operation failed.
 */
extern int                      dsputc(int c, Ds *pds);

/**
 * @brief Debugging print implementation.
 * Returns the number of characters printed.
 */
extern int                     dsTechFPrint(FILE *restrict out, const Ds *restrict ds, const char *restrict name);
static inline int              dsTechPrint(const Ds *restrict pds, const char *restrict name) {
    return dsTechFPrint(stdout, pds, name);
}

#define DSTECHFPRINT(out, ds)   dsTechFPrint((out), &(ds), #ds)   
#define DSTECHPRINT(ds)         dsTechPrint( &(ds), #ds)

/**
 * @brief Return pointer to buffer for DS_STR, DS_FS and DS_CONSTSTR
 *  
 * @param[in,out] ds Pointer to the data source.
 * @return pointer to c-str buffer
 */
static inline const char       *dsStrbuf(const Ds *pds) {
    switch (pds->type) {
        case DS_STR:
            return pds->ptr;
        case DS_CONSTSTR:
            return pds->constptr;
        case DS_FS:
            return pds->s.v;
        default:
            return NULL;
    }
}
/**
 * @brief Return pointer to buffer for DS_STR and DS_CONSTSTR
 *  
 * @param[in,out] ds Pointer to the data source.
 * @return true if Ds is c-string source
 */
static inline bool              dsIsstr(const Ds *pds) {
    return pds->type == DS_CONSTSTR || pds->type == DS_STR;
}

/**
 * @brief Saves the current position of the data source for later restoration.
 * 
 * @param pds Pointer to the data source.
 */
static inline void              dsSavepos(Ds *pds) {
    switch (pds->type) {
        case DS_FILE:
            pds->filesavepos = ftell(pds->fp);
            break;
        case DS_STR: case DS_CONSTSTR: case DS_FS:
            pds->strssavepos = pds->pos;
            break;
    }
}

/**
 * @brief Restores the position of the data source to the last saved state.
 * 
 * @param pds Pointer to the data source.
 * @return true if successful, false otherwise.
 */
static inline bool              dsRestorepos(Ds *pds) {
    switch (pds->type) {
        case DS_FILE:
            return fseek(pds->fp, pds->filesavepos, SEEK_SET) == 0;
        case DS_STR: case DS_CONSTSTR: case DS_FS:
            pds->pos = pds->strssavepos;
            break;
    }
    return true;
}

/**
 * @brief Resets the position of the data source to the beginning.
 * 
 * @param pds Pointer to the data source.
 * @return true if successful, false otherwise.
 */
static inline bool              dsReset(Ds *pds) {
    switch (pds->type) {
        case DS_FILE:
            return fseek(pds->fp, 0L, SEEK_SET) == 0;
        case DS_STR: case DS_CONSTSTR: case DS_FS:
            pds->pos = 0;
            break;
    }
    return true;
} 

#endif /* !_DS_H */
