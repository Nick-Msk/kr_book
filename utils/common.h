#ifndef _COMMON_H
#define _COMMON_H

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#include "bool.h"
#include "log.h"

/***************************************************************
				USEFUL MACRO AND FUNCTIONS
***************************************************************/

static const int    G_GLOB_AVERAGE = INT_MAX;

// universale comparator (for simple type cast in qsort)
typedef int(*Comparator)(const void *, const void *);

//#define 			COUNT(arr) (int)(sizeof arr/sizeof(typeof(*arr)) )
#define             COUNT(arr) (int)(sizeof(arr) / sizeof((arr)[0]))

static inline int               countstrings(const char * const *p){
    const char * const *t = p;
    while (*t)
        t++;
    return t - p;
}

static inline const char        *skip_leading_spaces(const char *str) {
    while (*str == ' ' || *str == '\t')
        str++;
    return str;
}

static const char 	NULLSTR[] = "(null)";

#define MIN(x, y)\
        ({ typeof (x) _x = (x); \
           typeof (y) _y = (y); \
       _x < _y? _x : _y; } )

#define MAX(x, y)\
        ({ typeof (x) _x = (x); \
           typeof (y) _y = (y); \
       _x > _y? _x : _y; } )

// TODO:
#define LEAST(a, ...) ({ typeof(a) _ARR[] = {a, ##__VA_ARGS__};\
	   				     typeof(a) _MIN = *_ARR;\
						 for (int i = 1; i < COUNT(_ARR); i++)\
							 if (_ARR[i] < _MIN)\
								 _MIN = _ARR[i];\
						 _MIN;\
						     })

#define GREATEST(a, ...) ({ typeof(a) _ARR[] = {a, ##__VA_ARGS__};\
                            typeof(a) _MAX = *_ARR;\
                        	for (int i = 1; i < COUNT(_ARR); i++)\
                            	if (_ARR[i] > _MAX)\
                                	_MAX = _ARR[i];\
                        	_MAX;\
                          })

#define STRING(x) #x

// check if compatible
#define IS_COMPATIBLE(x, T) _Generic((x), T: 1, default 0)

#define FREE(x) { free(x); (x) = 0; }

#ifndef NDEBUG

	#include <stdio.h>

	// a big qwestion abount printing float
	#define TYPEFORMAT(x) _Generic((x), \
    	char					: "%c", \
    	signed char				: "%hhd", \
    	unsigned char			: "%hhu", \
    	signed short			: "%hd", \
    	unsigned short			: "%hu", \
    	signed int				: "%d", \
    	unsigned int			: "%u", \
    	long int				: "%ld", \
    	unsigned long int		: "%lu", \
    	long long int			: "%lld", \
    	unsigned long long int	: "%llu", \
    	float					: "%f", \
    	double					: "%f", \
    	long double				: "%Lf", \
    	char *					: "%s", \
		const char *			: "%s",\
    	void *					: "%p",\
		const void *			: "%p"\
	)

	#define typeprint(a) { printf(#a " = ");  printf(TYPEFORMAT(a), a); putchar('\n'); }

#else /* !NDEBUG */
	#define typeprint(a)
#endif /* NDEBUG */

#define				DUMMY

#define				CASE_RETURN(x) case x:  return #x

// -------------------------------------- Random ------------------------------

static inline void
rndinit(void){
    #if defined(__unix__) || defined(__APPLE__)
        srand48(time(0));
    #endif
    srand(time(0));
}

// TODO: refactor here!
// random from 0 till max
static inline int
rndint(int max)
{
    return (long)rand() * max / RAND_MAX;
}
// probably rework is required
static inline unsigned
rnduint(unsigned max)
{
    return (unsigned long)rand() * max / RAND_MAX;
}
static inline long
rndlong(unsigned max)
{
    return (long)rand() * max / RAND_MAX;
}
// probably rework is required
static inline unsigned long
rndulong(unsigned long max)
{
    return (unsigned long)rand() * max / RAND_MAX;
}

static inline double
drand(void)
{
    #if defined(__unix__) || defined(__APPLE__)
        return drand48();
    #else
        return (double) rand() / (RAND_MAX + 1.0);
    #endif
}

// random from 0 till dmax
static inline double
rnddbl(double dmax)
{
    return drand() * dmax;
}
// ----------------------------------- Comparators ----------------------------

