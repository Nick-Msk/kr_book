#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

long		calc_max_long(void);
short		calc_max_short(void);
float		calc_max_float(float, float, float);


int			main(int argc, const char *argv[]){
	
	float 	init_mul = 100.0, epsilon = 1.000001, sqrt_pow = 0.5;
	
	short	max_short = calc_max_short();
	long	max_long = calc_max_long();

	if (argc > 1)
		init_mul = atof(argv[1]);
	if (argc > 2)
		sqrt_pow = atof(argv[2]);
	if (argc > 3)
		epsilon = atof(argv[3]);

	printf("run with mul = %f, sqrt = %f, epsilon = %f\n", init_mul, sqrt_pow, epsilon); 
	float	max_float = calc_max_float(init_mul, sqrt_pow, epsilon);

	printf("calculated max short %d, long = %ld\n", max_short, max_long);
	printf("SHRT_MAX = %d, LONG_MAX = %ld\n", SHRT_MAX, LONG_MAX);

	printf("calculated float = %e, FLT_MAX = %e, diff = %e, diff = %f%%\n", 
			max_float, FLT_MAX, fabs(max_float - FLT_MAX), 100 * fabs(max_float - FLT_MAX) / FLT_MAX);

	return 0;
}

// just for test alrogithm
short		calc_max_short(void){
	short		val = 1, res, prev;

	// calc max 2^n > 0, but 2^n+1 <0 
	while (val > 0){
		res = val;
		val *= 2;
	}
	// calc descending from res
	val = res;
	int		i = 1;
	while ((val >> i) > 0){
		prev = res;
		res += val >> i;
		//printf("val = %d, res = %d, i = %d, >> = %d\n", val, res, i, val >> i);
		if (res <= 0)	// overflow => restore
			res = prev;
		 i++;
	}
	return res;
}

// version 0.1 
long        calc_max_long(void){
	long		val = 1, res, prev;

	while (val > 0){
		res = val;
		val *= 2;
	}
	val = res;
	while (val > 0){
		prev = res;
		val >>= 1;
		res += val;
		if (res <= 0)
			res = prev;		// or res -= val
	}
	return res;
}

float       calc_max_float(float init_mul, float sqrt_mul, float epsilon_mul){
	
	float		res = 1.0, init_val = 1.0;
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
		printf("mul = %1.8f, cnt = %d, res = %e\n", init_mul, cnt, res);
	}

	printf("Total cnt = %d\n", cnt);
	return res;
}


