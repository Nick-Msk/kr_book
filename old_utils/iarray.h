#ifndef IARRAY_H
#define IARRAY_H

// ---------------------------------------------------------------------------------
// ---------------------- Public Integer array API ---------------------------------
// ---------------------------------------------------------------------------------

// --------------- Includes -----------------------

#include <bool.h>
#include <string.h>
#include <log.h>

// ----------- CONSTANTS AND GLOBALS ---------------

// ------------------- TYPES -----------------------

typedef struct {
	int				*arr;
	int				 sz, cnt;
} iarray;

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

iarray                       ia_increasesize(iarray *data, int newcnt);

static inline iarray		 ia_null(void){
	return (iarray) {.arr = 0, .sz = 0, .cnt = 0};
}

// create iarray with cnt elements (sz >= cnt)
static inline iarray		 iacreate(int cnt){
	iarray	res = ia_increasesize(0, cnt);
	return logsimpleret(res, "Created [%d : %d]", res.cnt, res.sz);
}

//
static inline iarray		 iacopy(iarray data){
	iarray	res = iacreate(data.cnt);
	memcpy(res.arr, data.arr, data.cnt * sizeof(sizeof(int)));
	return logsimpleret(res, "copy is created (%d)", res.cnt);
}

void                         ia_free(iarray *data);

// -------------- ACCESS AND MODIFICATION ----------

// get element, create if not exists
int							*ia_elem(iarray *data, int pos);

//   get elemen, must exists
static inline int			*ia_get(iarray *data, int pos){
	return data->arr + pos;
}

// fill array with zero
static inline void			 iafillzero(iarray data){
	logsimple("till %d (%p)", data.cnt, data.arr);
	memset(data.arr, '\0', data.cnt * sizeof(int));
}

iarray						*ia_concat(iarray *restrict dest, iarray *restrict src);

// check if contains value
static inline bool			 iavaluable(iarray data){
	return data.arr != 0;
}

// ----------------- PRINTERS ----------------------

static inline int			iafprintinfo(FILE *out, iarray data){
	return fprintf(out, "%p [%d : %d]\n", data.arr, data.cnt, data.sz);
}

static inline int			iaprintinfo(iarray data){
	return iafprintinfo(stdout, data);
}

// declaration from .c file
int                          iafprintf_all(FILE *out, iarray data);

static inline int            iaprintf_all(iarray data){
	return iafprintf_all(stdout, data);
}

// ------------------ ETC. -------------------------

#define						iafree(ar)			 ia_free(&(ar))
#define						iaelem(ar, pos)		*ia_elem(&(ar), pos)
#define 					iaget(ar, pos)		*ia_get(&(ar), pos)
#define						iaconcat(dst, src)  *ia_concat(&(dst), &(src))
#define						ianull				 ia_null()

#endif /* !IARRAY_H */


