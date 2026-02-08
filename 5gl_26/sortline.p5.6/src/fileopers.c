#include <stdlib.h>
#include "fileopers.h"

// --------------------------- API ---------------------------------

int             readlines(char *ptr[], int maxline){

}

int             writelines(const char *ptr[], int maxline){

}

int             freelines(const char *ptr[], int maxline){
    while (--maxline > 0)
        free(*prt++);
}

