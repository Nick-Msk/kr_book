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


typedef struct {
    fs          *ps;
//#ifdef FSNAMED
//    const char  *name;          // only STATIC pointer here!!!  NOT USED FOR NOW
//#endif
} namedfs;

typedef struct fsarray {
    int         sz;     // total fs * allocated
    int         cnt;    // not sure if needed
    int         ptr;    // pointer for attach
    namedfs    *ar;
} fsarray;

// linked faststring!
typedef struct fsl {
    union {
        //struct fs;
        fs      s;
    };
    int     pos;        // position in fsarray
} fsl;

// type-support functions

// ------------------------ CONSTRUCTOTS/DESTRUCTORS -------------------

//if fs_array.h is included then
//fist fsinit create the fsarray *_FS_ARRAY;  // for current function!
// not sure how to do that for now

// returns count of freed
extern int              fsarr_free(fsarray *arr);

extern fsarray          fsarr_init(int cnt);

static inline fsarray  fsarr_empty(){
    return fsarr_init(G_FSARRAY_DEFAULT_INIT);
}

#define fsarrfree(s)    fsarr_free(&(s))
#define FSL(...)        (fsl) { .s = FS(), .pos = 0, ##__VA_ARGS__}
#define FSARRAY(...)    (fsarray) {.sz = 0, .cnt = 0, .ptr = 0, .ar = 0, ##__VA_ARGS__};
// -------------------- ACCESS AND MODIFICATORS ------------------------

// low level
extern int              fsarr_attach(fsarray *restrict arr, fs *restrict s);

// low level deatch
static inline fs*       fsarr_detach(fsarray *arr, int pos){
    fs *s = arr->ar[pos].ps;
    arr->ar[pos].ps = 0;
    arr->cnt--;     // just for logging now, but arr->pos remains the same!
    return s;       // probably it's possible to not return noting
}

// check if position exists
static inline bool      fsarr_ex(const fsarray *arr, int pos){
    return arr->ar[pos].fs != 0;
}

// return a fs
static inline fs       *fsarr_get(const fsarray *arr, int pos){
    return arr->ar[pos].ps;
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

static inline int       fsl_techfprint(FILE *f, fsl fl){
    return fprintf(f, "FSL: sz %d, len %d, pos %d v %p\n", fl.s.sz, fl.s.len, fl.pos, fl.s.v);
}

static inline int       fsl_techprint(fsl fl){
    return fsl_techfprint(stdout, fl);
}

// ------------------------------ ETC. ---------------------------------

// TODO:
extern int              fsarr_save(const char *restrict fname, const fsarray *restrict arr);
extern int              fsarr_savef(FILE *restrict f, const fsarray *restrict arr);

extern fsarray          fsarr_load(const char *fname, const fsarray *restrict arr);
extern fsarray          fsarr_loadf(FILE *restrict f, const fsarray *restrict arr);

#endif /* !_FS_ARRAY_H */
