/**
 * @file ds.h
 * @brief Universal interface for reading/writing data from various sources.
 *
 * This module provides a @ref DS abstraction that allows the same 
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
 * @struct DS
 * @brief A wrapper for the data source.
 *
 * This structure encapsulates the logic for both file and buffer reading,
 * providing a unified interface for sequential data access.
 */
typedef struct DS {
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
} DS;

#define DS(...) (DS) {.type = DS_STR, .pos = 0L, .ptr = NULL,\
     .strssavepos = 0L, __VA_ARGS__}
#define DSFILE(...) (DS) {.type = DS_FILE, .fp = NULL,\
     .filesavepos = 0L, __VA_ARGS__}
#define DSSTR(...) (DS) {.type = DS_STR, .pos = 0L, .ptr = NULL,\
     .strssavepos = 0L, .cap = 0L, __VA_ARGS__}
#define DSCONST(...) (DS) {.type = DS_CONSTSTR, .pos = 0L, .constptr = NULL, \
     .strssavepos = 0L, __VA_ARGS__}

#ifndef NO_FSDS
    #define DSFS(...) (DS) {.type = DS_FS, .pos = 0L, .s = FS(), .strssavepos = 0L, __VA_ARGS__}
#endif  /* !NO_FSDS */    


/**
 * @brief Initializes a DS object for reading from a file.
 * 
 * @param[out] DS Pointer to the DS structure to be initialized.
 * @param[in]  fp Pointer to the input file (FILE*).
 */
extern bool                     dsInitf(DS *restrict pds, FILE *restrict fp);
/**
 * @brief Initializes a DS object for reading from a file. wrapper for dsInitf
 * 
 * @param[in]  fp Pointer to the input file (FILE*).
 * @return     DS (initialied or not)
 */
static inline DS                dsCreatef(FILE *fp) {
    DS      res = DSFILE();
    if (dsInitf(&res, fp) )
        return res;
    res = (DS) {0};
    return res;
}
extern bool                     dsInitFilename(DS *restrict pds, const char *restrict fname, const char *restrict mode);

static inline DS                dsCreateFilename(const char *restrict fname, const char *restrict mode) {
    DS      res = DSFILE();
    if (dsInitFilename(&res, fname, mode) )
        return res;
    res = (DS) {0};
    return res;
}
/**
 * @brief Initializes a Datasource (DS) object for reading from a c-string.
 * 
 * @param[out] DS Pointer to the Datasource (DS) structure to be initialized.
 * @param[in]  cap Capacity, if 0L then till '\0' 
 * @param[in]  buf Pointer to a null-terminated string (const char*).
 */
extern bool                    dsInitstrCap(DS *restrict ds, char *restrict buf, size_t cap);
/**
 * @brief Initializes a Datasource (DS) object for reading from a c-string.
 * 
 * @param[out] DS Pointer to the Datasource (DS) structure to be initialized.
 * @param[in]  buf Pointer to a null-terminated string (const char*).
 */
static inline bool             dsInitstr(DS *restrict ds, char *restrict buf) {
    return dsInitstrCap(ds, buf, 0L);
}
/**
 * @brief Initializes a Datasource (DS) object for reading from a c-string.
 * 
 * @param[in]  buf Pointer to a null-terminated string (const char*).
 * @param[in]  cap Capacity must be > 0L
 * @return     DS (initialied or not)
 */
static inline DS               dsCreatestrCap(char *buf, size_t cap) {
    DS      tmp = DSSTR();
    if (cap > 0L && dsInitstrCap(&tmp, buf, cap) )
        return tmp;
    DS      empty = {0};
    return empty;
}
/**
 * @brief Initializes a Datasource (DS) object for reading from a c-string.
 * 
 * @param[in]  buf Pointer to a null-terminated string (const char*).
 * @return     DS (initialied or not)
 */
static inline DS               dsCreatestr(char *buf) {
    DS      tmp = DSSTR();
    if (dsInitstr(&tmp, buf) )
        return tmp;
    DS      empty = {0};
    return empty;
}
/**
 * @brief Initializes a Datasource (DS) object for reading from a const c-string.
 * 
 * @param[out] DS Pointer to the Datasource (DS) structure to be initialized.
 * @param[in]  buf Pointer to a null-terminated string (const char*).
 */
extern bool                    dsInitconst(DS *restrict pds, const char *restrict buf);
/**
 * @brief Initializes a Datasource (DS) object for reading from a const c-string.
 * 
 * @param[in]  buf Pointer to a null-terminated string (const char*).
 * @return     DS (initialied or not)
 */
