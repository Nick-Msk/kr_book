#include <stdio.h>
#include <time.h>
#include <unistd.h>
//#include <bool.h>

#define false   0
#define true    1

static int
time_check(struct timespec *t, int init);

int
main(int argc, char *argv[]){

	struct timespec beg;
	long			N = 500000;

	if (time_check(&beg, true) == 0){
		fprintf(stderr, "Unable to get timespec");
		return 1;
	}
	// do something useless
	{
        long t = 0;
		for (long i = 0; i < N; i++)
			for (long j = 0; j < N; j++)
				    t++;
		sleep(1);
	}

	if (time_check(&beg, false) == 0){
		fprintf(stderr, "Unable to get timespec");
		return 1;
	}

	return 0;
}

static int
time_check(struct timespec *t, int init){
	if (init)
		return timespec_get(t, TIME_UTC);
	else {
		struct timespec tmp;
		if (!timespec_get(&tmp, TIME_UTC))
			return 0;
		printf("ELAPSED: %ld sec %ld msec\n", tmp.tv_sec - t->tv_sec, (tmp.tv_nsec - t->tv_nsec) / 1000000);
		return TIME_UTC;
	}
}

