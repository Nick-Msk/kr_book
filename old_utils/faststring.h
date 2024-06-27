#ifndef FASTSTRING_H
#define FASTSTRING_H

// ---------------------------------------------------------------------------------
// --------------------------- Public Faststring API -------------------------------
// ---------------------------------------------------------------------------------

// --------------- Includes -----------------------

#include <bool.h>
#include <string.h>
#include <log.h>
#include <common.h>
#include <error.h>

// ----------- CONSTANTS AND GLOBALS ---------------

// ------------------- TYPES -----------------------

typedef enum ITER {FS_FLAG_STATIC = 0x1
				 , FS_FLAG_CONST  = 0x2		// Not user for now
				 , FS_FLAG_LOCAL  = 0x4
} FS_FLAGS;

// type-support functions

// to string
static inline const char * fs_flag_str(FS_FLAGS flag){
	switch(flag){
		CASE_RETURN(FS_FLAG_STATIC);		// not sure whether is good to use CASE_RETURN
		CASE_RETURN(FS_FLAG_CONST);
		CASE_RETURN(FS_FLAG_LOCAL);
		default:		 return "Unknown action";
	}
}

// faststring
typedef struct fs {
    int         len, sz;
    FS_FLAGS    flags;
    char       *s;
} fs;

// flag checkers
static inline bool fs_flag_static(FS_FLAGS fl){
	return fl & FS_FLAG_STATIC;
}

static inline bool fs_statis(const fs *s){
	return fs_flag_static(s->flags);
}

static inline bool fs_flag_const(FS_FLAGS fl){
    return fl & FS_FLAG_CONST;
}

static inline bool fs_const(const fs *s){
	return fs_flag_const(s->flags);
}

static inline bool fs_flag_local(FS_FLAGS fl){
	return fl & FS_FLAG_LOCAL;
}

static inline bool fs_local(const fs *s){
	return fs_flag_local(s->flags);
}

static inline bool fs_flag_heap(FS_FLAGS fl){
	return fl == 0;	// heap marker
}

static inline bool fs_heap(const fs*s){
	return fs_flag_heap(s->flags);
}

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

extern const char          *fs_tostr(const fs *s);

// literal - static

// returns faststing pointed to the static string str. MUST be unchanged, must be not null
static inline fs			literal(const char *str){
	int		len = strlen(str);
	fs res = (fs){.len = len, .sz = len + 1, .flags = FS_FLAG_STATIC, .s = (char *)str};
	return logsimpleret(res, "Fs literal is created (len = %d)", len);
}

// dynamic - heap

extern fs					initn(int n);

static inline fs            init(const fs *s){
    fs      res = initn(s->len);
    memcpy(res.s, s->s, s->len);
    return  logsimpleret(res, "created: %s", fs_tostr(&res));
}

static inline fs			initstr(const char *str){
	int		n = strlen(str);		// no care about s == NULL
	fs		res = init( &((fs){.len = n, .sz = n + 1, .s = (char *)str, .flags = FS_FLAG_STATIC} )); // !! literal here?
	return  logsimpleret(res, "created: %s", fs_tostr(&res));
}

// empty, but not literal, dynamic
static inline fs			empty(){
	return literal("");	// empty faststring is just static literal, but then, increatesize will convert it into heap type
}

// destructor
static inline void			fs_free(fs *s){
	if (fs_heap(s))
		logsimpleact(free(s->s), "freed...");
	s->sz = s->len = s->flags = 0;
	s->s = 0;
}

// -------------- ACCESS AND MODIFICATION ----------

// direct access, NO change len or sz, position MUST be < sz
static inline char			*fs_get(const fs *s, int pos){
	return logsimpleret(s->s + pos, "Getting %p[%c]", s->s + pos, s->s[pos]);
}

// automatically adjust len (??) and sz (realloc)
extern char					*fs_elem(fs *s, int pos);

// shrink to real len + 1 ( + 1 because '\0' is ASSUMED)
extern fs					*fs_shrink(fs *s);

// move block TODO: !!!
extern fs					*fs_move(fs *restrict dst, fs *restrict src);

// convert c-string to fs (replace), dst->s became c-string too
static inline fs			*fs_movest(fs *restrict dst, const char *restrict src){
	if (!src)
		return logsimpleret(dst, "No changes, nullable src c-string");
	else {
		fs		tmp = initstr(src);
		return logsimpleret(fs_move(dst, initstr(src)), "...");
	}
		// TODO: move to fs_move it!
		if (len > dst->sz)
			increasesize(dst, len);
		memcpy(dst->s, src, len);
		return logsimpleret(dst, "moved, dst [%s]", fs_tostr(dst));

}

static inline fs			*fs_movec(fs *dst, char c){
	char	tmp[2] = {c, '\0'};
	return fs_movestr(tmp);
}



// ----------------- PRINTERS/CHECKERS ----------------------

int						fs_techfprint(FILE *restrict out, const fs *restrict s);
static inline int		fs_techprint(const fs *restrict s){
	return fs_techfprint(stdout, s);
}

// --------------------------- ETC. -------------------------

// common initializer
#define EMPTY (fs){.sz = 1, .len = 0, .flags = FS_FLAG_STATIC, .sz = ""};

// particular

// ------------------------ ITERATORS -----------------------
// TODO: this, no iterators for now

#endif /* !FASTSTRING_H */


