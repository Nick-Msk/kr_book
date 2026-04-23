#ifndef _FS_H
#define _FS_H

// ---------------------------------------------------------------------------------
// --------------------------- Public Faststring API -------------------------------
// ---------------------------------------------------------------------------------

// --------------- Includes -----------------------

#include <stdio.h>
#include <string.h>
//
#include "bool.h"
#include "log.h"
#include "common.h"
#include "error.h"
#include "checker.h"

// ----------- CONSTANTS AND GLOBALS ---------------

// ------------------- TYPES -----------------------

typedef enum  {
               FS_FLAG_ALLOC  = 0x0     // standard allocation
             , FS_FLAG_STATIC = 0x1
             , FS_FLAG_CONST  = 0x2     // Not user for now
             , FS_FLAG_LOCAL  = 0x4
             //, FS_FLAG_MOVED  = 0x8     // for fsarr_move()
} FS_FLAGS;

// type-support functions

// to string
static inline const char * fs_flag_str(FS_FLAGS flag){
    switch(flag){
        CASE_RETURN(FS_FLAG_STATIC);        // not sure whether is good to use CASE_RETURN
        CASE_RETURN(FS_FLAG_CONST);
        CASE_RETURN(FS_FLAG_LOCAL);
        CASE_RETURN(FS_FLAG_ALLOC);
        //CASE_RETURN(FS_FLAG_MOVED);
        default:         return "Unknown action";
    }
}

// faststring
typedef struct fs {
    int         len, sz; // sz >= len + 1 because of last '\0'
    FS_FLAGS    flags;
    char       *v;      // with '\0'
} fs;

// flag checkers
static inline bool          fs_flag_static(FS_FLAGS fl){
    return fl & FS_FLAG_STATIC;
}

static inline bool          fs_static(const fs *s){
    return fs_flag_static(s->flags);
}

static inline bool          fs_flag_const(FS_FLAGS fl){
    return fl & FS_FLAG_CONST;
}

static inline bool          fs_const(const fs *s){
    return fs_flag_const(s->flags);
}

static inline bool          fs_flag_local(FS_FLAGS fl){
    return fl & FS_FLAG_LOCAL;
}

static inline bool          fs_local(const fs *s){
    return fs_flag_local(s->flags);
}

static inline bool          fs_flag_alloc(FS_FLAGS fl){
    return fl == FS_FLAG_ALLOC; // heap marker
}

static inline bool          fs_alloc(const fs *s){
    return fs_flag_alloc(s->flags);
}

/*static inline bool          fs_flag_moved(FS_FLAGS fl){
    return fl == FS_FLAG_MOVED;
}

static inline bool          fs_moved(const fs *s){
    return fs_flag_moved(s->flags);
}*/

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

#define             FSEMPTY (fs){.sz = 0, .len = 0, .flags = FS_FLAG_STATIC, .v = ""};

#define             FSINITSTATIC(...)  (fs){.sz = 1, .len = 0, .flags = FS_FLAG_STATIC, .v = "", ##__VA_ARGS__}

#define             FS(...)   (fs){.sz = 0, .len = 0, .flags = FS_FLAG_ALLOC, .v = 0, ##__VA_ARGS__}

#define             fsfree(s) fs_free(&(s))

#define             fsfreeall(...) fs_freeall( (fs *[]){__VA_ARGS__, 0} )

#define             CONCATENATE(prefix, name) prefix ## _ ## name
#define             fslocal(name, leng) char CONCATENATE(_FS_TMP_, name)[(leng) + 1];\
 fs name = (fs) {.len = (leng), .sz = (leng) + 1, .flags = FS_FLAG_LOCAL, .v = CONCATENATE(_FS_TMP_, name) }

// destructor, macro wrapper will be
extern void                 fs_free(fs *s);

// mass destructor
static inline void          fs_freeall(fs **arr){
    fs  *s;
    while ( (s = *arr++) )
        if (s->v)
            fs_free(s);
}
// --------------------------------------------------------------------------
extern fs                   fs_clone(const fs *s);

static inline fs            fsclone(fs s){
    return fs_clone(&s);
}