static inline int               compare_char(unsigned char v1, unsigned char v2){
    if (v1 > v2)
        return 1;
    else if (v1 < v2)
        return -1;
    else
        return 0;
}
static inline int               compare_int(int v1, int v2){
    if (v1 > v2)
        return 1;
    else if (v1 < v2)
        return -1;
    else
        return 0;
}
static inline int               compare_uint(unsigned v1, unsigned v2){
    if (v1 > v2)
        return 1;
    else if (v1 < v2)
        return -1;
    else
        return 0;
}
static inline int               compare_long(long v1, long v2){
    if (v1 > v2)
        return 1;
    else if (v1 < v2)
        return -1;
    else
        return 0;
}
static inline int               compare_ulong(unsigned long v1, unsigned long v2){
    if (v1 > v2)
        return 1;
    else if (v1 < v2)
        return -1;
    else
        return 0;
}
static inline int               compare_dbl(double v1, double v2){
    if (isnan(v1) && isnan(v2))
        return 0;
    if (isnan(v1))
        return -1;
    if (isnan(v2))
        return 1;
    if (v1 > v2)
        return 1;
    if (v1 < v2)
        return -1;
    return 0; // равны (включая случай +0.0 и -0.0)
}
static inline int               compare_ptr(const void *restrict v1, const void *restrict v2){
    uintptr_t a = (uintptr_t)v1;
    uintptr_t b = (uintptr_t)v2;
    if (a > b)
        return 1;
    else if (a < b)
        return -1;
    else
        return 0;
}

// ------------------- Pointer -------------------
// simple char comparator
static inline int               pchar_cmp(const void *restrict s1, const void *restrict s2){
    return compare_char( *(const unsigned char *) s1, *(const unsigned char *) s2);
}
// simple char reverse comparator
static inline int               pchar_revcmp(const void *s1, const void *s2){
    return -pchar_cmp(s1, s2);
}
// simple comparator pointer int
static inline int               pint_cmp(const void *restrict i1, const void *restrict i2){
    return compare_int( *(const int *) i1, *(const int *) i2);
}
// simple reverse comparator pointer int
static inline int               pint_revcmp(const void *i1, const void *i2){
    return -pint_cmp(i1, i2);
}
// simple comparator pointer long
static inline int               plong_cmp(const void *l1, const void *l2){
    return compare_long( *(const int *) l1, *(const int *) l2);
}
// simple reverse comparator pointer long
static inline int               plong_revcmp(const void *l1, const void *l2){
    return -plong_cmp(l1, l2);
}
// simple comparator pointer to pointer TODO: remove???
static inline int               pptr_cmp(const void *p1, const void *p2){
    return compare_ptr(p1, p2);
}
// simple reverse comparator pointer to pointer
static inline int               pptr_revcmp(const void *p1, const void *p2){
    return -pptr_cmp(p1, p2);
}
// simple comparator pointer double
extern int                      pdbl_cmp(const void *d1, const void *d2);
// simple reverse comparator pointer double
extern int                      pdbl_revcmp(const void *d1, const void *d2);

// (void *) comparator
//static inline int               pointer_cmp(const void *p1, const void *p2){
//    return *(const void **) p1 - *(const void **) p2;   // (void *) - (void *)!
//}

// ----------------------------------- Exchangers ----------------------------------

// simple char exhanger
static inline void              char_exch(char *s1, char *s2){
    char c = *s1;
    *s1 = *s2;
    *s2 = c;
}
// simple int exchanger
static inline void              int_exch(int *i1, int *i2){
    int tmp = *i1;
    *i1 = *i2;
    *i2 = tmp;
}
// simple long exchanger
static inline void              long_exch(long *i1, long *i2){
    long tmp = *i1;
    *i1 = *i2;
    *i2 = tmp;
}
// simple double exchanger  TODO: think about generic exchanger
static inline void              dbl_exch(double *d1, double *d2){
    double tmp = *d1;
    *d1 = *d2;
    *d2 = tmp;
}

// simple pointer exchanger  TODO: think about generic exchanger
static inline void              ptr_exch(void **v1, void **v2){
    void    *tmp = *v1;
    *v1 = *v2;
    *v2 = tmp;
}
// str exch
static inline void              str_exch(const char **s1, const char **s2){
    const char *tmp = *s1;
    *s1 = *s2;
    *s2 = tmp;
}

// -------------------------------- Converters ---------------------------
static inline int               ctoi(char c){
    return c - '0';
}
static inline int               ctoihex(char c){
    if (isdigit(c) )
        return c - '0';
    else
        return tolower(c) - 'a';        // a - f
}
static inline char              itoc(int c){
    return c + '0';
}
static inline char              itohex(int c){
    if (c < 10)
        return c + '0';
    else
        return c - 10 + 'A';
}
static inline char              itoupper(int c){
    return c + 'A';
}
static inline char              itolower(int c){
    return c + 'a';
}
// ----------------------------------- Utilities -------------------------------------------

//  binary char search
static inline char              *bcharsearch(char c, const char *pt, int len){
    return  bsearch(&c, pt, len, 1, pchar_cmp /* from common.h */);
}

static inline char              *sort_str(char *s, int len, bool asc){
    qsort(s, len, 1, asc ? pchar_cmp : pchar_revcmp);
    return s;
}

// --------------------------------------- Fillers -----------------------------------------

// make s unique by symbols (no odering!!!)
extern char                     *uniq_str(char *s, int *p_len);

// int[] filler
extern void                     fill_int(int *arr, int cnt, int value);

// long[] filler
extern void                     fill_long(long *arr, int cnt, long value);

// double[] filler
extern void                     fill_double(double *arr, int cnt, double value);

