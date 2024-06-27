#include <stdio.h>
#include <limits.h>
#include <common.h>
#include <float.h>

int		main(void){
	printf("From limits.h\n\n");
	typeprint(CHAR_BIT);
	typeprint(SCHAR_MIN);
	typeprint(SCHAR_MAX);
	typeprint(UCHAR_MAX);
	typeprint(CHAR_MIN);
	typeprint(CHAR_MAX);
	typeprint(MB_LEN_MAX);

	typeprint(SHRT_MIN);
	typeprint(SHRT_MAX);
	typeprint(USHRT_MAX);

	typeprint(INT_MIN);
	typeprint(INT_MAX);
	typeprint(UINT_MAX);

	typeprint(LONG_MIN);
	typeprint(LONG_MAX);
	typeprint(ULONG_MAX);

	typeprint(LLONG_MIN);
	typeprint(LLONG_MAX);
	typeprint(ULLONG_MAX);

	printf("\nFrom stdio.h\n\n");

	typeprint(BUFSIZ);

	typeprint(FOPEN_MAX);
	typeprint(FILENAME_MAX);
	typeprint(L_tmpnam);
	typeprint(TMP_MAX);

	printf("\nFrom stdlib.h\n\n");

	typeprint(EXIT_FAILURE);
	typeprint(EXIT_SUCCESS);
	typeprint(RAND_MAX);
	typeprint(MB_CUR_MAX);

	printf("\nFrom float.h\n\n");
	typeprint(FLT_RADIX);
	typeprint(FLT_ROUNDS);
	typeprint(FLT_DIG);
	typeprint(FLT_EPSILON);		printf("\t\t%g\n", FLT_EPSILON);
	typeprint(FLT_MANT_DIG);
	typeprint(FLT_MAX);
	typeprint(FLT_MAX_EXP);
	typeprint(FLT_MIN);			printf("\t\t%g\n", FLT_MIN);
	typeprint(FLT_MIN_EXP);
	printf("\n");

	typeprint(DBL_DIG);
	typeprint(DBL_EPSILON);		printf("\t\t%g\n", DBL_EPSILON);
	typeprint(DBL_MANT_DIG);
	typeprint(DBL_MAX);
	typeprint(DBL_MAX_EXP);
	typeprint(DBL_MIN);			printf("\t\t%g\n", DBL_MIN);
	typeprint(DBL_MIN_EXP);
}
