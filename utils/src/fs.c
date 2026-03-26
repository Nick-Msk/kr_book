#include <stdarg.h>

#include "common.h"
#include "log.h"
#include "fs.h"
#include "error.h"
#include "fileutils.h"

/********************************************************************
                    FAST STRING MODULE IMPLEMENTATION
********************************************************************/

#if defined(FS_ALLOCATOR)
    static char               **g_fs_ptr                = 0;
    static int                  g_alloc                 = 0;
    static const int            g_initsize              = 32;   // not sure
#endif

static const int                FS_SPRINTF_SZ           = 8192;

// external contol
int                             FS_MIN_ACCOC            = 128;
int                             FS_TECH_PRINT_COUNT     = 100; // symplos to print

// mem leak checking
static int                      g_alloc_cnt             = 0;
static int                      g_free_cnt              = 0;

// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers -------------------


// ------------------------------ Utilities -------------- ----------

// should depent on increase strategy, simplest return n + 1;
static inline int               calcnewsize(int n){
    int sz = round_up_2(n);
    return sz; //logsimpleret(sz, "newsz = %d", sz);
}

#if defined(FS_ALLOCATOR)
// ------------------------- ALLOCATOR ------------------------------

// non static, for atexit()
void                            fsfreeall(void){
    for (int i = 0; i < g_alloc; i++)
        if (g_fs_ptr[i]){
            free(g_fs_ptr[i]);
            g_fs_ptr[i] = 0;
        }
}

// find or increase size, raise if unable
static int                      findempty(void){
    // 1-st ver stupid alg
    int found = -1;
    for (int i = 0; i < g_alloc; i++)
        if (!g_fs_ptr[i])
            return logsimpleret(i, "found empty %d", i);
    // alloc new
    int tmpsz = calcnewsize(g_initsize) * sizeof(const char *);
    char **tmp = realloc(g_fs_ptr, tmpsz);
    if (!tmp)
        sysraiseint("Unable to allocate %d", tmpsz);  // return???
    found = g_alloc;
    for (; g_alloc < tmpsz; g_alloc++)
        tmp[g_alloc] = 0;
    g_alloc = tmpsz;
    g_fs_ptr = tmp;
    return logsimpleret(found, "Newly allocated %d", found);
}

static int                      findstr(const void *v){
    // now it's a stupid iteration via g_fs_ptr
    for (int i = 0; i < g_alloc; i++)
        if (g_fs_ptr[i] == v)
            return logsimpleret(i, "Found %p on %d", v, i);
    return logsimpleerr(-1, "Not found");
}

static bool                     detach(int pos){
    g_fs_ptr[pos] = 0;
        return logsimpleret(true, "%d detached", pos);
}



static bool                     check_duplicate(void){
    // TODO:
    int     totsz = g_alloc * sizeof(const char *);
    const char **ptr = malloc(totsz);
    if (!ptr){
        return logsimpleerr(false, "Unable to allocated sortarea (%d)", totsz);
    }
    memcpy(ptr, g_fs_ptr, totsz);
    qsort(ptr, g_alloc, sizeof(void *), pointer_cmp);
    for (int i = 1; i < g_alloc; i++)
        if (ptr[i - 1] == ptr[i] && ptr[i] != 0){
            fprintf(stderr, "ptr[%d] = ptr[%d] (%p)", i - 1, i, ptr[i]);    // not sure
            free(ptr);
            return logsimpleret(false, "ptr[%d] = ptr[%d] (%p)", i - 1, i, ptr[i]);
        }
    free(ptr);
    return true;
}

static bool                     fschecker(void){
    logenter("check allocation");
    if (!check_duplicate() ){
        printf("Duplicate found\n");
        return logerr(false, "Dupl failed");
    }
    printf("Checking is Passed\n");
    return logret(true, "Ok");
}

// technical printer
static int                      ffsprintall(FILE *f, int max){
    int         cnt = 0;
    bool        prev = false;
    static int  count = 0;
    max = (max > g_alloc || max == 0) ? g_alloc : max ;
    for (int i = 0; i < max; i++){
        if (g_fs_ptr[i]){
            cnt++;
            if (prev)
                fprintf(f, " - till [%d]", i - 1);
            fprintf(f, "alloc[%d] = %p ", i, g_fs_ptr[i]); count++;
            prev = false;
        } else if (!prev){
             fprintf(f, "alloc[%d] = 0x0 ", i); count++;
             prev = true;
        }
        if (count % 15 == 0)
            putchar('\n');
    }
    // count of valuable
    fprintf(f, "\nTotal: %d\n", cnt);
    return cnt;
}

