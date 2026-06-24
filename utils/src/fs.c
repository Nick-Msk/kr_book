#include <stdarg.h>

#include "getword.h"
#include "common.h"
#include "log.h"
#include "fs.h"
#include "error.h"
#include "fileutils.h"
#include "checker.h"
#include "guard.h"

/********************************************************************
                    FAST STRING MODULE IMPLEMENTATION
********************************************************************/

#if defined(FS_ALLOCATOR)
    static char               **g_fs_ptr                = 0;
    static int                  g_alloc                 = 0;
    static const int            g_initsize              = 32;   // not sure
#endif

// external contol
int                             FS_MIN_ACCOC            = 128;
int                             FS_TECH_PRINT_COUNT     = 100; // symplos to print
// mem leak checking
static int                      g_alloc_cnt             = 0;
static int                      g_free_cnt              = 0;
static int                      g_alloc_body_cnt        = 0;
static int                      g_free_body_cnt         = 0;
// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers -------------------

// ------------------------------ Utilities -------------- ----------

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
    int tmpsz = calcnewsize(SIZE_POWER2, g_initsize) * sizeof(const char *);
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
            newsz = calcnewsize(SIZE_POWER2, newsz);    // with increase type
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

// low level copy, NOT check anything
static inline fs                       *strcopy(fs *restrict target, const fs *restrict source){
    memcpy(target->v + target->len, source->v, source->len + 1);   // with last '\0'
    target->len = target->len + source->len;
    return target;
}

// --------------------------- API ---------------------------------

// ------------------ General functions ----------------------------

// move only heap alloc fs
fs                                       fs_move(fs *orig){
    if (!fs_alloc(orig) )
        userraiseint(ERR_FS_NOT_ALLOC_FLAG, "Unable to move not allocated fs (type %s)", fs_flag_str(orig->flags) );    // 10001 interrupt
    fs tmp = *orig;
    *orig = FS();
    return logsimpleret(tmp, "fs moved %d: %p", tmp.sz, tmp.v);
}
// move whole fs (body and string)
fs                                      *fs_moveto_heap(fs *orig){
    invraisecode(orig != NULL, ERR_NULLABLE_PTR, "Null pointer");
    if (! (fs_alloc(orig) || fs_static(orig) ) )
        userraiseint(ERR_UNSUPPORTED_TYPE, "Unable to moveall not allocated fs (type %s)", fs_flag_str(orig->flags) );
    fs  *tmp = fs_create();
    *tmp = *orig;
    tmp->flags |= FS_FLAG_BODYALLOC;
    // clear orig
    orig->v = NULL;
    fs_free(orig);  // orig->v is null, so won't be freed, but orig can be FS_FLAG_BODYALLOC too
    return tmp;
}

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

// sprintf to particular position
int                                     fs_sprintf_position(fs *restrict s, int pos, const char *restrict fmt, va_list ap)
{
    logenter("len %d pos %d", s->len, pos);
    va_list ap2;
    va_copy(ap2, ap);
    int needed = vsnprintf(NULL, 0, fmt, ap);
    // va_end(ap);
    if (needed < 0)
        return logret(-1, "vsnprintf length failed");

    if (s->sz < pos + 1 + needed)
        increasesize(s, pos + 1 + needed, true);
    int cnt = vsnprintf(fs_str(s) + pos, s->sz - pos, fmt, ap2);
    //if (pos + needed > s->len)
    s->len = pos + needed;      // note: string CAN BE CUTTED!
    va_end(ap2);
    return logret(cnt, "printed %d to pos %d", cnt, pos);
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
    //memcpy(target->v + target->len, source.v, source.len + 1);   // with last '\0'
    //target->len = sumlen;
    return *strcopy(target, &source);
}

// put beginner at the start position of the target
fs                                      fs_rev_catstr(fs *restrict target, const char *restrict beginner){
    int len;
    if (beginner && (len = strlen(beginner) ) != 0 ){
        fs_resize(target, target->len + len);
        memmove(target->v + len, target->v, target->len);
        memcpy(target->v, beginner, len);
        fs_setlen(target, len + target->len);
        logsimple("Origin moved to the end (%d)", len);
    }
    return *target;
}

// fast in-place!
fs                                      fs_substr(fs *s, int from, int len){
    invraisecode(s != 0 && from >= 0 && len >= 0,
        ERR_NULLABLE_PTR, "Input violation %p, from %d, len %d", s, from, len);     // asssertion if NOINVARIANT is NOT defined

    if (from >= s->len) {
        fs_setlen(s, 0);
        return logsimpleret(*s, "[]");
    }
    len = MIN(len, s->len - from);
    if (from > 0)
        memmove(s->v, s->v + from, len);     // TODO: probably to use strcopy?
    fs_setlen(s, len);
    return logsimpleret(*s, "[%.20s]", s->v);
}

// constructor version
fs                                      fs_newsubstr(const fs *s, int from, int len){
    invraisecode(s != 0 && from >= 0 && len >= 0,
        ERR_NULLABLE_PTR, "%p from %d, len %d", s, from, len);     // asssertion if NOINVARIANT is NOT defined

    if (from >= s->len) {
        fs tmp = fsinit(1);
        fsetlen(tmp, 0);
        return logsimpleret(tmp, "from %d over length", from);
    }

    len = MIN(len, s->len - from);
    fs tmp = fsinit(len + 1);  // not possible to use fs_cpy or fs_cat here!
    if (from > 0)
        memmove(tmp.v, s->v + from, len);
    fsetlen(tmp, len);
    return logsimpleret(tmp, "Created substr %d", tmp.len);
}
// right padding up to len
fs                                       fs_rpad(fs *restrict str, int len, const fs *restrict pad){
    invraisecode(str != 0 && len >= 0 && pad != 0,
            ERR_NULLABLE_PTR, "Null pointers or negative len %p %p %d", str, pad, len);
    if (fs_len(str) >= len )
        return logsimpleret(fs_substr(str, 0, len), "Cuted to %d", len);
    if (fs_len(pad) > 0){
        fs_resize(str, len + 1);
        for (int i = fs_len(str); i < len; i++)
            fs_str(str)[i] = fs_str( (fs *) pad)[ (i - fs_len(str) ) % fs_len(pad)];
        fs_setlen(str, len);
    }
    return logsimpleret(*str, "padded to %d", fs_len(str) );
}
// left padding up to len
fs                                       fs_lpad(fs *restrict str, int len, const fs *restrict pad){
    invraisecode(str != 0 && len >= 0 && pad != 0,
            ERR_NULLABLE_PTR, "Null pointers or negative len %p %p %d", str, pad, len);
    if (fs_len(str) >= len )
        return logsimpleret(fs_substr(str, 0, len), "Cuted to %d", len);
    // str->len < len => check size
    if (fs_len(pad) > 0){
        fs_resize(str, len + 1);
        memmove(fs_str(str) + len - fs_len(str), fs_str(str), fs_len(str) );
        for (int i = 0; i < len - fs_len(str); i++)
            fs_str(str)[i] = fs_str( (fs *) pad)[i % fs_len(pad)];
        fs_setlen(str, len);
    }
    return logsimpleret(*str, "padded to %d", fs_len(str) );
}
// limiter search (primitive alg)
int                                     fs_lim_instr(const fs* restrict str1, const fs* restrict str2, int lim, bool lowercase){
    invraise(str1 != 0 && str2 != 0 && lim >= 0, "%p %p, %d", str1, str2, lim);
    const char *s1 = str1->v, *s2 = str2->v;
    int pos = 0;

    for (pos = 0; pos < MIN(str1->len, lim) - str2->len; pos++){
        int j = pos, i = 0;
        while (clower(s1[j], lowercase) == clower(s2[i], lowercase) && s1[j] != '\0' && s2[i] != '\0' && RGUARDM)
            i++, j++;
        if (s2[i] == '\0')
            return logsimpleret(pos, "Found %d", pos);
    }
    return logsimpleret(-1, "substr not found");
}

// sorting as array
void                                     fs_sort(fs *s, bool asc){
    invraise(s || s != 0, "Nullable pointer or fs");
    qsort(s->v, s->len, 1, asc ? pchar_cmp : pchar_revcmp);
}

// -------------------------- (API) printers -----------------------
// this is not limit!
int                                     fs_fprint(FILE *restrict out, const fs *restrict s, const char *restrict name){
    int     cnt = 0;
    if (s){
        cnt = fprintf(out, "[%s%s%s]", name ? name : "", name ? ": " : "", s->v);
    }
    return cnt;
}