// lit MUST BE static!
static inline fs            fsliteral(const char *lit){
    fs s = FSEMPTY;
    s.v = (char *) lit;
    s.len = strlen(lit);
    s.sz = s.len + 1;
    return s;
}
extern fs                   fsinit(int sz);

static inline fs            fsempty(void){
    return FS();
}

static inline fs            fscopy(const char *str){
    fs         tmp = fsliteral(str);
    return  fs_clone(&tmp);
}


#if defined(FS_ALLOCATOR)
// detach from allocator! Must be freed manually
extern bool                 fsdetach(fs *s);

// free all allocated
extern void                 fsfreeall(void);
#endif

// TODO: fs_const()

#define                     fsmove(s) fs_move(&(s) )

// -------------------- ACCESS AND MODIFICATORS ------------------------

static inline fs            fs_move(fs *orig){
    if (!fs_alloc(orig) )
        userraiseint(ERR_FS_NOT_ALLOC_FLAG, "Unable to move not allocated fs (type %s)", fs_flag_str(orig->flags) );    // 10001 interrupt
    fs tmp = *orig;
    *orig = FS();
    return logsimpleret(tmp, "fs moved %d: %p", tmp.sz, tmp.v);
}

// direct access, NO change len or sz, position MUST be < sz
static inline char          *fs_get(const fs *s, int pos){
    //return logsimpleret(s->v + pos, "Getting %p[%c]", s->v + pos, s->v[pos]);
    return s->v + pos;
}

// automatically adjust len (??) and sz (realloc)
extern char                 *fs_elem(fs *s, int pos);

static inline char          *fs_setlen(fs *s, int poslen){
    s->len = poslen;
    *fs_elem(s, poslen) = '\0';
    return fs_get(s, poslen);
}

static inline char          *fs_clear(fs *s){
    return fs_setlen(s, 0);
}

static inline int           fs_len(const fs *s){
    return s->len;
}

static inline int           fslen(fs s){
    return s.len;
}

static inline bool          fsisempty(fs s){
    return fslen(s) == 0;
}

// just and FS() or after freed
static inline bool          fsisnull(fs s){
    return s.sz == 0 && s.v == 0;
}

static inline int           fssz(fs s){
    return s.sz;
}

// shrink to real len + 1 ( + 1 because '\0' is ASSUMED)
extern fs                   *fs_shrink(fs *s);

extern fs                   *fs_resize(fs *s, int newsz);

static inline fs            *fs_increase(fs *s, int inc){
    return fs_resize(s, s->sz + inc);
}

static inline char          *fs_str(fs *s){
    return s->v;
}

static inline const char    *fs_strdup(fs *s){
    return (const char *) strdup(s->v);
}

// pointer version
static inline int            fs_cmp(const fs *restrict str1, const fs *restrict str2){
    return strcmp(str1->v, str2->v);
}
// pointer version, limited
static inline int            fs_ncmp(const fs *restrict str1, const fs *restrict str2, int len){
    return strncmp(str1->v, str2->v, len);
}
// local version
static inline int            fscmp(fs str1, fs str2){
    return strcmp(str1.v, str2.v);
}
// local version, limited
static inline int            fsncmp(fs str1, fs str2, int len){
    return strncmp(str1.v, str2.v, len);
}
// pointer version, insensitive
static inline int            fs_icmp(const fs* restrict str1, const fs* restrict str2){
    return strcasecmp(str1->v, str2->v);
}
// pointer version, insensitive, limited
static inline int            fs_nicmp(const fs* restrict str1, const fs* restrict str2, int len){
    return strncasecmp(str1->v, str2->v, len);
}
// local version, insensitive
static inline int            fsicmp(fs str1, fs str2){
    return strcasecmp(str1.v, str2.v);
}
// local version, insensitive, limited
static inline int            fsnicmp(fs str1, fs str2, int len){
    return strncasecmp(str1.v, str2.v, len);
}