static inline int                       fsprintall(int max){
    return ffsprintall(stdout, max);
}
static inline int                       attach(int pos, char *v){
    if (pos >= 0)   // if not detached!
        g_fs_ptr[pos] = v;
    return logsimpleret(pos, "%p attached to %d", v, pos);
}
#endif /* FS_ALLOCATOR */

// only for FS_FLAG_ALLOC
static fs                               increasesize(fs *s, int newsz, bool init){
    logenter("oldsz %d, newsz %d, init %s v %p, is alloc? %s", s->sz, newsz, bool_str(init), s->v, bool_str(fs_alloc(s)) );
    if (fs_alloc(s) ){
        if (init)
            newsz = calcnewsize(newsz);
        if (newsz > s->sz || !init){
                char *v = realloc(s->v, newsz);
                #if defined(FS_ALLOCATOR)
                if (init)   // init
                    s->pos = findempty();   // try to find an place
                attach(s->pos, s->v);
                #endif
                if (!v)
                    userraiseint(10, "Unable to allocate %d bytes", newsz);
                // now it's ok
                if (s->v == 0)      // only in case of REALLY new allocation
                    logauto(++g_alloc_cnt);
                s->v = v;
                s->sz = newsz;
         }
    }
    return logret(*s, "increased to %d", s->sz);
}


// --------------------------- API ---------------------------------

// ------------------ General functions ----------------------------

fs                                      *fs_shrink(fs *s){
    if (fs_alloc(s)){
        int     newsz = s->len + 1; // final '\0' is assumed!
        increasesize(s, newsz, false);
        return logsimpleret(s, "Shrinked %d", s->sz);
    } else
        return logsimpleret(s, "Not heap: shrink is skipped");
}

// can increase sz and len
char                                    *fs_elem(fs *s, int pos){
    if (pos >= s->sz){
        increasesize(s, pos < FS_MIN_ACCOC ? FS_MIN_ACCOC : pos, true);   // len remains the same here! sz is changed
        logsimple("size is adjusted to %d (pos %d)", s->sz, pos);
    }
    return fs_get(s, pos);
}

// TODO:
//int                     fs_sprintfend(fs *restrict s, const char *restrict fmt, ...){
//}

// snprint()
int                                     fs_sprintf(fs *restrict s, const char *restrict fmt, ...){
    static char buf[FS_SPRINTF_SZ]; // NO thread-safe this is

    va_list argp;
    va_start(argp, fmt);
    vsnprintf(buf, FS_SPRINTF_SZ - 1, fmt, argp);
    va_end(argp);
    //cnt = MIN(cnt, FS_SPRINTF_SZ - 1);
    fs_cpystr(s, buf);  // TODO: think about fs_cpynstr(fs, s, n);
    return s->len;
}

fs                                      *fs_resize(fs *s, int newsz){
    if (newsz > s->sz){
        increasesize(s, newsz, false);
        logsimple("size is adjusted to %d (pos %d)", s->sz, newsz);
    }
    return s;
}

fs                                      fs_cat(fs *target, fs source){
    int sumlen = target->len + source.len;
    if (target->sz <= sumlen) // sz must be at least len1 + len2 + 1
        increasesize(target, sumlen + 1, true);
    memcpy(target->v + target->len, source.v, source.len + 1);   // with last '\0'
    target->len = sumlen;
    return *target;
}

// -------------------------- (API) printers -----------------------
// this is not limit!
int                                     fs_fprint(FILE *restrict out, const fs *restrict s, const char *restrict name){
    int     cnt = 0;
    if (s){
        cnt = fprintf(out, "[%s: %s]", name, s->v);
    }
    return cnt;
}

// with  limit!
int                                     fs_fprintlim(FILE *restrict out, const fs *restrict s, int lim, const char *restrict name){
    int     cnt = 0;
    if (s){
        cnt = fprintf(out, "[%s: %.*s]", name, lim, s->v);
    }
    return cnt;
}