// with  limit!
int                                     fs_fprintlim(FILE *restrict out, const fs *restrict s, int lim, const char *restrict name){
    int     cnt = 0;
    if (s){
        cnt = fprintf(out, "[%s%s%.*s]", name ? name : "", name ? ": " : "", lim, s->v);
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
// TODO: refactor that!!!!!!!
extern bool                             fs_free_body_alloc_checker(int *freecnt, int *alloccnt){
    if (freecnt)
        *freecnt = g_free_body_cnt;
    if (alloccnt)
        *alloccnt= g_alloc_body_cnt;
    return logsimpleret(g_free_body_cnt == g_alloc_body_cnt, "body allocated %d, freed %d", g_alloc_cnt, g_free_cnt);
}
// TODO: refactor that!!!!!!!
bool                                    fs_free_alloc_checker(int *freecnt, int *alloccnt){
    if (freecnt)
        *freecnt = g_free_cnt;
    if (alloccnt)
        *alloccnt= g_alloc_cnt;
    return logsimpleret(g_free_cnt == g_alloc_cnt, "string allocated %d, freed %d", g_alloc_cnt, g_free_cnt);
}
// TODO: refactor that!!!!!!!
// internal, for testing
static bool                             check_leak(bool raise){
    int     free_v, alloc_v;
    fs_free_alloc_checker(&free_v, &alloc_v);
    if (free_v != alloc_v){
        if (raise)
            userraiseint(WARN_MEM_LEAK_DETECTED, "string allocated %d, freed %d", alloc_v, free_v);
        else
            userraise(false, WARN_MEM_LEAK_DETECTED, "WARNING: string allocated %d, freed %d", alloc_v, free_v);
    }
    if (free_v != alloc_v)
        return false;
    if (free_v != alloc_v){
        if (raise)
            userraiseint(WARN_MEM_LEAK_DETECTED, "body allocated %d, freed %d", alloc_v, free_v);
        else
            userraise(false, WARN_MEM_LEAK_DETECTED, "WARNING: body allocated %d, freed %d", alloc_v, free_v);
    }
    return free_v == alloc_v;
}

// just a wrapper for check_leak
bool                                    fs_alloc_check(bool raise){
    return check_leak(raise);
}
// just print
bool                                    fs_fprint_checker_cnt(FILE *restrict out, const char *str){
    if (!out)
        return false;
    fprintf(out, "%.6s: g_free_cnt %d g_alloc_cnt %d g_free_body_cnt %d g_alloc_body_cnt %d\n",
        str, g_free_cnt, g_alloc_cnt, g_free_body_cnt, g_alloc_body_cnt);
    return true;
}

// --------------------------------- SERIALIZATION -----------------------------------------

// seqialization (strictly FULL save into the steam with only FS and .len info), out must be opened for write
int                                     fs_fsave(FILE *restrict out, const fs *restrict str){
    int cnt = 0;
    if (out){
        fprintf(out, "FS(%d):[%s]\n", str->len, str->v);
        cnt++;
    }
    return cnt;
}

int                                     fs_save(const char *restrict fname, const fs *restrict str){ 
    FILE *out = fopen(fname, "w");
    int     cnt = 0;
    if (out){
        cnt = fs_fsave(out, str);
        fclose(out);
    } else
        userraiseint(ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open %s for write", fname);
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
    int     cnt = 0;
    if (out){
        cnt = fs_fsave_arr(out, arr);
        fclose(out);
    } else
        userraiseint(ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open %s for write", fname);
    return cnt;
}
//
fs                                     *fs_fscanf(FILE *restrict in, fs *restrict s){
    invraisecode(in != NULL, ERR_NULLABLE_PTR, "Null pointer %p - %p", in, s);

    fs *original_s = s;
    s = fs_init_or_use(s);
    if (getpurestring(in, s) )
        return logsimpleret(s, "Line read len %d", fs_len(s) );
    else {
        if (!original_s)
            fs_free(s);
        return logsimpleerr(NULL, "EOF detectod");
    }
}

// raise int in case of wrong format
fs                                      fs_fload(FILE *restrict in, fs *restrict s){
    invraisecode(in != 0, ERR_NULLABLE_PTR, "Null pointer %p - %p", in, s);

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
// local creation
fs                                      fsinit(int n){
    fs      res = FS();     // fsalloc flag
    increasesize(&res, n, true);
    *res.v = '\0';
    return logsimpleret(res, "Created empty with sz %d", res.sz);
}
// just create fs in heap!
fs                                     *fs_create(void){
    fs  *new_fs = malloc(sizeof(fs) );
    if (!new_fs)
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to allocate fs body");
    else {
        *new_fs = FS();
        new_fs->flags |= FS_FLAG_BODYALLOC;   // чтобы fs_free удалил и тело, и будущую строку
        logauto(++g_alloc_body_cnt);
    }
    return new_fs;
}

fs                                     *fs_heapcreate(const fs *orig) {
    fs      *new_fs = fs_create();       // выделяет fs в куче, ставит FS_FLAG_BODYALLOC
    *new_fs = fs_clone(orig);       // копирует строку и поля
    new_fs->flags |= FS_FLAG_BODYALLOC; // гарантируем флаг перемещения
    return new_fs;
}

// clone as body local
fs                                      fs_clone(const fs *s){
    fs tmp = FS();
    if (s && s->v){
        tmp = fsinit(s->len + 1);
        strcopy(&tmp, s);
    }
    return tmp;
}
// destructor, macro wrapper will be
// free fs string and fs body if FS_FLAG_BODYALLOC
void                                    fs_free(fs *s){
    if (!s)
        return;
    bool  moved = fs_bodyalloc(s);  // flags based
    if (fs_alloc(s) )    // actualy alloc must be a flag, but not statememnt TODO:
        if (s->v){
            logauto(++g_free_cnt);   // calculate only  if really free memory
            logsimple("freed... %p", s->v);   // WOW, logsimpleact?
            free(s->v);
        }
    s->sz = s->len = 0; // destroy even literals
    s->v = 0;
    if (moved){
        s->flags = 0;   // clear  FS_FLAG_BODYALLOC
        logsimple("fs body %p freed (g_free_body_cnt %d)", s, ++g_free_body_cnt);
        free(s);
    }
}

#if defined(FS_ALLOCATOR)
// detach from allocator! Must be freed manually
bool                                    fsdetach(fs *s){
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

// ------------------------- SIMPLE TEST fs_close ---------------------------------

static TestStatus
tf_fs_clone(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Клонирование обычной строки (fscopy) */
    test_sub("subtest %d: clone normal string", ++subnum);
    {
        const char *text = "hello, world";
        fs      orig = fscopy(text);
        fs      clone = fs_clone(&orig);

        // Оригинал не изменился
        test_validatefree(
            strcmp(fsstr(orig), text) == 0 && fslen(orig) == (int)strlen(text),
            (fsfree(orig), fsfree(clone)),
            "Original must be unchanged after clone"
        );
        // Клон содержит ту же строку
        test_validatefree(
            strcmp(fsstr(clone), text) == 0 && fslen(clone) == (int)strlen(text),
            (fsfree(orig), fsfree(clone)),
            "Clone must contain the same string, got '%s'", fsstr(clone)
        );
        // Клон физически независим: меняем оригинал – клон не меняется
        fs_sprintf(&orig, "modified");
        test_validatefree(
            strcmp(fsstr(clone), text) == 0,
            (fsfree(orig), fsfree(clone)),
            "Clone must remain unchanged after original modification"
        );

        fsfree(orig);
        fsfree(clone);
        fs_alloc_check(true);
    }

    /* 2. Клонирование пустой строки (v == NULL, созданной через FS()) */
    test_sub("subtest %d: clone empty string (v=NULL)", ++subnum);
    {
        fs      orig = FS();                    // v == NULL, len == 0
        fs      clone = fs_clone(&orig);

        test_validatefree(
            clone.v == NULL && fslen(clone) == 0,
            fsfree(clone),
            "Clone of empty (v=NULL) must have v=NULL and len=0, got v=%p, len=%d",
            (void*)clone.v, fslen(clone)
        );
        test_validatefree(
            orig.v == NULL && fslen(orig) == 0,
            fsfree(clone),
            "Original must stay empty after clone"
        );

        fsfree(clone);
        fs_alloc_check(true);
    }

    /* 3. Клонирование пустой строки с v != NULL (fscopy("")) */
    test_sub("subtest %d: clone empty string (v!=NULL)", ++subnum);
    {
        fs      orig = fscopy("");              // v != NULL, len == 0
        fs      clone = fs_clone(&orig);

        test_validatefree(
            clone.v != NULL && fslen(clone) == 0,
            fsfree(clone),
            "Clone of empty (v!=NULL) must have v!=NULL and len=0, got v=%p, len=%d",
            (void*)clone.v, fslen(clone)
        );
        test_validatefree(
            strcmp(fsstr(clone), "") == 0,
            fsfree(clone),
            "Clone must be empty string, got '%s'", fsstr(clone)
        );

        fsfree(orig);
        fsfree(clone);
        fs_alloc_check(true);
    }

    /* 4. Клонирование литерала (FS_FLAG_STATIC) */
    test_sub("subtest %d: clone literal", ++subnum);
    {
        fs      orig = fsliteral("static string");
        fs      clone = fs_clone(&orig);

        test_validatefree(
            strcmp(fsstr(clone), fsstr(orig)) == 0 && fslen(clone) == fslen(orig),
            fsfree(clone),
            "Clone of literal must have same content, got '%s'", fsstr(clone)
        );
        // Клон не должен быть статическим (должен быть копией в куче)
        // Проверим, что clone можно освободить (fsfree не упадёт) и что он не ссылается на ту же память
        test_validatefree(
            fsstr(clone) != fsstr(orig),        // указатели должны различаться
            fsfree(clone),
            "Clone of literal must have its own memory"
        );

        fsfree(clone);
        // orig – литерал, не освобождаем
        fs_alloc_check(true);
    }

    /* 5. Последовательные клонирования и проверка утечек */
    test_sub("subtest %d: multiple clones (leak check)", ++subnum);
    {
        const char *words[] = {"one", "two", "three"};
        fs      clones[COUNT(words)];

        for (int i = 0; i < COUNT(words); i++) {
            fs      orig = fscopy(words[i]);
            clones[i] = fs_clone(&orig);
            fsfree(orig);                       // оригинал больше не нужен
        }

        for (int i = 0; i < COUNT(words); i++) {
            test_validatefree(
                strcmp(fsstr(clones[i]), words[i]) == 0,
                (fsfree(clones[0]), fsfree(clones[1]), fsfree(clones[2])),
                "Clone %d must be '%s', got '%s'", i, words[i], fsstr(clones[i])
            );
            fsfree(clones[i]);
        }
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
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
    char        buf[1000];
    const char  fmt[] = "%d, %f, %s\n";

    test_sub("subtest %d: fs_sprintf EMPTY", ++subnum);
    {
        fs s1 = FS();

        snprintf(buf, sizeof(buf), fmt, 1223, 1.445, "Blablabla");
        fs_sprintf(&s1, fmt, 1223, 1.445, "Blablabla");

        // Проверяем длину
        test_validatefree(
            (int)strlen(buf) == fslen(s1),
            fsfree(s1),
            "Length mismatch: buf=%zu, s1=%d", strlen(buf), fslen(s1)
        );
        // Проверяем содержимое
        test_validatefree(
            strcmp(buf, fsstr(s1)) == 0,
            fsfree(s1),
            "Content mismatch: buf='%s', s1='%s'", buf, fsstr(s1)
        );

        fsfree(s1);
    }
    /* ---- 1. fs_sprintf в большой буфер ---- */
    test_sub("subtest %d: fs_sprintf (large buffer)", ++subnum);
    {
        fs s1 = fsinit(1000);

        snprintf(buf, sizeof(buf), fmt, 1223, 1.445, "Blablabla");
        fs_sprintf(&s1, fmt, 1223, 1.445, "Blablabla");

        // Проверяем длину
        test_validatefree(
            (int)strlen(buf) == fslen(s1),
            fsfree(s1),
            "Length mismatch: buf=%zu, s1=%d", strlen(buf), fslen(s1)
        );
        // Проверяем содержимое
        test_validatefree(
            strcmp(buf, fsstr(s1)) == 0,
            fsfree(s1),
            "Content mismatch: buf='%s', s1='%s'", buf, fsstr(s1)
        );
        fsfree(s1);
    }

    /* ---- 2. fs_sprintf в крошечный буфер (расширение) ---- */
    test_sub("subtest %d: fs_sprintf (small buffer, expands)", ++subnum);
    {
        fs s2 = fsinit(1);   // начальный размер 1

        snprintf(buf, sizeof(buf), fmt, 12345, 9.8765, "XXXYYYYZZZZZZZZZRRRRR");
        fs_sprintf(&s2, fmt, 12345, 9.8765, "XXXYYYYZZZZZZZZZRRRRR");

        test_validatefree(
            (int)strlen(buf) == fslen(s2),
            fsfree(s2),
            "Length mismatch after expand: buf=%zu, s2=%d", strlen(buf), fslen(s2)
        );
        test_validatefree(
            strcmp(buf, fsstr(s2)) == 0,
            fsfree(s2),
            "Content mismatch after expand: buf='%s', s2='%s'", buf, fsstr(s2)
        );
        fsfree(s2);
    }

    /* ---- 3. fs_sprintf_concat (дописывание) ---- */
    test_sub("subtest %d: fs_sprintf_concat", ++subnum);
    {
        fs s = fsinit(10);

        // Начальная строка
        fs_sprintf(&s, "Hello");
        // Конкатенация
        fs_sprintf_concat(&s, ", %s!", "World");

        const char expected[] = "Hello, World!";
        test_validatefree(
            (int)strlen(expected) == fslen(s),
            fsfree(s),
            "Concat length mismatch: expected=%zu, got=%d", strlen(expected), fslen(s)
        );
        test_validatefree(
            strcmp(expected, fsstr(s)) == 0,
            fsfree(s),
            "Concat content mismatch: expected='%s', got='%s'", expected, fsstr(s)
        );
        fsfree(s);
    }

    /* ---- 4. Пустой fs_sprintf ---- */
    test_sub("subtest %d: fs_sprintf empty format", ++subnum);
    {
        fs s = fsinit(16);
        int cntres = fs_sprintf(&s, "");
        test_validatefree(
            cntres == 0,
            fsfree(s),
            "Empty format should return cntres == 0, got %d", fslen(s)
        );
        test_validatefree(
            fslen(s) == 0,
            fsfree(s),
            "Empty format should produce length 0, got %d", fslen(s)
        );
        test_validatefree(
            strcmp("", fsstr(s)) == 0,
            fsfree(s),
            "Empty format should be empty string, got '%s'", fsstr(s)
        );
        fsfree(s);
    }

    check_leak(true);
    return logret(TEST_PASSED, "done");
}
/*
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
}  */

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
        const char fname[] = "res/fs/test_save_load.dat";
        fs s = fscopy("Tra la la 1234567890");
        if (fssave(fname, s) != 1)
            return logacterr(fsfree(s), TEST_FAILED, "Fssave returns != 1 value");
        fs s2 = fs_load(fname, 0); // w/o macro
        fstechfprint(logfile, s2);
        if (s.len != s2.len || fscmp(s, s2) != 0)
            return logacterr(fsfreeall(&s, &s2), TEST_FAILED, "Strings not equal (%d - %d)[%s] - [%s]", s.len, s2.len, fsstr(s), fsstr(s2) );

    test_sub("subtest %d: fs_save multiples fs", ++subnum);

        const char   fname2[] = "res/fs/test_save_load_mass.dat";
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
            snprintf(buf, sizeof(buf) - 1, "Check val %d", i);    // make a pattern
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

// ------------------------- TEST 13 ---------------------------------

static TestStatus
tf13(const char *name)
{
    logenter("%s", name);

    int         subnum = 0;

    test_sub("subtest %d: simple fs_move", ++subnum);
    {
        const char pt[] = "qwertyuiop1234567890";
        fs s1 = fscopy(pt);
        fs s2 = fsmove(s1);
        if (!fsisnull(s1) )
            return logacterr( fsfree(s2), TEST_FAILED, "s1 must becaume nullable after moving, but not (%d:%d:%p)", s1.len, s1.sz, s1.v);
        if (s2.len != (int) strlen(pt) )
            return logacterr( fsfree(s2), TEST_FAILED, "Len of origin pattern %lu must be equal of len move fs %d", strlen(pt), s2.len);
        if (strcmp(fsstr(s2), pt) != 0)
            return logacterr( fsfree(s2), TEST_FAILED, "origin [%s] must be equal move fs [%s]", pt, fsstr(s2) );
        fsfree(s2);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 14 ---------------------------------
/*
static TestStatus
tf14(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fssubstr", ++subnum);
    {
        const char  pt[] = "qwertyuiop1234567890";
        int         len = 3;
        fs          s1 = fscopy(pt);

        fssubstr(s1, 10, len);
        test_validatefree(fslen(s1) == len, fsfree(s1),
                "Len of substr %d must be equal to %d", fslen(s1), len);
        test_validatefree(strcmp(fsstr(s1), "123") == 0, fsfree(s1),
                "substr [%s] must be equal of %s", fsstr(s1), "123");

        fsfree(s1);
    }
    test_sub("subtest %d: fssubstr", ++subnum);
    {
        const char  pt[] = "qwertyuiop1234567890";
        int         len = 5;
        fs          s1 = fscopy(pt);
        fs          s2 = fsnewsubstr(s1, 10, len);

        test_validatefree(fslen(s2) == len, fsfreeall(&s1, &s2),
                "Len of substr %d must be equal to %d", fslen(s2), len);
        test_validatefree(strcmp(fsstr(s2), "12345") == 0, fsfreeall(&s1, &s2),
                "substr [%s] must be equal of %s", fsstr(s2), "12345");

    test_sub("subtest %d: right part of a string", ++subnum);

        int         offset = 1;
        fs          s3 = fsnewsubstr(s1, offset, 100);
        test_validatefree(fslen(s3) == strlen(pt) - offset, fsfreeall(&s1, &s2, &s3),
                "Len of substr %d must be equal to %lu - %d", fslen(s3), strlen(pt), offset);
        test_validatefree(strcmp(fsstr(s3), pt + offset) == 0, fsfreeall(&s1, &s2, &s3),
                "substr [%s] must be equal of %s", fsstr(s3), pt + offset);

        fsfreeall(&s1, &s2, &s3);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
} */
static TestStatus
tf14(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    /* ================= fs_substr (in-place) ================= */

    /* 1. Обрезание с начала (from = 0) до меньшей длины */
    test_sub("subtest %d: fs_substr cut from start", ++subnum);
    {
        const char *orig = "hello world";
        int         new_len = 5;
        fs          s = fscopy(orig);
        fs          result = fs_substr(&s, 0, new_len);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == new_len,
            fsfree(s),
            "fs_substr cut: length must be %d, got %d", new_len, fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "hello") == 0,
            fsfree(s),
            "fs_substr cut: expected 'hello', got '%s'", fsstr(s)
        );
        fsfree(s);
    }

    /* 2. Взятие подстроки с середины */
    test_sub("subtest %d: fs_substr mid part", ++subnum);
    {
        const char *orig = "abcdefghij";
        int         from = 3, to = 4;
        fs          s = fscopy(orig);
        fs          result = fs_substr(&s, from, to);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == to,
            fsfree(s),
            "fs_substr mid: length must be %d, got %d", to, fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "defg") == 0,
            fsfree(s),
            "fs_substr mid: expected 'defg', got '%s'", fsstr(s)
        );
        fsfree(s);
    }

    /* 3. from выходит за пределы строки -> пустая строка */
    test_sub("subtest %d: fs_substr from beyond length", ++subnum);
    {
        const char *orig = "abc";
        int         from = 10, to = 2;
        fs          s = fscopy(orig);
        fs          result = fs_substr(&s, from, to);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == 0,
            fsfree(s),
            "fs_substr beyond: length must be 0, got %d", fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "") == 0,
            fsfree(s),
            "fs_substr beyond: expected empty, got '%s'", fsstr(s)
        );
        fsfree(s);
    }

    /* 4. Пустая исходная строка */
    test_sub("subtest %d: fs_substr on empty string", ++subnum);
    {
        fs          s = FS();               /* пустая строка */
        fs          result = fs_substr(&s, 0, 5);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == 0,
            fsfree(s),
            "fs_substr empty: length must be 0, got %d", fslen(s)
        );
        fsfree(s);
    }

    test_sub("subtest %d: fs_substr len beyond length", ++subnum);
    {
        const char *orig = "abcdefgh";
        int         from = 5, to = 200;
        fs          s = fscopy(orig);
        fs          result = fs_substr(&s, from, to);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == (int) strlen(orig) - from,
            fsfree(s),
            "fs_substr beyond: length must be %d, got %d", (int) strlen(orig) - from, fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "fgh") == 0,
            fsfree(s),
            "fs_substr beyond: expected '%s', got '%s'", "fgh", fsstr(s)
        );
        fsfree(s);
    }

    /* ================= fs_newsubstr (constructor) ================= */

    /* 5. Копирование подстроки с середины (оригинал не меняется) */
    test_sub("subtest %d: fs_newsubstr copy mid part", ++subnum);
    {
        const char *orig_str = "abcdefghij";
        int         from = 2, to = 5;
        fs          orig = fscopy(orig_str);
        fs          sub = fs_newsubstr(&orig, from, to);

        // Проверяем подстроку
        test_validatefree(
            fslen(sub) == to,
            (fsfree(orig), fsfree(sub)),
            "fs_newsubstr copy: length must be %d, got %d", to, fslen(sub)
        );
        test_validatefree(
            strcmp(fsstr(sub), "cdefg") == 0,
            (fsfree(orig), fsfree(sub)),
            "fs_newsubstr copy: expected 'cdefg', got '%s'", fsstr(sub)
        );
        // Проверяем, что оригинал не изменился
        test_validatefree(
            strcmp(fsstr(orig), orig_str) == 0 && fslen(orig) == (int) strlen(orig_str),
            (fsfree(orig), fsfree(sub)),
            "fs_newsubstr copy: original must stay unchanged, got '%s' (len %d)",
            fsstr(orig), fslen(orig)
        );
        fsfree(orig);
        fsfree(sub);
    }

    /* 6. from за пределами -> пустая строка, оригинал цел */
    test_sub("subtest %d: fs_newsubstr from beyond length", ++subnum);
    {
        const char *orig_str = "abc";
        int         from = 10, to = 2;
        fs          orig = fscopy(orig_str);
        fs          sub = fs_newsubstr(&orig, from, to);

        test_validatefree(
            fslen(sub) == 0,
            (fsfree(orig), fsfree(sub)),
            "fs_newsubstr beyond: result length must be 0, got %d", fslen(sub)
        );
        test_validatefree(
            strcmp(fsstr(sub), "") == 0,
            (fsfree(orig), fsfree(sub)),
            "fs_newsubstr beyond: result must be empty, got '%s'", fsstr(sub)
        );
        test_validatefree(
            strcmp(fsstr(orig), orig_str) == 0,
            (fsfree(orig), fsfree(sub)),
            "fs_newsubstr beyond: original must stay '%s', got '%s'", orig_str, fsstr(orig)
        );
        fsfree(orig);
        fsfree(sub);
    }

    /* 7. to больше оставшейся длины -> обрезается до конца строки */
    test_sub("subtest %d: fs_newsubstr to exceeds length", ++subnum);
    {
        const char *orig_str = "hello";
        int         from = 2, to = 100;
        fs          orig = fscopy(orig_str);
        fs          sub = fs_newsubstr(&orig, from, to);

        int         expected_len = (int)strlen(orig_str) - from;
        test_validatefree(
            fslen(sub) == expected_len,
            (fsfree(orig), fsfree(sub)),
            "fs_newsubstr exceed: length must be %d, got %d", expected_len, fslen(sub)
        );
        test_validatefree(
            strcmp(fsstr(sub), orig_str + from) == 0,
            (fsfree(orig), fsfree(sub)),
            "fs_newsubstr exceed: expected '%s', got '%s'", orig_str + from, fsstr(sub)
        );
        fsfree(orig);
        fsfree(sub);
    }

    /* 8. Пустая исходная строка */
    test_sub("subtest %d: fs_newsubstr on empty string", ++subnum);
    {
        fs          orig = FS();
        fs          sub = fs_newsubstr(&orig, 0, 5);

        test_validatefree(
            fslen(sub) == 0,
            (fsfree(orig), fsfree(sub)),
            "fs_newsubstr empty: result length must be 0, got %d", fslen(sub)
        );
        fsfree(orig);
        fsfree(sub);
    }

    check_leak(true);
    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 15 ---------------------------------

static TestStatus
tf15(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fs_ifnotin", ++subnum);   // sensitive, negative
    {
        const char  pt[] = "qwe1";
        fs          s = fscopy(pt);

        if (!fs_ifnotin(s, "a", "in", "z", "bbbbm", "qqqqq") )
            return logacterr( fsfree(s), TEST_FAILED, "NOT IN return false, but it must be true");

        if (fs_ifnotin(s, "a", "in", "z", "bbbbm", "qqqqq", pt) )
            return logacterr( fsfree(s), TEST_FAILED, "NOT IN return true, but it must be false");

    test_sub("subtest %d: fs_ifinotin", ++subnum);  // insensetive, netagive

        if (!fs_ifinotin(s, "a", "in", "z", "bbbbm", "QQQQQ") )
            return logacterr( fsfree(s), TEST_FAILED, "NOT IN return false, but it must be true");

        if (fs_ifinotin(s, "a", "in", "z", "VVBDFDFFDS", "qqqqq", "QWE1", "sds") )
            return logacterr( fsfree(s), TEST_FAILED, "NOT IN return true, but it must be false");

    test_sub("subtest %d: fs_ifin", ++subnum);  // sensetive, positive

        if (fs_ifin(s, "a", "in", "z", "bbbbm", "qqqqq") )
            return logacterr( fsfree(s), TEST_FAILED, "IN return true, but it must be false");

        if (!fs_ifin(s, "a", "in", "z", "bbbbm", "qqqqq", pt) )
            return logacterr( fsfree(s), TEST_FAILED, "IN return false, but it must be true");

    test_sub("subtest %d: fs_ifiin", ++subnum);  // insensetive, positive

        if (fs_ifiin(s, "a", "in", "z", "bbbbm", "qqqqq") )
            return logacterr( fsfree(s), TEST_FAILED, "IN return true, but it must be false");

        if (!fs_ifiin(s, "a", "in", "z", "bbbbm", "qqqqq", "QWE1", "sds") )
            return logacterr( fsfree(s), TEST_FAILED, "IN return false, but it must be true");

        fsfree(s);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}


// ------------------------- TEST 16 ---------------------------------

static TestStatus
tf16(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fs_instr", ++subnum);   // sensitive, negative
    {
        const char  pt[] = "qwertyuiop1234567890";
        fs          orig = fsliteral(pt);   // statis, not alloc
        int         pos = 8, res;
        fs          s = fsnewsubstr(orig, pos, 6);

        test_validatefree( (res = fs_instr(&orig, &s) ) == pos,
                          fsfree(s), "Must be %d but returns %d", pos, res);

        fscatstr(s, "Not exists in original pattern"); 
        test_validatefree( (res = fs_instr(&orig, &s) ) == -1,
                          fsfree(s), "Must be %d but returns %d", -1, res);

        fsfree(s);
    }
    test_sub("subtest %d: fs_iinstr", ++subnum);   // sensitive, negative
    {
        const char  pt[] = "qweRTYUIOP1234567890III";
        fs          orig = fsliteral(pt);
        int         pos = 8, res;
        fs          s = fsnewsubstr(orig, pos, 6);
        fs_tolower(&s);

        test_validatefree( (res = fs_iinstr(&orig, &s) ) == pos,
                          fsfree(s), "Must be %d but returns %d", pos, res);

        fscatstr(s, "Not exists in original pattern");
        test_validatefree( (res = fs_iinstr(&orig, &s) ) == -1,
                          fsfree(s), "Must be %d but returns %d", -1, res);


        fsfree(s);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 17 ---------------------------------

static TestStatus
tf17(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    fs          s = FS();

    test_sub("subtest %d: fs_chr tests (unlim)", ++subnum);   // sensitive, negative
    {
        const char  pt[] = "qwertyuiop1234567890";
        fs          lit = fsliteral(pt);
        int         pos = 7, res;
        s = fs_newsubstr(&lit, pos, 1);
        char        c = *fsstr(s);

        test_validatefree( (res = fs_chr(&lit, c) ) == pos, fsfree(s),
                        "Position must be %d but returns %d", pos, res);
        c = toupper(c);
        test_validatefree( (res = fs_chr(&lit, c) ) == -1, fsfree(s),
                        "Position must be -1 (not found) but returns %d", res);
        fsfree(s);
    }

    test_sub("subtest %d: fs_nchr tests (lim)", ++subnum);   // sensitive, negative
    {
        const char  pt[] = "qwertyuiop1234567890xxxxx";
        fs          lit = fsliteral(pt);
        int         pos = 10, res, lim = 5;
        fs          s = fs_newsubstr(&lit, pos, 1);
        char        c = *fsstr(s);

        test_validatefree( (res = fs_nchr(&lit, c, 20) ) == pos, fsfree(s),
                        "Position must be %d but returns %d", pos, res);
        test_validatefree( (res = fs_nchr(&lit, c, lim) ) == -1, fsfree(s),
                        "Position must be -1 (not found) because of limitation %d but returns %d", lim, res);
        fsfree(s);
    }
    test_sub("subtest %d: fs_nchr iteration tests (lim)", ++subnum);   // sensitive, negative
    {
        const char  pt[] = "qwertyuiop1234567890xxxxx";
        fs          lit = fsliteral(pt);
        int         pos = 9, res, lim = 5;
        fs          s = fs_newsubstr(&lit, pos, 1);
        char        c = *fsstr(s);

        for (lim = 1; lim < (int) sizeof(pt); lim++){
            if (lim <= pos){
                test_validatefree( (res = fs_nchr(&lit, c, lim) ) == -1, fsfree(s),
                     "[%c] Position must be -1 (not found) because of limitation %d but returns %d\n%s\n%s\n", c, lim, res, pt, "0123456789ABCDE");
            } else {
                test_validatefree( (res = fs_nchr(&lit, c, 20) ) == pos, fsfree(s),
                    "[%c] Position must be %d but returns %d", c, pos, res);
            }
        }
        fsfree(s);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 18 ---------------------------------

static TestStatus
tf18(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fs_rchr tests (unlim)", ++subnum);   // sensitive
    {
        const char  pt[] = "qwertyuiop1234567890";
        fs          lit = fsliteral(pt);
        int         pos = 7, res;
        fs          s = fs_newsubstr(&lit, pos, 1);
        fstechprint(s);
        fstechprint(lit);
        char        c = *fsstr(s);

        test_validatefree( (res = fs_rchr(&lit, c) ) == pos, fsfree(s),
                        "Position must be %d but returns %d", pos, res);

        c = 'z';
        test_validatefree( (res = fs_rchr(&lit, c) ) == -1, fsfree(s),
                        "Position must be -1 but returns %d", res);
        fsfree(s);
    }
    test_sub("subtest %d: fs_irchr tests (unlim)", ++subnum);   // insensitive reverse
    {
        const char  pt[] = "qwertyuiop1234567890";
        fs          lit = fsliteral(pt);
        int         pos = 7, res;
        fs          s = fs_newsubstr(&lit, pos, 1);
        char        c = toupper(*fsstr(s) );

        test_validatefree( (res = fs_irchr(&lit, c) ) == pos, fsfree(s),
                        "Position must be %d but returns %d", pos, res);

        c = toupper('z');
        test_validatefree( (res = fs_irchr(&lit, c) ) == -1, fsfree(s),
                        "Position must be -1 but returns %d", res);
        fsfree(s);

    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 19 ---------------------------------

static TestStatus
tf19(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fs_ninstr test (lim)", ++subnum);   // sensitive
    {
        const char  pt[] = "qwertyuiop1234567890xxxxxxxxxxxxxxx";
        fs          lit = fsliteral(pt);
        int         pos = 7, cnt = 11, res;
        fs          s = fs_newsubstr(&lit, pos, cnt);

        test_validatefree( (res = fs_ninstr(&lit, &s, 20) ) == pos, fsfree(s),
                    "Position must be %d but returns %d", pos, res);
        test_validatefree( (res = fs_ninstr(&lit, &s, 15) ) == -1, fsfree(s),
                    "Position must be %d (not found) but returns %d" , -1, res);

        fsfree(s);
    }
    test_sub("subtest %d: fs_ninstr iteration test (lim)", ++subnum);   // sensitive
    {
        const char  pt[] = "qwertyuiop1234567890xxxxxxxxxxxxxxx";
        fs          lit = fsliteral(pt);
        int         pos = 5, cnt = 10, res;
        fs          s = fs_newsubstr(&lit, pos, cnt);
        logauto(lit.v);

        for (int lim = 1; lim < fslen(lit); lim++)
            if (lim >= pos + cnt){       // positive case
                test_validatefree( (res = fs_ninstr(&lit, &s, 20) ) == pos, fsfree(s),
                        "Position must be %d but returns %d", pos, res);
            } else {    // -1
                test_validatefree( (res = fs_ninstr(&lit, &s, 15) ) == -1, fsfree(s),
                        "Position must be %d (not found) but returns %d" , -1, res);
            }

        fsfree(s);
    }
    test_sub("subtest %d: fs_niinstr test (lim)", ++subnum);   // insensitive
    {
        const char  pt[] = "qwertyuiop1234567890xxxxxxxxxxxxxxx";
        fs          lit = fsliteral(pt);
        int         pos = 7, cnt = 11, res;
        fs          s = fs_newsubstr(&lit, pos, cnt);
        fs_toupper(&s);

        test_validatefree( (res = fs_niinstr(&lit, &s, 20) ) == pos, fsfree(s),
                    "Position must be %d but returns %d", pos, res);
        test_validatefree( (res = fs_niinstr(&lit, &s, 15) ) == -1, fsfree(s),
                    "Position must be %d (not found) but returns %d" , -1, res);

        fsfree(s);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 20 ---------------------------------

static TestStatus
tf20(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fsrevcatstr simple", ++subnum);
    {
        const char  pt[] = "qwertyuiop1234567890";
        const char  ptinit[] = "ABCDE";
        fs          s = fscopy(ptinit);

        fsrevcatstr(s, pt);

        test_validatefree(fslen(s) == strlen(pt) + strlen(ptinit), fsfree(s),
                        "Length of final fs (%d) must be sum of length of original (%zu + %zu)", fslen(s), strlen(ptinit), strlen(pt) );

        test_validatefree(strncmp(pt, fsstr(s), strlen(pt) ) == 0,  fsfree(s),
                        "Pattern [%s] and first %zu symbols of fs [%.*s] must be equal", pt, strlen(pt), (int) strlen(pt), fsstr(s) );

        test_validatefree(strcmp(ptinit, fsstr(s) + strlen(pt) ) == 0,  fsfree(s),
                        "Init Pattern [%s] and last %zu symbols of fs [%s] must be equal", ptinit, strlen(ptinit), fsstr(s) + strlen(pt) );
        fsfree(s);
    }
    test_sub("subtest %d: fsrevcatstr iteration", ++subnum);
    {
        const char  pt[] = "qwertyuiop1234567890111";
        const char  ptinit[] = "ABCDEF";
        fs          s = fscopy(ptinit);
        int         cnt = 10;

        for (int i = 0; i < cnt; i++){
            fsrevcatstr(s, pt);
        }

        test_validatefree(fslen(s) == strlen(pt) * cnt + strlen(ptinit), fsfree(s),
                             "Length of final fs (%d) must be sum of length of original (%zu + %zu * %d)", fslen(s), strlen(ptinit), strlen(pt), cnt );

        test_validatefree(strncmp(pt, fsstr(s), strlen(pt) ) == 0,  fsfree(s),
                        "Pattern [%s] and first %zu symbols of fs [%.*s] must be equal", pt, strlen(pt), (int) strlen(pt), fsstr(s) );

        test_validatefree(strcmp(ptinit, fsstr(s) + strlen(pt) * cnt ) == 0,  fsfree(s),
                        "Init Pattern [%s] and last %zu symbols of fs [%s] must be equal", ptinit, strlen(ptinit), fsstr(s) + strlen(pt) * cnt );

        fsfree(s);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 21 ---------------------------------

static TestStatus
tf21(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: just simple", ++subnum);
    {
        const char  pt[] = "qwertyuiop      IOPKKNKNMN_1234567890";

        fs s = fscopy(pt);
        fssort(s, false);

        for (int i = 1; i < s.len; i++)
            test_validatefree(s.v[i - 1] >= s.v[i], fsfree(s),
                "s[%d] == [%c] must be >= s[%d] == [%c]", i - 1, s.v[i - 1], i, s.v[i]);

        fsfree(s);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 22 ---------------------------------

static TestStatus
tf22(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    fs s = FS();
    const char shift[] = "----------";

    test_sub("subtest %d: getint simple", ++subnum);
    {
        int     ival = 2347890;
        fssprintf(s, "%s%d", shift, ival);

        int     ires = fsgetintpos(s, sizeof(shift) - 1);

        test_validatefree(ires == ival, fsfree(s),
                "%d must be == %d", ires, ival);
    }
    test_sub("subtest %d: getlong simple", ++subnum);
    {
        long    lval = 987654321099887;
        fssprintf(s, "%s%ld", shift, lval);

        fstechfprint(logfile, s);
        long    lres = fsgetlongpos(s, sizeof(shift) - 1);

        test_validatefree(lres == lval, fsfree(s),
                "%ld must be == %ld", lres, lval);
    }
    test_sub("subtest %d: getdouble simple", ++subnum);
    {
        double  dval = 12.345;
        fssprintf(s, "%s%lf", shift, dval);

        double  dres = fsgetdoublepos(s, sizeof(shift) - 1);

        test_validatefree(dres == dval, fsfree(s),
                "%lf must be == %lf", dres, dval);
    }
    fsfree(s);
    check_leak(true);

    // additional
    /* ========== fs_getint ========== */

    /* 1. Обычное число */
    test_sub("subtest %d: fs_getint normal", ++subnum);
    {
        fs s = fscopy("42");
        test_validatefree(
            fs_getint(&s) == 42,
            fsfree(s),
            "fs_getint('42') must return 42"
        );
        fsfree(s);
    }

    /* 2. Отрицательное число */
    test_sub("subtest %d: fs_getint negative", ++subnum);
    {
        fs s = fscopy("-123");
        test_validatefree(
            fs_getint(&s) == -123,
            fsfree(s),
            "fs_getint('-123') must return -123"
        );
        fsfree(s);
    }

    /* 3. Нечисловая строка – должна упасть (перехватываем сигнал) */
    test_sub("subtest %d: fs_getint non‑numeric", ++subnum);
    {
        fs s = fscopy("abc");
        if (!try()) {
            int val = fs_getint(&s);   // должно вызвать userraiseint
            test_validatefree(
                false,
                fsfree(s),
                "fs_getint('abc') must raise error, but returned %d", val
            );
        } else {
            test_validatefree(
                true,
                fsfree(s),
                "fs_getint('abc') correctly raised error"
            );
        }
        fsfree(s);
    }

    /* 4. Пустая строка – должна упасть */
    test_sub("subtest %d: fs_getint empty", ++subnum);
    {
        fs s = FS();   // пустая строка, v == NULL
        if (!try()) {
            fs_getint(&s);
            test_validatefree(false, fsfree(s), "fs_getint(empty) must raise error");
        } else {
            test_validatefree(true, fsfree(s), "fs_getint(empty) correctly raised error");
        }
        fsfree(s);
    }

    /* 5. Работа с позицией: fs_getintpos */
    test_sub("subtest %d: fs_getintpos", ++subnum);
    {
        fs s = fscopy("abc 123");
        test_validatefree(
            fs_getintpos(&s, 4) == 123,
            fsfree(s),
            "fs_getintpos('abc 123', 4) must return 123"
        );
        fsfree(s);
    }

    /* ========== fs_getlong ========== */

    /* 6. Обычное значение */
    test_sub("subtest %d: fs_getlong normal", ++subnum);
    {
        fs s = fscopy("9999999999");
        test_validatefree(
            fs_getlong(&s) == 9999999999L,
            fsfree(s),
            "fs_getlong('9999999999') must return 9999999999"
        );
        fsfree(s);
    }

    /* 7. Нечисловая строка */
    test_sub("subtest %d: fs_getlong non‑numeric", ++subnum);
    {
        fs s = fscopy("xyz");
        if (!try()) {
            fs_getlong(&s);
            test_validatefree(false, fsfree(s), "fs_getlong('xyz') must raise error");
        } else {
            test_validatefree(true, fsfree(s), "fs_getlong('xyz') correctly raised error");
        }
        fsfree(s);
    }

    /* 8. Пустая строка */
    test_sub("subtest %d: fs_getlong empty", ++subnum);
    {
        fs s = FS();
        if (!try()) {
            fs_getlong(&s);
            test_validatefree(false, fsfree(s), "fs_getlong(empty) must raise error");
        } else {
            test_validatefree(true, fsfree(s), "fs_getlong(empty) correctly raised error");
        }
        fsfree(s);
    }

    /* 9. Позиционное чтение */
    test_sub("subtest %d: fs_getlongpos", ++subnum);
    {
        fs s = fscopy("--- -777");
        test_validatefree(
            fs_getlongpos(&s, 4) == -777L,
            fsfree(s),
            "fs_getlongpos('--- -777', 4) must return -777"
        );
        fsfree(s);
    }

    /* ========== fs_getdouble ========== */

    /* 10. Обычное число */
    test_sub("subtest %d: fs_getdouble normal", ++subnum);
    {
        fs s = fscopy("3.1415");
        test_validatefree(
            fabs(fs_getdouble(&s) - 3.1415) < 0.0001,
            fsfree(s),
            "fs_getdouble('3.1415') must return ~3.1415"
        );
        fsfree(s);
    }

    /* 11. Специальные значения (inf, nan) */
    test_sub("subtest %d: fs_getdouble special", ++subnum);
    {
        fs s_inf = fscopy("inf");
        double val = fs_getdouble(&s_inf);
        test_validatefree(
            isinf(val) && val > 0,
            fsfree(s_inf),
            "fs_getdouble('inf') must be +inf"
        );
        fsfree(s_inf);

        fs s_nan = fscopy("nan");
        val = fs_getdouble(&s_nan);
        test_validatefree(
            isnan(val),
            fsfree(s_nan),
            "fs_getdouble('nan') must be NaN"
        );
        fsfree(s_nan);
    }

    /* 12. Нечисловая строка – должна упасть */
    test_sub("subtest %d: fs_getdouble non‑numeric", ++subnum);
    {
        fs s = fscopy("not_a_number");
        if (!try()) {
            fs_getdouble(&s);
            test_validatefree(false, fsfree(s), "fs_getdouble('not_a_number') must raise error");
        } else {
            test_validatefree(true, fsfree(s), "fs_getdouble('not_a_number') correctly raised error");
        }
        fsfree(s);
    }

    /* 13. Пустая строка – должна упасть */
    test_sub("subtest %d: fs_getdouble empty", ++subnum);
    {
        fs s = FS();
        if (!try()) {
            fs_getdouble(&s);
            test_validatefree(false, fsfree(s), "fs_getdouble(empty) must raise error");
        } else {
            test_validatefree(true, fsfree(s), "fs_getdouble(empty) correctly raised error");
        }
        fsfree(s);
    }

    /* 14. Позиционное чтение double */
    test_sub("subtest %d: fs_getdoublepos", ++subnum);
    {
        fs s = fscopy("val: 2.71828");
        test_validatefree(
            fabs(fs_getdoublepos(&s, 5) - 2.71828) < 0.00001,
            fsfree(s),
            "fs_getdoublepos('val: 2.71828', 5) must return ~2.71828"
        );
        fsfree(s);
    }

    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 23 ---------------------------------


static TestStatus
tf23(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: local version fscmp_strinct", ++subnum);
    {
        const char pt[] = "Qwertyuiop12345";
        fs  s = fscopy(pt);;
        fs  s1 = fsliteral(pt);

        test_validatefree(
            fscmp_strict(s, s1), (fsfree(s), fsfree(s1) ),
            "Literal and normal fs must be equal '%s' != '%s' || %d != %d", fsstr(s), fsstr(s1), fslen(s), fslen(s1)
        );

        fs_sprintf(&s, "bla bla bla %s %s %s %s ...", pt, pt, pt, pt);
        fs  s2 = fsclone(s);

        test_validatefree(
            fscmp_strict(s, s2), (fsfree(s), fsfree(s1), fsfree(s2) ),
             "Origin and clone fs must be equal '%s' != '%s' || %d != %d", fsstr(s), fsstr(s2), fslen(s), fslen(s2)
        );
        fsfree(s), fsfree(s1), fsfree(s2);
    }
    test_sub("subtest %d: pointer version fs_cmp_strinct", ++subnum);
    {

    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 24 ---------------------------------

static TestStatus
tf24(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: move local", ++subnum);
    {
        const char pattern[] = "Qwertyuiop12345";
        fs      s = fscopy(pattern);
        fs      *sp = fsmoveto_heap(s);
        test_validatefree(
            s.v == NULL && s.sz == 0 && s.len == 0, fs_free(sp),
            "Must be empty after moving but not %p %d: %d %d", s.v, s.sz, s.len, s.flags
        );
        test_validatefree(
            strcmp(pattern, sp->v) == 0 && fs_len(sp) == strlen(pattern), fs_free(sp),
            "Must be equal '%s' and '%s'", pattern, sp->v
        );
        fsfree(s);  // should work!
        fs_free(sp);
    }
    test_sub("subtest %d: move literal", ++subnum);
    {
        const char pattern[] = "Qwertyuiop1234567890";
        fs      lit = fsliteral(pattern);
        fs      *sp = fsmoveto_heap(lit);

        test_validatefree(
            strcmp(pattern, fs_str(sp) ) == 0 && fs_len(sp) == strlen(pattern), fs_free(sp),
            "Must be equal '%s' and '%s'", pattern, fs_str(sp)
        );
        //fstechprint(*sp);
        //fstechprint(lit);
        test_validatefree(
             strcmp(pattern, fs_str(sp) ) == 0 && fs_len(sp) == strlen(pattern), fs_free(sp),
             "Origin and clone fs must be equal '%s' != '%s' || %lu != %d", pattern, fs_str(sp), strlen(pattern), fs_len(sp)
        );
        fsfree(lit);  // should work!
        fs_free(sp);
    }
    test_sub("subtest %d: move MOVED fs", ++subnum);
    {
        const char pattern[] = "Qwertyuiop1234567890!!!!!!!!";
        fs         s = fscopy(pattern);
        fs        *sp = fsmoveto_heap(s);
        fs        *sp2 = fs_moveto_heap(sp);
        //fstechprint(s);
        ////fstechprint(*sp); // impossible, sp pointer to nowhere
        //fstechprint(*sp2);
        test_validatefree(
            strcmp(pattern, fs_str(sp2) ) == 0 && fs_len(sp2) == strlen(pattern), fs_free(sp2),
            "Origin and clone fs must be equal '%s' != '%s' || %lu != %d", pattern, fs_str(sp), strlen(pattern), fs_len(sp)
        );
        fsfree(s);
        //fs_free(sp);  // impossible, sp pointer to nowhere
        fs_free(sp2);   // !!!
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 25 ---------------------------------
static TestStatus
tf25(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: fscanf nothing", ++subnum);
    {
        const char   fname[] = "res/fs/fscanf.dat";
        FILE        *f = fopen(fname, "w+");
        if (!f)
            userraiseint(ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open %s for w+", fname);
        fclose(f);
        f = fopen(fname, "r");
        if (!f)
            userraiseint(ERR_UNABLE_OPEN_FILE_READ, "Unable to open %s for r", fname);
        fs  s = FS();
        test_validatefree(
            fs_fscanf(f, &s) == false,
            (fsfree(s), fclose(f) ),
            "Line must NOT be read, cnt %d [%s]", fslen(s), fsstr(s)
        );
        fsfree(s);
        fclose(f);
    }
    test_sub("subtest %d: just fscanf several lines", ++subnum);
    {
        const char   fname[] = "res/fs/fscanf.dat";
        FILE        *f = fopen(fname, "w+");
        if (!f)
            userraiseint(ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open %s for w+", fname);
        const int       cnt = 20;
        char            buf[200];
        for (int i = 0; i < cnt; i++){
            snprintf(buf, sizeof(buf) - 1, "just a string %4d --- hz", i);
            fprintf(f, "%s\n", buf);
        }
        fflush(f);
        rewind(f);
        fs  s = FS();
        for (int i = 0; i < cnt; i++){
            test_validatefree(
                fs_fscanf(f, &s),
                (fsfree(s), fclose(f) ),
                "Unable to fs_fscanf() %d line", i
            );
            snprintf(buf, sizeof(buf) - 1, "just a string %4d --- hz", i);
            test_validatefree(
                strcmp(buf, fsstr(s)) == 0,
                (fsfree(s), fclose(f) ),
                "Line %d not matched [%s] vs fs [%s]", i, buf, fsstr(s)
            );
        }
        fclose(f);
        fsfree(s);
    }
    /* 3. Пустая строка (только перевод строки) */
    test_sub("subtest %d: read empty line", ++subnum);
    {
        const char fname[] = "res/fs/fscanf_empty.dat";
        FILE *f = fopen(fname, "w+");
        if (!f)
            userraiseint(ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open %s", fname);
        fprintf(f, "\n");
        fflush(f);
        rewind(f);

        fs s = FS();
        test_validatefree(
            fs_fscanf(f, &s) && fslen(s) == 0,
            (fsfree(s), fclose(f)),
            "Empty line must be read with length 0, got len=%d, str='%s'", fslen(s), fsstr(s)
        );
        fsfree(s);
        fclose(f);
    }

    /* 4. Последняя строка без завершающего перевода строки */
    test_sub("subtest %d: read last line without newline", ++subnum);
    {
        const char fname[] = "res/fs/fscanf_last.dat";
        FILE *f = fopen(fname, "w+");
        if (!f)
            userraiseint(ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open %s", fname);
        fprintf(f, "line1\nline2");
        fflush(f);
        rewind(f);

        fs s = FS();

        // читаем первую строку
        test_validatefree(
            fs_fscanf(f, &s) && strcmp(fsstr(s), "line1") == 0,
            (fsfree(s), fclose(f)),
            "First line must be 'line1', got '%s'", fsstr(s)
        );

        // читаем вторую (последнюю) строку без \n
        test_validatefree(
            fs_fscanf(f, &s) && strcmp(fsstr(s), "line2") == 0,
            (fsfree(s), fclose(f)),
            "Last line without newline must be 'line2', got '%s'", fsstr(s)
        );

        fsfree(s);
        fclose(f);
    }

    /* 5. Строка с пробелами и спецсимволами */
    test_sub("subtest %d: read line with spaces and special chars", ++subnum);
    {
        const char fname[] = "res/fs/fscanf_special.dat";
        FILE *f = fopen(fname, "w+");
        if (!f)
            userraiseint(ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open %s", fname);
        const char *special_line = "  hello,\tworld!  ";
        fprintf(f, "%s\n", special_line);
        fflush(f);
        rewind(f);

        fs s = FS();
        test_validatefree(
            fs_fscanf(f, &s) && strcmp(fsstr(s), special_line) == 0,
            (fsfree(s), fclose(f)),
            "Line with spaces must be '%s', got '%s'", special_line, fsstr(s)
        );
        fsfree(s);
        fclose(f);
    }

    /* 6. Очень длинная строка (2000 символов) */
    test_sub("subtest %d: read long line", ++subnum);
    {
        const char fname[] = "res/fs/fscanf_long.dat";
        FILE *f = fopen(fname, "w+");
        if (!f)
            userraiseint(ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open %s", fname);

        // генерируем длинную строку
        int     long_len = 200000;
        char   *long_str = malloc(long_len + 1);
        if (!long_str)
            userraiseint(ERR_UNABLE_ALLOCATE, "malloc failed");
        for (int i = 0; i < long_len; i++)
            long_str[i] = itoupper(i % 26);   // 'A' + (i % 26);
        long_str[long_len] = '\0';

        fprintf(f, "%s\n", long_str);
        fflush(f);
        rewind(f);

        fs s = FS();
        test_validatefree(
            fs_fscanf(f, &s) && fslen(s) == long_len && strcmp(fsstr(s), long_str) == 0,
            (fsfree(s), free(long_str), fclose(f)),
            "Long line length must be %d, got len=%d", long_len, fslen(s)
        );
        fsfree(s);
        free(long_str);
        fclose(f);
    }

    /* 7. Перезапись существующего fs (повторный вызов fs_fscanf) */
    test_sub("subtest %d: overwrite existing fs", ++subnum);
    {
        const char fname[] = "res/fs/fscanf_overwrite.dat";
        FILE *f = fopen(fname, "w+");
        if (!f)
            userraiseint(ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open %s", fname);
        fprintf(f, "old line11111\nnew line\n");
        fflush(f);
        rewind(f);

        fs s = FS();

        // читаем первую строку
        test_validatefree(
            fs_fscanf(f, &s) && strcmp(fsstr(s), "old line11111") == 0,
            (fsfree(s), fclose(f)),
            "First line must be 'old line', got '%s'", fsstr(s)
        );

        // читаем вторую строку в тот же объект s – он должен перезаписаться
        test_validatefree(
            fs_fscanf(f, &s) && strcmp(fsstr(s), "new line") == 0,
            (fsfree(s), fclose(f)),
            "After second read, s must be 'new line', got '%s'", fsstr(s)
        );

        fsfree(s);
        fclose(f);
    }
    check_leak(true);
    /* 8. Вызов fs_fscanf(f, NULL) – успешное чтение */
    test_sub("subtest %d: fs_fscanf with NULL (create new)", ++subnum);
    {
        const char fname[] = "res/fs/fscanf_null_new.dat";
        FILE *f = fopen(fname, "w+");
        if (!f)
            userraiseint(ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open %s", fname);
        fprintf(f, "hello from NULL\n");
        rewind(f);

        fs *sp = fs_fscanf(f, NULL);   // создаёт новый fs
        test_validatefree(
            sp != NULL && strcmp(fs_str(sp), "hello from NULL") == 0,
            (sp ? fs_free(sp) : (void)0, fclose(f)),
            "fs_fscanf(NULL) should return new fs with correct string, got %p, str='%s'",
            (void*)sp, sp ? fs_str(sp) : "null"
        );
        //if (sp)
        fs_free(sp);    // MUST work even if sp is NULL
        fclose(f);
    }

    /* 9. Вызов fs_fscanf(f, NULL) – EOF (пустой файл) */
    test_sub("subtest %d: fs_fscanf with NULL (EOF)", ++subnum);
    {
        const char fname[] = "res/fs/fscanf_null_eof.dat";
        FILE *f = fopen(fname, "w+");   // создали пустой файл
        if (!f)
            userraiseint(ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open %s", fname);
        rewind(f);

        fs *sp = fs_fscanf(f, NULL);
        test_validatefree(
            sp == NULL,
            fclose(f),
            "fs_fscanf(NULL) on empty file must return NULL"
        );
        // just to be sure
        //if (sp)
        fs_free(sp); // MUST work even if sp is NULL
        // если бы вернулся не NULL, надо было бы освободить, но этого не произошло
        fclose(f);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 26 ---------------------------------
static TestStatus
tf26(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    /* 1. Пустой формат */
    test_sub("subtest %d: empty format", ++subnum);
    {
        fs      s = fscopyf("");
        test_validatefree(
            fslen(s) == 0,
            fsfree(s),
            "Empty format must have length 0, got %d", fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "") == 0,
            fsfree(s),
            "Empty format must produce empty string, got '%s'", fsstr(s)
        );
        fsfree(s);
    }
    check_leak(true);

    /* 2. Простой текст (без спецификаторов) */
    test_sub("subtest %d: plain text", ++subnum);
    {
        const char *text = "hello, world";
        fs      s = fscopyf("%s", text);   /* можно и просто fscopyf("hello, world") */
        test_validatefree(
            fslen(s) == (int) strlen(text),
            fsfree(s),
            "Plain text: length mismatch, got %d, expected %zu", fslen(s), strlen(text)
        );
        test_validatefree(
            strcmp(fsstr(s), text) == 0,
            fsfree(s),
            "Plain text: content mismatch, got '%s', expected '%s'", fsstr(s), text
        );
        fsfree(s);
    }
    check_leak(true);

    /* 3. Формат с числами и строкой */
    test_sub("subtest %d: formatted string", ++subnum);
    {
        int     ival = 42;
        double  dval = 3.1415;
        const char *sval = "test";
        char    expected[200];
        snprintf(expected, sizeof(expected), "int=%d, double=%.4f, str=%s", ival, dval, sval);

        fs      s = fscopyf("int=%d, double=%.4f, str=%s", ival, dval, sval);
        test_validatefree(
            fslen(s) == (int) strlen(expected),
            fsfree(s),
            "Formatted length: got %d, expected %zu", fslen(s), strlen(expected)
        );
        test_validatefree(
            strcmp(fsstr(s), expected) == 0,
            fsfree(s),
            "Formatted content: got '%s', expected '%s'", fsstr(s), expected
        );
        fsfree(s);
    }
    check_leak(true);

    /* 4. Длинная строка (проверка расширения буфера) */
    test_sub("subtest %d: long string (expands buffer)", ++subnum);
    {
        // Генерируем длинную строку из повторяющихся символов
        int     repeat = 20000;
        char    *long_text = malloc(repeat + 1);
        if (!long_text)
            userraiseint(ERR_UNABLE_ALLOCATE, "malloc failed");
        memset(long_text, 'A', repeat);
        long_text[repeat] = '\0';

        fs      s = fscopyf("%s", long_text);
        test_validatefree(
            fslen(s) == repeat,
            fsfree(s),
            "Long string length: got %d, expected %d", fslen(s), repeat
        );
        test_validatefree(
            strcmp(fsstr(s), long_text) == 0,
            fsfree(s),
            "Long string content mismatch"
        );

        free(long_text);
        fsfree(s);
    }
    check_leak(true);

    /* 5. Последовательные вызовы (проверка утечек) */
    test_sub("subtest %d: sequential calls (no leaks)", ++subnum);
    {
        // Несколько созданий и удалений, чтобы убедиться, что память не утекает
        for (int i = 0; i < 50; i++) {
            fs s = fscopyf("iteration_%d", i);
            fsfree(s);
        }
        // Если бы были утечки, check_leak в конце функции их обнаружит
    }
    check_leak(true);

    /* 6. Строка со спецсимволами и пробелами */
    test_sub("subtest %d: special chars and spaces", ++subnum);
    {
        const char *special = "  \t\n\r  hello  \t\n\r  ";
        fs      s = fscopyf("%s", special);
        test_validatefree(
            strcmp(fsstr(s), special) == 0,
            fsfree(s),
            "Special chars mismatch: got '%s', expected '%s'", fsstr(s), special
        );
        fsfree(s);
    }
    check_leak(true);

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 27 ---------------------------------

static TestStatus
tf27(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    /* ================= fs_rpad ================= */

    /* 1. rpad: короткая строка, pad длиннее */
    test_sub("subtest %d: rpad short string with longer pad", ++subnum);
    {
        int     newlen = 7;
        fs      s = fscopy("abc");
        fs      pad = fscopy("XY");
        fs      result = fs_rpad(&s, newlen, &pad);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == newlen,
            (fsfree(s), fsfree(pad) ),
            "rpad: length must be %d, got %d", newlen, fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "abcXYXY") == 0,
            (fsfree(s), fsfree(pad) ),
            "rpad: expected 'abcXYXY', got '%s'", fsstr(s)
        );
        fsfree(s);
        fsfree(pad);
    }
    /* 2. rpad: обрезание длинной строки */
    test_sub("subtest %d: rpad cut long string", ++subnum);
    {
        int     newlen = 5;
        fs      s = fscopy("hello world");
        fs      pad = fscopy(".");
        fs      result = fs_rpad(&s, newlen, &pad);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == newlen,
            (fsfree(s), fsfree(pad)),
            "rpad cut: length must be %d, got %d", newlen, fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "hello") == 0,
            (fsfree(s), fsfree(pad)),
            "rpad cut: expected 'hello', got '%s'", fsstr(s)
        );
        fsfree(s);
        fsfree(pad);
    }
    /* 3. rpad: пустой pad – строка остаётся без изменений (короткая) */
    test_sub("subtest %d: rpad empty pad (short string)", ++subnum);
    {
        int     newlen = 5;
        fs      s = fscopy("abc");
        fs      pad = FS();               /* пустая строка */
        fs      result = fs_rpad(&s, newlen, &pad);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == 3,
            (fsfree(s), fsfree(pad)),
            "rpad empty pad: length must stay 3, got %d", fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "abc") == 0,
            (fsfree(s), fsfree(pad)),
            "rpad empty pad: expected 'abc', got '%s'", fsstr(s)
        );
        fsfree(s);
        fsfree(pad);
    }
    /* 4. rpad: циклическое заполнение (pad короче) */
    test_sub("subtest %d: rpad cyclic pad", ++subnum);
    {
        int     newlen = 8;
        fs      s = fscopy("AB");
        fs      pad = fscopy("123");
        fs      result = fs_rpad(&s, newlen, &pad);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == newlen,
            (fsfree(s), fsfree(pad)),
            "rpad cyclic: length must be %d, got %d", newlen, fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "AB123123") == 0,
            (fsfree(s), fsfree(pad)),
            "rpad cyclic: expected 'AB123123', got '%s'", fsstr(s)
        );
        fsfree(s);
        fsfree(pad);
    }
    /* 5. rpad: len == 0, строка непустая – обрезается до пустой */
    test_sub("subtest %d: rpad len=0 (cut to empty)", ++subnum);
    {
        int     newlen = 0;
        fs      s = fscopy("cutme");
        fs      pad = fscopy("-");
        fs      result = fs_rpad(&s, newlen, &pad);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == 0,
            (fsfree(s), fsfree(pad)),
            "rpad len=0: length must be 0, got %d", fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "") == 0,
            (fsfree(s), fsfree(pad)),
            "rpad len=0: expected empty, got '%s'", fsstr(s)
        );
        fsfree(s);
        fsfree(pad);
    }

    /* ================= fs_lpad ================= */

    /* 6. lpad: короткая строка, pad длиннее */
    test_sub("subtest %d: lpad short string with longer pad", ++subnum);
    {
        int     newlen = 7;
        fs      s = fscopy("abc");
        fs      pad = fscopy("XY");
        fs      result = fs_lpad(&s, newlen, &pad);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == newlen,
            (fsfree(s), fsfree(pad)),
            "lpad: length must be %d, got %d", newlen, fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "XYXYabc") == 0,
            (fsfree(s), fsfree(pad)),
            "lpad: expected 'XYXYabc', got '%s'", fsstr(s)
        );
        fsfree(s);
        fsfree(pad);
    }
    /* 7. lpad: обрезание длинной строки */
    test_sub("subtest %d: lpad cut long string", ++subnum);
    {
        int     newlen = 5;
        fs      s = fscopy("hello world");
        fs      pad = fscopy(".");
        fs      result = fs_lpad(&s, newlen, &pad);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == newlen,
            (fsfree(s), fsfree(pad)),
            "lpad cut: length must be %d, got %d", newlen, fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "hello") == 0,
            (fsfree(s), fsfree(pad)),
            "lpad cut: expected 'hello', got '%s'", fsstr(s)
        );
        fsfree(s);
        fsfree(pad);
    }
    /* 8. lpad: пустой pad – строка не меняется (короткая) */
    test_sub("subtest %d: lpad empty pad (short string)", ++subnum);
    {
        int     newlen = 5;
        fs      s = fscopy("abc");
        fs      pad = FS();
        fs      result = fs_lpad(&s, newlen, &pad);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == 3,
            (fsfree(s), fsfree(pad)),
            "lpad empty pad: length must stay 3, got %d", fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "abc") == 0,
            (fsfree(s), fsfree(pad)),
            "lpad empty pad: expected 'abc', got '%s'", fsstr(s)
        );
        fsfree(s);
        fsfree(pad);
    }
    /* 9. lpad: циклическое заполнение */
    test_sub("subtest %d: lpad cyclic pad", ++subnum);
    {
        int     newlen = 8;
        fs      s = fscopy("AB");
        fs      pad = fscopy("123");
        fs      result = fs_lpad(&s, newlen, &pad);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == newlen,
            (fsfree(s), fsfree(pad)),
            "lpad cyclic: length must be %d, got %d", newlen, fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "123123AB") == 0,
            (fsfree(s), fsfree(pad)),
            "lpad cyclic: expected '123123AB', got '%s'", fsstr(s)
        );
        fsfree(s);
        fsfree(pad);
    }
    /* 10. lpad: len == 0 */
    test_sub("subtest %d: lpad len=0 (cut to empty)", ++subnum);
    {
        int     newlen = 0;
        fs      s = fscopy("cutme");
        fs      pad = fscopy("-");
        fs      result = fs_lpad(&s, newlen, &pad);
        fstechfprint(logfile, result);

        test_validatefree(
            fslen(s) == 0,
            (fsfree(s), fsfree(pad)),
            "lpad len=0: length must be 0, got %d", fslen(s)
        );
        test_validatefree(
            strcmp(fsstr(s), "") == 0,
            (fsfree(s), fsfree(pad)),
            "lpad len=0: expected empty, got '%s'", fsstr(s)
        );
        fsfree(s);
        fsfree(pad);
    }
    check_leak(true);
    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 28 ---------------------------------
static TestStatus
tf28(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Создание копии из пустой строки (FS()) */
    test_sub("subtest %d: heapcreate from empty FS", ++subnum);
    {
        fs      orig = FS();                    // пустая строка
        fs     *copy = fs_heapcreate(&orig);

        // Оригинал не должен пострадать
        test_validatefree(
            fslen(orig) == 0 && fsstr(orig) == NULL,
            fs_free(copy),
            "Original must stay empty after heapcreate"
        );
        // Копия должна быть пустой, но валидной
        test_validatefree(
            copy != NULL && fs_len(copy) == 0,
            fs_free(copy),
            "Copy must be non-NULL with length 0"
        );
        test_validatefree(
            fs_bodyalloc(copy),   // проверяем FS_FLAG_BODYALLOC
            fs_free(copy),
            "Copy must have FS_FLAG_BODYALLOC flag"
        );

        fs_free(copy);
        fs_alloc_check(true);
    }

    /* 2. Создание копии из обычной строки (fscopy) */
    test_sub("subtest %d: heapcreate from normal string", ++subnum);
    {
        const char *text = "test string";
        fs      orig = fscopy(text);
        fs     *copy = fs_heapcreate(&orig);

        // Оригинал не должен измениться
        test_validatefree(
            strcmp(fsstr(orig), text) == 0 && fslen(orig) == (int)strlen(text),
            (fsfree(orig), fs_free(copy)),
            "Original must be unchanged after heapcreate"
        );
        test_validatefree(
            !fs_bodyalloc(&orig),
            (fsfree(orig), fs_free(copy)),
            "Original must NOT have FS_FLAG_BODYALLOC"
        );

        // Копия должна содержать ту же строку и быть независимой
        test_validatefree(
            strcmp(fs_str(copy), text) == 0 && fs_len(copy) == (int)strlen(text),
            (fsfree(orig), fs_free(copy)),
            "Copy must contain the same string"
        );
        test_validatefree(
            fs_bodyalloc(copy),
            (fsfree(orig), fs_free(copy)),
            "Copy must have FS_FLAG_BODYALLOC flag"
        );

        // Модифицируем оригинал – копия не должна измениться
        fs_sprintf(&orig, "modified");
        test_validatefree(
            strcmp(fs_str(copy), text) == 0,
            (fsfree(orig), fs_free(copy)),
            "Copy must remain unchanged after original modification"
        );

        fsfree(orig);
        fs_free(copy);
        fs_alloc_check(true);
    }

    /* 3. Копирование уже перемещённого объекта (после fs_moveto_heap) */
    test_sub("subtest %d: heapcreate from moved object", ++subnum);
    {
        const char *text = "move_then_copy";
        fs      orig = fscopy(text);
        fs     *moved = fs_moveto_heap(&orig);      // orig опустошён, moved в куче с FS_FLAG_BODYALLOC

        // orig больше не владеет строкой
        test_validatefree(
            fslen(orig) == 0 && fsstr(orig) == NULL,
            fs_free(moved),
            "After moveto_heap, original must be empty"
        );

        // Теперь создаём копию из перемещённого объекта
        fs     *copy = fs_heapcreate(moved);

        test_validatefree(
            strcmp(fs_str(copy), text) == 0 && fs_len(copy) == (int)strlen(text),
            (fs_free(moved), fs_free(copy)),
            "Copy from moved object must contain original string"
        );
        test_validatefree(
            fs_bodyalloc(copy),
            (fs_free(moved), fs_free(copy)),
            "Copy from moved object must have FS_FLAG_BODYALLOC"
        );

        fs_free(moved);
        fs_free(copy);
        fs_alloc_check(true);
    }

    /* 4. NULL-указатель – ожидаем аварийное завершение (перехватываем сигнал) */
    test_sub("subtest %d: heapcreate with NULL pointer (must raise error)", ++subnum);
    {
        if (!try()) {
            fs *copy = fs_heapcreate(NULL);     // должно упасть внутри fs_heapcreate
            // Если мы здесь, ошибки не было – тест провален
            test_validate(
                false,
                "fs_heapcreate(NULL) must raise an error, but it didn't"
            );
            if (copy) fs_free(copy);            // на всякий случай
        } else {
            // Сигнал перехвачен – это ожидаемое поведение
            test_validate(
                true,
                "fs_heapcreate(NULL) correctly raised an error"
            );
        }
        fs_alloc_check(true);
    }

    /* 5. Последовательные вызовы и проверка утечек */
    test_sub("subtest %d: heapcreate multiple calls (leak check)", ++subnum);
    {
        const char *words[] = {"one", "two", "three"};
        fs     *copies[3];

        for (int i = 0; i < COUNT(words); i++) {
            fs      orig = fscopy(words[i]);
            copies[i] = fs_heapcreate(&orig);
            fsfree(orig);   // оригинал больше не нужен
        }

        // Проверяем содержимое
        for (int i = 0; i < COUNT(words); i++) {
            test_validatefree(
                strcmp(fs_str(copies[i]), words[i]) == 0,
                (fs_free(copies[0]), fs_free(copies[1]), fs_free(copies[2])),
                "Copy %d must be '%s'", i, words[i]
            );
        }

        for (int i = 0; i < COUNT(words); i++) fs_free(copies[i]);
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST fs_heapcopy ---------------------------------
static TestStatus
tf_fs_heapcopy(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Обычная строка */
    test_sub("subtest %d: heapcopy normal string", ++subnum);
    {
        const char *text = "hello, world";
        fs     *copy = fs_heapcopy(text);

        test_validatefree(
            strcmp(fs_str(copy), text) == 0 && fs_len(copy) == (int)strlen(text),
            fs_free(copy),
            "Heapcopy must contain '%s', got '%s' (len %d)", text, fs_str(copy), fs_len(copy)
        );
        test_validatefree(
            fs_bodyalloc(copy),
            fs_free(copy),
            "Copy must have FS_FLAG_BODYALLOC flag"
        );

        fs_free(copy);
        fs_alloc_check(true);
    }

    /* 2. Пустая строка "" */
    test_sub("subtest %d: heapcopy empty string", ++subnum);
    {
        const char *text = "";
        fs     *copy = fs_heapcopy(text);

        test_validatefree(
            fs_len(copy) == 0 && copy->v != NULL && fs_str(copy)[0] == '\0',
            fs_free(copy),
            "Heapcopy of empty string must have len=0, v!=NULL, got len=%d, v=%p", fs_len(copy), (void*)copy->v
        );
        test_validatefree(
            fs_bodyalloc(copy),
            fs_free(copy),
            "Copy must have FS_FLAG_BODYALLOC flag"
        );

        fs_free(copy);
        fs_alloc_check(true);
    }

    /* 3. Несколько вызовов и проверка утечек */
    test_sub("subtest %d: heapcopy multiple calls (leak check)", ++subnum);
    {
        const char *words[] = {"one", "two", "three"};
        fs     *copies[3];

        for (int i = 0; i < COUNT(words); i++) {
            copies[i] = fs_heapcopy(words[i]);
        }

        for (int i = 0; i < COUNT(words); i++) {
            test_validatefree(
                strcmp(fs_str(copies[i]), words[i]) == 0,
                (fs_free(copies[0]), fs_free(copies[1]), fs_free(copies[2])),
                "Copy %d must be '%s', got '%s'", i, words[i], fs_str(copies[i])
            );
            fs_free(copies[i]);
        }
        fs_alloc_check(true);
    }

    /* 4. NULL-указатель (ожидаем ошибку) */
    test_sub("subtest %d: heapcopy NULL pointer (must raise error)", ++subnum);
    {
        if (!try()) {
            fs *copy = fs_heapcopy(NULL);
            test_validate(
                false,
                "fs_heapcopy(NULL) must raise an error, but it didn't"
            );
            if (copy) fs_free(copy);
        } else {
            test_validate(
                true,
                "fs_heapcopy(NULL) correctly raised an error"
            );
        }
        fs_alloc_check(true);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main( /* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 = tf1,              .num =  1, .name = "Simple init and validate test"              , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf2,              .num =  2, .name = "Access read/write test"                     , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf3,              .num =  3, .name = "Elem() test"                                , .desc=""                , .mandatory=true)
    // non numeral
      , testnew(.f2 = tf_fs_clone,      .num =  4, .name = "fs_clone() simple test"                     , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf4,              .num =  5, .name = "fs_cat/fs_catstr test"                      , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf5,              .num =  6, .name = "fs_cpy/fs_cpystr test"                      , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf6,              .num =  7, .name = "fsfreeall test"                             , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf7,              .num =  8, .name = "fsprint/printlim manual test"               , .desc="always ok, for the manual check"                , .mandatory=true)
      , testnew(.f2 = tf8,              .num =  9, .name = "fsprint_arr manual test"                    , .desc="always ok, for the manual check"                , .mandatory=true)
      , testnew(.f2 = tf9,              .num = 10, .name = "fs_sprintf formatted test"                  , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf10,             .num = 11, .name = "fslocal simple test"                        , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf11,             .num = 12, .name = "fs_save/load test"                          , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf12,             .num = 13, .name = "fs_free_alloc_checker test"                 , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf13,             .num = 14, .name = "fs_move simple test"                        , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf14,             .num = 15, .name = "fs_substr/newsubstr simple test"            , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf15,             .num = 16, .name = "fs_ifnotin/fs_ifinotin simple test"         , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf16,             .num = 17, .name = "fs_instr/fs_iinstr simple test"             , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf17,             .num = 18, .name = "fs_(n)(i)chr simple tests"                  , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf18,             .num = 19, .name = "fs_(i)rchr simple tests"                    , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf19,             .num = 20, .name = "fs_n(i)instr simple tests"                  , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf20,             .num = 21, .name = "fs_rev_catstr simple tests"                 , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf21,             .num = 22, .name = "fs_str simple tests"                        , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf22,             .num = 23, .name = "fs_get<int/long/double>pos simple tests"    , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf23,             .num = 24, .name = "fs_cmp_strict simple tests"                 , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf24,             .num = 25, .name = "fs_moveto_heap simple tests"                , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf25,             .num = 26, .name = "fs_fscanf simple tests"                     , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf26,             .num = 27, .name = "fscopyf simple tests"                       , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf27,             .num = 28, .name = "fs_rpad()/lpad() simple tests"              , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf28,             .num = 29, .name = "fs_heapcreate() simple tests"               , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf_fs_heapcopy,   .num = 30, .name = "fs_heapcopy() simple tests"                 , .desc=""                , .mandatory=true)
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* FSTESTING */

