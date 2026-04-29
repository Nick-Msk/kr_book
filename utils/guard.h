#ifndef _GUARG_H
#define _GUARG_H

#include "error.h"

#ifndef NODEBUG

#define     RGUARDRAISE userraiseint(ERR_GUARD_RAISE, "Guard rased %s:%s:%d\n", __FILE__, __func__, __LINE__ );

#define     GUARDK ({ static int _guard_val = 1000;             _guard_val-- != 0; })
#define     GUARDM ({ static int _guard_val = 1000000;          _guard_val-- != 0; })
#define     GUARDB ({ static int _guard_val = 1000000000;       _guard_val-- != 0; })
#define     GUARDL ({ static int _guard_val = 1000000000000;    _guard_val-- != 0; })

#define     RGUARDK ({ static int _guard_val = 1000;             if(_guard_val-- == 0) RGUARDRAISE; true; })
#define     RGUARDM ({ static int _guard_val = 1000000;          if(_guard_val-- == 0) RGUARDRAISE; true; })
#define     RGUARDB ({ static int _guard_val = 1000000000;       if(_guard_val-- == 0) RGUARDRAISE; true; })
#define     RGUARDL ({ static int _guard_val = 1000000000000;    if(_guard_val-- == 0) RGUARDRAISE; true; })

#else /* NODEBUG */

#define     GUARDK (true)
#define     GUARDM (true)
#define     GUARDB (true)
#define     GUARDL (true)

#define     RGUARDK (true)
#define     RGUARDM (true)
#define     RGUARDB (true)
#define     RGUARDL (true)

#endif /* !NODEBUG */

#define    SINGLETON(action)\
        ({ static bool _SINGLE_11 = true;\
            bool tmp = _SINGLE_11;\
          if (_SINGLE_11){\
                (action);\
                _SINGLE_11 = false;\
          }\
          tmp;\
        })

#define   MODEXEC(modval, action)\
        ({ static unsigned _MODVAL = 0;\
           bool res = (++_MODVAL % (modval) == 0);\
           if (res)\
               (action);\
            res;\
        })

#define   IFMOD(modval)\
            ({ static unsigned _MODVAL = 0;\
               ++_MODVAL % (modval) == 0; })

#define   MODEXECL(modval, action)\
        ({ static unsigned long _MODVALL = 0;\
           bool res = (++_MODVAL % (modval) == 0);\
           if (res)\
               (action);\
            res;\
        })

#define   IFMODL(modval)\
            ({ static unsigned long _MODVALL = 0;\
               ++_MODVAL % (modval) == 0; })

#endif /* !_GUARG_H */