// ------------------------------------------ substring srearch ---------------------------------------------------
// pointer, sensitive, unlim
static inline int            fs_instr(const fs* restrict str1, const fs* restrict str2){
    const char *p = strstr(str1->v, str2->v);
    return p == 0 ? -1 : p - str1->v;
}
// pointer, insensitive, unlim
static inline int            fs_iinstr(const fs* restrict str1, const fs* restrict str2){
    const char *p = strcasestr(str1->v, str2->v);   // NOTE: not portable solution
    return p == 0 ? -1 : p - str1->v;
}
// local, sensitive, unlim
static inline int            fsinstr(fs str1, fs str2){
    const char *p = strstr(str1.v, str2.v);   // NOTE: not portable solution
    return p == 0 ? -1 : p - str1.v;
}
// local, insensitive, unlim
static inline int            fsiinstr(fs str1, fs str2){
    const char *p = strcasestr(str1.v, str2.v);   // NOTE: not portable solution
    return p == 0 ? -1 : p - str1.v;
}
// TODO: fs_ninstr(fs *, fs *, int); fs_niinstr(fs *, fs *, n);
// common search for limited!
extern int                   fs_lim_instr(const fs* restrict str1, const fs* restrict str2, int lim, bool lowercase);

// pointer, sensitive, lim
static inline int            fs_ninstr(const fs* restrict str1, const fs* restrict str2, int lim){
    return  fs_lim_instr(str1, str2, lim, false);
}
// pointer, insensitive, lim
static inline int            fs_niinstr(const fs* restrict str1, const fs* restrict str2, int lim){
    return  fs_lim_instr(str1, str2, lim, true);
}
// local, sensitive, lim
static inline int            fsninstr(fs str1, fs str2, int lim){
    return  fs_lim_instr(&str1, &str2, lim, false);
}
// local, insensitive, lim
static inline int            fsniinstr(fs str1, fs str2, int lim){
    return  fs_lim_instr(&str1, &str2, lim, true);
}
// ------------------------------------------ one symbol search --------------------------------------------------------
// pointer, sensitive, unlim (library call)
static inline int            fs_chr(const fs *str, char c){
    const char *p = strchr(str->v, c);
    return p ? p - str->v : -1;
}
// pointer, sensitive, lim
static inline int            fs_nchr(const fs *str, char c, int lim){
    const char *p = str->v;
    while (lim-- > 0 && *p != '\0' && *p != c)
        p++;
    return *p != '\0' && lim > 0 ? p - str->v : -1;
}
// pointer, insensitive, lim
static inline int            fs_nichr(const fs *str, char c, int lim){
    const char *p = str->v;
    c = tolower(c);
    while (lim-- > 0 && *p != '\0' && tolower(*p) != c)
        p++;
    return *p != '\0' && lim > 0 ? p - str->v : -1;
}
// pointer, insensitive, unlim
static inline int            fs_ichr(const fs *str, char c){
    return fs_nichr(str, c, INT_MAX);
}
// local version
// local, sensitive, unlim
static inline int            fschr(fs str, char c){
    return fs_chr(&str, c);
}
// local, sensitive, lim
static inline int            fsnchr(fs str, char c, int lim){
    return fs_nchr(&str, c, lim);
}
// local, insensitive, lim
static inline int            fsnichr(fs str, char c, int lim){
    return fs_nichr(&str, c, lim);
}
// local, insensitive, unlim
static inline int            fsichr(fs str, char c){
    return fs_ichr(&str, c);
}
// ------------------------------------------ ONE SYMBOL REVERSE SEARCH  --------------------------------------------------------
// pointer, sensitive, unlim
static inline int            fs_rchr(const fs *str, char c){
    const char *p = str->v;
    int         pos = str->len;
    while (pos >= 0 && p[pos] != c)
        pos--;
    return p[pos] == c ? pos : -1;
}
// pointer, insensitive, unlim
static inline int            fs_irchr(const fs *str, char c){
    const char *p = str->v;
    int         pos = str->len;
    c = tolower(c);
    while (pos >= 0 && tolower(p[pos]) != c)
        pos--;
    return tolower(p[pos]) == c ? pos : -1;
}
// local, sensitive, unlim
static inline int            fsrchr(fs str, char c){
    return fs_rchr(&str, c);
}
// pointer, insensitive, unlim
static inline int            fsirchr(fs str, char c){
    return fs_irchr(&str, c);
}
// ------------------------------------------------------ EXCNAHGERS ------------------------------------------------------------

