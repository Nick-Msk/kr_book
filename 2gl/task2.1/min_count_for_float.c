#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

static int                  g_firstrun = 1;

typedef struct inf_float{
    int     cnt;            // count if iteration for calculate
    float   val;            // calculated value
    float   rel_diff;      //  fabs(FLT_MAX - val) / FLT_MAX
    int     flags;
} inf_float;

#define                     FLAG_OK             0
#define                     FLAG_UNABLE_CACL    1
#define                     FOUND_MAX           512
#define                     THRESHOLD           0.000005
#define                     EPSILON_MUL         1.0000001

inf_float			calc_max_float(float init_mul, float sqrt_mul, float epsilon_mul, float percent_threshold);


int			        main(int argc, const char *argv[]){

    // LOAD DEFAULT RUN PARAMS
	float 	min_mul             = 10.0;
    float   max_mul             = 2000000.0;
    float   step_mul            = 1.0;
	float 	epsilon             = EPSILON_MUL;	// constant
    float   threshold           = THRESHOLD; // to check! 
	float  	min_sqrt_pow        = 0.00001;
    float   max_sqrt_pow        = 0.1;
    float   step_sqrt_pow       = 0.00001;
	int		mincnt              = INT_MAX;
    int     mins;
    long    stop_cnt            = 1000000000; // just to avoid endless cycle
    long    totalrun            = 0;    // total count of run
    long    rejrun              = 0;    // rejected runs

	float	found_mul[FOUND_MAX], found_sqrt_pow[FOUND_MAX], rel_diff[FOUND_MAX]; // no need to found_epsilon

    // TODO: rework via par=val method
	if (argc > 1)
		min_mul = atof(argv[1]);
	if (argc > 2)
		max_mul = atof(argv[2]);
	if (argc > 3)
		step_mul = atof(argv[3]);
	if (argc > 4)
		min_sqrt_pow = atof(argv[4]);
	if (argc > 5)
		max_sqrt_pow = atof(argv[5]);
	if (argc > 6)
		step_sqrt_pow = atof(argv[6]);
	if (argc > 7)
		epsilon = atof(argv[7]);
    // add percent_threshold here

    /*
    // TODO: reword via check_input() with all checking inside
	if (min_sqrt_pow <= 0 || min_sqrt_pow >= 1 ||
		max_sqrt_pow <= 0 || max_sqrt_pow >= 1)
	{
		fprintf(stderr, "min(max)_sqrt_pow = %f(%f) must be between 0 and 1\n", min_sqrt_pow, max_sqrt_pow);
		return 1;
	}
	if (epsilon > 1.5 || epsilon <= 1){
		fprintf(stderr, "epsilon (%f) must be between 1 and 1.5\n", epsilon);
		return 2;
	} */

	printf("RUN with: mul [%.0f : %.0f by %.2f]\nsqrt_pow [%.8f : %.8f by %.8f]\nepsilon = %.8f, threshold = %e\n", 
			min_mul, max_mul, step_mul, 1/min_sqrt_pow, 1/max_sqrt_pow, step_sqrt_pow, epsilon, threshold);
    // TODO: move that into launcher()
    float mul, sqrt_pow;
	for (mul = min_mul; mul <= max_mul + FLT_EPSILON; mul += step_mul){

		for (sqrt_pow = min_sqrt_pow; sqrt_pow <= max_sqrt_pow + FLT_EPSILON && totalrun++ < stop_cnt; sqrt_pow += step_sqrt_pow){
            inf_float res = calc_max_float(mul, sqrt_pow, epsilon, threshold);
            if (res.flags == FLAG_OK){
			    if (res.cnt < mincnt){
				    mins = 0;
				    mincnt = res.cnt;
				    found_mul[mins] = mul;
				    found_sqrt_pow[mins] = sqrt_pow;
                    rel_diff[mins] = res.rel_diff;
			    } else if (res.cnt == mincnt){
				    mins++;		// one more minimum
                    if (mins < FOUND_MAX){
                        found_mul[mins] = mul;
                        found_sqrt_pow[mins] = sqrt_pow;
                        rel_diff[mins] = res.rel_diff;
                    }
                }
            } else {
                if (g_firstrun){
                    fprintf(stderr,  "RUN with: mul [%.0f : %.0f by %.2f]\nsqrt_pow [%.8f : %.8f by %.8f]\nepsilon = %.8f, threshold = %.8f\n", 
                         min_mul, max_mul, step_mul, 1/min_sqrt_pow, 1/max_sqrt_pow, step_sqrt_pow, epsilon, threshold);
                    g_firstrun = 0;
                }
                fprintf(stderr, "Unable: sqrt_pow %.8f, mul %.2f, threshold %e, epsilon %.8f\n"
                        , 1/sqrt_pow, mul, threshold, epsilon);
                rejrun++;
            }
		}
       // printf("min_mul = %f, mincnt = %d\n", mul, mincnt);
	}

    printf("RUNS/REJ (%ld/%ld): min count = %d (%d)\n", 
            totalrun, rejrun, mincnt, ++mins);

    for (int i = 0; i < mins && i < FOUND_MAX; i++)
	    printf("Min: mul = %.2f, pow = %.8f, rel diff = %e\n",
		    found_mul[i], 1/found_sqrt_pow[i], rel_diff[i]);

/*	printf("calculated float = %e, FLT_MAX = %e, diff = %e, diff = %f%%\n", 
			max_float, FLT_MAX, fabs(max_float - FLT_MAX), 100 * fabs(max_float - FLT_MAX) / FLT_MAX);*/

	return 0;
}


// TODO: double!!!
inf_float	       calc_max_float(float init_mul, float sqrt_mul, float epsilon_mul, float threshold){

    //printf("calc_max_float: mul %f, pow %f\n", init_mul, sqrt_mul);
    inf_float   r;
	float	    res = 1.0, init_val = 1.0;
	int			cnt = 0;

	while (init_mul >= epsilon_mul){
		// cycle internal: using init_mul
		while (!isinf(init_val)){
			res = init_val;
			init_val *= init_mul;
			cnt++;
			//printf("%d - %e\n", i++, res);
		}
		init_val = res;
		init_mul = pow(init_mul, sqrt_mul);
		//printf("mul = %1.8f, cnt = %d, res = %e\n", init_mul, cnt, res);
	}
    // fill the output
    r.cnt = cnt;
    r.val = (float)res;
    r.rel_diff = fabs(FLT_MAX - r.val) / FLT_MAX;
    if (r.rel_diff < threshold)
        r.flags = FLAG_OK;
    else 
        r.flags = FLAG_UNABLE_CACL;
	//printf("Exec: Total cnt = %d\n", cnt);
	return r;
}


