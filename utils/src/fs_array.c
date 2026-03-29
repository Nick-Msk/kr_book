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
        cnt += fprintf(f, "FSARRAY: sz %d, cnt %d, ptr %p:\n", arr->sz, arr->cnt, arr->ar);
        if (arr->ar)
            for (int i = 0; i < MIN(arr->cnt, lim); i++){
                cnt += fstechfprint(f, arr->ar[i]);   // directly name!
            }
        if (lim < arr->cnt)
            cnt += fprintf(f, "and more... (%d)\n", arr->cnt - lim);
    }
    return cnt;
}

int                             fsarr_techfprint(FILE *restrict f, const fsarray *restrict arr){
    return fsarr_techfprintlim(f, arr, arr->sz);
}

// ------------------------------ Utilities -------------------------

static int                      fsarr_mass_init(fsarray *arr, int min, int max){
    int cnt = 0;
    for (int i = min; i < max; i++)
        arr->ar[i] = FS(), cnt++;  // with alloc flag
    return cnt;
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

// compare as pointer
static int                      fsptrcmp(const void *ns1, const void *ns2){
    return ((fs *) ns1)->v - ((fs *) ns2)->v; // OMG
}

// sorting via qsort()
static fsarray                  sortfs(const fsarray *origin){
    // make a copy first
    fsarray tmp = FSARRAY();
    increasesize(&tmp, origin->cnt, false);  // exactly count of origin
    int     cnt = 0;
    for (int i = 0; i < origin->cnt; i++)
        if ( !fsisnull(origin->ar[i] ) )
            tmp.ar[cnt++] = origin->ar[i];    // all namedfs, not reeally need actually
    // sort
    tmp.sz = cnt;
    if (tmp.sz > 0)
        qsort(tmp.ar, tmp.sz, sizeof(fs), fsptrcmp);
    return tmp;
}


// --------------------------- API ---------------------------------
// ------------------ General functions ----------------------------

#define             FS_VALIDATE_FAILED(out, msg, ...) {\
                    if (out)\
                        fprintf( (out), "%s: ", __func__), fprintf( (out), (msg), ##__VA_ARGS__), fputc('\n', (out) );\
                    return logerr(false, (msg), ##__VA_ARGS__); }

// never raise exception! just return true/false and put messages into f (if != 0)
bool                fsarr_validate(FILE *restrict out, const fsarray *restrict arr){
    logenter("%p", out);
    logmsg("check 1");
    if (arr->sz < 0 || arr->cnt < 0 || arr->cnt > arr->sz){
        FS_VALIDATE_FAILED(out, "sz(%d) and cnt(%d) must be positive, and sz >= cnt", arr->sz, arr->cnt);
    }
    if (arr->cnt > 0){
        logmsg("cnt > 0, check 2");
        fsarray   tmp = sortfs(arr);   // make a sorting by pointer arr.ar[].v, tmp is shrinked and NOT valid, all fs are pointer to fs of arr
        int     duplcount = 0;
        for (int i = 1; i < tmp.sz; i++)
            if (tmp.ar[i - 1].v == tmp.ar[i].v){
                if (out)
                    fprintf(out, "elem [%d] == elem [%d] == %p:[%s]", i - 1, i, tmp.ar[i].v, tmp.ar[i].v);
                duplcount++;
            }
        free(tmp.ar);       // free temporary array (not fs!)
        if (duplcount > 0)
            FS_VALIDATE_FAILED(out, "Total duplicates %d", duplcount);
    }
    logmsg("all tests for now...");
    return logret(true, "Ok");
}


// -------------------- ACCESS AND MODIFICATORS ------------------------

// TODO: via fsl ( struct fsl { int loc; }; 
// fsl f1 = getfsl(fs_array a);
// f1.elem() = ... 
// f1.get() = ...

int                         fsarr_increase(fsarray *arr, int newsize){
    logenter("newsize %d", newsize);
    if (newsize > arr->sz)
        increasesize(arr, newsize, true);
    fsarr_mass_init(arr, arr->cnt, newsize);
    arr->cnt = newsize;
    return logret(arr->sz, "New sz %d, cnt %d", arr->sz, arr->cnt);
}

int                         fsarr_shrink(fsarray *arr, int newsize){
    if (newsize < arr->sz && newsize >= 0){
        if (newsize < arr->cnt)
            fsarr_mass_free(arr, newsize, arr->cnt), arr->cnt = newsize; // from from newsize to current cnt
        increasesize(arr, newsize, false);
    }
    return logsimpleret(arr->sz, "Shrinked to %d, new cnt %d", arr->sz, arr->cnt);
}

// -------------------------- (API) printers -----------------------

// this is normal printer
int                         fsarr_fprint(FILE *restrict f, const fsarray *restrict arr){
    int  cnt = 0;
    for (int i = 0; i < arr->cnt; i++){
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
    int     cnt = fsarr_fsave(out, arr);
    fclose(out);
    return cnt;
}

// f is open for write
int                         fsarr_fsave(FILE *restrict f, const fsarray *restrict arr){
    // signature
    fprintf(f, "FSARRAY(%d):[\n", arr->cnt);
    if (!arr->ar)
        return userraise(ERR_NULLABLE_PTR, -1, "Nullable ar, can't serialize"); 
    for (int i = 0; i < arr->cnt; i++){
        fprintf(f, "%4d:", i);
        fs_fsave(f, fsarr_get(arr, i) );
        //fputc('\n', f);  not sure
    }
    fprintf(f, "]");
    return logsimpleret(arr->cnt, "Total saved %d", arr->cnt);
}

fsarray                     fsarr_load(const char *fname){
    FILE    *out = fopen(fname, "r");
    if (!out)
        userraiseint(ERR_UNABLE_OPEN_FILE_READ, "Unable to open %s for read", fname);
    fsarray fa = fsarr_fload(out);
    fclose(out);
    return fa;
}

fsarray                     fsarr_fload(FILE *restrict f){
    // format: "FSARRAY: sz %d cnt %d ptr %d:[\n"
    FUSKIPFORMAT(f, "FSARRAY(");
    int cnt = FUGETUNSIGNED(f);

    logauto(cnt);
    FUSKIPFORMAT(f, "):[\n");
    fsarray     a = fsarr_init(cnt);
    // cycle for fs
    for (int i = 0; i < cnt; i++){
        FUSKIPFORMATPRINTF(f, "%4d:", i); 
        a.ar[i] = fs_fload(f, 0);       // TODO: probably to usr fsarr_get()?
    }
    FUSKIPFORMAT(f, "]");
    return a;
}

// ------------------ API Constructs/Destrucor  ----------------------------

int                 fsarr_free(fsarray *arr){

    logenter("%p: %p, cnt %d", arr, arr ? arr->ar : 0, arr ? arr->cnt : 0);
    int   cnt = 0;
    if (arr)
        cnt = fsarr_mass_free(arr, 0, arr->cnt);
    free(arr->ar);
    arr->ar = 0;
    arr->sz = arr->cnt = 0;
    return logret(cnt, "Freed %d", cnt);
}

// initialize
fsarray             fsarr_init(int cnt){
    fsarray ar = FSARRAY();
    increasesize(&ar, cnt, true);
    fsarr_mass_init(&ar, 0, ar.cnt = cnt);
    return logsimpleret(ar, "Created with sz %d, cnt %d", ar.sz, ar.cnt);
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

    test_sub("subtest %d: init", ++subnum);
    {

        int cnt = 50;
        fsarray fa = fsarr_init(cnt);

        fsarr_techfprint(logfile, &fa);
        test_validatefree(fsarr_validate(stderr, &fa), fsarr_free(&fa), "Validation failed");
        //if (!fsarr_validate(stderr, &fa) )
        //    return logacterr(fsarr_free(&fa), TEST_FAILED, "Validation failed");

        if (fa.sz < cnt)
            return logacterr(fsarr_free(&fa), TEST_FAILED, "sz(%d) must be >= initial cnt(%d)", fa.sz, cnt);

        if (fa.cnt != cnt)
            return logacterr(fsarr_free(&fa), TEST_FAILED, "cnt(%d) must be = initial cnt(%d)", fa.cnt, cnt);

        test_sub("subtest %d: freeall", ++subnum);

        fsarr_techfprint(logfile, &fa);
        fsarr_free(&fa);
        fsarr_techfprint(logfile, &fa);
    }
    fs_alloc_check(true);
    return logret(TEST_MANUAL, "done"); // TEST_FAILED, TEST_PASSED
}

// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf2(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: init + validation", ++subnum);
    {

        int cnt = 30;
        fsarray fa = fsarr_init(cnt);

        test_validatefree(fa.sz >= cnt && fa.cnt == cnt, fsarr_free(&fa), "sz(%d) must be >= initial cnt(%d) and cnt(%d) must be equal to initial", fa.sz, fa.cnt, cnt);

        for (int i = 0; i < cnt; i++){
            if (i < cnt / 2)     // fs_attach to be here?
                fa.ar[i] = fsinit(100);        // 50 with init, 50 - automatic
            // introdured fssprintf(s, format, ...);
            fssprintf(fa.ar[i], "str - %d", i);
        }
        fsarr_techfprint(logfile, &fa);
        test_validate(fsarr_validate(stderr, &fa), "Validation failed");

    test_sub("subtest %d: check lines", ++subnum);

        char buf[100];
        // check the lines
        for (int i = 0; i < cnt; i++){
            snprintf(buf, sizeof(buf) - 1, "str - %d", i);        // ok in this case
            test_validatefree(strcmp(fa.ar[i].v, buf) == 0, fsarr_free(&fa), 
                "fsarr[%d]: '%s' != origin '%s'", i, fa.ar[i].v, buf);
        }

    test_sub("subtest %d: freeall", ++subnum);

        fsarrfree(fa); // via macro
        fsarr_techfprint(logfile, &fa);
        test_validate(fsarr_validate(stderr, &fa), "Validation failed after free");
    }
    fs_alloc_check(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED
}

// ------------------------- TEST 3 ---------------------------------

static TestStatus
tf3(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: cycle attach + validation", ++subnum);
    {

        int cnt = 50;
        fsarray fa = fsarr_init(cnt);
        fs      s;

        for (int i = 0; i < cnt; i++){
            s = FS();
            fssprintf(s, "value bla bla %d", i);
            fsarrattach(fa, i, s);
        }
        fsarr_techfprintlim(logfile, &fa, 3);

        test_validate(fsarr_validate(stderr, &fa), "Validation failed");

    test_sub("subtest %d: check array", ++subnum);

        for (int i = 0; i < cnt; i++){
            char buf[100];
            snprintf(buf, sizeof(buf) - 1, "value bla bla %d", i);
            test_validatefree(strcmp(buf, fa.ar[i].v) == 0, fsarrfree(fa), "[%s] must  be equal fs[%d] [%s]", buf, i, fa.ar[i].v );
        }

    test_sub("subtest %d: freeall", ++subnum);

        fsarrfree(fa); // via macro
        fsarr_techfprintlim(logfile, &fa, 3);
        test_validate(fsarr_validate(stderr, &fa), "Validation failed after free");
    }
    fs_alloc_check(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED
}

// ------------------------- TEST 4 ---------------------------------

static TestStatus
tf4(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d: fsarr_save", ++subnum);

        int     cnt = 10;
        fsarray  fa = fsarr_init(cnt);
        //fs      s1[sz]; //temporary
        for (int i = 0; i < cnt; i++){
            fssprintf(fsarrget(fa, i), "Just test string %d", i);
        }
        int     res = fsarr_save("res/fsarr2.sv", &fa);
        test_validatefree(res == cnt, fsarrfree(fa), "fsarr_save have to return %d but not %d", cnt, res);
        fsarrfree(fa); // via macro
    }
    fs_alloc_check(true);
    return logret(TEST_MANUAL, "done"); // TEST_FAILED, TEST_PASSED
}

// ------------------------- TEST 5 ---------------------------------

static TestStatus
tf5(const char *name)
{
    logenter("%s", name);
    int             subnum = 0;
    const char     *fname = "res/fsarr_save_load2.sv";

    test_sub("subtest %d: fsarr_save", ++subnum);
    {

        int     cnt = 10;
        fsarray fa = fsarr_init(cnt);
        //fs      s1[sz]; //temporary
        for (int i = 0; i < cnt; i++){
            fssprintf(fa.ar[i], "Just test string %d", i);
        }
        int     res = fsarr_save(fname, &fa);
        test_validatefree(res == cnt, fsarrfree(fa), "fsarr_save have to return %d but not %d", cnt, res);

    test_sub("subtest %d: fsarr_load", ++subnum);

        fsarray fa1 = fsarr_load(fname);

        test_validatefree(fa1.cnt == fa.cnt, (fsarrfree(fa1), fsarrfree(fa) ), "Loaded cnt = %d must be equal to ogiginal cnt %d", fa1.cnt, fa.cnt);

        for (int i = 0; i < fa.cnt; i++) {
            test_validatefree(fslen(fa.ar[i] ) == fslen(fa.ar[i] ), (fsarrfree(fa1), fsarrfree(fa) ),
                    "Length of fs %d of origin(%d) and loaded(%d) array must be euqal", i, fslen(fa.ar[i] ), fslen(fa.ar[i] ) );
            if (fs_cmp(fsarr_get(&fa, i), fsarr_get(&fa1, i) ) != 0)
                    test_validatefree(fs_cmp(fsarr_get(&fa, i), fsarr_get(&fa1, i) ) == 0,
                                        (fsarrfree(fa1), fsarrfree(fa) ),
                                         "%d Loaded string [%s] != origin string [%s]", i, fs_str(fsarr_get(&fa, i)), fs_str(fsarr_get(&fa1, i) ) );
        }
        fsarrfree(fa1);
        fsarrfree(fa);
    }
    fs_alloc_check(true);
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
      , testnew(.f2 = tf4, .num = 4, .name = "fsarr_save test"                              , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf5, .num = 5, .name = "fsarr_save/load test"                         , .desc=""                , .mandatory=true)
    );

    return logcloseret(0, "end...");
}

#endif /* FSARRAYTESTING */


