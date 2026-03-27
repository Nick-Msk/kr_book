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

static const int                G_FSARRAY_DEFAULT_INIT = 10;

// --------------------------------- TYPES -----------------------------

typedef struct fsarray {
    int         sz;     // total fs * allocated
    int         cnt;    // not sure if needed
    int         ptr;    // pointer for attach
    fs         *ar;
} fsarray;

// returns count of freed
extern int              fsarr_free(fsarray *arr);

extern fsarray          fsarr_init(int cnt);

static inline fsarray   fsarr_empty(){
    return fsarr_init(G_FSARRAY_DEFAULT_INIT);
}

#define fsarrfree(s)    fsarr_free(&(s))
#define FSARRAY(...)    (fsarray) {.sz = 0, .cnt = 0, .ptr = 0, .ar = 0, ##__VA_ARGS__};
// -------------------- ACCESS AND MODIFICATORS ------------------------

// low level deatch
static inline fs        fsarr_detach(fsarray *arr, int pos){
    fs s = fsmove(arr->ar[pos]);    // move via macro, fs_move undelaying
    arr->cnt--;     // just for logging now, but arr->pos remains the same!
    return s;       // probably it's possible to not return noting
}

// check if position exists
static inline bool      fsarr_exists(const fsarray *arr, int pos){
    return fsisnull(arr->ar[pos]);
}

// return a fs
static inline fs       *fsarr_get(const fsarray *arr, int pos){
    return arr->ar[pos];        // even if fsnull
}


#define                 fsarrattach(arr, s) fsarr_attach(&(arr), s)
// ------------------------ PRINTERS/CHECKERS --------------------------

extern int              fsarr_techfprint(FILE *f, fsarray arr);
static inline int       fsarr_techprint(fsarray arr){
    return fsarr_techfprint(stdout, arr);
}

extern int              fsarr_techfprintlim(FILE *f, fsarray arr, int lim);
static inline int       fsarr_techprintlim(fsarray arr, int lim){
    return fsarr_techfprintlim(stdout, arr, lim);
}

// ------------------------------ ETC. ---------------------------------

extern int              fsarr_save(const char *restrict fname, const fsarray *restrict arr);
extern int              fsarr_savef(FILE *restrict f, const fsarray *restrict arr);

extern fsarray          fsarr_load(const char *fname);
extern fsarray          fsarr_loadf(FILE *f);

#endif /* !_FS_ARRAY_H */

