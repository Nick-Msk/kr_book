#include "tmap.h"
#include <stdlib.h>
#include <log.h>
#include <assert.h>

/********************************************************************
                    TMAP MODULE IMPLEMENTATION
********************************************************************/

// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers -------------------

static int			printtmapdata(FILE *out, tmapdata d, bool printdelta){
	int 	res = fprintf(out, "val [%ld],\tcnt [%d]", d.val, d.cnt);

	static bool 		first_run = true;
	static int			prev_cnt;

	if (printdelta && !first_run)
		res += fprintf(out, ",\tdelta [%d]", - d.cnt + prev_cnt);
	res += fprintf(out, "\n");
	prev_cnt = d.cnt;
	first_run = false;
	return res;
}

int 	printorderedval(FILE *restrict out, const tmap *restrict v, char delim){
	int     res = 0;
	if (v){
		res += printorderedval(out, v->left, delim);
		res += fprintf(out, "%ld%c", v->d.val, delim);
		res += printorderedval(out, v->right, delim);
	}
	return res;
}

int     printorderedcnt(FILE *restrict out, const tmap *restrict v, char delim){
    int     res = 0;
    if (v){
        res += printorderedcnt(out, v->left, delim);
        res += fprintf(out, "%d%c", v->d.cnt, delim);	// TODO: можно через callback, через tmap_foreach()
        res += printorderedcnt(out, v->right, delim);
    }
	return res;
}

// ------------------------------ Utilities ------------------------

static tmap			*alloc(long v){
	tmap 		*node = malloc(sizeof(tmap));
	if (!node)
		return logsimpleerr((void *) 0, "Unable to allocate %zu bytes", sizeof(tmap));

	node->d = (tmapdata) {.cnt = 1, .val = v};		// TODO: не уверен, что именно так лучше
	node->left = node->right = 0;
	return logsimpleret(node, "Allocated %zu - %p (%lu)", sizeof(tmap), node, v);
}

// make a clone of current node
// v must be not null
static tmap			*clone(const tmap *v){
	tmap		*t = alloc(v->d.val);
	t->d.cnt = v->d.cnt;
	return logsimpleret(t, "cloned cnt [%d]", t->d.cnt);
}

// find node for the val, return into root if found, otherwise return false and uplink node in the root
static bool			findroot(const tmap *restrict v, long val, tmap ** const restrict root){
	logenter("%p, %ld (%p)", v, val, root);

	bool		 	 found = false;
	const tmap		*uplink = 0;

	while(v && !found){
		uplink = v;
		logmsg("v.val = %ld", v->d.val);
		if (val > v->d.val)			// TODO: think about #define value(v) v->d.val
			logmsg("right"), v = v->right;	// logact(v = v->right, "go right");
		else if (val < v->d.val)
			logmsg("left"), v = v->left;
		else
			found = true;
	}
	if (root)
		*root = uplink;
	return logret(found, "%s, uplink %p", bool_str(found), uplink);
}

// --------------------------- API ---------------------------------

// first implementation, w/o iterator
void            tmap_free(tmap *v){
	if (!v)
		return;
	logsimple("%p", v);

	tmap_free(v->left);
	tmap_free(v->right);
	free(v);
}

// simple copy, w/o resuffle
tmap                *tmap_copy(const tmap *v){
	logenter("%p", v);

	tmap		*tmp = 0;
	if (v){
		if (! (tmp = clone(v)) )
			return logerr(tmp, "Unable to clone %p (val - %ld, cnt - %d)", v, v->d.val, v->d.cnt);	// TODO: error MUST be used here!!!

		logmsg("cloning left %p", v->left);
		tmp->left  = tmap_copy(v->left);
		logmsg("cloning right %p", v->right);
		tmp->right = tmap_copy(v->right);
	}

	return logret(tmp, "%s... %p", tmp ? "created" : "skipped", tmp);
}

// -------------------------- (API) printers -----------------------

