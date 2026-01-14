#ifndef CHECKER_H
#define CHECKER_H
// ---------------------------------------------------------------------------------
// --------------------------- Public Checker API ----------------------------------
// ---------------------------------------------------------------------------------

#include <stdio.h>
#include <stdbool.h>
#include "error.h"
#include "common.h"

// ----------- CONSTANTS AND GLOBALS ---------------

static const int						ERRNUM_INVARIANT_VIOLATION = 100;		// Error code for error.h

// ------------------- TYPES -----------------------

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

// -------------- ACCESS AND MODIFICATION ----------

int						inv_fprintf_int(FILE *restrict out, const char *restrict expr, int res, int val, const char *restrict msg, ...);

// ----------------- PRINTERS ----------------------

// ------------------ ETC. -------------------------

#ifndef NOINVARIANT

// TODO: add ACTION here
// basic invariant checker, raise SIGINT, but returns true/false
#define	inv(expr, msg, ...)\
	({ 	bool _INV_RES = (expr);\
	   	if (! _INV_RES){\
			inv_fprintf_int(stderr, #expr, 0, 0, msg, ##__VA_ARGS__);\
			userraiseint(0, ERRNUM_INVARIANT_VIOLATION, msg, ##__VA_ARGS__);\
	   	}\
	   	_INV_RES;\
	})

// should be generic, now onl int or unsigned???
#define	inv2(expr, val, msg, ...)\
	({	typeof(val)  _RES, _VAL = (val);\
		_RES = (expr);\
		if (_VAL != _RES){\
			inv_fprintf_int(stderr, #expr, _RES, _VAL, msg, ##__VA_ARGS__);\
			userraiseint(0, ERRNUM_INVARIANT_VIOLATION, msg, ##__VA_ARGS__);\
		}\
		_RES == _VAL;\
	})

#else /* NOINVARIANT */

#define inv(expr, msg, ...)\
	(expr)

#define inv2(expr, val, msg, ...)\
	(expr) == (val)

#endif /* NOINVARIANT */

// NEW checker strategy (22-jul-2024)
#define             check_arg(argcmax, msg, ...)\
    ( argc < (argcmax) ? fprintf(stderr, msg, ##__VA_ARGS__), 0 : 1)

#endif /* !CHECKER_H */
