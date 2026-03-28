#include <stdlib.h>

#include "common.h"
#include "log.h"
#include "fs.h"
#include "error.h"
#include "fs_array.h"
#include "fileutils.h"

/********************************************************************
                    FAST STRING ARRAY MODULE IMPLEMENTATION
********************************************************************/

// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers --------------------

int                             fsarr_techfprintlim(FILE *restrict f, const fsarray *restrict arr, int lim){
    int cnt = 0;
    if (arr){
        cnt += fprintf(f, "FSARRAY: sz %d, ptr %p:\n", arr->sz, arr->ar);
        if (arr->ar)
            for (int i = 0; i < MIN(arr->sz, lim); i++){
                cnt += fstechfprint(f, arr->ar[i]);   // directly name!
            }
        if (lim < arr->sz)
            cnt += fprintf(f, "and more... (%d)\n", arr->sz - lim);
    }
    return cnt;
}

int                             fsarr_techfprint(FILE *restrict f, const fsarray *restrict arr){
    return fsarr_techfprintlim(f, arr, arr->sz);
}

// ------------------------------ Utilities -------------------------

static int                      fsarr_mass_init(fsarray *arr, int min, int max){
    for (int i = min; i < max; i++)
        arr->ar[i] = FS();  // with alloc flag
    return min <= max ? max - min : 0;
}

static int                      fsarr_mass_free(fsarray *arr, int min, int  max){
    int cnt = 0;
    for (int i = max - 1; i >= min; i--)
        if (!fsisnull(arr->ar[i]) )
            fsfree(arr->ar[i]), cnt++;
    return cnt;
}

static int                      increasesize(fsarray *fa, int newsz, bool init){
    logenter("oldsz %d, newsz %d init %s", fa->sz, newsz, bool_str(init));
    if (init)   // from fsarr_init
        newsz = calcnewsize(SIZE_POWER2, newsz);
    if (newsz > fa->sz || !init) {   // if exec from faarr_increate() then resize anyway
        fs *tmp = realloc(fa->ar, newsz * sizeof (fs) );
        if (!tmp) {
            return userraise(-1, 10, "Unable to allocate %lu bytes", newsz * sizeof (fs) );  // what about LG_LV here??/ TODO:
        }
        fa->sz = newsz;
        fa->ar = tmp;
    }
    return logret(newsz, "Increased to %d elements", newsz);
}

// --------------------------- API ---------------------------------
// ------------------ General functions ----------------------------

// -------------------- ACCESS AND MODIFICATORS ------------------------

// TODO: via fsl ( struct fsl { int loc; }; 
// fsl f1 = getfsl(fs_array a);
// f1.elem() = ... 
// f1.get() = ...

int                         fsarr_increase(fsarray *arr, int newsize){
    int sz = arr->sz;
    increasesize(arr, newsize, false);
    fsarr_mass_init(arr, sz, newsize);
    return logsimpleret(arr->sz, "New sz %d", arr->sz);
}

int                         fsarr_shrink(fsarray *arr, int newsize){
    if (newsize < arr->sz && newsize >= 0){
        fsarr_mass_free(arr, newsize, arr->sz);
        increasesize(arr, newsize, false);
    }
    return logsimpleret(arr->sz, "Shrinked to %d", arr->sz);
}

// -------------------------- (API) printers -----------------------

// this is normal printer
int                         fsarr_fprint(FILE *restrict f, const fsarray *restrict arr){
    int  cnt = 0;
    for (int i = 0; i < arr->sz; i++){
        if ( !fsisnull(arr->ar[i] ) ) {
            fsfprint(f, arr->ar[i]);   // directly name!
            cnt++;
            fputc('\n', f);
        }
    }
    return cnt;     // total printed items
}

// -------------------------- (API) Serialization  -----------------------
int                         fsarr_save(const char *restrict fname, const fsarray *restrict arr){
    FILE    *out = fopen(fname, "w");
    if (!out)
        return userraise(ERR_UNABLE_OPEN_FILE_READ, -1, "Unable to open %s for write", fname);
    int     cnt = fsarr_savef(out, arr);
    fclose(out);
    return cnt;
}

// f is open for write
int                         fsarr_fsave(FILE *restrict f, const fsarray *restrict arr){
    // signature
    fprintf(f, "FSARRAY(%d):[\n", arr->sz);
    if (!arr->ar)
        //return logsimpleret(-1, "Nullable ar, can't serialize");    // userraise?
        return userraise(ERR_NULLABLE_PTR, -1, "Nullable ar, can't serialize"); 
    for (int i = 0; i < arr->sz; i++){
        fprintf(f, "%4d:", i);
        fs_fsave(f, fsarr_get(arr, i) );
        fputc('\n', f);
    }
    fprintf(f, "]");
    return logsimpleret(arr->sz, "Total saved %d", arr->sz);
}

fsarray                     fsarr_load(const char *fname){
    FILE    *out = fopen(fname, "r");
    if (!out)
        userraiseint(ERR_UNABLE_OPEN_FILE_READ, "Unable to open %s for read", fname);
    fsarray fa = fsarr_loadf(out);
    fclose(out);
    return fa;
}

fsarray                     fsarr_fload(FILE *restrict f){
    // format: "FSARRAY: sz %d cnt %d ptr %d:[\n"
    FUSKIPFORMAT(f, "FSARRAY(");
    int cnt = FUGETUNSIGNED(f);

    FUSKIPFORMAT(f, "):[\n");
    fsarray     a = fsarr_init(cnt);
    // cycle for fs
    for (int i = 0; i < cnt; i++){
        a.ar[i] = fs_fload(f, 0);       // TODO: probably to usr fsarr_get()?
    }
    FUSKIPFORMAT(f, "]");
    return a;
}

// ------------------ API Constructs/Destrucor  ----------------------------

int                 fsarr_free(fsarray *arr){

    logenter("%p: %p, %d", arr, arr ? arr->ar : 0, arr ? arr->sz : 0);
    int   cnt = 0;
    if (arr)
        cnt = fsarr_mass_free(arr, 0, arr->sz);
    return logret(cnt, "Freed %d", cnt);
}

// initialize
fsarray             fsarr_init(int sz){
    fsarray ar = FSARRAY();
    increasesize(&ar, sz, true);
    fsarr_mass_init(&ar, 0, ar.sz);
    return logsimpleret(ar, "Created with sz %d", ar.sz);
}

// -------------------------------Testing --------------------------
#ifdef FSARRAYTESTING

#include "test.h"
#include "checker.h"

//types for testing

// ------------------------- TEST 1 ---------------------------------

// ------------------------------------------------------------------
int
main( /* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Simple init free test"                        , .desc=""                , .mandatory=true)
     /* , testnew(.f2 = tf2, .num = 2, .name = "Init/free test with valid fs (random)"        , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf3, .num = 3, .name = "fsarr_attach test"                            , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf4, .num = 4, .name = "fsarr_save test"                              , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf5, .num = 5, .name = "fsarr_save/load test"                         , .desc=""                , .mandatory=true) */
    );

    return logcloseret(0, "end...");
}

#endif /* FSARRAYTESTING */


