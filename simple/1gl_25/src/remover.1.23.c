#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "bool.h"

// comment remover
static int          comment_remover(void){
    int cnt = 0;
    int c, c2;
    bool string = false, letter = false, ct1 = false, ct2 = false;  // ct1 - // comment, ct2 - /* */ comment
    
    const char *tmp = " /*hjghjghghgjjhgh */ ";
    /* main cycle 
    -----
    */
    while ( (c = getchar()) != EOF){
        // bare
        if (!string && !letter && !ct1 && !ct2){
            if (c == '\'')
                letter = true;
            if (c == '"')
                string = true;
            if (c == '/'){  // in that case we have to get one more symbol
                c2 = getchar();
                if (c2 == '/')
                    ct1 = true, cnt++;
                else if (c2 == '*')
                    ct2 = true, cnt++;
                else
                    ungetc(c2, stdin);
            }
        } else if (string){
            if (c == '"')
                string = false;
        } else if (letter){
            if (c == '\'')
                letter = false;
            else if (c == '\n')
                letter = false;
        } else if (ct1){
            if (c == '\n')
                ct1 = false;
        } else if (ct2){
            if (c == '*'){
                c2 = getchar();
                if (c2 == '/'){
                    ct2 = false;
                    continue;
                }
                else
                    ungetc(c2, stdin);
            }
        }
        if (!ct1 && !ct2)
            putchar(c);
    }
    return cnt;
}

// start
int                 main(void){

    int cnt = comment_remover();

    printf("%d comments was/were removed\n", cnt);
    /* it is ok! return 0 */
    return 0;
}
