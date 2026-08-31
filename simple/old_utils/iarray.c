#include "iarray.h"
#include <common.h>
#include <log.h>

/********************************************************************
                    TMAP MODULE IMPLEMENTATION
********************************************************************/

// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers -------------------

// ------------------------------ Utilities ------------------------

static int 					 calc_next_cnt(int newcnt){
	int 	newsz = newcnt + MAX(newcnt/10, 10);	// TODO: improve to выравнять по 16 байт
	return logautoret(newsz);
}



// utility, if data is null, then alloc new
// data->cnt is changed only in case of null data
iarray						 ia_increasesize(iarray *data, int newcnt){
	logenter("%p, newcnt = %d", data, newcnt);

	int			newsz = calc_next_cnt(newcnt);
	iarray		tmp;
	if (!data){		// alloc
		if ( (tmp.arr = malloc(sizeof(int) * newsz)) == 0)
			return logerr(tmp, "Unable to malloc %d elements", newsz);		// no change to raise here! (
		tmp.sz  = newsz;
		tmp.cnt = newcnt;
	} else {
		tmp = *data;
		logmsg("sz = %d, newcnt = %d", data->sz, newcnt);
		if (data->sz < newcnt){		// not enough place in iarray
			int 	*arr = realloc(data->arr, sizeof(int) * newsz);
			if (!arr)
				return logerr(tmp, "Unable to realloc new %d elements", newsz);  // TODO: errraise must be used here
			data->sz  = newsz;
			data->arr = arr;
			tmp = *data;
		}
	}

	return logret(tmp, "increased size to %d", newsz);
}

// --------------------------- API ---------------------------------

int                         *ia_elem(iarray *data, int pos){
	logenter("cur sz = %d, cur cnt = %d, pos = %d", data->sz, data->cnt, pos);

	if (data->sz <= pos)
		ia_increasesize(data, pos + 1);
	if (data->cnt <= pos)
		data->cnt = pos + 1;

	return logret(ia_get(data, pos), "new sz = %d, new cnt = %d", data->sz, data->cnt);
}

// dest must be valid iarray
iarray                      *ia_concat(iarray *restrict dest, iarray *restrict src){
	logenter("concat from %p (%d - %d) to %p (%d - %d)"
		, src, src ? src->sz : 0, src ? src->cnt : 0
		, dest, dest ? dest->sz : 0, dest ? dest->cnt : 0);

	if (!src)
		return logret(dest, "No nothing");

	int 		newsz = dest->cnt + src->cnt;

	if (!dest || dest->sz < newsz)				// working even dest is NULL
		ia_increasesize(dest, newsz);			// TODO: как тут обработать исключительную ситуцию???

	memcpy(dest->arr + dest->cnt, src->arr, src->cnt * sizeof(int));
	dest->cnt += src->cnt;			// new cnt

	return logret(dest, "copied, new cnt = %d", dest->cnt); // ?
}

void						 ia_free(iarray *data){
	logsimple("free %p", data);
	if (data){
		free(data->arr);
		data->arr = 0;
		data->sz = data->cnt = 0;
	}
}

// -------------------------- (API) printers -----------------------

// returns count of bytes
int			                 iafprintf_all(FILE *out, iarray data){
	int	res = iafprintinfo(out, data);

	for (int i = 0; i < data.cnt; i++)
		res += fprintf(out, "arr[%d] = %d\n", i, data.arr[i]);
	return res;
}

// ------------------ General functions ----------------------------

// -------------------------------Testing --------------------------
#ifdef IARRAYTESTING

#include "testing.h"
#include <common.h>

//types for testing

// ------------------------- TEST 1 ---------------------------------

