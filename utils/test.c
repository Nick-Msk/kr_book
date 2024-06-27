#include "bool.h"
#include "test.h"

#include <stdlib.h>
#include <stdarg.h>
#include <error.h>
#include <log.h>
#include <string.h>
#include <ctype.h>

static const int		G_OFFSET_INC 	= 4;
static 		 int 		g_offset 	 	= 0;		// offset
static		FILE 	   *g_out 			= 0;		// 	out stream, required for subtest TODO: refactor it!!
static 		 bool		g_prev_subtest 	= false;	//  requited for subtest, TODO: refactor it

// #define OFFSETINC(x) (g_offset += G_OFFSET_INC * (x));  // TODO: подумать, может как-то красивей назвать
static inline int
offsetinc(int x)
{
	return g_offset += G_OFFSET_INC * x;
}

// utility

typedef int (*t_f)(const void *, const void *);

static inline int
fprint(FILE *restrict out, const char *restrict msg, ...) __attribute__ ((format (printf, 2, 3)));

// passed checher
static inline bool
is_passed(TestStatus status)
{
	return status == TEST_PASSED || status == TEST_MANUAL;
}

//comparator
static int
cmp(const Utest *u1, const  Utest *u2)
{
	return u1->num - u2->num;
}

// sort tests by num
static void
sort(Utest *arr, int from, int to)
{
	if (from >= to)
		return;

	qsort(arr + from, to - from + 1, sizeof(Utest), (t_f) cmp);
}

// printf with offset and with va list
static int
vfprint(FILE *restrict out, const char *restrict msg, va_list ap)
{
	int		c = 0;
	if (!out)
		out = stdout;
	c += fprintf(out, "%*c", g_offset, '\0');
	c += vfprintf(out, msg, ap);
	return c;
}

// printf if out not null
static inline int
fprint(FILE *restrict out, const char *restrict msg, ...)
{
	int 		c;
	va_list 	ap;
	va_start(ap, msg);
	c = vfprint(out, msg, ap);
	va_end(ap);
	return c;
}

// via macros
#define print(msg, ...)	fprint(stdout, (msg), ##__VA_ARGS__)

// duplicate number checker, tests must be sorted by .num
static bool
check_duplicate_num(Utest *tests, int cnt, int *num)
{
	for (int i = 0; i < cnt - 1; i++)
		if (tests[i].num != 0 && tests[i].num == tests[i + 1].num)		// multiple 0 is allowed
		{
			if (num)
				*num = tests[i].num;
				return false;
		}
	return true;
}

static inline const char *
get_test_result_message(TestStatus st)
{
    switch (st){
        case TEST_PASSED:               return "PASSED";
        case TEST_MANUAL:               return "PASSED (manual checking is required)";
        case TEST_FAILED:               return "FAILED";
        case TEST_FAILED_EXCEPTION:     return "FAILED (with exception)";
        default:                        return "(Unknown result status)";
    }
}

// TODO: refactor it to avoid use global static
static void
test_sub_close(TestStatus status)
{
	if (g_prev_subtest)
	{
		g_prev_subtest = false;		// no more subtest mode
		fprint(g_out, "Subtest is\t\t\t%s\n", get_test_result_message(status));
		offsetinc(-1);
	}
}

// dep checker
static bool
check_dependencies(FILE *out, Utest *tests, int testnum, int cnt)
{
	// check if all dependent tests are passed
	Utest 	*t = tests + testnum;
	int		 dep, j;

    fprint(out, "Start cheking dependency for test %d (%d)\n", t->num, testnum);

	offsetinc(1);
    for (j = 0; j < TEST_MAX_DEP; j++)
        if ((dep = t->dep_list[j]))
        {
            Utest key_test = testnew(.num = dep);
            Utest *dep_t = bsearch(&key_test, tests, cnt - 1, sizeof(Utest), (t_f)cmp);
            if (!dep_t)
                fprint(out, "Dependency %d for test (%d) %s not found\n", dep, testnum, t->name);
            else
                if (dep_t->status != TEST_PASSED && dep_t->status != TEST_MANUAL)       // passed statuses
                {
                    fprint(out, "Dependency %d(%d) for test %d is NOT PASSED\n", dep_t->num, j, testnum);
                    break;
                }
        }

	offsetinc(-1);
    if (j == TEST_MAX_DEP)
        fprint(out, "(All dependencies are passed)\n\n");
    else
    {
        fprint(out, "(Dependencies are not passed, SKIP test)\n\n");
        t->status = TEST_SKIPPED;
        return false;
    }
	return true;
}

