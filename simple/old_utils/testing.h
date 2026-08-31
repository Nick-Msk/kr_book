#ifndef _TESTING_H
#define _TESTING_H

#include <stdio.h>
#include "bool.h"
#include "log.h"
#include "error.h"

static const int 		TEST_MAX_DEP		= 5;		// max count of dependencies
static const int		TEST_MAX_NAME		= 40;
static const int		TEST_MAX_DESC		= 255;
static const int		TEST_PATTERN_MAX_SZ = 4096;		// maximusm size of pattern for comparator

typedef enum
{ 	  TEST_NOT_RUN = 0			// initial, test isn't runned yet
	, TEST_PASSED				// test is runned and ok
	, TEST_SKIPPED				// test is NOT runned because of some prereq is failed
	, TEST_FAILED				// test is runned but without success
	, TEST_MANUAL				// test is runned and PROBANLY ok, but manual log checking is required (for ex. for print some data)
	, TEST_RUNNING 				// test is in running now...
	, TEST_FAILED_EXCEPTION		// failed with expection (filled by launcher)
} TestStatus;


// summary info about tests run
typedef struct TestRes
{
	bool	passed;				// status of particular run
	int		pass_total;			// count of passed tests
	int		err_total;			// count of failed tests
	int		exception_total;	// count of failed with exception (<= err_total)
	int		skip_total;			// count of skipped tests
	int		total;				// number of runs
	//int		sub_err_total;	// count of failed subtest, reserved for future using
} TestRes;

typedef struct TFILE
{
	FILE	*f;				// TODO: think if it is possible to pass FILE here directly
	char 	*name;			// dynamically
} TFILE;

// active
typedef TestStatus (*t_test2)(void);

typedef bool (*Tcomp)(const char *restrict, int, const char *restrict);

#define testnew(...) {.status = TEST_NOT_RUN, .count = 1, .is_pub = false,   __VA_ARGS__ }  // .num = 0 TODO: remove obsolete passed! TODO: try to get rif of warning about dowble init

#define testdep(...) .dep_list = { __VA_ARGS__ }

#define testengine(out, ...) test_engine2((Utest []) { __VA_ARGS__ , testnew(.f2 = 0) }, out).passed

#define testenginestd(...) testengine(stdout, __VA_ARGS__)

typedef struct
            {
				int         num;                	// logical number of test, '0' is not allowed here!
				char        name[TEST_MAX_NAME];    // name of test
				char 		desc[TEST_MAX_DESC];	// description
				bool        mandatory;          	// mandatory flag
				int         dep_list[TEST_MAX_DEP];	// dep list
				// ------
				int			count;					// if == 0 then test is NOT active, if > 1 then one tests repeats count time or until failed
				bool		is_pub;					// if true, then this test is public an dMUST use only public API. Suggested function name is tf<x>_pub
				t_test2		f2;						// pointer to testing function
                TestStatus  status;					// out: status of test
            } Utest;

//-------------------------------------- PUBLIC API ------------------------------------------------

static inline FILE *
tfile(TFILE tf)
{
	return tf.f;
}

// engine function
extern TestRes
test_engine2(Utest *, FILE *);

// subtest, close previous one if it was
extern void
test_sub(const char *msg, ...) __attribute__ ((format (printf, 1, 2)));

// open temporary test file
extern TFILE
test_fopen(void);

// close and remove temporary test file
extern bool
test_fclose(TFILE tf);

// Compare functions (equal, equal_ne, like, ulike, regexp_like)
extern bool
test_str_equal(const char *source, int sz, const char *pattern);

extern bool
test_str_equal_ne(const char *source, int sz, const char *pattern);

extern bool
test_str_like(const char *source, int sz, const char *pattern);

extern bool
test_str_ulike(const char *source, int sz, const char *pattern);

// basic comparator, comp - compare function
extern bool
test_compare_engine(FILE *restrict tf, long from, long to, const char *restrict pt, Tcomp comp);

// file comparators
static inline bool
test_file_equal(FILE *restrict tf, long from, long to, const char *restrict pt){
	return test_compare_engine(tf, from, to, pt, test_str_equal);
}

static inline bool
test_file_equal_ne(FILE *restrict tf, long from, long to, const char *restrict pt){
	 return test_compare_engine(tf, from, to, pt, test_str_equal_ne);
}

static inline bool
test_file_like(FILE *restrict tf, long from, long to, const char *restrict pt){
	return test_compare_engine(tf, from, to, pt, test_str_like);
}

static inline bool
test_file_ulike(FILE *restrict tf, long from, long to, const char *restrict pt){
	return test_compare_engine(tf, from, to, pt, test_str_ulike);
}

static inline long
test_ftell(FILE *tf)
{
	long	res;
	if ( (res = ftell(tf)) == -1)
        return sysraiseint("Unable to get position info");
	return res;
}

// simple wrappers for whole file comparators
static inline bool
test_equal(FILE *restrict tf, const char *restrict pt)
{
	return test_file_equal(tf, 0L, 0L, pt);
}

static inline bool
test_equal_ne(FILE *restrict tf, const char *restrict pt)
{
	return test_file_equal_ne(tf, 0L, 0L, pt);
}

static inline bool
test_like(FILE *restrict tf, const char *restrict pt)
{
	return test_file_like(tf, 0L, 0L, pt);
}

static inline bool
test_ulike(FILE *restrict tf, const char *restrict pt)
{
	return test_file_ulike(tf, 0L, 0L, pt);
}

// actiojn checkers
#define test_act_basic(comp, ACTION, tf, pt)	({	long _test_from = test_ftell(tf);\
													{ ACTION; }\
													long _test_to = test_ftell(tf);\
													test_file_##comp(tf, _test_from, _test_to, pt);\
												})

#define test_act_equal(ACTION, tf, pt)		test_act_basic(equal	, ACTION, tf, pt)

#define test_act_equal_ne(ACTION, tf, pt)	test_act_basic(equal_ne	, ACTION, tf, pt)

#define test_act_like(ACTION, tf, pt)		test_act_basic(like		, ACTION, tf, pt)

#define test_act_ulike(ACTION, tf, pt)		test_act_basic(ulike	, ACTION, tf, pt)

#endif /* !_TESTING_H */

