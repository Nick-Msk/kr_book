#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <math.h>

int			calc_max_int(void);
long		calc_max_long(void);
float		calc_max_float(float, float);

int			main(void){

	printf("MAX: int = %d, long = %ld, char = %d, short %hd\n", INT_MAX, LONG_MAX, CHAR_MAX, (short) SHRT_MAX);
	printf("MAX Unsigned: int = %u, long = %lu, char = %u, short %hu\n", UINT_MAX, ULONG_MAX, UCHAR_MAX, (unsigned short) USHRT_MAX);
	printf("MIN: int = %d, long = %ld, char = %d, short %hd\n", INT_MIN, LONG_MIN, CHAR_MIN, (short) SHRT_MIN);
	
	printf("MAX double = %e, fload %e\n", DBL_MAX, FLT_MAX);
	printf("MIN double = %e, fload %e\n", DBL_MIN, FLT_MIN);
	printf("DBL_DIG = %d, FLT_DIG = %d\n", DBL_DIG, FLT_DIG);
	printf("DBL_MANT_DIG = %d, FLT_MANT_DIG = %d\n", DBL_MANT_DIG, FLT_MANT_DIG);
	printf("DBL_MIN_EXP = %d, FLT_MIN_EXP = %d\n", DBL_MIN_EXP, FLT_MIN_EXP);
	printf("DBL_MIN_10_EXP = %d, FLT_MIN_10_EXP = %d\n", DBL_MIN_10_EXP, FLT_MIN_10_EXP);
	printf("DBL_MAX_EXP = %d, FLT_mAX_EXP = %d\n", DBL_MAX_EXP, FLT_MAX_EXP);
	printf("DBL_MAX_10_EXP = %d, FLT_mAX_10_EXP = %d\n", DBL_MAX_10_EXP, FLT_MAX_10_EXP);
	printf("DBL_EPSILON = %e, FLT_EPSILON = %e\n", DBL_EPSILON, FLT_EPSILON);	

	printf("FLT_EVAL_METHOD = %d\n" , FLT_EVAL_METHOD);
	
	// determine max/min for int
	// and max for unsigned int
	
	/*
	int		max_int = calc_max_int();
	long	max_long = calc_max_long();

	printf("calculated max int = %d, long = %ld\n", max_int, max_long);
	*/


	float max_float = calc_max_float(10000.0, 2.0);

	printf("calculated float = %e, FLT_MAX = %e, diff = %e, diff%% = %f\n", 
			max_float, FLT_MAX, fabs(max_float - FLT_MAX), 100 * fabs(max_float - FLT_MAX) / FLT_MAX);
	
	return 0;
}

int         calc_max_int(void){
	int  i = 1, j;
	while (i > 0){
		j = i;	// prev
		i++;
	}
	return j;
}

// TODO: not sure, probably there is a better solution
long		calc_max_long(void){
	long	i = 2, j;
	while (i - 1 > 0){
		j = i - 1;
		i <<= 1;
		//i *= 2;
		//printf("%ld\n", i);
	}
	return j;
}


// TODO: calculate max using isnan() or isinf() function! A bit later 
float       calc_max_float(float init_val, float init_mul){
	int			i = 0;		// iter count		
	float		res = 1.0;

	// cycly 1: using init_mul
	while (!isinf(init_val)){
		res = init_val;
		init_val *= init_mul;
		printf("%d - %e\n", i++, res);
	}

	return res;
}