static bool
launcher(FILE *out, Utest *t)
{
	bool  				 res;
	TestStatus  		 status;
	int 				 cnt = t->count;

	// init global static for subtest (TODO: refactor here!!!!)
	g_out = out;
	g_prev_subtest = false;

	offsetinc(1);
	fprint(out, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	fprint(out, "%s (%d): %s [%s]\n\n", "START TEST", t->num, t->name, t->desc);
    t->status = TEST_RUNNING;

	offsetinc(1);

	if (!try())
	{
		// perform testing
		while (cnt-- && is_passed(status = t->f2()))
		{
			test_sub_close(status);     // in case if there were subtests
			g_prev_subtest = false;		// clear flag for subtest TODO: refactor here
		}
	} else
		status = TEST_FAILED_EXCEPTION;

	if (!is_passed(status))
	{
		test_sub_close(status);		// in case if f2 failad
	}

	err_resetenv();				// TODO: error.c: think if it is possible to avoid this

	offsetinc(-1);
    t->status = status;     // update tests after status TEST_RUNNING

	// determine result by test status
	res = is_passed(status);

	fputc('\n', out);
    fprint(out, "%s (%d): %s is\t\t\t%s\n", "END OF TEST", t->num, t->mandatory ? "(mandatory)" : "           ", get_test_result_message(status));
	fprint(out, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");
	offsetinc(-1);

	return res;
}

// print summary info
static inline int
print_summary(FILE *out, const TestRes *restrict summary, const char *restrict msg, int total)
{
	return fprint(out, "\n%s(%s):\n[PASSED\t/FAILED\t/EXCEPTION FAILED\t/SKIPPED\t/TOTAL RUNNED\t/TOTAL]\n[%d\t/%d\t/%d\t\t\t/%d\t\t/%d\t\t/%d]\n\n",
                  msg, summary->passed ? "PASSED" : "FAILED",
			      summary->pass_total, summary->err_total, summary->exception_total, summary->skip_total, summary->total, total);
}

#ifdef TESTTESTING
// status print as string
static inline const char *print_test_status(TestStatus s)
{
#define PRINT_TEST_STATUS(n) case n: return #n
    switch(s)
    {
        PRINT_TEST_STATUS(TEST_NOT_RUN);
        PRINT_TEST_STATUS(TEST_PASSED);
        PRINT_TEST_STATUS(TEST_SKIPPED);
        PRINT_TEST_STATUS(TEST_FAILED);
        PRINT_TEST_STATUS(TEST_MANUAL);
		PRINT_TEST_STATUS(TEST_FAILED_EXCEPTION);
        default: return "Unknown status";
    }
#undef PRINT_TEST_STATUS
}


// detailed test info print
static inline int
print_test(FILE *out, const Utest *test)
{
	int 	cnt = 0;
	cnt += fprint(out, "Logical num: %d\n", test->num);
	cnt += fprint(out, "Name       : %s\n", test->name);
	cnt += fprint(out, "Mandatory  : %s\tStatus %s\n", bool_str(test->mandatory), print_test_status(test->status));
	cnt += fprint(out, "Func ptr   : %p\n", test->f2);
	cnt += fprint(out, "Dep list   : ");
	for (int i = 0; i < TEST_MAX_DEP; i++)
		cnt += fprint(out, "%d%c", test->dep_list[i], i == TEST_MAX_DEP - 1 ?' ': ',');
	cnt += fprint(out, "\n");
	return cnt;
}

// detailed all test print
static int
print_all_test(FILE *out, const Utest *restrict tests)
{
	int 	cnt = 0;
	while (tests->f2)	// TODO: replace to .f
	{
		fprint(out, "\n(%d)\n", cnt);		// phisical number of test
		print_test(out, tests);
		tests++; cnt++;
	}
	return cnt;
}

#endif /* TESTTESTING */

// ------------------------------- interface ----------------------------------------

// tests must have final .f=0, if out==0 then output is disabled
TestRes
test_engine2(Utest *tests, FILE *out)
{
	static const char 	*START_ALL	= "START OF TESTING";
	static const char 	*END_ALL	= "END OF TESTING, TOTAL";

	TestRes		summary = {.passed = true, .pass_total = 0, .err_total = 0, .exception_total = 0, .total = 0};
	int 		cnt = 0, dubl;

	while (tests[cnt].f2)	// determine count of tests  TODO: rename to .f
		cnt++;

	// sorting elements by 'num'
	sort(tests, 0, cnt - 1);

	if (!check_duplicate_num(tests, cnt, &dubl))
	{
		fprint(out, "Incorrect input, dublicate tests number is found %d\n", dubl);
		summary.passed = false;
		return summary;
	}

	fprint(out, "\n\n%s\n\n", START_ALL);
	offsetinc(1);

	fprint(out, "Trying to set handler\n\n");

	if (!errsethandler())
		fprint(stderr, "Warning: unable to set up handler for SIGINT, but continue\n");

	for (int i = 0; i < cnt && tests[i].f2 && tests[i].status == TEST_NOT_RUN && summary.passed; i++)	// TODO: rename to .f
	{
		if (tests[i].count == 0)		// skip not active tests (status doesn't changed)
			continue;

		summary.total++;

		// check dependencies
		if (!check_dependencies(out, tests, i, cnt))
		{
			summary.skip_total++;
			continue;
		}
		// exception handling is moved to launcher()
		if (!launcher(out, tests + i))
		{
			summary.err_total++;
            if (tests[i].mandatory)
                summary.passed = false; // interrupt
			if (tests[i].status == TEST_FAILED_EXCEPTION)
				summary.exception_total++;
		} else
			summary.pass_total++;
	}

	offsetinc(-1);
	print_summary(out, &summary, END_ALL, cnt);

	//print_all_test(out, tests);
	return summary;
}

// subtest, close previous one if it was TODO: refactor here, to avoid global static
void
test_sub(const char *msg, ...)
{
	test_sub_close(TEST_PASSED);	// pass ok to close subtest if any
	fprint(g_out, "-------------------------------------------\n");
	g_prev_subtest = true;		// mark starting of subtest
	offsetinc(1);

	va_list ap;
	va_start(ap, msg);
	vfprint(g_out, msg, ap);
	va_end(ap);
	fputc('\n', g_out);
}

// open temporary test file
TFILE
test_fopen(void)
{
	TFILE	tf = (TFILE){.f = 0, .name = 0};

	char 	name[] = "TEST.XXXXXXXX";
	if ( (tf.name = strdup(mktemp(name))) ==0)
		sysraiseint("Unable to make a strdup");

	print("Temporary file name %s\n", tf.name);

	if ( (tf.f = fopen(tf.name, "w+")) == NULL)
	{
		perror("Unable to open file for rw\n");
		free(tf.name);
		sysraiseint("Unable to open file for 'rw'");
	}
	return tf;
}

// close and remove temporary test file
bool
test_fclose(TFILE tf)
{
	bool	res = true;
	if (tf.f)
		fclose(tf.f);

	if (tf.name)
	{
		logsimple("trying to remove %s", tf.name);
		if (remove(tf.name) == -1)
		{
			fprint(stderr, "Unable to remove [%s]\n", tf.name);
			res = false;
		}
		free(tf.name);
	}
	logsimple("Temporary file closed and removed");
	return  res;
}

// Compare functions (equal, equal_ne, like, ulike, regexp_like)
bool
test_str_equal(const char *restrict source, int sz, const char *restrict pattern)
{
	return strncmp(source, pattern, sz) == 0;
}

// ignore delimeters
// NOTE: will be checked separately
bool
test_str_equal_ne(const char *restrict source, int sz, const char *restrict pattern)
{
	logsimple("source [%s], pattern [%s], sz = %d", source, pattern, sz);
	int i = 0, j = 0;
	while (i < sz && pattern[j])
	{
		char	s = source[i], p = pattern[j];
		if (isspace(s))
		{
			i++;
			continue;
		}
		if (isspace(p))
		{
			j++;
			continue;
		}
		if (s != p)		// find not equal symbols
			break;
		i++, j++;
	}
	// true if source and pattern are done
	return pattern[j] == '\0' && i == sz;
}

bool
test_str_like(const char *restrict source, int sz, const char *restrict pattern)
{
	return strnstr(source, pattern, sz) != 0;
}

// NOTE: will be checked separately
bool
test_str_ulike(const char *source, int sz, const char *pattern)
{
	logsimple("source [%s], pattern [%s], sz = %d", source, pattern, sz);
	int i = 0, j = 0, i_prev = -1;
	while (i < sz && pattern[j] != '\0')
	{
		char s = toupper(source[i]), p = toupper(pattern[j]);

		if (s != p)			// TODO: криво, подумать над более корректной реализацией
		{
			j = 0;	//reset
			if (i_prev != -1)
				i = i_prev;
			i_prev = -1;
		} else	// s == p
		{
			if (i_prev == -1)	// first time, save
				i_prev = i;
			j++;
		}
		i++;
	}
	return pattern[j] == '\0';
}

// basic comparator, comp - compare function
bool
test_compare_engine(FILE *restrict tf, long from, long to, const char *restrict pt, Tcomp comp)
{
	unsigned long sz;
	if (!pt)
    {
	    fprint(stderr, "Nullable pattern");
	    return false;       // TODO: think, may be use userraiseint ?
	}

	if (from == 0 && to == 0)	// full file reading mode
	{
		if (fseek(tf, 0L, SEEK_END) < 0)
			return sysraiseint("Unable to seek to the end of file (full file mode)");
		from = test_ftell(tf);
	} else // normal mode
	if (to <= from)
		{
			fprint(stderr, "Bad file position, from %ld, to %ld\n", from, to);
			return false;
		}

	logauto(sz = to - from);	// > 0
	if (sz > TEST_PATTERN_MAX_SZ)
		return sysraiseint("Size of data is too much (%ld, but must be less than %d)", sz, TEST_PATTERN_MAX_SZ);	// TODO: щачем тут возвращаемое значение???

	char buf[sz + 1];

	if (fseek(tf, from, SEEK_SET) < 0)
		return sysraiseint("Unable to seek to position %ld", from);


	unsigned long cnt;
	if ( (cnt = fread(buf, sizeof(char), sz, tf)) != sz)
		return sysraiseint("Wrong fread... (%lu)", cnt);		// TODO: зачем тут false этот вообще????????

	buf[sz] = '\0';

	return comp(buf, sz, pt);
}

#ifdef TESTTESTING

#include "common.h"

static TestStatus
f1(void)
{
	print("just simple passed %s\n", __func__);
	return TEST_PASSED;
}

static TestStatus
f2(void)
{
	print("just simple manual %s\n", __func__);
	return TEST_MANUAL;
}

static TestStatus
f3(void)
{
	print("just simple failed %s\n", __func__);
	return TEST_FAILED;
}

// Exception test,
static TestStatus
tf20(void)
{
	print("Exception test %s\n", __func__);

	// raise error number 33
	userraiseint(33, "Something goes wrong");

	print("NEVER SHOULD SEE THIS MESSAGE!!!\n");
	return TEST_PASSED;
}

// Exception complex test

static void
tf21_exception(void)
{
	userraiseint(34, "Something goes wrong from %s", __func__);
	print("NEVER SHOULD SEE THIS MESSAGE!!!\n");
}

static TestStatus
tf21(void)
{
	print("Exception complex test %s\n", __func__);

	print("exec tf21_exception function\n");
	tf21_exception();

	return TEST_PASSED;
}

static TestStatus
tf22_pub(void)
{
	print("%s should be passed at the end\n", __func__);

	test_sub("Bla bla bla sub1");
	test_sub("Qwerty sub2");
	test_sub("Asdfghj sub3");

	return TEST_MANUAL;
}

static TestStatus
tf23_pub(void)
{
	print("%s should NOT be passed at the end\n", __func__);

	test_sub("Bla bla bla sub1");
	test_sub("Qwerty sub2, then failed");
	return TEST_FAILED;

	test_sub("Never this subtest!!! sub3");

	print("!!!!!!!!!!!!!!!! Never here!!\n");
	return TEST_PASSED;
}

static TestStatus
tf24_pub(void)
{
	print("%s should NOT be passed at the end, exception 'll occur\n", __func__);

	test_sub("1-st");
	test_sub("2-nd sub");
	userraiseint(333, "Something goes wrong from %s", __func__);

	print("!!!!!!!!!!!!!!!! Never here!!\n");
	return TEST_PASSED;
}

static TestStatus
tf25_pub(void)
{
	print("%s: open test file, print data, then close\n", __func__);

	int				res = 0, total = 0;
	TFILE			f = test_fopen();
	const char 	   *msg;

	test_sub("%d", ++total);
	msg = "Ha ha ha";
	if (!test_act_equal(fprintf(tfile(f), "%s", msg), tfile(f), msg))
		print("Not equal (%d)\n", ++res);

	test_sub("%d", ++total);
	msg = "Bla bla bla";
	if (!test_act_equal(fprintf(tfile(f), "%s", msg), tfile(f), msg))
		print("Not equal (%d)\n", ++res);

	test_sub("%d", ++total);
	msg = "Tru la la la";
	if (!test_act_equal(fprintf(tfile(f), "%s",  msg), tfile(f), msg))
        print("Not equal (%d)\n", +res);

	test_sub("%d", ++total);
	msg = "	werty";
	if (test_act_equal(fprintf(tfile(f), "%s",  msg), tfile(f), "Qwerty"))
        print("Not equal (%d)\n", ++res);

	test_fclose(f);

	print("%d failed of %d total\n", res, total);

	return res == 0 ? TEST_PASSED : TEST_FAILED;
}

static TestStatus
tf26_pub(void)
{
    print("%s: test with ignoring spaces\n", __func__);

    int				res = 0, total = 0;
    TFILE           f = test_fopen();
    const char     *msg;

	if (!tfile(f)){
		print("Unable to open temporary tile\n");
		return TEST_FAILED;
	}

    test_sub("%d", ++total);
	msg = "Ha             ha     ha";
    if (!test_act_equal_ne(fprintf(tfile(f), "%s", msg), tfile(f), "Ha ha ha"))
        print("Not equal (%d)\n", ++res);

    test_sub("%d", ++total);
    msg = "Bla bla bla";
    if (!test_act_equal_ne(fprintf(tfile(f), "%s", msg), tfile(f), msg))
        print("Not equal (%d)\n", ++res);

    test_sub("%d", ++total);
    msg = "Tru la la la";
    if (test_act_equal_ne(fprintf(tfile(f), "%s",  msg), tfile(f), "Trulala"))
        print("Not equal (%d)\n", ++res);

	test_sub("%d", ++total);
	if (!test_act_equal_ne(fprintf(tfile(f), "%s",  msg), tfile(f), "Trulalala"))
        print("Not equal (%d)\n", ++res);

    test_fclose(f);

	print("%d failed of %d total\n", res, total);

    return res == 0 ? TEST_PASSED : TEST_FAILED;
}

static TestStatus
tf27_pub(void)
{
	print("%s: test of comparator - like\n", __func__);

	int				res = 0, total = 0;
	TFILE			f = test_fopen();
	const char	   *msg;

	if (!tfile(f)){
		print("Unable to open temporary tile\n");
		return TEST_FAILED;
	}

	test_sub("%d", ++total);
	msg = "Test message for checking like";
	if (!test_act_like(fprintf(tfile(f), "%s", msg), tfile(f), "che"))
		printf("Not liked (%d)\n", ++res);

	test_sub("%d", ++total);
	if (test_act_like(fprintf(tfile(f), "%s", msg), tfile(f), "sss"))
	    printf("Liked, nut must be not (%d)\n", ++res);

	test_fclose(f);

	print("%d failed of %d total\n", res, total);

	return res == 0 ? TEST_PASSED : TEST_FAILED;
}

static TestStatus
tf28_pub(void)
{
	print("%s: test of comparator - Ulike\n", __func__);

	int             res = 0, total = 0;
	TFILE           f = test_fopen();
	const char     *msg;

	if (!tfile(f)){
		print("Unable to open temporary tile\n");
		return TEST_FAILED;
	}

	test_sub("%d", ++total);
	msg = "Test message for checking ulike";
	if (!test_act_ulike(fprintf(tfile(f), "%s", msg), tfile(f), "CHE"))
		print("Not liked (%d)\n", ++res);

	test_sub("%d", ++total);
	if (test_act_ulike(fprintf(tfile(f), "%s", msg), tfile(f), "ESE"))
		print("Liked, but must be not (%d)\n", ++res);

	test_sub("%d", ++total);
	msg = "AAABBbbbbbcccDDDhhhhhh345G";

	if (!test_act_ulike(fprintf(tfile(f), "%s", msg), tfile(f), "345g"))
		print("Not liked (%d)\n", ++res);

	test_sub("%d", ++total);
	if (!test_act_ulike(fprintf(tfile(f), "%s", msg), tfile(f), "bbc"))
		print("Not liked (%d)\n", ++res);

	test_fclose(f);

	print("%d failed of %d total\n", res, total);
	return res == 0 ? TEST_PASSED : TEST_FAILED;
}


int
main(int argc, char *argv[])
{
	printf("start testing with %d (%p)\n", argc, argv);

	logsimpleinit("loggin of test");

	/*
	testenginestd(testnew(.f2=f1, .name="f1"), testnew(.f2=f2, .name="f2"), testnew(.f2=f3, .name="f3"));	//  логика должна быть такая, если num=0, то == индексу

	testenginestd(testnew(.f2=f1, .num=1, .name="f1")
						, testnew(.f2=f2, .num=3, .name="f2")
						, testnew(.f2=f3, .num=2, .name="f3"));

	testenginestd(testnew(.f2=f1, .num=1, .mandatory=true, .name="f1")
                        , testnew(.f2=f2, .num=3, .name="f2")
                        , testnew(.f2=f3, .num=2, .name="f3"));

	testenginestd(testnew(.f2=f1, .num=1, .mandatory=true, .name="f1")
                        , testnew(.f2=f2, .num=2, .name="f2")
                        , testnew(.f2=f3, .num=3, .mandatory=true, .name="f3")
						, testnew(.f2=f1, .num=4, .name="f1_4"));
	*/
	testenginestd(testnew(.f2 = f1		, .num = 1					, .name = "f1_1")
			    , testnew(.f2 = f1		, .num = 4 , testdep(1)		, .name = "f1_4")
				, testnew(.f2 = f3		, .num = 5					, .name = "f3_5")
				, testnew(.f2 = f3		, .num = 8 , testdep(5)		, .name = "f3_8")
				, testnew(.f2 = f2		, .num = 11, testdep(4)		, .name = "f1_11")	// testdep(8)
				, testnew(.f2 = tf20	, .num = 20					, .name = "Exception test"					, .mandatory = false)
				, testnew(.f2 = tf21	, .num = 21                 , .name = "Exception complex test"			, .mandatory = false)
				, testnew(.f2 = tf22_pub, .num = 22 				, .name = "Subtest manual test (ok)"		, .mandatory = true)
				, testnew(.f2 = tf23_pub, .num = 23                 , .name = "Subtest manual test (fail)"		, .mandatory = false)
				, testnew(.f2 = tf24_pub, .num = 24                 , .name = "Subtest manual (exception fail)" , .mandatory = false)
				, testnew(.f2 = tf25_pub, .num = 25                 , .name = "File comparator combined test"	, .mandatory = true)
				, testnew(.f2 = tf26_pub, .num = 26                 , .name = "NE compator test"   				, .mandatory = true)
				, testnew(.f2 = tf27_pub, .num = 27                 , .name = "Like compator test"              , .mandatory = true)
				, testnew(.f2 = tf28_pub, .num = 28                 , .name = "Like in uppercase compator test" , .mandatory = true)
	);

	logclose("Done");

	// test of COUNT
	/*int p[]={[0]=4, [11]=2, [15]=7773};
	for (int i=0; i<COUNT(p); i++)
		printf("%d %d\n", i, p[i]); */
	return 0;
}

#endif /* TESTTESTING */

