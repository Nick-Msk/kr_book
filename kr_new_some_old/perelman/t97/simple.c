#include <stdio.h>
#include <stdlib.h>
#include <log.h>
#include <error.h>
#include <bool.h>			// for bool_str()

bool		checkval(int val);
int			checkinterval(int from, int to, bool print);

// Divider checker (), usage: t97 <from> <to>, default [10 100]
// pseudocode is used for now for some cases

int			main(int argc, char *argv[]){
	int			from = 10, to = 100, cnt;
	bool		isprint;

	logsimpleinit("");

	if (argc > 1)
		from = atoi(argv[1]);
	if (argc > 2)
		to = atoi(argv[2]);

	isprint = to - from < 10000 ? true : false;

	// print exec data
	printf("exec interval [%d - %d]\n", from, to);		// TODO: logact here?

	cnt = checkinterval(from, to, isprint);

	printf("Total %d combination(s) are/is passed\n", cnt);
	logclose("end with %d", cnt);
}

// check if val is conform to the predicate
bool		checkval(int val){
	int			sum = 0, tmp = val;

	// calculate sum of digits, note: it can be done in simple way but summing and divizion
	while (tmp){
		sum += tmp % 10;
		tmp /= 10;
	}
	logauto(sum);
	// check condition
	bool		res = sum * sum == val;
	return logsimpleret(res, "res = %s", bool_str(res));
}

int         checkinterval(int from, int to, bool print){
	logenter("from = %d, to = %d, print = %s", from, to, bool_str(print));
	int			cnt = 0;

	for (int i = from; i < to; i++){
		if (checkval(i)){
			cnt++;
			if (print)
				printf("\t%d\n", i);
		}
	}
	return logret(cnt, "found %d", cnt);
}