int              tmap_fprintall(FILE *restrict out, const tmap *restrict v, bool printdelta){
	logenter("out %p, %p [%ld - %d]", out, v, v ? v->d.val : 0, v ? v->d.cnt : 0);

	int		res = 0;
	if (v){
		logmsg("print left - %p", v->left);
		res += tmap_fprintall(out, v->left, printdelta);
		res += printtmapdata(out, v->d, printdelta);
		logmsg("print right - %p", v->right);
		res += tmap_fprintall(out, v->right, printdelta);
	}
	return logautoret(res);		 //logret(res, "res = %d", res);
}

int				tmap_fprintcsv(FILE *restrict out, const tmap *restrict v){
	logenter("out %p, %p [%ld - %d]", out, v, v ? v->d.val : 0, v ? v->d.cnt : 0);
	int res = printorderedval(out, v, ';');
	res += fprintf(out, "\n");
	res +=    printorderedcnt(out, v, ';');
	return logret(res, "...");
}

// ------------------ general functions ----------------------------

tmap            *tmap_putval(tmap *v, long val){
	logenter("%p, %ld", v, val);
	if (!v){			// too bad, refactoring is required
		if (! (v = alloc(val)) )
			return logerr((void *) 0, "Unable to alloc new node");
		else
			return logret(v, "Initial node %ld", val);
	}

	tmap	*root;
	if (findroot(v, val, &root)){	// record is found, just increase counter
		root->d.cnt++;
		logauto(root->d.cnt);
	}
	else {
		tmap		*newnode = alloc(val);
		if (!newnode)
			return logerr((void *) 0, "Unable to alloc new node");
		// attach new node, val != root->d.val here
		assert(val != root->d.val);		// to check
		if (val > root->d.val){
			assert(root->right == 0);		// TODO: remove after testing
			root->right = newnode;
		} else {
			assert(root->left == 0);
			root->left = newnode;
		}
	}
	return logret(v, "%p (%ld) is added to the tree", v, val);
}

int                  tmap_getnodecnt(const tmap *v){
	logenter("%p", v);

	int		cnt = 0;

	if (v){
		logmsg("check left - %p", v->left);
		cnt += tmap_getnodecnt(v->left);
		cnt++;
		logmsg("check right - %p", v->right);
		cnt += tmap_getnodecnt(v->right);
	}

	return logret(cnt, "Return cnt %d", cnt); // logautoret(cnt);		TODO: auto have to work in common mode too, not only in simple (this is refactoring of logger)
}

int					 tmap_gettotal(const tmap *v){
	logenter("%p [%ld - %d]", v, v ? v->d.val : 0, v ? v->d.cnt : 0);	// TODO: коряво как-то

	long 	total = 0;

	if (v){
		logmsg("Count left");
		total += tmap_gettotal(v->left);
		logmsg("Current node = %d", v->d.cnt);
		total += v->d.cnt;		//  TODO:  log.h  аналог logacterr:  logact(action, fmt, ...)        logact(total += v->d.cnt, "Current node = %d", v->d.cnt);
		logmsg("Count right");
		total += tmap_gettotal(v->right);
	}

	return  logret(total, "Total = %ld", total);	// logautoret(total); // , "Total = %ld", total);		// TODO: подумать об автологировании, что-то типа return logautoret(total);
}

// find node by val
tmap                 *tmap_getnode(const tmap *v, long val){
	logenter("%p [%ld - %d], find %ld", v, v ? v->d.val : 0, v ? v->d.cnt : 0, val);			// [%ld - %d]", v, v ? v->d.val : 0, v ? v->d.cnt : 0

	bool		 found = false;
	const tmap	*res = 0;

	while (v && !found){
		if (val < v->d.val)
			v = v->left;
		else if (val > v->d.val)
			v = v->right;
		else
			res = v, found = true;
	}

	return logret((tmap *) res, "found %p", res);
}


// -------------------------------Testing --------------------------
#ifdef TMAPTESTING

#include "testing.h"
#include <common.h>

//types for testing

// ------------------------- TEST 1 ---------------------------------