// float[] filler
extern void                     fill_float(float *arr, int cnt, float value);

// fill with 0.0 cnt elements
static inline void              clean_double(double *arr, int cnt){
    return fill_double(arr, cnt, 0.0);
}
// fill with 0.0f cnt elements
static inline void              clean_float(float *arr, int cnt){
    return fill_float(arr, cnt, 0.0f);
}
// no filler, only cleaner for pointers!
static inline void              clean_ptr(void **arr, int cnt){
    memset(arr, 0, cnt * sizeof(void *) );
}

// ------------------------------- BITS Operations ------------------------------------------

// print int as bits
extern int                      fprint_bits(FILE *f, const char *str, unsigned val);

// bits to string (STATIC for now)
extern const char              *bits_str(char *buf, int len, unsigned val);

// print int as bits
static inline int               print_bits(const char *str, unsigned  val){
    return fprint_bits(stdout, str, val);
}

// cycle
static inline int               cycleinc(int val, int cycle){
    if (val >= cycle - 1)
        val = 0;
    else
        val++;
    return val;
}
// print n chars to f
extern int                      fprintn(FILE *f, const char *str, int sz);

static inline int               printn(const char *str, int sz){
    return fprintn(stdout, str, sz);
}

// stdc_bit_ceil_ TODO: check
static inline unsigned          round_up_2(unsigned val){
    int prev = 0;
    while (val)
        val &= ( (prev = val) - 1);
    return prev << 1;
}

// reverse string
extern char                     *reverse(char *s, int len);

static inline char              *reversel(char *s){
    int     len = strlen(s);
    return reverse(s, len);
}

// ----------------------------------- CHAR ----------------------------------------
// isalpha or '_'
static inline bool              isalpha_u(int c){
    return isalpha(c) || c == '_';
}

// isalnum or '_'
static inline bool              isalnum_u(int c){
    return isalnum(c) || c == '_';
}

static inline bool              isdigit_signed(int c){
    return isdigit(c) || c == '+' || c == '-';
}

static inline int clower(int c, bool lower){
    return lower ? tolower(c) : c;
}

static inline int cupper(int c, bool upper){
    return upper ? toupper(c) : c;
}

typedef enum {SIZE_NONE = 0, SIZE_POWER2, SIZE_MIN10 } Tincrease;

// should depent on increase strategy, simplest return n + 1;
static inline int               calcnewsize(Tincrease t, int n){
    int sz = n;
    switch (t){
        case SIZE_NONE:
            // do, nothing
        break;
        case SIZE_MIN10:
            if (sz < 10)
                sz = 10;
        break;
        case SIZE_POWER2:
            sz = round_up_2(sz);
        break;
        default:
            logsimple("Unknow size grouth type %d", t);
        break;
    }
    return sz; //logsimpleret(sz, "newsz = %d", sz);
}
// int SQL not in ver2 (with size)
static inline bool              common_int_notin2(int val, const int *arr, int sz){
    const int *iter = arr;
    while (iter - arr < sz)
        if (*iter++ == val)
            return false;
    return true;
}
// int SQL in ver2 (with size)
static inline bool              common_int_in2(int val, const int *arr, int sz){
    const int *iter = arr;
    while (iter - arr < sz)
        if (*iter++ == val)
            return true;
    return false;
}
#define int_in(val, ...)    common_int_in2   ( (val), (const int []) { __VA_ARGS__ }, COUNT(((const int[]){__VA_ARGS__})) )
#define int_notin(val, ...) common_int_notin2( (val), (const int []) { __VA_ARGS__ }, COUNT(((const int[]){__VA_ARGS__})) )

//#define int_notin(val, ...) common_int_notin2( (val), (const int []) { __VA_ARGS__ }, COUNT((const int[]){__VA_ARGS__}) )
//#define int_in(val, ...)    common_int_in2   ( (val), (const int []) { __VA_ARGS__ }, COUNT((const int[]){__VA_ARGS__}) )

// int SQL not in
static inline bool              common_int_notin(int val, const int *arr){
    while (*arr != val)
        if (val == *arr++)
            return false;
    return true;
}
// int SQL in
static inline bool              common_int_in(int val, const int *arr){
    while (*arr != INT_MIN)
        if (val == *arr++)
            return true;
    return false;
}
/*
// TODO: think about typeof
#define                         int_notin(val, ...) common_int_notin( (val),  (const int []) { __VA_ARGS__, INT_MIN} )
#define                         int_in(val, ...)    common_int_in( (val),     (const int []) { __VA_ARGS__, INT_MIN} )
*/

extern bool              try_parse_int(const char *restrict str, int *restrict res);
extern bool              try_parse_long(const char *restrict str, long *restrict res);
extern bool              try_parse_double(const char *restrict str, double *restrict res);
extern bool              try_parse_uint(const char *restrict str, unsigned *restrict res);
extern bool              try_parse_ulong(const char *restrict str, unsigned long *restrict res);

#endif /* ! _COMMON_H */
