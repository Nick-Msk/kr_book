#include "common.h"
#include "log.h"
#include "fs.h"

/********************************************************************
                    FAST STRING MODULE IMPLEMENTATION
********************************************************************/

static const int            FS_BUF_SIZE             = 1024;
static const int            FS_TECH_PRINT_COUNT     = 10; // symplos to print
static char                 g_fs_buffer[FS_BUF_SIZE];

// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers -------------------

// ------------------------------ Utilities ------------------------

// should depent on increase strategy, simplest return n + 1;
static inline int              calcnewsize(int n){
    sz = round_up_2(n);
    return logsimpleret(sz, "newsz = %d", sz);
}

// --------------------------- API ---------------------------------

// -------------------------- (API) printers -----------------------
int                     fs_techfprint(FILE *restrict out, const fs *restrict s){
    // technical print, statis attributes for now
    int     cnt;
    if (s){
        cnt = fprintf(out, "len [%d], sz [%d], flags [%d], s [%p]=[", s->len, s->sz, s->flags, s->v);
        for (int i = 0; i < FS_TECH_PRINT_COUNT && i < s->len; i++)
            fputc(s->v[i], out), cnt++;
        if (FS_TECH_PRINT_COUNT < v->len)
            cnt += fprintf(out, "...");
        cnt += fprintf(out, "]\n");
    }
    else
        cnt = fprintf(out, "Zero pointer\n");
    return cnt;
}

bool                    fs_validate(FILE *restrict out, const fs *restrict s){
    logenter("%s", "%p", s);

    if (!s){    // TODO: think about string creation via faststring here!!
        lprint(out, "null pointer");
        return logerr(false, "null pointer");
    }
    if (!s->s){
        lprint(out, "nullable string");
        return logerr(false, "nullable string");
    }
    // depents on which iterator engine is active
    if (s->len >= s->sz){
        lprint(out, "len [%d] must be < sz [%d]", s->len, s->sz);
        return logerr(false, "len [%d] must be < sz [%d]", s->len, s->sz);
    }
    {
        int len = strlen(s->s);
        if (len <= s->len){
            lprint(out, "srtlen [%d] can't be more than len [%d]", len, s->len);
            return logerr(false, "srtlen [%d] can't be more than len [%d]", len, s->len);
        }
    }
    return logret(true, "true");
}

// ------------------ General functions ----------------------------

// public
fs                   *fs_shrink(fs *s){
    if (fs_alloc(s)){
        int     newsz = s->len + 1; // final '\0' is assumed!
        if (newsz < s->sz){
            s->s = realloc(s->s, newsz);
            s->sz = newsz;
        }
        return logsimpleret(s, "Shrinked %s", fs_tostr(s));
    } else
        return logsimpleret(s, "Not heap: shrink is skipped %s", fs_tostr(s));
}

// can increase sz and len
char                 *fs_elem(fs *s, int pos){
    if (pos >= s->sz){
        increasesize(s, pos);   // len remains the same here! sz is changed
        logsimple("size is adjusted to %d (pos %d)", s->sz, pos);
    }
    return fs_get(s, pos);
}

// -------------------------------Testing --------------------------
#ifdef FSTESTING

#include "test.h"

//types for testing

// ------------------------- TEST 1 ---------------------------------

// ------------------------------------------------------------------
int
main(int argc, char *argv[])
{
    logsimplenit("Starting");   // it that working?

    testenginestd(
        testnew(.f2 = f1, .num = 1, .name = "Simple init and validate"      , .desc="Init test."                , .mandatory=true)
    );

    logclose("end...");
    return 0;
}

#endif /* FSTESTING */