// Create and free test
static int
f1(void *restrict data)
{
    logenter("%p", data);

	int		 cnt, val = 123;
	tmap	*map = tmap_putval(0, val);

	tmap_fprintall(logfile, map, true);

	if ( (cnt = tmap_getnodecnt(map)) != 1)
		return logderr(tmap_free(map), TEST_FAILED, "count of nodes = %d, but must be 1", cnt);

	if ( (cnt = tmap_getcnt(map, val)) != 1)
		return logderr(tmap_free(map), TEST_FAILED, "count of %d == %d, but must be 1", val, cnt);

	tmap_free(map);

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 2 ---------------------------------

// Create several vals test
static int
f2(void *restrict data)
{
	logenter("%p", data);

	const int		count_iter = 100;
	int 	 		cnt, val = 345;
	tmap		   *map = 0;

	for (int i = 0; i < count_iter; i++){
		map = tmap_putval(map, val);
		if (!map)
			return logerr(TEST_FAILED, "Something wrong with allocation");
	}
	tmap_fprintall(log_file(), map, true);

	if ( (cnt = tmap_getnodecnt(map)) != 1)
        return logderr(tmap_free(map), TEST_FAILED, "count of nodes = %d, but must be 1", cnt);

	if ( (cnt = tmap_gettotal(map)) != count_iter)
		return logderr(tmap_free(map), TEST_FAILED, "total count %d, but must be %d", cnt, count_iter);

	if ( (cnt = tmap_getcnt(map, val)) != count_iter)
        return logderr(tmap_free(map), TEST_FAILED, "count of %d == %d, but must be %d", val, cnt, count_iter);

	tmap_free(map);		// TODO: logdret(ACTION, retcode, fmt, ...)

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 3 ---------------------------------

// Several values test
static int
f3(void *restrict data)
{
    logenter("%p", data);

	long		 vals[] = {3L, 7L, 11L, 4L, 2L, 16L};
	int			 cnts[] = {40, 24,  44, 11, 33, 12};
	tmap 		*map = 0;

	logmsg("Fill map with data");
	for (int i = 0; i < COUNT(vals); i++){
		for (int j = 0; j < cnts[i]; j++)
			if ( !(map = tmap_putval(map, vals[i])) )
				return logret(TEST_FAILED, "Something wrong with allocation");

	}

	logmsg("Checking...");

	long  		total_sum = 0, sum;
	int 		total_cnt = COUNT(vals), cnt;

	cnt = tmap_getnodecnt(map);

	if (total_cnt != cnt)
		return logacterr(tmap_free(map), TEST_FAILED, "Expected count of nodes = %d, actual = %d", total_cnt, cnt);

	for (int i = 0; i < total_cnt; i++){
		int		curcnt = tmap_getcnt(map, vals[i]);
		if (cnts[i] != curcnt)
			return logacterr(tmap_free(map), TEST_FAILED, "count [%d] = %d, while cnts[i] = %d", i, curcnt, cnts[i]);
		total_sum += cnts[i];
	}

	logmsg("One by one done. Check total sum %ld", total_sum);

	sum = tmap_gettotal(map);
	if (total_sum != sum)
		return logacterr(tmap_free(map), TEST_FAILED, "Expected sum = %ld, actual sum = %ld", total_sum, sum);

	{
		logmsg("Checking for no data found of number 5");
		int 		cnt1;
		if ( (cnt1 = tmap_getcnt(map, 5L)) != 0)
			return logacterr(tmap_free(map), TEST_FAILED, "Value 5 is not presented in the map, but getcnt returns %d", cnt1);
	}

	tmap_free(map);
	return logret(TEST_PASSED, "done"); // TEST_FAILED
}


// -------------------------------------------------------------------
int
main(int argc, char *argv[])
{
    const char *logfilename = "tmap.log";

	if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false, 0, "Starting");

	testenginestd(
		testnew(.f2=f1, .num=1, .name = "Create and free test", .desc="Simple test.", .mandatory=true),
		testnew(.f2=f2, .num=2, .name = "Create several vals test", .desc="Several test.", .mandatory=true),
		testnew(.f2=f3, .num=3, .name = "Several values test", .desc="Several diferent values test.", .mandatory=true)
	);

	logclose("end...");
    return 0;
}


#endif /* TMAPTESTING */