static inline void           fs_exch(fs *s1, fs *s2){
    fs tmp = *s1;
    *s1 = *s2;
    *s2 = tmp;
}

static inline fs             fs_reverse(fs str){
    reverse(str.v, str.len);
    return str;
}
// ----------------------------------------------------------------- CAT/CPY/ETC -------------------------------------------------
extern fs                    fs_cat(fs *target, fs source);
static inline fs             fs_catstr(fs *restrict target, const char *restrict source){
    fs l = fsliteral(source);
    return fs_cat(target, l);
}
// put beginner at the start position of the target
extern fs                    fs_rev_catstr(fs *restrict target, const char *restrict beginner);

static inline fs             fs_cpy(fs *target, fs source){
    target->len = 0;
    return fs_cat(target, source);
}

static inline fs             fs_cpystr(fs *restrict target, const char *restrict source){
    fs l = fsliteral(source);
    return fs_cpy(target, l);
}

extern int                   fs_sprintf(fs *restrict s, const char *restrict fmt, ...) __attribute__ (( format (printf, 2, 3) ) );

// fast in-place left substring
static inline fs             fs_left(fs *s, int cnt){
    if (cnt < 0)
        cnt = 0;
    if (cnt > s->len)
        cnt = s->len;
    fs_setlen(s, cnt);
    return *s;
}

// fast in-place!
extern fs                    fs_substr(fs *s, int from, int to);

// constructor version
extern fs                    fs_newsubstr(const fs *s, int from, int to);

static inline fs            *fs_tolower_interval(fs *str, int from, int to){
    // TODO: probably fs_iter - think about it
    for (int i = MAX(from, 0); i < MIN(to, str->len); i++)
        str->v[i] = tolower(str->v[i]);
    return str;
}

static inline fs            *fs_tolower(fs *str){
    return fs_tolower_interval(str, 0, str->len);
}

static inline fs            *fs_toupper_interval(fs *str, int from, int to){
    // TODO: probably fs_iter - think about it
    for (int i = MAX(from, 0); i < MIN(to, str->len); i++)
        str->v[i] = toupper(str->v[i]);
    return str;
}
static inline fs            *fs_toupper(fs *str){
    return fs_toupper_interval(str, 0, str->len);
}

// full scan version
static inline bool           fs_notin(fs s, const char *strs[]){
    while (*strs){
        if (strcmp(s.v, *strs) == 0)
            return false;
        else
            strs++;
    }
    return true;
}

// full scan version, insensitive
static inline bool           fs_inotin(fs s, const char *strs[]){
    while (*strs){
        if (strcasecmp(s.v, *strs) == 0)
            return false;
        else
            strs++;
    }
    return true;
}

// full scan version
static inline bool           fs_in(fs s, const char *strs[]){
    while (*strs){
        if (strcmp(s.v, *strs) == 0)
            return true;
        else
            strs++;
    }
    return false;
}

// full scan version, insensitive
static inline bool           fs_iin(fs s, const char *strs[]){
    while (*strs){
        if (strcasecmp(s.v, *strs) == 0)
            return true;
        else
            strs++;
    }
    return false;
}

// TODO: ordered version, via binsearch

#define                      get(s, pos) *fs_get(&(s), (pos) )
#define                      getv(s) (s.v)
#define                      elem(s, pos) *fs_elem( &(s), (pos) )

#define                      fsetlen(s, poslen) *fs_setlen( (&s), (poslen) )
// NOT sure if need fsend()
#define                      fsend(s, poslen) *fs_setlen( (&s), (poslen) )
#define                      fsclear(s) *fs_clear(&s)

#define                      fsstr(s) fs_str(&(s) )

#define                      fsincrease(s, inc) fs_increase( &(s), (inc) )

#define                      fsshrink(s) fs_shrink(&(s) )

#define                      fscat(t, s) fs_cat(&(t), s)
#define                      fscatstr(t, s) fs_catstr(&(t), s)
#define                      fsrevcatstr(t, s) fs_rev_catstr( &(t), s)

#define                      fscpy(t, s) fs_cpy(&(t), s)
#define                      fscpystr(t, s) fs_cpystr(&(t), s)

#define                      fssprintf(s, fmt, ...) fs_sprintf(&(s), (fmt), ##__VA_ARGS__)

