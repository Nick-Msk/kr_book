#include <stdio.h>
#include <stdbool.h>

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

int         main(void){

    for (int i = 0; i < 50; i++){
        printf("%d\n", i);
        bool val =
        /*({ static bool SINGLE_11 = true;
            bool tmp = SINGLE_11;
          if (SINGLE_11){
                SINGLE_11 = false; printf("Single run only\n");
            }
            tmp;
        });
        val = */ SINGLETON(printf("Hello!!!!!\n") );
        if (val)
            printf("Macro returns true\n");
        bool val2 = MODEXEC(13, printf("mod 13 %d\n", i) );
    }
    return 0;
}
