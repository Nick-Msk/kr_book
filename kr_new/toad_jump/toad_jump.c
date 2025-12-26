#include <stdio.h>
#include <stdlib.h>
#include <bool.h>	// ? <bool.h>
#include <log.h>
#include "tmap.h"	// calcalator
#include <iarray.h>

// ------------------------------ Common sequence interface -------------------------------

// Arithmetic sequence generator   x(n+1) = x(n) + 1
static int			gen_ar_prog_1(int  val){
	return val + 1;
}

// Arithmetic sequence generator   x(n+1) = x(n) + 2
static int          gen_ar_prog_2(int  val){
    return val + 2;
}

static int			get_next_value(bool reset, int (*f)(int)){
	static int			current = 0;		// state variable
	if (reset){
		current = 0;
		logsimple("counter reset");	// TODO:  нельзя ли убрать действие в logsimple по аналогии с logsimpleret?
	}
	return logsimpleret(current = f(current), "%d", current);;	// indirect call
}


// -----------------------------------------------------------------------------------------

// ------------------------------- Array interface -----------------------------------------

// Calculate sum of all elements in array, TODO: think about SEPARATE file for integer array
static long			calc_array(const iarray dt){
	long 		res = 0;
	// internal counter for logging purpose
	static long iter_count = 0;
	++iter_count;

	for (int i = 0; i < dt.cnt; i++)
		res += dt.arr[i];
	return logsimpleret(res, "Sum [%ld], interation number [%ld]", res, iter_count);
}

static void			fprint_array(FILE *restrict out, const iarray dt){
	logsimple("Printing %p(%d)", dt.arr, dt.cnt);
	for (int i = 0; i < dt.cnt; i++)
		fprintf(out, "%d ", dt.arr[i]);
	fprintf(out, "\n");
}

static inline void  print_array(const iarray dt){
    fprint_array(stdout, dt);
}

// ------------------------------- Calculation interface ---- ------------------------------

tmap			*iterate_sum(tmap *restrict map, iarray dt, int pos, long test_value, bool print){
	logenter("iter sz = %d, pos = %d", dt.cnt, pos);

	if (pos == dt.cnt){
		// end of iteration - check result
		long 	res = calc_array(dt);
		if (res == test_value){
			printf("Found!: ");
			if (print)
				print_array(dt);
			else
				putchar('\n');
		}
	//	print_array(arr, sz);
	//	printf("res=%ld\n", res); 		// TODO: remove
		return logret(tmap_putval(map, res), "res = %ld", res);
	}
	// checking for '+' on pos
	map = iterate_sum(map, dt, pos + 1, test_value, print);
	// checking for '-' on pos
	dt.arr[pos] = -dt.arr[pos];
	map = iterate_sum(map, dt, pos + 1, test_value, print);
	return logret(map, "end of checking pos = %d", pos);
}

// entry point
tmap			*calc_sums(iarray dt, long test_value, bool print){
	logenter("sz = %d, test value = %ld, print = %s", dt.cnt, test_value, bool_str(print));

	tmap		*res = 0;
	// start iteration from position 0
	res = iterate_sum(res, dt, 0, test_value, print);

	return logret(res, "res %p, count of nodes %d", res, tmap_getnodecnt(res));
}

// -----------------------------------------------------------------------------------------

static iarray			generate_values(int total, int (*f)(int)){
	logenter("%d", total);

	iarray		dt = iacreate(total);
	if (!iavaluable(dt))
		return logerr(dt, "Unable to allocate %d int's", total);

	for (int i = 0; i < dt.cnt; i++)
		dt.arr[i] = get_next_value(false, f);

	// to check in logs
	fprint_array(logfile, dt);

	return logret(dt, "Generated");
}

// General process method
static int			process(int step_cnt, long test_value, bool print, const char *csv_name, bool printdelta){
	logenter("cnt %d test value %ld, cvs_name [%s]", step_cnt, test_value, csv_name);

	FILE 	*csv = 0;

	if (csv_name && *csv_name != '\0' && (csv = fopen(csv_name, "w")) == 0)
		return logerr(-1, "Unable to open file %s for writing", csv_name);

	iarray	dt = generate_values(step_cnt, gen_ar_prog_1);
	if (!iavaluable(dt))
		return logerr(-1, "Unable to generate");

	// get the weitghted tree of output

	tmap	*lv = calc_sums(dt, test_value, print);
	if (!lv){
		iafree(dt);
		return logerr(-1, "Unable to calculate result");
	}
	tmap_printall(lv, printdelta);

	printf("Count of distinct elements %d\n", tmap_getnodecnt(lv));
	printf("Total sum of counts - %d\n", tmap_gettotal(lv));

	if (csv){
		tmap_fprintcsv(csv, lv);
		fclose(csv);
	}

	int 	cnt = tmap_getcnt(lv, test_value);

	iafree(dt);
	tmap_free(lv);
	return logret(cnt, "Found %d", cnt);
}


int					main(int argc, char *argv[]){

	bool		print;
	int			count, match_count;
	long 		test_value;
	char	    csv_name[1000] = "";

	loginit("toad_jump.log", false, 0, "Starting with %d", argc);

	if (argc < 3){
		fprintf(stderr, "Usage: %s <count of steps> <final value>\n", *argv);
		return logerr(1, "Bad params");
	}

	count 	   = atoi(argv[1]);
	test_value = atol(argv[2]);

	if (argc >= 4)
        snprintf(csv_name, sizeof csv_name - 1, "%s_%d.csv", argv[3], count);

	if (count <= 0 || test_value < 0){
		fprintf(stderr, "Inconsistent input: count %d and test_value %ld must not be negative)\n", count, test_value);
		return logerr(2, "Inconsistent input");
	}

	print = test_value < 50? true : false;
	printf("Run with count %d, test value %ld, print %s, cvs_name [%s]\n", count, test_value, bool_str(print), csv_name);

	if ((match_count = process(count, test_value, print, csv_name, true)) < 0){
		fprintf(stderr, "Process failed\n");
		return logerr(3, "Process failed");
	}
	else if (match_count == 0)
		printf("Unbale to find matching %ld for count of steps %d\n", test_value, count);
	else
		printf("Found %d matches for %ld for count of steps %d\n", match_count, test_value, count);

	return logret(0, "Done");
}

