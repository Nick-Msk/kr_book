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
            if (fsstr(arr.ar[i].s) ){
                cnt += fs_techfprint(f, &arr.ar[i].s, arr.ar[i].name);   // directly name!
            }
        }
    if (lim < arr.sz)
        cnt += fprintf(f, "and more...\n");
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

static int                      namedfscmp(const void *ns1, const void *ns2){
    return ((namedfs *) ns1)->s.v - ((namedfs *) ns2)->s.v; // OMG
}

// sorting via qsort()
static fsarray                  sortfs(const fsarray *origin){
    // make a copy first
    fsarray tmp = FSARRAY();
    increasesize(&tmp, origin->cnt, false);  // exactly count of origin
    int     j = 0;
    for (int i = 0; i < origin->sz; i++)
        if (fsstr(origin->ar[i].s) ) {
            tmp.ar[j++] = origin->ar[i];    // all namedfs, not reeally need actually
        }
    // sort
    qsort(tmp.ar, origin->cnt, sizeof(namedfs), namedfscmp);
    return tmp;
}

// --------------------------- API ---------------------------------
// ------------------ General functions ----------------------------

// never raise exception! just return true/false and put messages into f (if != 0)
bool                fsarr_validate(FILE *f, fsarray arr){
    logenter("%p", f);
    // check 1
    if (arr.sz < 0 || arr.cnt < 0){
        fprintf(f, "%s: sz and cnt must be positive", __func__);
        return logerr(false, "sz and cnt must be positive");
    }
    // check 2
    if (arr.sz < arr.cnt){
        fprintf(f, "sz %d must be >= cnt %d", arr.sz, arr.cnt);
        return logerr(false, "sz %d must be > cnt %d", arr.sz, arr.cnt);
    }
    // check 3
    // now check total valid fs via cnt
    int     cnt = 0;
    for (int i = 0; i < arr.sz; i++)
        if (fsstr(arr.ar[i].s) )
            cnt++;  // found a vliad fs

    if (cnt != arr.cnt){
        fprintf(f, "Total valid fs = %d but cnt = %d", cnt, arr.cnt);
        return logerr(false, "Total valid fs = %d but cnt = %d", cnt, arr.cnt);
    }
    if (arr.sz > 0){
        // check 4
        fsarray   tmp = sortfs(&arr);   // make a sorting by arr.ar.s.v, tmp is shrinked and NOT valid, all fs are pointer to fs of arr
        int     duplcount = 0;
        for (int i = 1; i < tmp.sz; i++)
            if (tmp.ar[i - 1].s.v == tmp.ar[i].s.v){
                fprintf(f, "elem [%d] == elem [%d] == %p:[%s]", i - 1, i, tmp.ar[i].s.v, tmp.ar[i].s.v);
                duplcount++;
            }
        free(tmp.ar);       // free temporary, no need to free ar[].s
        if (duplcount > 0){
            fprintf(f, "Total duplicates %d", duplcount);
            return logerr(false, "Total duplicates %d", duplcount);
        }
    }
    // all tests for now...
    return logret(true, "Ok");
}

// -------------------- ACCESS AND MODIFICATORS ------------------------

fsl                 fsarr_attach(fsarray *arr, fs s){
    // cnt is'nt used for now, just ++
    if (arr->ptr >= arr->sz)
        increasesize(arr, arr->sz * 2, false);
    // TODO: refactor to some king of iter
    arr->ar[arr->ptr].s = s;    //  for now just a copy! actually for gc() it's reuiqred only fs->v
    fsl fl = FSL(.s = s, .pos = arr->ptr++);
    arr->cnt++; // just for recoding
    return fl;
}

// -------------------------- (API) printers -----------------------

