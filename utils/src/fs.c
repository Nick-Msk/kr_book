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

// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers -------------------


// ------------------------------ Utilities ------------------------

// should depent on increase strategy, simplest return n + 1;
static inline int               calcnewsize(int n){
    int sz = round_up_2(n);
    return logsimpleret(sz, "newsz = %d", sz);
}

// only for FS_FLAG_ALLOC
static fs                       increasesize(fs *s, int newsz, bool incr){
    if (incr)
        newsz = calcnewsize(newsz);
    if (newsz < s->sz || !incr){
            s->v = realloc(s->v, newsz);
            if (!s->v)
                userraiseint(10, "Unable to allocate %d bytes", newsz);
            s->sz = newsz;
    }
    return *s;
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
        increasesize(s, pos, true);   // len remains the same here! sz is changed
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

    fs      res = FSINIT(.len = 0, .sz = 0, .flags = FS_FLAG_ALLOC, .v = 0);
    increasesize(&res, n, true);
    *res.v = '\0';
    return logsimpleret(res, "Created empty with sz %d", res.sz);
}

fs                      fsclone(fs s){
    fs tmp = fsinit(s.len + 1);
    tmp.sz = s.len + 1;
    memcpy(tmp.v, s.v, tmp.sz);
    return tmp;
}

// -------------------------------Testing --------------------------
#ifdef FSTESTING

#include "test.h"

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

        test_sub("subtest %d", ++subnum);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------------------------------------------------
int
main(int argc, char *argv[])
{
    logsimpleinit("Starting");   // it that working?

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Simple init and validate"      , .desc="Init test."                , .mandatory=true)
    );

    logclose("end...");
    return 0;
}

#endif /* FSTESTING */