static inline DS               dsCreateconst(const char *buf) {
    DS      tmp = DSCONST();
    if (dsInitconst(&tmp, buf) )
        return tmp;
    DS      empty = {0};
    return empty;
}
// ----------------------------------------------------------------------
#ifndef NO_FSDS
/**
 * @brief Initializes a DS object for reading from a fs.
 * 
 * @param[out] DS Pointer to the DS structure to be initialized.
 * @param[in]  s Pointer to the fs.
 * @note       fs s is MOVED into DS structure!
 */
extern bool                     dsInitfs(DS *restrict pds, fs *restrict s);
/**
 * @brief Initializes a DS object for reading from a file. wrapper for dsInitf
 * 
 * @param[in]  s Pointer to the fs.
 * @return     DS (initialied or not).
 * @note       fs s is MOVED into DS structure!
 */
static inline DS                dsCreatefs(fs *s) {
    DS      tmp = DSFS();
    if (dsInitfs(&tmp, s) )
        return tmp;
    DS      empty = {0};
    return empty;
}
// for wrire mostly
static inline DS                dsCreatefsempty(void) {
    fs  tmp = FS();
    return dsCreatefs(&tmp);
}

#endif  /* !NO_FSDS */   

// DS_FS or DS_FILE
static inline void              dsFree(DS *pds) {
    if (pds) {
#ifndef NO_FSDS
        if (pds->type == DS_FS)
            fsfree(pds->s);
#endif  /* !NO_FSDS */   
        if (pds->type == DS_FILE)
            fclose(pds->fp);
        *pds = (DS) {0};
    }
}

#define DSFREE(ds) dsFree(&(ds))

// ----------------------------------------------------------------------

/**
 * @brief Reads the next character from the source.
 * 
 * @param[in,out] DS Pointer to the data source.
 * @return The next character (0-255) or @c EOF if the end of the source is reached.
 * @note For @c DS_BUFFER mode, EOF is returned when the '\0' character is encountered.
 */
extern int                     dsgetc(DS *pds);
/**
 * @brief Pushes a character back into the stream or rolls back the position.
 * 
 * @param[in]  c The character to push back.
 * @param[in,out] ds Pointer to the data source.
 * @return The character 'c' if successful, or @c EOF if the operation failed.
 */
extern int                     dsungetc(int c, DS *pds);

/**
 * @brief Pushes ANY character back into the stream (DS_STR, DS_FS)
 *  
 * @param[in]  c The character to push back.
 * @param[in,out] ds Pointer to the data source.
 * @return The character 'c' if successful, or @c EOF if the operation failed.
 */
extern int                      dsreplacec(int c, DS *pds);

/**
 * @brief Pushes ANY character into the stream (DS_STR, DS_FS, DS_FILE)
 *  
 * @param[in]  c The character to push back.
 * @param[in,out] ds Pointer to the data source.
 * @return The character 'c' if successful, or @c EOF if the operation failed.
 */
extern int                      dsputc(int c, DS *pds);
/**
 * @brief Pushes Ecraned sequence for character into the stream
 *  
 * @param[in]  c The ecraning character to push back.
 * @param[in,out] ds Pointer to the data source.
 * @return The character 'c' if successful, or @c EOF if the operation failed.
 */
extern int                      dsputcEcran(int c, DS *pds);
/**
 * @brief Pushes c-string into the stream
 *  
 * @param[in]  ptr The pointrer to c-string.
 * @param[in]  len Real length of c-string
 * @param[in,out] ds Pointer to the data source.
 * @return The character 'c' if successful, or @c EOF if the operation failed.
 */
extern long                     dswrite(DS *restrict out, const char *ptr, size_t len);

/**
 * @brief Debugging print implementation.
 * Returns the number of characters printed.
 */
extern int                     dsTechFPrint(FILE *restrict out, const DS *restrict ds, const char *restrict name);
static inline int              dsTechPrint(const DS *restrict pds, const char *restrict name) {
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
static inline const char       *dsStrbuf(const DS *pds) {
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
 * @return true if DS is c-string source
 */
static inline bool              dsIsstr(const DS *pds) {
    return pds->type == DS_CONSTSTR || pds->type == DS_STR;
}

/**
 * @brief Saves the current position of the data source for later restoration.
 * 
 * @param pds Pointer to the data source.
 */
static inline void              dsSavepos(DS *pds) {
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
static inline bool              dsRestorepos(DS *pds) {
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
static inline bool              dsReset(DS *pds) {
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
