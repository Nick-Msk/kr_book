#include "common.h"
#include "log.h"
#include "fs.h"
#include "error.h"

/********************************************************************
                    FAST STRING MODULE IMPLEMENTATION
********************************************************************/

#if defined(FS_ALLOCATOR)
    static char               **g_fs_ptr                = 0;
    static int                  g_alloc                 = 0;
    static const int            g_initsize              = 32;   // not sure
#endif

// external contol
int                         FS_MIN_ACCOC            = 128;
int                         FS_TECH_PRINT_COUNT     = 10; // symplos to print

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

static inline int               fsprintall(int max){
    return ffsprintall(stdout, max);
}
static inline int                      attach(int pos, char *v){
    if (pos >= 0)   // if not detached!
        g_fs_ptr[pos] = v;
    return logsimpleret(pos, "%p attached to %d", v, pos);
}
#endif /* FS_ALLOCATOR */

// only for FS_FLAG_ALLOC
static fs                       increasesize(fs *s, int newsz, bool init){
    logenter("newsz %d, init %s v %p, is alloc? %s", newsz, bool_str(init), s->v, bool_str(fs_alloc(s)) );
    if (fs_alloc(s) ){
        if (init)
            newsz = calcnewsize(newsz);
        if (newsz > s->sz || !init){
                s->v = realloc(s->v, newsz);
                #if defined(FS_ALLOCATOR)
                if (init)   // init
                    s->pos = findempty();   // try to find an place
                attach(s->pos, s->v);
                #endif
                if (!s->v)
                    userraiseint(10, "Unable to allocate %d bytes", newsz);
                s->sz = newsz;
         }
    }
    return logret(*s, "increased to %d", s->sz);
}


// --------------------------- API ---------------------------------

// ------------------ General functions ----------------------------

fs                     *fs_shrink(fs *s){
    if (fs_alloc(s)){
        int     newsz = s->len + 1; // final '\0' is assumed!
        increasesize(s, newsz, false);
        return logsimpleret(s, "Shrinked %d", s->sz);
    } else
        return logsimpleret(s, "Not heap: shrink is skipped");
}

// can increase sz and len
char                   *fs_elem(fs *s, int pos){
    if (pos >= s->sz){
        increasesize(s, pos < FS_MIN_ACCOC ? FS_MIN_ACCOC : pos, true);   // len remains the same here! sz is changed
        logsimple("size is adjusted to %d (pos %d)", s->sz, pos);
    }
    return fs_get(s, pos);
}

fs                   *fs_resize(fs *s, int newsz){
    if (newsz > s->sz){
        increasesize(s, newsz, false);
        logsimple("size is adjusted to %d (pos %d)", s->sz, newsz);
    }
    return s;
}

// -------------------------- (API) printers -----------------------
int                     fs_techfprint(FILE *restrict out, const fs *restrict s){
    // technical print, statis attributes for now
    int     cnt;
    cnt = fprintf(out, "len [%d], sz [%d], flags [%d], s [%p]=[", s->len, s->sz, s->flags, s->v);
    if (s->v){
        for (int i = 0; i < FS_TECH_PRINT_COUNT && i < s->len; i++)
             fputc(s->v[i], out), cnt++;
        if (FS_TECH_PRINT_COUNT < s->len)
            cnt += fprintf(out, "...");
        cnt += fprintf(out, "]\n");
    }
    else
        cnt = fprintf(out, "]\n");
    return cnt;
}

bool                    fs_validate(FILE *restrict out, const fs *restrict s){
    logenter("%p - %p", s, s ? s->v: 0);

    if (!s){    // TODO: think about string creation via faststring here!!
        fprintf(out, "null pointer");
        return logerr(false, "null pointer");
    }
    if (!s->v){
        fprintf(out, "nullable string");
        return logerr(false, "nullable string");
    }
    // depents on which iterator engine is active
    if (s->len >= s->sz){
        fprintf(out, "len [%d] must be < sz [%d]", s->len, s->sz);
        return logerr(false, "len [%d] must be < sz [%d]", s->len, s->sz);
    }
    {
        int len = strlen(s->v);
        if (len < s->len){
            fprintf(out, "srtlen [%d] can't be more than len [%d]", len, s->len);
            return logerr(false, "srtlen [%d] can't be more than len [%d]", len, s->len);
        }
    }
    return logret(true, "true");
}

// ------------------ API Constructs/Destrucor  ----------------------------

fs                      fsinit(int n){

    fs      res = FS();
    increasesize(&res, n, true);
    *res.v = '\0';
    return logsimpleret(res, "Created empty with sz %d", res.sz);
}

