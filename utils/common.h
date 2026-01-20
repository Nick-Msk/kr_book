#ifndef _COMMON_H
#define _COMMON_H

#include <stdlib.h>
#include <string.h>

/***************************************************************
				USEFUL MACRO AND FUNCTIONS
***************************************************************/

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


// simple comparator
static inline int        char_cmp(const void *s1, const void *s2){
    const char *c1 = s1;
    const char *c2 = s2;
    return *c1 - *c2;
}

// simple char exhanger
static inline void      char_exch(char *s1, char *s2){
    char c = *s1;
    *s1 = *s2;
    *s2 = c;
}

// simple int iexchanger
static inline void      int_exch(int *i1, int *i2){
    int tmp = *i1;
    *i1 = *i2;
    *i2 = tmp;
}

static inline char      itoc(int c){
    return c + '0';
}

//  binary char search
static inline char              *bcharsearch(char c, const char *pt, int len){
    return  bsearch(&c, pt, len, 1, char_cmp /* from common.h */);
}

static inline char              *sort_str(char *s, int len){
    qsort(s, len, 1, char_cmp);
    return s;
}

// simpe file reader, must call free(s); after usage!!
extern char                     *read_from_file(FILE *f, int *p_cnt);

// make s unique by symbols (no odering!!!)
extern char                     *uniq_str(char *s, int *p_len);

// fill with 0.0 cnt elements
extern void                     cleaner_double(void *arr, int cnt);

// for now in common.c, then will be moved out
extern int                      get_line(char *line, int lim);

// print int as bits
extern int                      fprint_bits(FILE *f, const char *str, unsigned val);

// bits to string (STATIC for now)
extern const char              *bits_str(char *buf, int len, unsigned val);

// print int as bits
static inline int               print_bits(const char *str, unsigned  val){
    return fprint_bits(stdout, str, val);
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

#endif /* ! _COMMON_H */