#define                      fssubstr(s, from, to) fs_substr( &(s), from, to)
#define                      fsnewsubstr(s, from, to) fs_newsubstr( &(s), from, to)

#define                      fs_ifnotin(s, ...)  fs_notin ( (s), (const char *[]) { __VA_ARGS__, 0 } )
#define                      fs_ifinotin(s, ...) fs_inotin( (s), (const char *[]) { __VA_ARGS__, 0 } )
#define                      fs_ifin(s, ...)  fs_in ( (s), (const char *[]) { __VA_ARGS__, 0 } )
#define                      fs_ifiin(s, ...) fs_iin( (s), (const char *[]) { __VA_ARGS__, 0 } )

// ------------------------ PRINTERS/CHECKERS --------------------------

extern int                   fs_techfprint(FILE *restrict out, const fs *restrict s, const char *name);
static inline int            fs_techprint(const fs *s, const char *name){
    return fs_techfprint(stdout, s, name);
}

extern int                   fs_fprint(FILE *restrict out, const fs *restrict s, const char *name);
static inline int            fs_print(const fs *restrict s, const char *restrict name){
    return fs_fprint(stdout, s, name);
}

extern int                   fs_fprintlim(FILE *restrict out, const fs *restrict s, int lim, const char *name);
static inline int            fs_printlim(const fs *restrict s, int lim, const char *restrict name){
    return fs_fprintlim(stdout, s, lim, name);
}

extern int                   fs_fprint_arr(FILE *restrict out, const fs *restrict arrs[]);
static inline int            fs_print_arr(const fs *restrict arrs[]){
    return fs_fprint_arr(stdout, arrs);
}


#define                      fstechfprint(out, s) fs_techfprint( (out), &(s), #s)
#define                      fstechprint(s)       fs_techprint( &(s), #s)

#define                      fsfprint(out, s) fs_fprint( (out), &(s), #s)
#define                      fsprint(s)       fs_print( &(s), #s)

#define                      fsfprintlim(out, s, lim) fs_fprintlim( (out), &(s), (lim), #s)
#define                      fsprintlim(s, lim)       fs_printlim( &(s), lim, #s)

#define                      fsfprint_arr(out, ...) fs_fprint_arr(out, (const fs *[]) { __VA_ARGS__, 0} )
#define                      fsprint_arr(...)       fs_print_arr( (const fs *[]) { __VA_ARGS__, 0} )

extern bool                  fs_validate(FILE *restrict out, const fs *restrict s);

extern bool                  fs_free_alloc_checker(int *freecnt, int *alloccnt);
extern bool                  fs_alloc_check(bool raise);

// --------------------------------- SERIALIZATION -----------------------------------------

// seqialization (strictly FULL save into the steam with only FS and .len info), out must be opened for write
extern int                   fs_fsave(FILE *restrict out, const fs *restrict str);
extern int                   fs_save(const char *restrict fname, const fs *restrict str);

//  arr must be a pointer to NULL terminated array!
extern int                   fs_fsave_arr(FILE *restrict out, const fs *restrict arr);
extern int                   fs_save_arr(const char *restrict fname, const fs *restrict arr);

extern fs                    fs_fload(FILE *restrict in, fs *restrict s);
extern fs                    fs_load(const char *restrict fname, fs *restrict s);
// NOTE: load_arr is part of fsarr functionnality

#define                      fsfsave(f, str)        fs_fsave((f), &(str))
#define                      fssave(fname, str)     fs_save((fname), &(str))

#define                      fsfsave_arr(out, ...)  fs_fsave_arr( (out), (const fs *[]) { __VA_ARGS__, 0} )
#define                      fssave_arr(fname, ...) fs_save_arr( (fname), (const fs *[]) { __VA_ARGS__, 0} )

#define                      fsfload(f, s)          fs_fload(f, (&s) )
#define                      fsload(fname, s)       fs_load(fname, &(s) )

// ------------------------------------ ETC. ------------------------------------------------

extern int                   FS_MIN_ACCOC;          // to config.c (via sqlite)
extern int                   FS_TECH_PRINT_COUNT;   // to config.c TODO:

#endif /* !_FS_H */

