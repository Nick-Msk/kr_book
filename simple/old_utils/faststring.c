#include <common.h>
#include <log.h>
#include <stdarg.h>
#include "faststring.h"

/********************************************************************
                    FAST STRING MODULE IMPLEMENTATION
********************************************************************/

static const int 		FS_BUF_SIZE				= 1024;
static const int		FS_TECH_PRINT_COUNT		= 10; // symplos to print
static char				g_fs_buffer[FS_BUF_SIZE];

// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers -------------------

// TODO: move to common.h ??
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

// should depent on increase strategy, simplest return n + 1;
static int				calcnewsize(int n){
	return n + 10;
}

// --------------------------- API ---------------------------------

fs                   	initn(int n){
	int 	 newsz = calcnewsize(n);
	char 	*s;
	if (!(s = malloc(n)))
		userraiseint(10, "Unable to allocate %d bytes", n);
	fs		 res = (fs) {.len = n, .sz = newsz, .flags = 0, .s = s};
	return logsimpleret(res, "Created %s", fs_tostr(&res));
}

// -------------------------- (API) printers -----------------------
int						fs_techfprint(FILE *restrict out, const fs *restrict s){
	// technical print, statis attributes for now
	int 	cnt;
	if (s){
		cnt = fprintf(out, "len [%d], sz [%d], flags [%d], s [%p]=[", s->len, s->sz, s->flags, s->s);
		for (int i = 0; i < FS_TECH_PRINT_COUNT && i < s->len; i++)
			fputc(s->s[i], out), cnt++;
		if (FS_TECH_PRINT_COUNT < s->len)
			cnt += fprintf(out, "...");
		cnt += fprintf(out, "]\n");
	}
	else
		cnt = fprintf(out, "Zero pointer\n");
	return cnt;
}

// to string for technical purpose, NO THREAD SAFE
const char 				*fs_tostr(const fs *s){
	if (s){
		int cnt = snprintf(g_fs_buffer, FS_BUF_SIZE, "len [%d], sz [%d], flags [%d], s [%p]=[", s->len, s->sz, s->flags, s->s);
		for (int i = 0; cnt + i < FS_BUF_SIZE - 3 && i < FS_TECH_PRINT_COUNT && i < s->len; i++)
			g_fs_buffer[cnt++] = s->s[i];		// what if '\0' ?
		// cnt < FS_BUF_DIZE - 3
		g_fs_buffer[cnt++] = ']';
		g_fs_buffer[cnt++] = '\n';
		g_fs_buffer[cnt++] = '\0';
	}
	else
		snprintf(g_fs_buffer, FS_BUF_SIZE, "Zero pointer\n");
	return g_fs_buffer;
}

bool					fs_validate(FILE *restrict out, const fs *restrict s){
	logenter("%s", fs_tostr(s));

	if (!s){	// TODO: think about string creation via faststring here!!
		lprint(out, "null pointer");
		return logerr(false, "null pointer");
	}
	if (!s->s){
		lprint(out, "nullable string");
		return logerr(false, "nullable string");
	}
	// depents on which iterator engine is active
	if (s->len >= s->sz){
		lprint(out, "len [%d] must be < sz [%d]", s->len, s->sz);
		return logerr(false, "len [%d] must be < sz [%d]", s->len, s->sz);
	}
	{
		int len = strlen(s->s);
		if (len <= s->len){
			lprint(out, "srtlen [%d] can't be more than len [%d]", len, s->len);
			return logerr(false, "srtlen [%d] can't be more than len [%d]", len, s->len);
		}
	}
	return logret(true, "true");
}

// ------------------ General functions ----------------------------

fs                   *fs_shrink(fs *s){
	if (fs_heap(s)){
		int		newsz = s->len + 1;	// final '\0' is assumed!
		if (newsz < s->sz){
			s->s = realloc(s->s, newsz);
			s->sz = newsz;
		}
		return logsimpleret(s, "Shrinked %s", fs_tostr(s));
	} else
		return logsimpleret(s, "Not heap: shrink is skipped %s", fs_tostr(s));
}

// can increase sz and len
char				 *fs_elem(fs *s, int pos){
	if (pos >= s->sz){
		increasesize(s, pos);	// len remains the same here! sz is changed
		logsimple("size is adjusted to %d (pos %d)", s->sz, pos);
	}
	if (pos > s->len)
		logsimpleact(s->len = pos, "len is adjusted to %d", pos);		// не понятно как илт нет, вообще, len имеет смысл при add или move, тут же просто соглашение, что если заполнена позиция pos, то 0..pos-1 считаются принадлежащими строке
	return fs_get(s, pos);
}

// -------------------------------Testing --------------------------
#ifdef FSTESTING

#include "testing.h"

//types for testing

// ------------------------- TEST 1 ---------------------------------

// Simple init and validate test
static TestStatus
f1(void)
{
	logenter("%s: Simple 'while' iterator test\n", __func__);

	iter 	i = iterinit(.from = 10, .to = 100, .f = iter_interval);

	if (!fsvalidate(logfile, i))
		logerr(TEST_FAILED, "validation is failed, see above for details");

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 2 ---------------------------------

//
static TestStatus
f2(void)
{
	logenter("%s: Simple 'while' iterator test\n", __func__);

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 3 ---------------------------------

//
static TestStatus
f3(void)
{
	logenter("%s: Very Simple 'while' iterator test\n", __func__);

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 4 ---------------------------------

//
static TestStatus
f4(void)
{
	logenter("%s: 'for' complex iterator test\n", __func__);

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 5 ---------------------------------

//
static TestStatus
f5(void)
{
	logenter("%s: Incorrect init test\n", __func__);

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------------------------------------------------
int
main(int argc, char *argv[])
{
    logsimplenit("Starting");

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

#endif /* FSTESTING */