// this is normal printer
int                 fsarr_fprint(FILE *f, fsarray arr){
    int  cnt = 0;
    for (int i = 0; i < arr.sz; i++){
        if (fsstr(arr.ar[i].s) ) {
            fs_fprint(f, &arr.ar[i].s
//#ifdef FSNAMED
                    , arr.ar[i].name   // directly name!
//#else
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
    logenter("%p: %p", arr, arr ? arr->ar : 0);
    int     cnt = 0;
    if (arr && arr->ar){
        for (int i = 0; i < arr->sz; i++){ // not cnt!! cnt is just a sum of attached fs
            if (fsstr(arr->ar[i].s) ) {
                fsfree(arr->ar[i].s);
                cnt++;
//#ifdef FSNAMED
                arr->ar[i].name = 0; // it's ok because name is STATIC
//#endif
            }
        }
        free(arr->ar);
        arr->cnt = arr->sz = arr->ptr = 0;
        arr->ar = 0;
    } else
        logsimple("Null pointer");
    return logret(cnt, "Freed %d", cnt);
}

// initialize
fsarray             fsarr_init(int sz){
    fsarray ar = FSARRAY();
    increasesize(&ar, sz, true);
    return logsimpleret(ar, "Created with sz %d", ar.sz);
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

        fsarr_techfprint(stdout, fa);
        if (!fsarr_validate(stderr, fa) )
            return logacterr(fsarr_free(&fa), TEST_FAILED, "Validation failed");

        if (fa.sz < sz)
            return logacterr(fsarr_free(&fa), TEST_FAILED, "sz (%d) must be >= initial sz %d", fa.sz, sz);

        if (fa.cnt != 0)
            return logacterr(fsarr_free(&fa), TEST_FAILED, "Cnt = %d but must be 0, because not any objects after init", fa.sz);

        test_sub("subtest %d: freeall", ++subnum);

        fsarr_free(&fa);
        fsarr_techfprint(stdout, fa);
    }
    return logret(TEST_MANUAL, "done"); // TEST_FAILED, TEST_PASSED
}

// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf2(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d: init + validation", ++subnum);

        int sz = 50;
        fsarray fa = fsarr_init(sz);

        if (fa.sz < sz)
            return logacterr(fsarr_free(&fa), TEST_FAILED, "sz (%d) must be >= initial sz %d", fa.sz, sz);

        if (fa.cnt != 0)
            return logacterr(fsarr_free(&fa), TEST_FAILED, "Cnt = %d but must be 0, because not any objects after init", fa.sz);

        // try to use array but it UNUSUAL way, just as array
        for (int i = 0; i < sz; i++){
            //fs s = fscopy("str1");
            fs s = fsinit(100);
            // introdured fssprintf(s, format, ...);
            fssprintf(s, "str - %d", i);
            // NOW just a stupid iteration, WITHOUT fsarr_attach()
            namedfs nf = {.s = s, .name = ""};
            fa.ar[fa.cnt++] = nf;
        }
        fsarr_techfprint(logfile, fa);
        if (!fsarr_validate(stderr, fa) )
            return logacterr(fsarr_free(&fa),TEST_FAILED, "Validation failed");

    test_sub("subtest %d: check lines", ++subnum);

        char buf[100];
        // check the lines
        for (int i = 0; i < sz; i++){
            sprintf(buf, "str - %d", i);        // ok in this case
            if (strcmp(fsstr(fa.ar[i].s), buf) != 0)
                return logacterr(fsarr_free(&fa), TEST_FAILED, "fsarr[%d]: '%s' != origin '%s'", 
                    i, fsstr(fa.ar[i].s), buf);
        }

    test_sub("subtest %d: freeall", ++subnum);

        fsarrfree(fa); // via macro
        fsarr_techfprint(logfile, fa);
        if (!fsarr_validate(stderr, fa) )
            return logerr(TEST_FAILED, "Validation failed after free");
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED
}

// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf3(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d: cycle attach + validation", ++subnum);

        int sz = 10;
        fsarray fa = fsarr_init(sz);

        for (int i = 0; i < sz * 50; i++){
            fs s1 = FS();
            fssprintf(s1, "value bla bla %d", i);
            fsl fl = fsarr_attach(&fa, s1);  // probably to check result instead of raise? not sure
            if (i % 100 == 0)
                fsl_techfprint(logfile, fl);
        }
        fsarr_techfprintlim(logfile, fa, 3);

        if (!fsarr_validate(stderr, fa) )
            return logacterr(fsarrfree(fa),TEST_FAILED, "Validation failed");

        test_sub("subtest %d: freeall", ++subnum);

        fsarrfree(fa); // via macro
        fsarr_techfprintlim(logfile, fa, 3);
        if (!fsarr_validate(stderr, fa) )
            return logerr(TEST_FAILED, "Validation failed after free");
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED
}

// ------------------------------------------------------------------
int
main( /* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Simple init free test"                        , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf2, .num = 2, .name = "Init/free test with valid fs (random)"        , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf3, .num = 3, .name = "fsarr_attach test"                            , .desc=""                , .mandatory=true)
    );

    logclose("end...");
    return 0;
}

#endif /* FSARRAYTESTING */

