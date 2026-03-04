#ifndef _FS_ARRAY_H
#define _FS_ARRAY_H

// ---------------------------------------------------------------------------------------
// --------------------------- Public Faststring Array API -------------------------------
// ---------------------------------------------------------------------------------------

// --------------- Includes -----------------------

#include <stdio.h>
#include <string.h>
//
#include "bool.h"
#include "log.h"
#include "common.h"
#include "error.h"
#include "fs.h"

// ------------------- CONSTANTS AND GLOBALS ---------------------------

// --------------------------------- TYPES -----------------------------

typedef struct {
    fs          s;
//#ifdef FSNAMED
    const char  *name;          // only STATIC pointer here!!!
//#endif
} namedfs;

typedef struct fsarray {
    int         sz; // total fs * allocated
    int         cnt;    // not sure if needed
    int         per;    // pointer for attach
    namedfs    *ar;
} fsarray;

// type-support functions

// ------------------------ CONSTRUCTOTS/DESTRUCTORS -------------------

//if fs_array.h is included then
//fist fsinit create the fsarray *_FS_ARRAY;  // for current function!
// not sure how to do that for now

// returns count of freed
extern int              fsarr_free(fsarray *arr);

extern fsarray          fsarr_init(int cnt);

#define fsarrfree(s)    fsarr_free(&(s))
#define FSARRAY(...)    (fsarray) {.sz = 0, .cnt = 0, .ar = 0, ##__VA_ARGS__};
// -------------------- ACCESS AND MODIFICATORS ------------------------

extern fs*              fsarr_attach(fsarray *arr, fs *s);
extern fs*              fsarr_detach(fsarray *arr, fs *s);  // not sure, because how to find s?
extern fsarray          fsarr_shrink(fsarray *arr);

// ------------------------ PRINTERS/CHECKERS --------------------------

int                     fsarr_techfprint(FILE *f, fsarray arr);
static inline int       fsarr_techprint(fsarray arr){
    return fsarr_techfprint(stdout, arr);
}


// ------------------------------ ETC. ---------------------------------

// TODO:
extern int              fsarr_save(const char *fname);
extern int              fsarr_savef(FILE *f);

extern fsarray          fsarr_load(const char *fname);
extern fsarray          fsarr_loadf(FILE *f);

#endif /* !_FS_ARRAY_H */
