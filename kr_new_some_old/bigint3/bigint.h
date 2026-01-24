#ifndef _BIGINT_H
#define _BIGINT_H

#include <stdio.h>
#include <bool.h>

static const unsigned					BI_F_SIGN	 			= 0x1;
static const unsigned					BI_F_INVALID 			= 0x2;
static const int						BI_MAX_DIGITS_IN_LING 	= 25;
// static const unsigned BI_F_FREE=0x4;


typedef struct {
	unsigned	 flags;
	int			 sz;
	int			 len;
	char		*val;
} Bigint;

// contructors

Bigint						*bi_create_10(int);     	// 10^power
Bigint 						*bi_create(const char *);
Bigint 						*bi_clone(Bigint *);

static inline 	Bigint		*bi_create_l(long l){
	char	buf[BI_MAX_DIGITS_IN_LING];
	sprintf(buf, "%ld", l);
	return bi_create(buf);
}

static inline   Bigint		*bi_create_zero(void){
	return bi_create("0");
}

static inline	Bigint		*bi_create_one(void){
	return bi_create("1");
}

static inline bool			bi_isnegative(Bigint *v){
	return v->flags & BI_F_SIGN;
}

Bigint                      *bi_move(Bigint *, Bigint *);
Bigint                      *bi_copy(Bigint *, Bigint *);

// destructor

int 						bi_free(Bigint *);
int 						bi_clear(void);

// comparators

int							bi_cmp(Bigint *, Bigint *);
bool						bi_is_zero(Bigint *);

// printers

// пока что пусть один
const char  				*bi_to_str(Bigint *, char *, int);
bool                        bi_getlong(Bigint *, long *);
// technical , dump view
int							bi_fdump(FILE *restrict, const Bigint *restrict);
int 						bi_fdump_all(FILE *restrict, bool);

static inline int           bi_dump(Bigint *v){
    return bi_fdump(stdout, v);
}

static inline int           bi_dump_all(bool print_null){
    return bi_fdump_all(stdout, print_null);
}

// operators
Bigint 						*bi_add(Bigint *, Bigint *);  // create new with v1+v2
Bigint						*bi_sub(Bigint *, Bigint *);
Bigint 						*bi_mul(Bigint *, Bigint *);
Bigint 						*bi_div(Bigint *, Bigint *);
Bigint                      *bi_mod(Bigint *, Bigint *);

// simplifier op's
Bigint 						*bi_inc(Bigint *);
Bigint						*bi_dec(Bigint *);

// self - operator
Bigint						*bi_se_negative(Bigint *);
// fast selt mult to 10^power
Bigint 						*bi_se_fastmul(Bigint *, int);
Bigint						*bi_se_fastdiv(Bigint *, int);



#endif
