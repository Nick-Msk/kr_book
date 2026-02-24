#ifndef _ERROR_H
#define _ERROR_H
// ---------------------------------------------------------------------------------
// --------------------------- Public Error API ------------------------------------
// ---------------------------------------------------------------------------------

#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <setjmp.h>

// ----------- CONSTANTS AND GLOBALS ---------------

static const int                        ERROR_MESSAGE_MAX_LENGTH = 256;

// ------------------- TYPES -----------------------

typedef enum {ERR_USER = 1, ERR_SYS} ErrorType;

// TODO: подумать, можно ли спрятать это в error.c...
typedef struct ExceptionData
{
    jmp_buf                             env;
    bool                                init_flag;      // true when env is filled with data
} ExceptionData;

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

// -------------- ACCESS AND MODIFICATION ----------

// general raise function
extern void
err_raise(ErrorType tp, int raise, int errcode, const char *msg, ...);

extern void
err_clean(bool force);

extern ExceptionData*
err_getexception_info();

extern bool
err_sethandler(sig_t handler);

static inline bool
err_resetenv()
{
	return err_getexception_info()->init_flag = false;
}

// ----------------- PRINTERS ----------------------

// printstacktrace
extern int
err_fprintstacktrace(FILE *out);

static inline int
err_printstacktrace(void)
{
	return err_fprintstacktrace(stderr);
}

static inline const char *
sig_str(int signal)
{
    switch(signal){
        case SIGHUP     : return "SIGHUP";
        case SIGINT     : return "SIGINT";
        case SIGQUIT    : return "SIGQUIT";
        case SIGILL     : return "SIGILL";
        case SIGTRAP    : return "SIGTRAP";
        case SIGABRT    : return "SIGABRT";
        case SIGEMT     : return "SIGEMT";
        case SIGFPE     : return "SIGFPE";
        case SIGKILL    : return "SIGKILL";
        case SIGBUS     : return "SIGBUS";
        case SIGSEGV    : return "SIGSEGV";
        case SIGSYS     : return "SIGSYS";
        case SIGPIPE    : return "SIGPIPE";
        case SIGALRM    : return "SIGALRM";
        case SIGTERM    : return "SIGTERM";
        case SIGURG     : return "SIGURG";
        case SIGSTOP    : return "SIGSTOP";
        case SIGTSTP    : return "SIGTSTP";
        case SIGCONT    : return "SIGCONT";
        case SIGCHLD    : return "SIGCHLD";
        case SIGTTIN    : return "SIGTTIN";
        case SIGTTOU    : return "SIGTTOU";
        case SIGIO      : return "SIGIO";
        case SIGXCPU    : return "SIGXCPU";
        case SIGXFSZ    : return "SIGXFSZ";
        case SIGVTALRM  : return "SIGVTALRM";
        case SIGPROF    : return "SIGPROF";
        case SIGWINCH   : return "SIGWINCH";
        case SIGINFO    : return "SIGINFO";
        case SIGUSR1    : return "SIGUSR1";
        case SIGUSR2    : return "SIGUSR2";
        default         : return "Unknown sig";
    }
}

static inline const char *
sig_str_desc(int signal)
{
    switch(signal){
        case SIGHUP     : return "terminal line hangup";
        case SIGINT     : return "interrupt program";
        case SIGQUIT    : return "quit program";
        case SIGILL     : return "illegal instruction";
        case SIGTRAP    : return "trace trap";
        case SIGABRT    : return "abort program (formerly SIGIOT)";
        case SIGEMT     : return "emulate instruction executed";
        case SIGFPE     : return "floating-point exception";
        case SIGKILL    : return "kill program";
        case SIGBUS     : return "bus error";
        case SIGSEGV    : return "segmentation violation";
        case SIGSYS     : return "non-existent system call invoked";
        case SIGPIPE    : return "write on a pipe with no reader";
        case SIGALRM    : return "real-time timer expired";
        case SIGTERM    : return "software termination signal";
        case SIGURG     : return "urgent condition present on socket";
        case SIGSTOP    : return "stop (cannot be caught or ignored)";
        case SIGTSTP    : return "stop signal generated from keyboard";
        case SIGCONT    : return "continue after stop";
        case SIGCHLD    : return "child status has changed";
        case SIGTTIN    : return "background read attempted from control terminal";
        case SIGTTOU    : return "background write attempted to control terminal";
        case SIGIO      : return "I/O is possible on a descriptor";
        case SIGXCPU    : return "cpu time limit exceeded";
        case SIGXFSZ    : return "file size limit exceeded";
        case SIGVTALRM  : return "virtual time alarm";
        case SIGPROF    : return "profiling timer alarm";
        case SIGWINCH   : return "Window size change";
        case SIGINFO    : return "status request from keyboard";
        case SIGUSR1    : return "User defined signal 1";
        case SIGUSR2    : return "User defined signal 2";
        default         : return "";
    }
}


