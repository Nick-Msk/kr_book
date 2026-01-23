#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>

#include "log.h"
#include "common.h"
#include "checker.h"
#include "getop.h"
#include "stack.h"

const char *usage_str = "Usage: %s (interactive)\n";

static void             launch(void);

int                     main(int argc, const char *argv[]){

    static const char *logfilename = "log/calc.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR calc p.89\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }

    launch();

    logclose("...");
    return 0;
}

static void             launch(void){
    int     type;
    double  op, op2;
    const   int MAXOP = 1000;
    char    buf[MAXOP];     // TODO: replace to faststring  fs

    while ( (type = lexic_getop(buf)) != EOF){
        switch (type){
            case LEXIC_NUMBER:
                if (!stack_push(atof(buf))
                    fprintf(stderr, "Stack overflow!\n");
            break;
            case '+':
                op2 = stack_pop();
                stack_push(stack_pop() + op2);
            break;
            case '-':
                op2 = stack_pop();
                stack_push(stack_pop() - op2);
            break;
            case '*':
                op2 = stack_pop();
                stack_push(stack_pop() * op2);
            break;
            case '/':
                op2 = stack_pop();
                if (op2 == 0.0){
                    fprintf(stderr, "Divizion by zero!\n");
                    stack_push(op2);
                } else
                    stack_push(stack_pop() / op2);
            break;
            case '\n':
                printf("\t%.8g\n", pop());
            break;
            default:
                fprintf(stderr, "Unknown command [%c]\n", type);
            break;
        }
    }
}

