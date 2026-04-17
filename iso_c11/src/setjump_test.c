// Testing of setjump / longjump

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

jmp_buf		g_env;

void f1(void);

void f2(int);

// TODO: refactor to use longjmp as exception raise
int		main(int argc, char *argv[]){

	printf("Start, input 7 is incorrect, \n");

	int		val = 4, exception_flag;	// default

	if (argc > 1)
		val = atoi(argv[1]);

	if ( (exception_flag = setjmp(g_env)) == 0){
		printf("Direct call, exec f1: ");

		// f1();
		f2(val);
	} else {
		printf("return : %d\n", exception_flag);
	}

	printf("Done\n");
}


void f1(void){
	printf("Bla bla bla from %s\n", __func__);

	printf("Exception now...\n");

	longjmp(g_env, 5);
}

void f2(int n)
{
	if (n != 7)
		printf("Everything ok, n == %d\n", n);
	else {
		// exception here!!!
		printf("Excetion!!\n");
		longjmp(g_env, 10);	// userraiseXXX
	}
}

