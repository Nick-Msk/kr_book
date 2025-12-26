#ifndef TMAP_H
#define TMAP_H
// ---------------------------------------------------------------------------------
// --------------------------- Public Tmap API -------------------------------------
// ---------------------------------------------------------------------------------

#include <stdio.h>
#include <log.h>

// ----------- CONSTANTS AND GLOBALS ---------------

// ------------------- TYPES -----------------------

typedef struct {
	long	val;
	int		cnt;
} tmapdata;

typedef struct tmap{
	tmapdata	d;
	struct tmap		*left, *right;
} tmap;

// init, not sure how to do it
#define tmapdinit(cnt, val) (tmapdata) {.cnt = (cnt), .val = (val)}

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

void			 	 tmap_free(tmap *v);

tmap				*tmap_copy(const tmap *v);

// -------------- ACCESS AND MODIFICATION ----------

tmap				*tmap_putval(tmap *v, long val);

// get sum of all counter in the tree
int					 tmap_gettotal(const tmap *v);

// get count of nodes
int					 tmap_getnodecnt(const tmap *v);

// gen node of element, which contains val
tmap				*tmap_getnode(const tmap *v, long val);

// get counter of node val
static inline int    tmap_getcnt(const tmap *v, long val){
	tmap 	*map = tmap_getnode(v, val);
	int		 res = map ? map->d.cnt : 0;
	return logautoret(res);		// logautoret(res) ??
}

// ----------------- PRINTERS ----------------------

int                  tmap_fprintall(FILE *restrict out, const tmap *restrict v, bool printdelta);

static inline int    tmap_printall(const tmap *restrict v, bool printdelta){
    return tmap_fprintall(stdout, v, printdelta);
}

int             	 tmap_fprintcsv(FILE *restrict out, const tmap *restrict v);

// ------------------ ETC. -------------------------

#endif /* !TMAP_H */

