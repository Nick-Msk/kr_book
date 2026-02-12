#include "common.h"
#include "log.h"
#include "fs.h"

/********************************************************************
                    FAST STRING MODULE IMPLEMENTATION
********************************************************************/

//static const int            FS_BUF_SIZE             = 1024;
static const int            FS_TECH_PRINT_COUNT     = 10; // symplos to print
//static char                 g_fs_buffer[FS_BUF_SIZE];
//static const int            g_initsize              = 32;   // not sure

static const int            FS_MIN_ACCOC            = 128;

// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers -------------------


// ------------------------------ Utilities ------------------------

// should depent on increase strategy, simplest return n + 1;
static inline int               calcnewsize(int n){
    int sz = round_up_2(n);
    return sz; //logsimpleret(sz, "newsz = %d", sz);
}

// only for FS_FLAG_ALLOC
static fs                       increasesize(fs *s, int newsz, bool incr){
    logenter("newsz %d, incr %s v %p, is alloc? %s", newsz, bool_str(incr), s->v, bool_str(fs_alloc(s)) );
    if (fs_alloc(s) ){
        if (incr)
            newsz = calcnewsize(newsz);
        if (newsz > s->sz || !incr){
                s->v = realloc(s->v, newsz);
                if (!s->v)
                    userraiseint(10, "Unable to allocate %d bytes", newsz);
                s->sz = newsz;
            logauto(s->sz);
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
char                 *fs_elem(fs *s, int pos){
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
    if (s){
        cnt = fprintf(out, "len [%d], sz [%d], flags [%d], s [%p]=[", s->len, s->sz, s->flags, s->v);
        for (int i = 0; i < FS_TECH_PRINT_COUNT && i < s->len; i++)
            fputc(s->v[i], out), cnt++;
        if (FS_TECH_PRINT_COUNT < s->len)
            cnt += fprintf(out, "...");
        cnt += fprintf(out, "]\n");
    }
    else
        cnt = fprintf(out, "Zero pointer\n");
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