fs                      fsclone(fs s){
    fs tmp = fsinit(s.len + 1);
    tmp.len = s.len;
    memcpy(tmp.v, s.v, tmp.len + 1);
    return tmp;
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

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);

        fs s = fsempty();
        if (s.len != 0 || s.sz != 0 || s.v != 0)
            return logerr(TEST_FAILED, "Empty fs validation failed");
        if (!fs_alloc(&s))
            return logerr(TEST_FAILED, "fs have no ALLOC");
        fsfree(s);  // should work normally
    }
    {
        test_sub("subtest %d", ++subnum);

        fs s = fsinit(100);
        if (!fs_validate(logfile, &s) )
            return logacterr(fsfree(s), TEST_FAILED, "Validation's failed");
        if (!inv(s.len == 0 && s.sz >= 100 && s.v != 0 && fs_alloc(&s), "Failed") )
            return logacterr(fsfree(s), TEST_FAILED, "Condition violated, len [%d], sz [%d], v [%p], is alloc? (%s)",
                    s.len, s.sz, s.v, bool_str(fs_alloc(&s) ) );
        fsfree(s);
    }
    {
        test_sub("subtest %d", ++subnum);

        const char *pattern = "1234567890";
        fs s = fsliteral(pattern);
        if (!fs_validate(logfile, &s) )
            return logerr(TEST_FAILED, "Validation's failed");
        if (!inv(s.len == (int)strlen(pattern) && s.sz == (int)strlen(pattern) + 1 && s.v != 0 && fs_static(&s), "Failed") )
            return logerr(TEST_FAILED, "Condition violated, len [%d], sz [%d], v [%p], is static? (%s)",
                    s.len, s.sz, s.v, bool_str(fs_static(&s) ) );
        fsfree(s);
    }
    {
        test_sub("subtest %d", ++subnum);

        const char *pattern = "1234567890";
        fs s = fscopy(pattern);
        if (!fs_validate(logfile, &s) )
            return logacterr(fsfree(s), TEST_FAILED, "Validation's failed");
        if (!inv(s.len == (int)strlen(pattern) && s.sz >= (int)strlen(pattern) + 1 && s.v != 0 && fs_alloc(&s), "Failed") )
            return logacterr(fsfree(s), TEST_FAILED, "Condition violated, len [%d], sz [%d], v [%p], is alloc? (%s)",
                s.len, s.sz, s.v, bool_str(fs_alloc(&s) ) );
        if (!inv(strcmp(fsstr(s), pattern) == 0, "Not equal") )
            return logacterr(fsfree(s), TEST_FAILED, "[%s] != pt [%s]", fsstr(s), pattern);

        test_sub("subtest %d", ++subnum);

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
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}


// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf2(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);

        const char *pt = "just pattern";
        fs s = fscopy(pt);

        char c = get(s, 0);
        if (!inv (c == 'j', "[0] returns [%c], but should be [%c]", c, 'j') )
            return logacterr( fsfree(s), TEST_FAILED, "[0] returns [%c], but should be [%c]", c, 'j');

        test_sub("subtest %d", ++subnum);
        get(s, 3) = 'y';
        if (!inv(strcmp(fsstr(s), "jusy pattern") == 0, "[%s] != pt [%s]", fsstr(s), "jusy pattern") )
            return logacterr(fsfree(s), TEST_FAILED, "[%s] != pt [%s]", fsstr(s), "jusy pattern");

        // check 3 for 'y'
        c = get(s, 3);
        if (!inv (c == 'y', "[0] returns [%c], but should be [%c]", c, 'y') )
            return logacterr( fsfree(s), TEST_FAILED, "[0] returns [%c], but should be [%c]", c, 'y');

        // reallocation test
        test_sub("subtest %d", ++subnum);

        elem(s, 100) = 'z';
        if (!fs_validate(logfile, &s) )
            return logacterr( fsfree(s), TEST_FAILED, "Validation's failed");
        c = get(s, 100);
        if (!inv (c == 'z', "[100] returns [%c], but should be [%c]", c, 'z') )
            return logacterr( fsfree(s), TEST_FAILED, "[100] returns [%c], but should be [%c]", c, 'z');
        fs_techfprint(logfile, &s);

        test_sub("subtest %d", ++subnum);
        fsshrink(s);
        if (!fs_validate(logfile, &s) )
            return logacterr( fsfree(s), TEST_FAILED, "Validation's failed");
        fs_techfprint(logfile, &s);

        fsfree(s);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 3 ---------------------------------

static TestStatus
tf3(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);

        int     i;
        fs      s = fsempty();  // TODO: think if avoid
        char    pt[] = "test parretn 1234567";

        for (i = 0; i < (int)sizeof(pt); i++)
            elem(s, i) = pt[i];
        fsetlen(s, i);

        if (strcmp(s.v, pt) != 0)
            return logacterr( fsfree(s), TEST_FAILED, "Must be [%s] but not [%s]", pt, s.v);
        fsfree(s);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------------------------------------------------
int
main(/* int argc, char *argv[] */)
{
    logsimpleinit("Starting");   // it that working?

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Simple init and validate test" , .desc="Init test."                , .mandatory=true)
      , testnew(.f2 = tf2, .num = 2, .name = "Access read/write test"        , .desc="Init test."                , .mandatory=true)
      , testnew(.f2 = tf3, .num = 3, .name = "Elem() test"                   , .desc="Init test."                , .mandatory=true)
    );

    logclose("end...");
    return 0;
}

#endif /* FSTESTING */

