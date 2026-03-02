#include <stdlib.h>

#include "common.h"
#include "log.h"
#include "fs.h"
#include "error.h"

/********************************************************************
                    FAST STRING ARRAY MODULE IMPLEMENTATION
********************************************************************/

// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers --------------------

int                             fsarray_techfprint(FILE *f, fsarray arr){
    int cnt = fprintf(f, "FSARRAY: sz %d, filled %d:");

    for (int i = 0; i < arr.sz; i++){
        if (arr[i].ar.s){
            fs_techprint(f, arr[i].ar.s
#ifdef FSNAMED
                    , arr[i].ar.name   // directly name!
#endif
            );
            cnt++;
            fputc('\n', f);
        }
    }
    return cnt;
}

// ------------------------------ Utilities -------------------------

// should depent on increase strategy, simplest return n + 1;
static inline int               calcnewsize(int n){
    int sz = round_up_2(n);
    return sz; //logsimpleret(sz, "newsz = %d", sz);
}

static int                      increasesize(fsarr *fa, int newsz, bool init){
    logmsg("oldsz %d, newsz %d init %s", fa-sz, newsz, bool_str(init));
    if (init)   // from fsarr_init
        newsz = calcnewsize(newsz);
    if (newsz > fa->sz || !init){   // if exec from faarr_increate() then resize anyway
        namedfs *tmp = realloc(newsz * sizeof (namesfs));
        if (!tmp){
            return userraise(-1, 10, "Unable to allocate %d bytes", newsz * sizeof (namesfs));
        }
        fa->sz = newsz;
        fs->ar = tmp;
    }
    return logret(newsz, "Increased to %d elements", newsz);
}

static int                      namedfs(const void *ns1, const void *ns2){
    return ns1->s - ns2->s; // OMG
}

// sorting via qsort()
static fsarr                    sortfs(const fsarr *origin){
    // make a copy first
    fsarr tmp = FSARRAY();
    increasesize(&tmp, origin->cnt, false);  // exactly count of origin
    int     j = 0;
    for (int i = 0; i < origin->sz; i++)
        if (origin->ar[i].ar.s){
            tmp.ar[j++] = origin->ar[i];    // all namedfs, not reeally need actually
        }
    // sort
    qsort(tmp.ar, origin->cnt, sizeof(namedfs), namedfscmp);
    return tmp;
}

// --------------------------- API ---------------------------------
// ------------------ General functions ----------------------------

// never raise exception! just return true/false and put messages into f (if != 0)
bool                fsarray_validate(FILE *f, fsarray arr){
    logenter("%p", f);
    bool res = true;
    // it's almost nothing for now
    if (arr.sz <= 0 || arr.cnt <= 0){
        fprintf(f, "%s: sz and cnt must be positive", __func__);
        return logerr(false, "sz and cnt must be positive");
    }
    // now check total valid fs via cnt
    int     cnt = 0;
    for (int i = 0; i < arr.sz; i++)
        if (arr.ar[i].s)
            cnt++;  // found a vliad fs

    if (cnt != arr.cnt){
        fprintf(f, "Total valid fs = %d but cnt = %d", cnt, arr.cnt);
        return logerr(false, "Total valid fs = %d but cnt = %d", cnt, arr.cnt);
    }

    fsarr   tmp = sortfs(&arr);   // make a sorting by arr.ar.s.v, tmp is shrinked and NOT valid, all fs are pointer to fs of arr
    int     duplcount = 0;
    for (int i = 1; i < tmp.sz; i++)
        if (tmp.ar[i - 1].s.v == tmp.ar[i].s.v){
            fprintf(f, "elem [%d] == elem [%d] == %p:[%s]", i - 1, i, tmp.ar[i].s.v, tmp.ar[i].s.v);
            duplcount++;
        }
    if (duplcount > 0){
        fprintf(f, "Total duplicates %d", duplcount);
        return logerr(false, "Total duplicates %d", duplcount);
    }
    // all tests for now...
    return logret(true, "Ok");
}

// -------------------------- (API) printers -----------------------

// this is normal printer
int                 fsarray_fprint(FILE *f, fsarray arr){
    int  cnt = 0;
    for (int i = 0; i < arr.sz; i++){
        if (arr[i].ar.s){
            fs_print(f, arr[i].ar.s
//#ifdef FSNAMED
                    , arr[i].ar.name   // directly name!
//#else
                    , ""
//#endif
            );
            cnt++;
            fputc('\n', f);
        }
    }
    return cnt;     // total printed items
}

// ------------------ API Constructs/Destrucor  ----------------------------


int                 fsarr_free(fsarray *arr){
    int     cnt = 0;
    for (int i = 0; i < arr.sz; i++){ // not cnt!! cnt is just a sum of attached fs
        if (arr[i].ar.s){
            fsfree(arr[i].ar.s);
            cnt++;
//#ifdef FSNAMED
            arr[i].ar.name = 0; // it's ok because name is STATIC
//#endif
        }
    }
    return cnt;
}

// initialize
fsarray             fsarr_init(int sz){
    fsarray ar = FSARRAY();
    increasesize(&ar, sz, true);
    return logsimpleret(ar, "Created with sz %d", arr.sz);
}

// -------------------------------Testing --------------------------
#ifdef FSARRAYTESTING

#include "test.h"
#include "checker.h"

//types for testing

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d: init", ++subnum);

        int sz = 100;
        fsarray fa = fsarr_init(sz);
        if (!fsarray_validate(stderr, fa)
            return logerr(TEST_FAILED, "Validation failed");

        if (fa.sz < sz)
            return logerr(TEST_FAILED, "sz (%d) must be >= initial sz %d", fa.sz, sz);

        if (fa.cnt != 0)
            return logerr(TEST_FAILED, "Cnt = %d but must be 0, because not any objects after init", fa.sz);

        test_sub("subtest %d: freeall", ++subnum);

        fsar_free(&fa);

    }
    return logret(TEST_MANUAL, "done"); // TEST_FAILED, TEST_PASSED
}

// ------------------------------------------------------------------
int
main( /* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Simple init free test"                , .desc=""                , .mandatory=true)
    );

    logclose("end...");
    return 0;
}

#endif /* FSARRAYTESTING */