// Create and free test
static int
f1(void *restrict data)
{
    logenter("%p", data);

	iarray	ar = iacreate(8);

	if (ar.cnt != 8 || ar.cnt > ar.sz)
		return logerr(TEST_FAILED, "Incorrect cnt %d and sz %d", ar.cnt, ar.sz);
	iaprintinfo(ar);

	iafree(ar);

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 2 ---------------------------------

// Create and several elem() test
static int
f2(void *data){
	logenter("%p", data);

	iarray 	ar = ianull;

	iaelem(ar, 5) = 45;
	if (ar.cnt < 5 || ar.sz < 5)
		return logacterr(iafree(ar), TEST_FAILED, "Cnt = %d, sz = %d, but must be >= 5", ar.cnt, ar.sz);

	int 	val = ar.arr[5];
	if (val != 45)
		return logacterr(iafree(ar), TEST_FAILED, "arr[5] = %d, but must be = %d", val, 45);

	if ( (val = iaget(ar, 5)) != 45)
		return logacterr(iafree(ar), TEST_FAILED, "iaget(ar, 5) = %d, but must be = %d", val, 45);

	if ( (val = iaelem(ar, 5)) != 45)
		return logacterr(iafree(ar), TEST_FAILED, "iaelem(ar, 5) = %d, but must be = %d", val, 45);

	iafree(ar);

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 3 ---------------------------------

//Several elem() test
static int
f3(void *data){
	logenter("%p", data);

	iarray 		ar = ianull;

	logmsg("many elems... randomly");
	for (int i = 0; i < 10000; i++){
		int 	t = rndint(50000);
	//	logmsg("i = %d, t = %d", i, t);
		iaelem(ar, t) = t;
	//	logmsg("...");
	//	iafprintinfo(logfile, ar);
		if (ar.arr[t] != t)
			return logacterr(iafree(ar), TEST_FAILED, "failed %d iter, %d != %d", i, ar.arr[t], t);
	}

	logmsg("check fillzero");
	iafillzero(ar);
	long	sum = 0;
	// TODO: iterator ?  think about it
	for (int i = 0; i < ar.cnt; i++)
		sum += ar.arr[i];

	if (sum != 0)
		return logacterr(iafree(ar), TEST_FAILED, "sum must be zero, but = %ld", sum);

	iafree(ar);
	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 4 ---------------------------------
// Concat test

static int
f4(void *data){
	logenter("%p", data);

	iarray 		ar = ianull;
	const int 	sz = 1000;

	logmsg("fill with random data");
	for (int i = 0; i < sz; i++)
		iaelem(ar, i) = rndint(50000);

	iafprintinfo(logfile, ar);

	iarray		ar2 = iacopy(ar);
	iafprintinfo(logfile, ar2);

	if (ar.cnt != ar2.cnt || ar.cnt != sz)
		return logacterr( (iafree(ar), iafree(ar2)), TEST_FAILED, "count of ar2 = %d, but must be = %d (sz = %d)", ar2.cnt, ar.cnt, sz);

	logmsg("check if equal");

	for (int i = 0; i < ar.cnt; i++)
		if (ar.arr[i] != ar2.arr[i])
			return logacterr( (iafree(ar), iafree(ar2)), TEST_FAILED, "ar[%d] == %d != ar2[%d] == %d", i, ar.arr[i], i, ar2.arr[i]);

	for (int i = 0; i < ar.cnt; i++){
		int val  = iaelem(ar, i);
		int val2 = iaelem(ar2, i);
		if (val != val2)
			return logacterr( (iafree(ar), iafree(ar2)), TEST_FAILED, "iaelem(ar, %d) == %d not equal iaelem(ar2, %d) == %d", i, ar.arr[i], i, ar2.arr[i]);
	}
	iafprintinfo(logfile, ar);
	iafprintinfo(logfile, ar2);

	logmsg("concat test");

	iaconcat(ar, ar2);
	iafprintinfo(logfile, ar);
	if (ar.cnt != 2 * ar2.cnt)
		return logacterr( (iafree(ar), iafree(ar2)), TEST_FAILED, "count of concatenated array = %d, but must be %d", ar.cnt, 2 * ar2.cnt);

	for (int i = 0; i < ar.cnt / 2; i++){
		int	val  = iaelem(ar, i);
		int	val2 = iaelem(ar, i + sz);
		if (val != val2)
			return logacterr( (iafree(ar), iafree(ar2)), TEST_FAILED, "ar[%d] == %d not equal to ar[%d] == %d", i, val, i + sz, val2);
	}

	iafree(ar);
	return logret(TEST_PASSED, "done"); // TEST_FAILED
}


// ------------------------------------------------------------------
int
main(int argc, char *argv[])
{
    const char *logfilename = "iarray.log";

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false, 0, "Starting");

    testenginestd(
        testnew(.f2 = f1, .num = 1, .name = "Create and free test"	, .desc="Simple test."		 , .mandatory=true)
      , testnew(.f2 = f2, .num = 2, .name = "Create and elem() test", .desc="Elem test."		 , .mandatory=true)
      , testnew(.f2 = f3, .num = 3, .name = "Several elem() test"	, .desc="Several elem test." , .mandatory=true)
	  , testnew(.f2 = f4, .num = 4, .name = "Concat test"   		, .desc="Concatenation test.", .mandatory=true)
    );

    logclose("end...");
    return 0;
}


#endif /* IARRAYTESTING */

