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
#include "var.h"
#include "buffer.h"

const char *usage_str = "Usage: %s (interactive) or <command(s):str>\n";

static int              launch(bool interactive);

int                     main(int argc, const char *argv[]){

    static const char *logfilename = "log/calc.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc == 2){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR calc p.89\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    bool interactive = true;
    if (argc > 1){ 
        ungetch(EOF);
        ungetch('\n');
        // unload input parameter to buffer!
        while (argc > 1){
            const char *s = argv[--argc];
            int len = strlen(s);
            while (len > 0)
                ungetch(s[--len]);
            ungetch(' ');
        }
        interactive = false;
    }
    printf("Total expressions: %d\n", launch(interactive) );

    logclose("...");
    return 0;
}

static int              print_command(){
    int     total = 11;
    printf("Commands: + - * / %% - arithmetic,\n p - print stack, e - exchange top1 and top2"\
            "c - clear, d - push the same, h - help, q - quit :<x> var (not implemented yes)");
    return total;   // total
}

static int              launch(bool interactive){
    logenter(" ");
    int     type, total = 0;
    double  op, op2;
    const   int MAXOP = 1000;
    char    buf[MAXOP];     // TODO: replace to faststring  fs
    bool    quit = false;

    if (interactive)
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
                logmsg("/ : op2 %f", op2);
                if (op2 == 0.0){
                    fprintf(stderr, "Divizion by zero!\n");
                    stack_push(op2);
                } else
                    stack_push(stack_pop() / op2);
            break;
            case '%':
                op2 = stack_pop();
                if (op2 == 0.0){
                    fprintf(stderr, "Divizion by zero!\n");
                    stack_push(op2);
                } else
                    stack_push(fmod(stack_pop(), op2));
            break;
            case LEXIC_POW: 
                op2 = stack_pop();
                op = stack_pop();
                stack_push(pow(op, op2));
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
                stack_push(0.0);
            break;
            case LEXIC_REMOVE:
                stack_pop();
                if (stack_count() < 1)
                    stack_push(0.0);
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
            case LEXIC_SIN:
                stack_push(sin(stack_pop()));
            break;
            case LEXIC_COS:
                stack_push(cos(stack_pop()));
            break;
            case LEXIC_TAN:
                stack_push(tan(stack_pop()));
            break;
            case LEXIC_VAR:
                if (strlen(buf) > 1)
                    fprintf(stderr, "Only 1 char variables are supported, but not (%s)\n", buf);
                else {
                    if (*buf == ':')        // print all vars
                        var_print();
                    else
                        if (!stack_push(var_get(*buf) ) )
                            fprintf(stderr, "Stack overflow!\n");
                }
            break;
            case LEXIC_ASSIGNMENT:
                if (strlen(buf) > 1)
                    fprintf(stderr, "Only 1 char variables are supported, but not (%s)\n", buf);
                else
                    var_set(*buf, stack_get());
            break;
            case '\n':
                var_set('?', stack_get() );
                printf("\t%.8g\n(%d)> ", stack_get(), stack_count());
                total++;
            break;
            default:
                fprintf(stderr, "Unknown command [%c]\n", type);
            break;
        }
    }
    return logret(total, " ");
}

