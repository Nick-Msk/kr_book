#ifndef ITER_H
#define ITER_H

// ---------------------------------------------------------------------------------
// --------------------------- Public Iterator API ---------------------------------
// ---------------------------------------------------------------------------------

// --------------- Includes -----------------------

#include <bool.h>
#include <string.h>
#include <log.h>

// ----------- CONSTANTS AND GLOBALS ---------------

// ------------------- TYPES -----------------------

typedef enum ITER {ITER_INIT, ITER_NEXT, ITER_CHECK} ITER;

static inline const char * iter_action_tostr(ITER act){
	switch(act){
		case ITER_INIT:  return "ITER_INIT";
		case ITER_NEXT:  return "ITER_NEXT";
		case ITER_CHECK: return "ITER_CHECK";
		default:		 return "Unknown action";
	}
}

struct iter;

typedef bool (*iter_func)(struct iter *, ITER);

typedef struct iter {
	int				from;
	int				to;
	int				step;
	// another attributes can be here. Now is 'static', but can be dynamic, I think
	int				intcurr;	// iteration point for int
	// long			longcurr;
	// double		doublecurr;
	iter_func		f;			// main iterator function
} iter;

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

//
static inline void			iter_init_curr(iter *i){
	logsimple("%p", i);		// TODO: better smtg like iter_to_str(i)
	i->f(i, ITER_INIT);		// setup intcurr to start position
}

// -------------- ACCESS AND MODIFICATION ----------

// no interaction with (f), just return counter, [type depend]
static inline int			iter_getint(const iter *i){
	return i->intcurr;
}

// checker, interacion with (f) [no type depend]
static inline bool			iter_hasnext(iter *i){
	return logsimpleret(i->f(i, ITER_CHECK), "hasnext %p", i);
}

// go to next element, interacion with (f), [no type depend]
static inline bool			iter_next(iter *i){
	return logsimpleret(i->f(i, ITER_NEXT), "next %p", i);
}

// simple usage, get value and switch to the next, interacion with (f) [type depend]
static inline int			iter_getnextint(iter *i){
	int		tmp = iter_getint(i);
	iter_next(i);
	return logsimpleret(tmp, "ret = %d", tmp);
}

// very simple usage, interacion with (f), [no type depend]
static inline bool			iter_simpleint(iter *restrict i, int *restrict val){
	if (iter_hasnext(i)){
		int tmp = iter_getnextint(i);
		if (val)
			*val = tmp;
		return logsimpleret(true, "curr %d", tmp);
	} else
		return logsimpleret(false, "out of range");
}

// ----------------- PRINTERS/CHECKERS ----------------------

extern int					iter_techfprint(FILE *restrict out, const iter *restrict i);

static inline int			iter_techprint(const iter *i){
	return iter_techfprint(stdout, i);
}

// if out == NULL, then just return result w/o printing details
extern bool					iter_validate(FILE *restrict out, const iter *restrict i);

// --------------------------- ETC. -------------------------

// common initializer
#define						iterinit(...)({\
		iter _tmp = (iter) {.step = 1, __VA_ARGS__};\
		iter_init_curr(&_tmp);\
		_tmp;\
	})

// particular
#define						iterinitinterval(_from, _to) 	iterinit(.from = (_from), .to = (_to), .f = iter_interval)
#define						itertechfprint(out, i)		iter_techfprint(out, &(i))
#define						itertechprint(i) 			itertechfprint(stdout, i)
#define 					itervalidate(out, i)		iter_validate((out), &(i))
#define						itergetnextint(i)			iter_getnextint(&(i))
#define						itersimpleint(i, val)		iter_simpleint(&(i), (val))
#define						iterhasnext(i)				iter_hasnext(&(i))
#define						itergetint(i)				iter_getint(&(i))
#define						iternext(i)					iter_next(&(i))

// ------------------------ ITERATORS -----------------------
//  NOTE: better to be in another file iter_eng.h/c, but currently it's here

extern bool					iter_interval(struct iter *i, ITER action);

#endif /* !ITER_H */


