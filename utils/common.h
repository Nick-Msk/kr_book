#ifndef _COMMON_H
#define _COMMON_H

#include <stdlib.h>
#include <string.h>

/***************************************************************
				USEFUL MACRO AND FUNCTIONS
***************************************************************/

static const int    G_GLOB_AVERAGE = INT_MAX;

// universale comparator (for simple type cast in qsort)
typedef int(*Comparator)(const void *, const void *);

#define 			COUNT(arr) (int)(sizeof arr/sizeof(typeof(*arr)) )

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

// random from 0 till max
static inline int
rndint(int max)
{
    return (long)rand() * max / RAND_MAX;
}


// random from 0 till dmax
static inline double
rnddbl(double dmax)
{
    return (long)rand() * dmax / RAND_MAX; // TODO: check if improve
}

// simple char comparator
static inline int               char_cmp(const void *s1, const void *s2){
    const char *c1 = s1;
    const char *c2 = s2;
    return *c1 - *c2;
}

// simple comparator pointer int
static inline int               pint_cmp(const void *i1, const void *i2){
    return *(const int *) i1 - *(const int *) i2;
}

// simple reverse comparator pointer int
static inline int               pint_revcmp(const void *i1, const void *i2){
    return *(const int *) i2 - *(const int *) i1;
}

// simple comparator pointer double
extern int                      pdbl_cmp(const void *d1, const void *d2);

// simple reverse comparator pointer double
extern int                      pdbl_revcmp(const void *d1, const void *d2);

// (void *) comparator
static inline int               pointer_cmp(const void *p1, const void *p2){
    return *(const void **) p1 - *(const void **) p2;   // (void *) - (void *)!
}

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

// simple double exchanger  TODO: think about generic exchanger
static inline void              dbl_exch(double *d1, double *d2){
    double tmp = *d1;
    *d1 = *d2;
    *d2 = tmp;
}

// str exch
static inline void              str_exch(const char **s1, const char **s2){
    const char *tmp = *s1;
    *s1 = *s2;
    *s2 = tmp;
}

static inline char              ctoi(char c){
    return c - '0';
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

//  binary char search
static inline char              *bcharsearch(char c, const char *pt, int len){
    return  bsearch(&c, pt, len, 1, char_cmp /* from common.h */);
}

static inline char              *sort_str(char *s, int len){
    qsort(s, len, 1, char_cmp);
    return s;
}

// make s unique by symbols (no odering!!!)
extern char                     *uniq_str(char *s, int *p_len);

// fill with 0.0 cnt elements
extern void                     cleaner_double(void *arr, int cnt);

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

static inline int               round_up_2(int val){
    int prev = 0;
    while (val)
        val &= ( (prev = val) - 1);
    return prev <<= 1;
}

// reverse string
extern char                     *reverse(char *s, int len);

static inline char              *reversel(char *s){
    int     len = strlen(s);
    return reverse(s, len);
}

#endif /* ! _COMMON_H */