int                                     fs_fprint_arr(FILE *restrict out, const fs *restrict arr[]){
    int cnt = 0, i = 0;
    if (arr)
        for (; arr[i] != 0 && i < G_GLOB_AVERAGE; i++) // G_GLOB_AVERAGE to avoid endless loop
            cnt += fprintf(out, "[fs_arr_%d: %s]", i, arr[i]->v);
    if (i == G_GLOB_AVERAGE)
        logsimple("G_GLOB_AVERAGE's reached!!! %d", G_GLOB_AVERAGE);
    return cnt;
}

int                                     fs_techfprint(FILE *restrict out, const fs *restrict s, const char *restrict name){
    // technical print, statis attributes for now
    int     cnt = 0;
    int     len = MIN(FS_TECH_PRINT_COUNT, s->len);
    if (s){
        cnt += fprintf(out, "FS: %s: len [%d], sz [%d], flags [%d], s [%.*s", name, s->len, s->sz, s->flags, len, s->v);
        if (FS_TECH_PRINT_COUNT < s->len)
            cnt += fprintf(out, "...");
        cnt += fprintf(out, "]\n");
    }
    return cnt;
}

// out == 0 is OK fow now
bool                                    fs_validate(FILE *restrict out, const fs *restrict s){
    logenter("%p - %p", s, s ? s->v: 0);

    if (!s){    // TODO: think about string creation via faststring here!!
        if (out)
            fprintf(out, "null pointer");
        return logerr(false, "null pointer");
    }
    if (!s->v){
        if (out)
            fprintf(out, "nullable string");
        return logerr(false, "nullable string");
    }
    // depents on which iterator engine is active
    if (s->len >= s->sz){
        if (out)
            fprintf(out, "len [%d] must be < sz [%d]", s->len, s->sz);
        return logerr(false, "len [%d] must be < sz [%d]", s->len, s->sz);
    }
    {
        int len = strlen(s->v);
        if (len < s->len){
            if (out)
                fprintf(out, "srtlen [%d] can't be more than len [%d]", len, s->len);
            return logerr(false, "srtlen [%d] can't be more than len [%d]", len, s->len);
        }
    }
    return logret(true, "true");
}

extern bool                             fs_free_alloc_checker(int *freecnt, int *alloccnt){
    if (freecnt)
        *freecnt = g_free_cnt;
    if (alloccnt)
        *alloccnt= g_alloc_cnt;
    return logsimpleret(freecnt == alloccnt, "allocated %d, freed %d", g_alloc_cnt, g_free_cnt);
}

// --------------------------------- SERIALIZATION -----------------------------------------

// seqialization (strictly FULL save into the steam with only FS and .len info), out must be opened for write
int                                     fs_fsave(FILE *restrict out, const fs *restrict str){
    int cnt = 0;
    if (str){
        fprintf(out, "FS(%d):[%s]\n", str->len, str->v);
        cnt++;
    }
    return cnt;
}