// ------------------ ETC. -------------------------

// setjmp/longjmp API
#define errenv 														(*err_getexception_info())

#define errsethandler()												err_sethandler(0)

// TODO: #define try try()

// NOT sure aboud 9999, probably better to raise SIGTERM/SIGSTOP
#define try() ({\
	int res;\
    if (errenv.init_flag)\
        logsimpleact(res = 9999, "Env buf is alredy activated");\
	else {\
    	res = setjmp(errenv.env);\
    	if (res == 0) logsimpleact(errenv.init_flag = true, "Activating env buffer");\
    	else logsimpleact(errenv.init_flag = false, "Returning from handler now! (res = %d) Clean env buffer", res);\
	}\
    res;\
})


// TODO: this if it is possible to avoid typeof (unspec)
// TODO: why ACTION in log module
#define	_generalraiseactsig(retcode, TYPE, ACTION, sig, errcode, msg, ...)	({ 	typeof(retcode) _RETCODE = retcode;\
																				logsimpleactret(ACTION, _RETCODE, msg, ##__VA_ARGS__);\
																				err_raise(ERR_USER, sig, errcode, msg, ##__VA_ARGS__);\
																				_RETCODE;\
																			})

// USER block
// user with ACTION and signal (common)
#define _userraiseactsig(retcode, ACTION, sig, errcode, msg, ...)	_generalraiseactsig(retcode, ERR_USER, ACTION, sig	, errcode, msg, ##__VA_ARGS__)

// user with ACTION w/o exception
#define	userraiseact(retcode, ACTION, errcode, msg, ...)			_userraiseactsig(retcode, ACTION, 0					, errcode, msg, ##__VA_ARGS__)

// user with ACTION with exception (interrupt)
#define userraiseactint(ACTION, errcode, msg, ...)         			_userraiseactsig(0, ACTION, SIGINT					, errcode, msg, ##__VA_ARGS__)

// user with signal (common)
#define _userraisesig(retcode, sig, errcode, msg, ...)				_userraiseactsig(retcode, , sig						, errcode, msg, ##__VA_ARGS__)

// user error w/o exception
#define userraise(retcode, errcode, msg, ...)          				_userraisesig(retcode, 0							, errcode, msg, ##__VA_ARGS__)

// user with exception (interrupt)
#define	userraiseint(errcode, msg, ...)								_userraisesig(0, SIGINT								, errcode, msg, ##__VA_ARGS__)


// SYSTEM block
// system, with action and signal (common)
#define	_sysraiseactsig(retcode, ACTION, sig, msg, ...)      		_generalraiseactsig(retcode, ERR_SYS, ACTION, sig, 0, msg, ##__VA_ARGS__)

// system with ACTION w/o expection
#define	sysraiseact(retcode, ACTION, msg, ...)            			_sysraiseactsig(retcode, ACTION, 0					, msg, ##__VA_ARGS__)

// system with ACTION with exception (interrupt)
#define	sysraiseactint(ACTION, msg, ...)							_sysraiseactsig(0, ACTION, SIGINT					, msg, ##__VA_ARGS__)

// system with signal (common)
#define	_sysraisesig(retcode, sig, msg, ...)               			_sysraiseactsig(retcode, , sig						, msg, ##__VA_ARGS__)

// system w/o exception
#define sysraise(retcode, msg, ...)									_sysraisesig(retcode, 0								, msg, ##__VA_ARGS__)

// system with exception (interrupt)
#define sysraiseint(msg, ...)										_sysraisesig(0, SIGINT								, msg, ##__VA_ARGS__)

#endif /* !_ERROR_H */

