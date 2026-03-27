#include <stdlib.h>

#include "common.h"
#include "log.h"
#include "fs.h"
#include "error.h"
#include "fs_array.h"

/********************************************************************
                    FAST STRING ARRAY MODULE IMPLEMENTATION
********************************************************************/

// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers --------------------

int                             fsarr_techfprintlim(FILE *f, fsarray arr, int lim){
    int cnt = fprintf(f, "FSARRAY: sz %d, cnt %d, ptr %d namedfs ptr %p:\n", arr.sz, arr.cnt, arr.ptr, arr.ar);

    if (arr.ar)
        for (int i = 0; i < MIN(arr.sz, lim); i++){
            if (arr.ar[i].ps) {
                cnt += fstechfprint(f, *arr.ar[i]);   // directly name!
            }
        }
    if (lim < arr.sz)
        cnt += fprintf(f, "and more... (%d)\n", arr.sz - lim);
    return cnt;
}

int                             fsarr_techfprint(FILE *f, fsarray arr){
    return fsarr_techfprintlim(f, arr, arr.sz);
}

// ------------------------------ Utilities -------------------------

// should depent on increase strategy, simplest return n + 1;
static inline int               calcnewsize(int n){ 
    int sz = round_up_2(n);
    return sz; //logsimpleret(sz, "newsz = %d", sz);
}

static int                      increasesize(fsarray *fa, int newsz, bool init){
    logenter("oldsz %d, newsz %d init %s", fa->sz, newsz, bool_str(init));
    if (init)   // from fsarr_init
        newsz = calcnewsize(newsz);
    if (newsz > fa->sz || !init) {   // if exec from faarr_increate() then resize anyway
        namedfs *tmp = realloc(fa->ar, newsz * sizeof (namedfs) );
        if (!tmp) {
            return userraise(-1, 10, "Unable to allocate %lu bytes", newsz * sizeof (namedfs) );  // what about LG_LV here??/ TODO:
        }
        fa->sz = newsz;
        fa->ar = tmp;
    }
    return logret(newsz, "Increased to %d elements", newsz);
}