int                                     fs_save(const char *restrict fname, const fs *restrict str){ 
    FILE *out = fopen(fname, "w");
    if (!out)
        userraiseint(ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open %s for write", fname);
    int cnt = fs_fsave(out, str);
    fclose(out);
    return cnt;
}

//  arr must be a pointer to NULL terminated array!
int                                     fs_fsave_arr(FILE *restrict out, const fs *restrict arr){
    int cnt = 0;
    while (arr)
        cnt += fs_fsave(out, arr++);
    return cnt;
}

// note: arr can be nullable, this mean 0 length array
int                                     fs_save_arr(const char *restrict fname, const fs *restrict arr){
    FILE *out = fopen(fname, "w");
    if (!out)
        userraiseint(ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open %s for write", fname);
    int cnt = fs_fsave_arr(out, arr);
    fclose(out);
    return cnt;
}

// raise int in case of wrong format
fs                                      fs_fload(FILE *restrict in, fs *restrict s){
    // FORMAT: FS(%d):[%s]\n
    unsigned     len = 0;
    char         pt1[] = "FS(", pt2[] = "):[", pt3[] = "]\n";

    // TODO: refactor that
    // TODO: FUSKIPFORMAT(in, pt) macro
    if (!freadpattern(in, pt1) )
        userraiseint(ERR_WRONG_INPUT_FORMAT, "Unable to read pattern '%s'", pt1);

    // TODO: FUGETVALUE() // int, char *, double are supported via _generic()
    if (fscanf(in, "%u", &len) < 1)
        userraiseint(ERR_WRONG_INPUT_FORMAT, "Unable to read fs length");

    if (!freadpattern(in, pt2) )
        userraiseint(ERR_WRONG_INPUT_FORMAT, "Unable to read pattern '%s'", pt2);

    fs          res = FS();
    // TODO: think is create a someting like: fs init_or_use(fs *origin)
    if (s)
        fs_resize(s, len + 1);
    else {
        res = fsinit(len + 1);
        s = &res;
    }
    // just read len bytes from current position
    if (fread(s->v, 1, len, in) < len)
        userraiseint(ERR_NOT_ENOGH_VALUES, "Unable to read %d bytes from stream", len);

    fsend(*s, len);  // fix the fs
    logsimple("%d[%s]", len, s->v);
    if (!freadpattern(in, pt3) )
        userraiseint(ERR_WRONG_INPUT_FORMAT, "Unable to read pattern '%s'", pt3);

    return *s;
}

fs                                      fs_load(const char *restrict fname, fs *restrict s){
    FILE *in = fopen(fname, "r");
    if (!in)
        userraiseint(ERR_UNABLE_OPEN_FILE_READ, "Unable to open %s for read", fname);
    fs str = fs_fload(in, s);
    fclose(in);
    return str;
}

// ------------------ API Constructs/Destrucor  ----------------------------

fs                                      fsinit(int n){
    fs      res = FS();     // fsalloc flag
    increasesize(&res, n, true);
    *res.v = '\0';
    return logsimpleret(res, "Created empty with sz %d", res.sz);
}

fs                                      fsclone(fs s){
    fs tmp = fsinit(s.len + 1);
    tmp.len = s.len;
    memcpy(tmp.v, s.v, tmp.len + 1);
    return tmp;
}

// destructor, macro wrapper will be
void                                    fs_free(fs *s){
    if (fs_alloc(s) || fs_moved(s)){    // actualy alloc must be a flag, but not statememnt TODO:
        logsimpleact(free(s->v), "freed... %p", s->v);   // WOW, logsimpleact?
        if (s->v)
            logauto(++g_free_cnt);   // calculate only  if really free memory
        s->sz = s->len = 0;
        s->v = 0;
    }
    if (fs_moved(s))
        free(s);    // because in that case fs in heap too
}

#if defined(FS_ALLOCATOR)
// detach from allocator! Must be freed manually
bool                    fsdetach(fs *s){
    bool        res = false;
    if (fs_alloc(s)){
        res = detach(s->pos);  // find in list and remove!
        s->pos = 0;
    }
    return res;
}
#endif

// -------------------------------Testing --------------------------
#ifdef FSTESTING

#include "test.h"
#include "checker.h"

//types for testing

static bool
check_leak(bool raise){
    int     f, a;
    fs_free_alloc_checker(&f, &a);
    if (f != a){
        if (raise)
            userraiseint(WARN_MEM_LEAK_DETECTED, "allocaed %d, freed %d", a, f);
        else
            userraise(false, WARN_MEM_LEAK_DETECTED, "WARNING: allocaed %d, freed %d", a, f);
    }
    return f == a;
}

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fsempty", ++subnum);
    {

        fs s = fsempty();
        if (s.len != 0 || s.sz != 0 || s.v != 0)
            return logerr(TEST_FAILED, "Empty fs validation failed");
        if (!fs_alloc(&s))
            return logerr(TEST_FAILED, "fs have no ALLOC");
        fsfree(s);  // should work normally
    }
    check_leak(true);
    test_sub("subtest %d: fsinit(100)", ++subnum);
    {
        fs s = fsinit(100);
        if (!fs_validate(logfile, &s) )
            return logacterr(fsfree(s), TEST_FAILED, "Validation's failed");
        if (!inv(s.len == 0 && s.sz >= 100 && s.v != 0 && fs_alloc(&s), "Failed") )
            return logacterr(fsfree(s), TEST_FAILED, "Condition violated, len [%d], sz [%d], v [%p], is alloc? (%s)",
                    s.len, s.sz, s.v, bool_str(fs_alloc(&s) ) );
        fsfree(s);
    }
    check_leak(true);
    test_sub("subtest %d: fsliteral", ++subnum);
    {
        const char *pattern = "1234567890";
        fs s = fsliteral(pattern);
        if (!fs_validate(logfile, &s) )
            return logerr(TEST_FAILED, "Validation's failed");
        if (!inv(s.len == (int)strlen(pattern) && s.sz == (int)strlen(pattern) + 1 && s.v != 0 && fs_static(&s), "Failed") )
            return logerr(TEST_FAILED, "Condition violated, len [%d], sz [%d], v [%p], is static? (%s)",
                    s.len, s.sz, s.v, bool_str(fs_static(&s) ) );
        fsfree(s);
    }
    check_leak(true);
    test_sub("subtest %d: fscopy", ++subnum);
    {

        const char *pattern = "1234567890";
        fs s = fscopy(pattern);
        if (!fs_validate(logfile, &s) )
            return logacterr(fsfree(s), TEST_FAILED, "Validation's failed");
        if (!inv(s.len == (int)strlen(pattern) && s.sz >= (int)strlen(pattern) + 1 && s.v != 0 && fs_alloc(&s), "Failed") )
            return logacterr(fsfree(s), TEST_FAILED, "Condition violated, len [%d], sz [%d], v [%p], is alloc? (%s)",
                s.len, s.sz, s.v, bool_str(fs_alloc(&s) ) );
        if (!inv(strcmp(fsstr(s), pattern) == 0, "Not equal") )
            return logacterr(fsfree(s), TEST_FAILED, "[%s] != pt [%s]", fsstr(s), pattern);

    test_sub("subtest %d: fsclone", ++subnum);

        fs s2 = fsclone(s);
        fsfree(s);

        if (!fs_validate(logfile, &s2) )
            return logacterr( fsfree(s2), TEST_FAILED, "Validation's failed");
        if (!inv(s2.len == (int)strlen(pattern) && s2.sz >= (int)strlen(pattern) + 1 && s2.v != 0 && fs_alloc(&s2), "Failed") )
            return logacterr( fsfree(s2), TEST_FAILED, "Condition violated, len [%d], sz [%d], v [%p], is alloc? (%s)",
                s2.len, s2.sz, s2.v, bool_str(fs_alloc(&s2) ) );
        if (!inv(strcmp(fsstr(s2), pattern) == 0, "Not equal") )
            return logacterr(fsfree(s2), TEST_FAILED, "[%s] != pt [%s]", fsstr(s2), pattern);

        fsfree(s2);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}


// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf2(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fscopy", ++subnum);
    {
        const char *pt = "just pattern";
        fs s = fscopy(pt);

        char c = get(s, 0);
        if (!inv (c == 'j', "[0] returns [%c], but should be [%c]", c, 'j') )
            return logacterr( fsfree(s), TEST_FAILED, "[0] returns [%c], but should be [%c]", c, 'j');

        test_sub("subtest %d: get", ++subnum);
        get(s, 3) = 'y';
        if (!inv(strcmp(fsstr(s), "jusy pattern") == 0, "[%s] != pt [%s]", fsstr(s), "jusy pattern") )
            return logacterr(fsfree(s), TEST_FAILED, "[%s] != pt [%s]", fsstr(s), "jusy pattern");

        // check 3 for 'y'
        c = get(s, 3);
        if (!inv (c == 'y', "[0] returns [%c], but should be [%c]", c, 'y') )
            return logacterr( fsfree(s), TEST_FAILED, "[0] returns [%c], but should be [%c]", c, 'y');

        // reallocation test
    test_sub("subtest %d: elem with realloc", ++subnum);

        elem(s, 100) = 'z';
        if (!fs_validate(logfile, &s) )
            return logacterr( fsfree(s), TEST_FAILED, "Validation's failed");
        c = get(s, 100);
        if (!inv (c == 'z', "[100] returns [%c], but should be [%c]", c, 'z') )
            return logacterr( fsfree(s), TEST_FAILED, "[100] returns [%c], but should be [%c]", c, 'z');
        fstechfprint(logfile, s);

        test_sub("subtest %d: validate", ++subnum);
        fsshrink(s);
        if (!fs_validate(logfile, &s) )
            return logacterr( fsfree(s), TEST_FAILED, "Validation's failed");
        fstechfprint(logfile, s);

        fsfree(s);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 3 ---------------------------------

static TestStatus
tf3(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: elem", ++subnum);
    {
        int     i;
        fs      s = fsempty();  // TODO: think if avoid
        char    pt[] = "test parretn 1234567";

        for (i = 0; i < (int) sizeof(pt); i++)
            elem(s, i) = pt[i];
        fsetlen(s, i);

        if (strcmp(s.v, pt) != 0)
            return logacterr( fsfree(s), TEST_FAILED, "Must be [%s] but not [%s]", pt, s.v);
        fsfree(s);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 4 ---------------------------------

static TestStatus
tf4(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fscat()", ++subnum);
    {
        const char  arr1[] = "q123456";
        const char  arr2[] = "zxcvbnm901";
        fs          s1 = fscopy(arr1);
        fs          s2 = fsliteral(arr2);

        fscat(s1, s2);
        fstechfprint(logfile, s1);   // for manual

        if (s1.len != (int) strlen(arr1) + strlen(arr2) )
            return logacterr( fsfree(s1), TEST_FAILED, "len = %d must be equal sum of l1 + l2 = %ld", fslen(s1), strlen(arr1) + strlen(arr2) );
        if (strstr(fsstr(s1), arr1) != fsstr(s1) )
            return logacterr( fsfree(s1), TEST_FAILED, "[%s] must start from [%s]", fsstr(s1), arr1);
        if (strstr(fsstr(s1), arr2) != fsstr(s1) + strlen(arr1) )
            return logacterr( fsfree(s1), TEST_FAILED, "[%s] must have [%s] on position %lu", fsstr(s1), arr2, strlen(arr1) );

    test_sub("subtest %d: fscatstr()", ++subnum);

        const char  arr3[] = "zaq1";
        int         len1 = fslen(s1);
        fscatstr(s1, arr3);
        fstechfprint(logfile, s1);   // for manual

        if (fslen(s1) != len1 + strlen(arr3) )
            return logacterr( fsfree(s1), TEST_FAILED, "len = %d must be equal sum of l1 + l2 = %lu", fslen(s1), len1 + strlen(arr3) );
        if (strstr(fsstr(s1), arr3) != fsstr(s1) + len1 )
            return logacterr( fsfree(s1), TEST_FAILED, "[%s] must have [%s] on position %d", fsstr(s1), arr3, len1 );
        if (strstr(fsstr(s1), arr1) != fsstr(s1) )  // the same test like in subtest 1
            return logacterr( fsfree(s1), TEST_FAILED, "[%s] must start from [%s]", fsstr(s1), arr1);

    test_sub("subtest %d: fscat() with empty initial test", ++subnum);
        {
            fs s3 = fscopy("");
            fstechfprint(logfile, s3);   // for manual
            fscat(s3, s2);
            fstechfprint(logfile, s3);   // for manual
            if (fslen(s3) != fslen(s2) )
                return logacterr( (fsfree(s1), fsfree(s3) ), TEST_FAILED, "len s3 = %d must be equal s2 = %d ", fslen(s3), fslen(s2) );
            if (fscmp(s3, s2) != 0)
                return logacterr( (fsfree(s1), fsfree(s3) ), TEST_FAILED, "%s not equal to %s", fsstr(s3), fsstr(s2) );
            fsfree(s3);
        }
        fsfree(s1);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 5 ---------------------------------

static TestStatus
tf5(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fscpy()", ++subnum);
    {

        const char  arr1[] = "q123456";
        const char  arr2[] = "zxcvbnm901";
        fs          s1 = fscopy(arr1);
        fs          s2 = fsliteral(arr2);

        fscpy(s1, s2);
        fstechfprint(logfile, s1);
        if (fslen(s1) != fslen(s2) )
            return logacterr( fsfree(s1), TEST_FAILED, "len s2 = %d must be equal s1 = %d ", fslen(s2), fslen(s1) );
        if (fscmp(s1, s2) != 0)
            return logacterr( fsfree(s1), TEST_FAILED, "%s not equal to %s", fsstr(s1), fsstr(s2) );

        test_sub("subtest %d: fscpy()", ++subnum);

        fscpystr(s1, arr1);
        if (fslen(s1) != (int)strlen(arr1) )
            return logacterr( fsfree(s1), TEST_FAILED, "len s2 = %lu must be equal s1 = %d ",strlen(arr1), fslen(s1) );
        if (strcmp(fsstr(s1), arr1) != 0)
            return logacterr( fsfree(s1), TEST_FAILED, "%s not equal to %s", fsstr(s1), arr1);

        fsfree(s1);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 6 ---------------------------------

static TestStatus
tf6(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fsfreeall", ++subnum);
    {

        fs i1 = fsinit(100);
        fs i2 = fsinit(1000);
        fs i3 = fsinit(10);
        fsfreeall(&i1, &i2, &i3);
        if (i1.v || i2.v || i3.v)
            return logacterr( (fsfree(i1), fsfree(i2), fsfree(i3) ), TEST_FAILED, "Still not null");
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 7 ---------------------------------

static TestStatus
tf7(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fsprint MANUAL", ++subnum);
    {

        char    arr[] = "Hello, World from fs!\n";

        fs s = fscopy(arr);
        fsprint(s);

        int     lim = 3;
        printf("Limit %d\n", lim);
        fsprintlim(s, lim);

        fsfree(s);
    }
    check_leak(true);
    return logret(TEST_MANUAL, "done"); // TEST_FAILED, TEST_PASSED
}

// ------------------------- TEST 8 ---------------------------------

static TestStatus
tf8(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fsprint_arr MANUAL", ++subnum);
    {

        fs s1 = fsliteral("123");
        fs s2 = fsliteral("345");
        fs s3 = fsliteral("67890");

        fsfprint_arr(logfile, &s1, &s2, &s3);

        fsfreeall(&s1, &s2, &s3);
    }
    check_leak(true);
    return logret(TEST_MANUAL, "done"); // TEST_FAILED, TEST_PASSED
}

// ------------------------- TEST 9 ---------------------------------

static TestStatus
tf9(const char *name)
{
    logenter("%s", name);

    int         subnum = 0;
    char buf[1000], fmt[] = "%d, %f, %s\n";

    test_sub("subtest %d: fs_printf with more than enough (1000 char)", ++subnum);
    {
        {
            fs s1 = fsinit(1000);

            snprintf(buf, sizeof(buf) - 1, fmt, 1223, 1.445, "Blablabla");
            fs_sprintf(&s1, fmt, 1223, 1.445, "Blablabla");
            fstechfprint(logfile, s1);  // for manual checking

            if ( (int)strlen(buf) != s1.len)
                return logacterr( fsfree(s1), TEST_FAILED, "len buf = %lu must be equal s1 = %d ", strlen(buf), fslen(s1) );
            // compary strings TODO:
            if (strcmp(buf, fsstr(s1) ) != 0)
                return logacterr( fsfree(s1), TEST_FAILED, "buf [%s] must be equal s1 [%s]", buf, fsstr(s1) );
            fsfree(s1);
        }
    test_sub("subtest %d: fs_printf with small init size (2 char)", ++subnum);
        {
            fs s2 = fsinit(1);
            // the same test, but for very small fs
            snprintf(buf, sizeof(buf) - 1, fmt, 12345, 9.8765, "XXXYYYYZZZZZZZZZRRRRR");
            fs_sprintf(&s2, fmt, 12345, 9.8765, "XXXYYYYZZZZZZZZZRRRRR");
            fstechfprint(logfile, s2);  // for manual checking

            if ( (int)strlen(buf) != s2.len)
                return logacterr( fsfree(s2), TEST_FAILED, "len buf = %lu must be equal s1 = %d ", strlen(buf), fslen(s2) );
            // compary strings TODO:
            if (strcmp(buf, fsstr(s2) ) != 0)
                return logacterr( fsfree(s2), TEST_FAILED, "buf [%s] must be equal s1 [%s]", buf, fsstr(s2) );
            fsfree(s2);
        }
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 10 ---------------------------------

static TestStatus
tf10(const char *name)
{
    logenter("%s", name);

    int         subnum = 0;

    test_sub("subtest %d: fslocal", ++subnum);
    {
        fslocal(s1, 15);
        elem(s1, 0) = 't';
        get(s1, 1) = 'z';
        fsend(s1, 2);
        char c = get(s1, 0), c1 = get(s1, 1);
        if (c != 't' || c1 != 'z')
            return logerr(TEST_FAILED, "s1[0] must be = 't' but not '%c', OR s1[1] must be 'z' but not '%c'", c, c1);

    test_sub("subtest %d: test with pattern", ++subnum);

        const char *pt = "qwe1234567890";

        strcpy(fsstr(s1), pt);
        // .len is still 2
        if (strcmp(fsstr(s1), pt) != 0)
            return logerr(TEST_FAILED, "fs [%s] != [%s]", fsstr(s1), pt);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 11 ---------------------------------

static TestStatus
tf11(const char *name)
{
    logenter("%s", name);

    int         subnum = 0;

    test_sub("subtest %d: fssave/load 1 fs", ++subnum);
    {
        const char fname[] = "res/fs_test_save_load.dat";
        fs s = fscopy("Tra la la 1234567890");
        if (fssave(fname, s) != 1)
            return logacterr(fsfree(s), TEST_FAILED, "Fssave returns != 1 value");
        fs s2 = fs_load(fname, 0); // w/o macro
        fstechfprint(logfile, s2);
        if (s.len != s2.len || fscmp(s, s2) != 0)
            return logacterr(fsfreeall(&s, &s2), TEST_FAILED, "Strings not equal (%d - %d)[%s] - [%s]", s.len, s2.len, fsstr(s), fsstr(s2) );

    test_sub("subtest %d: fs_save multiples fs", ++subnum);

        const char   fname2[] = "res/fs_test_save_load_mass.dat";
        FILE        *f = fopen(fname2, "w+");
        if (!f)
            return logacterr(fsfreeall(&s, &s2), TEST_FAILED, "Unable to open %s for w+", fname2);
        for (int i = 0; i < 15; i++){
            fssprintf(s, "Check val %d", i);
            if (fsfsave(f, s) != 1)
                return logacterr(fsfreeall(&s, &s2), TEST_FAILED, "Fssave returns != 1 value");
        }
        // try to load
    test_sub("subtest %d: fs_load multiples fs", ++subnum);

        rewind(f);  // from the start
        char        buf[100];
        for (int i = 0; i < 15; i++){
            s2 = fsfload(f, s2);
            sprintf(buf, "Check val %d", i);    // make a pattern
            if (strcmp(buf, fsstr(s2) ) != 0)
                return logacterr( (fclose(f), fsfreeall(&s, &s2) ), TEST_FAILED, "Strings not equal, loaded %s != pattern %s", fsstr(s2), buf);
        }
        fclose(f);
        fsfreeall(&s, &s2);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 12 ---------------------------------

static TestStatus
tf12(const char *name)
{
    logenter("%s", name);

    int         subnum = 0;
    int         freed, allocated;

    test_sub("subtest %d: simple init", ++subnum);
    {
        fs s1 = fsinit(100);
        fsfree(s1);

        fs_free_alloc_checker(&freed, &allocated);
        if (freed !=  allocated)
            return logerr(TEST_FAILED, "must be equal f/a counter  0:0, but not %d:%d", freed, allocated);
    }
    test_sub("subtest %d: via elem()", ++subnum);
    {
        fs s1 = FS();   // empty, alloc
        for (int i = 0; i < 1000000; i++)
            elem(s1, i) = 'a';
        fsfree(s1);
        fs_free_alloc_checker(&freed, &allocated);
        if (freed !=  allocated)
            return logerr(TEST_FAILED, "must be equal f/a counter  0:0, but not %d:%d", freed, allocated);
    }
    test_sub("subtest %d: via clone()", ++subnum);
    {
        fs s1 = fsliteral("12345");
        fs s2 = fsclone(s1);
        elem(s2, 10000) = 'c';
        fsfree(s2);
        fs_free_alloc_checker(&freed, &allocated);
        if (freed !=  allocated)
            return logerr(TEST_FAILED, "must be equal f/a counter  0:0, but not %d:%d", freed, allocated);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main( /* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 = tf1,  .num =  1, .name = "Simple init and validate test" , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf2,  .num =  2, .name = "Access read/write test"        , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf3,  .num =  3, .name = "Elem() test"                   , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf4,  .num =  4, .name = "fs_cat/fs_catstr test"         , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf5,  .num =  5, .name = "fs_cpy/fs_cpystr test"         , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf6,  .num =  6, .name = "fsfreeall test"                , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf7,  .num =  7, .name = "fsprint/printlim manual test"  , .desc="always ok, for the manual check"                , .mandatory=true)
      , testnew(.f2 = tf8,  .num =  8, .name = "fsprint_arr manual test"       , .desc="always ok, for the manual check"                , .mandatory=true)
      , testnew(.f2 = tf9,  .num =  9, .name = "fs_sprintf formatted test"     , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf10, .num = 10, .name = "fslocal simple test"           , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf11, .num = 11, .name = "fs_save/load test"             , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf12, .num = 12, .name = "fs_free_alloc_checker test"    , .desc=""                , .mandatory=true)
    );

    logclose("end...");
    return 0;
}

#endif /* FSTESTING */

