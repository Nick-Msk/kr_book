#ifndef _GUARG_H
#define _GUARG_H

#define     GUARDK ({ static int _guard_val = 1000;             _guard_val-- != 0; })
#define     GUARDM ({ static int _guard_val = 1000000;          _guard_val-- != 0; })
#define     GUARDB ({ static int _guard_val = 1000000000;       _guard_val-- != 0; })
#define     GUARDL ({ static int _guard_val = 1000000000000;    _guard_val-- != 0; })

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
           bool res = (++_MODVAL % modval == 0);\
           if (res)\
               (action);\
            res;\
        })

#define   MODEXECL(modval, action)\
        ({ static unsigned long _MODVAL = 0;\
           bool res = (++_MODVAL % modval == 0);\
           if (res)\
               (action);\
            res;\
        })

#endif /* !_GUARG_H */
