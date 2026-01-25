#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>

#include "log.h"
#include "common.h"
#include "checker.h"
#include "getop.h"
#include "stack.h"

const char *usage_str = "Usage: %s (interactive)\n";

static int              launch(void);

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

    printf("Total expressions: %d\n", launch() );

    logclose("...");
    return 0;
}

static int              print_command(){
    int     total = 11;
    printf("Commands: + - * / %% - arithmetic,\n p - print stack, e - exchange top1 and top2"\
            "c - clear, d - push the same, h - help, q - quit :<x> var (not implemented yes)");
    return total;   // total
}

static int              launch(void){
    logenter(" ");
    int     type, total = 0;
    double  op, op2;
    const   int MAXOP = 1000;
    char    buf[MAXOP];     // TODO: replace to faststring  fs
    bool    quit = false;

    printf("> ");
    stack_push(0.0);    // init
    while (!quit && (type = lexic_getop(buf, MAXOP) ) != EOF){
        switch (type){
            case LEXIC_NUMBER:
                op = atof(buf);
                logmsg("Number: %f", op);
                if (!stack_push(op) )
                    fprintf(stderr, "Stack overflow!\n");
            break;
            case '+':
                op2 = stack_pop();
                op = stack_pop();
                logmsg("PLUC: %f + %f = ", op, op2);
                stack_push(op + op2);
            break;
            case '-':
                op2 = stack_pop();
                op = stack_pop();
                logmsg("MINUS: %f - %f = ", op, op2);
                stack_push(op - op2);
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
            case '%':
                op2 = stack_pop();
                if (op2 == 0.0){
                    fprintf(stderr, "Divizion by zero!\n");
                    stack_push(op2);
                } else
                    stack_push(fmod(stack_pop(), op2));
            break;
            case 'd':   // put the same
                if (!stack_pushsame())
                    fprintf(stderr, "Stack overflow!\n");
            break;
            case 'e':   // exchange
                stack_exch();
            break;
            case 'c':
                stack_clear();
            break;
            case 'p':
                stack_print();
            break;
            case 'h':
                print_command();
            break;
            case 'q':
                quit = true;
            break;
            case '\n':
                printf("\t%.8g\n(%d)> ", stack_get(), stack_count());
                stack_fprint(logfile);
                total++;
            break;
            default:
                fprintf(stderr, "Unknown command [%c]\n", type);
            break;
        }
    }
    return logret(total, " ");
}

