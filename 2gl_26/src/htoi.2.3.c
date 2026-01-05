#include <stdio.h>
#include <strings.h>
#include <ctype.h>

// htoi
static int              htoi(const char *s);

int                     main(int argc, const char *argv[]){
   if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s htoi KR task 2.3\nUsage: %s <0x0-9,A-F>\n", __FILE__, *argv);
            return 0;
        }
    }
    int res = htoi(argv[1]);
    printf("%d\n", res);
    return 0;
}

// using isxdigit() from ctype
//static inline bool    is_hex_letter(char c);


static inline int       toxdigit(char c){
    c = tolower(c);
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a';
   return c; 
}

// htoi
// 0x123AF4 
static int            htoi(const char *s){
    int res = 0;
     
    if (*s == '0'){
        s++;
        if (tolower(*s) != 'x') 
            return 0;   // wrong format
        else
            s++;
    }
    while (isxdigit(*s)){
        res = res * 16 + toxdigit(*s++);
    }
    return res;
}

