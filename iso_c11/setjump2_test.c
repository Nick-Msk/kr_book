#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <bool.h>
#include <unistd.h>


// wrapper, error.h
#define errenv (*err_getexception_info())

// globals (should be in error.c)

typedef struct ExceptionData
{
	jmp_buf		env;
	bool		init_flag;
} ExceptionData;

// global from error.c
static		ExceptionData	g_env;

// from error.h
static inline ExceptionData*
err_getexception_info()
{
	return &g_env;		// access to global
}

// from error.c
static void
err_default_handler(int sig)
{
	printf("HANDLER %d\nJumping out from here!\n", sig);
	if (!errenv.init_flag)
	{
		fprintf(stderr, "Env buffer is empty, terminating SIGSTOP\n");
		raise(SIGSTOP);
	}
	longjmp(errenv.env, 111);
}

// from error.c
bool
err_sethandler(sig_t handler)
{
	if (!handler)
		handler = err_default_handler;

	if (signal(SIGINT, handler) == SIG_ERR)
	{
		perror("Unable to setup err_default_handler for SIGINT\n");
		return false;
	}
	printf("err_default_handler is activated\n");
	return true;
}

#define try() ({\
	if (errenv.init_flag)\
	{\
		fprintf(stderr, "Env buf is alredy activated\n");\
		return 9999;\
	}\
	int		res = setjmp(errenv.env);\
    if (res == 0)\
	{\
		printf("Activating env buffer\n");\
		errenv.init_flag = true;\
	} else\
	{\
		printf("Returning from handler now! (res = %d)\nClean env buffer\n", res);\
		errenv.init_flag = false;\
	}\
	res;\
})

// -----------------------------------------------

// main module
void simple_code(void);

int
main(int argc, char *argv[])
{

	// err-sethandler(f)
	// exec set up handler from error.c
	if (!err_sethandler(0))		// set up default error handling
	{
		fprintf(stderr, "Unable to setup default handler\n");
		return 1;
	}
	int		go_next = 1;

	while (go_next == 1)
	{
		// make try macro then
		int t = try();
		if (!t)
		{
			printf("Normal code here!\n");

			simple_code();
		}
		if (t != 0) {
			printf("EXCEPTION!!!\n");
		}
		printf("Go next loop (1/0)? (t=%d)", t);
		sleep(1);
		scanf("%d", &go_next);
	}


	printf("Quit now\n");
	return 0;
}


void simple_code(void)
{
	printf("Hello, world!\n");

	// something wrong here!
	printf("Something goes wrong...SIGINT\n");
	raise(SIGINT);		// userraisesig!

	printf("Shouldn't be there!\nBYu!\n");
}


