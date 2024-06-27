#include "iter.h"
#include <common.h>
#include <log.h>
#include <stdarg.h>

/********************************************************************
                    ITERATION MODULE IMPLEMENTATION
********************************************************************/

static const int 		ITER_BUF_SIZE = 1024;
static char				g_iter_buffer[ITER_BUF_SIZE];

// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers -------------------

static int lprint(FILE *restrict out, const char *restrict msg, ...){
	int 		cnt = 0;
	if (out){
		va_list ap;
		va_start(ap, msg);
		cnt = vfprintf(out, msg, ap);
		cnt += fprintf(out, "\n");
		va_end(ap);
	}
	return cnt;
}

// ------------------------------ Utilities ------------------------

// --------------------------- API ---------------------------------

// -------------------------- (API) printers -----------------------
int						iter_techfprint(FILE *restrict out, const iter *restrict i){
	// technical print, statis attributes for now
	int 	cnt;
	if (i)
		cnt = fprintf(out, "step [%d], from [%d], to [%d], currint [%d]\n", i->step, i->from, i->to, i->intcurr);
	else
		cnt = fprintf(out, "Zero pointer\n");
	return cnt;
}

// to string, NO THREAD SAFE
const char 				*iter_tostr(const iter *i){
	if (i)
		snprintf(g_iter_buffer, ITER_BUF_SIZE, "step [%d], from [%d], to [%d], currint [%d]\n", i->step, i->from, i->to, i->intcurr);
	else
		snprintf(g_iter_buffer, ITER_BUF_SIZE, "Zero pointer\n");
	return g_iter_buffer;
}

bool					iter_validate(FILE *restrict out, const iter *restrict i){
	logenter("%p", i);

	if (!i){	// TODO: think about string creation via faststring here!!
		lprint(out, "nullable iterator");
		return logerr(false, "");
	}
	if (!i->f){
		lprint(out, "nullable iterator function");
		return logerr(false, "");
	}
	// depents on which iterator engine is active
	if (i->f == iter_interval)
		if (i->step == 0){
			lprint(out, "iter_interval: step must be != 0");
			return logerr(false, "");
		}
		if ((i->to < i->from && i->step > 0) || (i->to > i->from && i->step <0)){
			lprint(out, "iter_interval: 'from' (%d) and 'to' (%d) must be qync with step (%d)", i->from, i->to, i->step);
			return logerr(false, "");
		}
	else {
		lprint(out, "Unsupported iterator %p", i->f);
		return logerr(false, "");
	}
	return logret(true, "true");
}

// -------------------------- API ENGINE (I) ------------------------

// simple interval iterator
bool					iter_interval(struct iter *i, ITER action){
	logenter("%p, %s", i, iter_action_tostr(action));

	bool		res = true;
	switch (action){
		case ITER_INIT:
			i->intcurr = i->step < 0 ? i->to : i->from;
		break;
		case ITER_NEXT:
			i->intcurr += i->step;
		case ITER_CHECK:	// no break frm iter_next
			if (i->step < 0)
				res = i->intcurr > i->from;
			else
				res = i->intcurr < i->to;
		break;
		default:
			logact(res = false, "Action %d is incorrect", action);
		break;
	}
	return logret(res, "res %s", bool_str(res));
}

// ------------------ General functions ----------------------------

// -------------------------------Testing --------------------------
#ifdef ITERTESTING

#include "testing.h"
#include <common.h>

//types for testing

// ------------------------- TEST 1 ---------------------------------

// Simple init and validate test
static TestStatus
f1(void)
{
	logenter("%s: Simple 'while' iterator test\n", __func__);

	iter 	i = iterinit(.from = 10, .to = 100, .f = iter_interval);

	if (!itervalidate(logfile, i))
		logerr(TEST_FAILED, "validation is failed, see above for details");

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 2 ---------------------------------

// Simple 'while' iterator test
static TestStatus
f2(void)
{
	logenter("%s: Simple 'while' iterator test\n", __func__);

	iter 	i = iterinitinterval(50, 80);
	int		j = 50, val;	// shadow

	while (iterhasnext(i)){
		val = itergetnextint(i);
		if (val != j)
			return logerr(TEST_FAILED, "i = %d (%d), j = %d", val, itergetint(i), j);
		j++;
	}
	if (itergetint(i) != j)
        return logerr(TEST_FAILED, "LAST i = %d (%d), j = %d", val, itergetint(i), j);

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 3 ---------------------------------

// Very Simple 'while' iterator test
static TestStatus
f3(void)
{
	logenter("%s: Very Simple 'while' iterator test\n", __func__);

	iter		i = iterinit(.from = 10, .to = 100, .f = iter_interval, .step = 3);
	int			j = 10, val;		// for the check

	while(itersimpleint(i, &val)){
		if (val != j)
            return logerr(TEST_FAILED, "i = %d (%d), j = %d", val, itergetint(i), j);
		j += 3;
	}
	if (itergetint(i) != j)
        return logerr(TEST_FAILED, "LAST i = %d (%d), j = %d", val, itergetint(i), j);

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 4 ---------------------------------

// 'for' complex iterator test
static TestStatus
f4(void)
{
	logenter("%s: 'for' complex iterator test\n", __func__);

	int 		j = 4, val;

	for (iter i = iterinitinterval(4, 20); iterhasnext(i); iternext(i)){
		val = itergetint(i);
		if (val != j)
            return logerr(TEST_FAILED, "i = %d (%d), j = %d", val, itergetint(i), j);
		j++;
	}

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 5 ---------------------------------

// Incorrect init test
static TestStatus
f5(void)
{
	logenter("%s: Incorrect init test\n", __func__);

	iter        i = iterinit(.from = 10, .to = 100, .f = iter_interval, .step = -3);
	itertechfprint(logfile, i);

	if (itervalidate(logfile, i)){
        return logerr(TEST_FAILED, "validation is passed, but HAVE TO BE FAILED, see above for details");
	}

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------------------------------------------------
int
main(int argc, char *argv[])
{
    const char *logfilename = "iter.log";

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false, 0, "Starting");

    testenginestd(
		testnew(.f2 = f1, .num = 1, .name = "Simple init and validate"      , .desc="Init test."				, .mandatory=true)
      , testnew(.f2 = f2, .num = 2, .name = "Simple 'while' iterator"		, .desc="Simple while test."		, .mandatory=true)
	  , testnew(.f2 = f3, .num = 3, .name = "Very Simple 'while' iterator"	, .desc="Simplest while test."      , .mandatory=true)
	  , testnew(.f2 = f4, .num = 4, .name = "'for' complex iterator"		, .desc="'for' test."				, .mandatory=true)
	  , testnew(.f2 = f5, .num = 5, .name = "Incorrect init"        		, .desc="Init test."                , .mandatory=true)
    );

    logclose("end...");
    return 0;
}

#endif /* ITERTESTING */

