#include <limits.h>
#include <float.h>
#include <stdio.h>

static int             calc_max_int(void);
static long            calc_max_long(void);
static float           calc_max_float(float, float);

int                     main(void){

        printf("ver: \n", __STDC_VERSION__);
        printf("Integers\n");
        printf("MAX: int = %d, long = %ld, long long = %lld, char = %d, short %hd\n", INT_MAX, LONG_MAX, LLONG_MAX, CHAR_MAX, (short) SHRT_MAX);
        printf("MAX Unsigned: int = %u, long = %lu, longlong = %llu, char = %u, short %hu\n", UINT_MAX, ULONG_MAX, ULLONG_MAX, UCHAR_MAX, (unsigned short) USHRT_MAX);
        printf("MIN: int = %d, long = %ld, char = %d, short %hd\n", INT_MIN, LONG_MIN, CHAR_MIN, (short) SHRT_MIN);
        // printf("WIDTH: int = %d, long = %ld, char = %d, short = %d, long long=%d\n", INT_WIDTH, LONG_WIDTH, CHAR_WIDTH, SHRT_WIDTH, ULONG_WIDTH);


        printf("Floating point\n");
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

        int     max_int = calc_max_int();
        long    max_long = calc_max_long();

        printf("calculated max int = %d, long = %ld\n", max_int, max_long);

        /*
        float max_float = calc_max_float(10000.0, 2.0);

        printf("calculated float = %e, FLT_MAX = %e, diff = %e, diff%% = %f\n", 
                        max_float, FLT_MAX, fabs(max_float - FLT_MAX), 100 * fabs(max_float - FLT_MAX) / FLT_MAX);
        */
        return 0;
}


static int             calc_max_int(void){
    int n = 1, maxint = 1, maxiter = 100;
    while (maxint <= n && maxiter-- > 0){
        n = n << 1 | n;
        if (maxint < n)
            maxint = n;
        //printf("n = %d\n", n);
    }
    return maxint;
}

static long            calc_max_long(void){
    long n = 1, maxlong = 1;
    while (maxlong <= n){
        n = n << 1 | n;
        if (maxlong < n)
            maxlong = n;
    }
    return maxlong;
}
