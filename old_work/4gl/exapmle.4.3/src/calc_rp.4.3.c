#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "log.h"
#include "common.h"
#include "bool.h"
#include "var.h"
#include "buffer.h"
#include "stack.h"
#include "getop.h"

bool            process_oper(Oper op);

int             main(int argc, const char *argv[]){

    logenter("%s module is started", *argv);

    Oper         op;

    // check if agrument is provided
    if (argc > 1)
        ungets(argv[1]);    // to use line as input!

    while (process_op(getop())){
        printf(">>\t%.8g\n>", gettop());
    }
    return logret(0, "Done calc");
}

// NOTE: never use getop() here! That should only process provided optype
int             process_oper(Oper op){
    logenter("type %d", optype);

    double      op1, op2;
    bool        quit = false;

    switch (op.type){
            case NUMBER:
                if (!push(op.dval))
                    fprintf(stderr, "Unable to add value %s to stack\n", s);
            break;
            case PLUS:
                push(pop() + pop());    // no need to check, if stack is empty pop() will return 0.0
            break;
            case MINUS:
                op2 = pop();
                push(pop() - op2);
            break;
            case MUL:
                push(pop() * pop());
            break;
            case DIV:
                op2 = pop();
                if (op2 == 0.0){
                    fprintf(stderr, "Error: Unable to divide to  zero!\n");
                    push(op2);      // move back!
                }
                else
                    push(pop() / op2);
            break;
            case MOD:
                op2 = pop();
                if (op2 == 0.0){
                    fprintf(stderr, "Error: Unable to divide to  zero!\n");
                    push(op2);      // move back!
                } else
                    push(fmod(pop(), op2));
            break;
            case POW:
                op2 = pop();
                push(pow(pop(), op2));
            break;
            case SIN:
                push(sin(pop()));
            break;
            case EXP:
                push(exp(pop()));
            break;
            case QUIT:
            case EOF:
                printf("Quitting...\n");
                quit = true;
            break;
            case HELP:
                printf("Usage: <num1> <num2> <op>\nOp is + - / * sin exp pow\nquit, help\npst\tclear\t\ndou\texch\n");
            break;
            case PRINT_STACK:
                print_stack(5);
            break;
            case CLEAR:
                clear_stack();
            break;
            case DOU:
                if (!push(gettop()))
                    fprintf(stderr, "Unable to add value %f to stack\n", op.dval);
            break;
            case EXCH:
                op1 = pop();
                op2 = pop();
                push(op1);
                push(op2);
            break;
            case GET:       // variable name in op.str[0]
                op2 = var_get(op.str);   // 0.0 if not exists
                if (var_isset(op.str))
                    push(op2);
                else
                    fprintf(stderr, "Var %s doesn'nt exist\n", op.str);
            break;
            case SET:       // variable name in s[0]
                if (!var_set(op.str, gettop()))
                    fprintf(stderr, "Unable to set var %s to %f\n", op.str, gettop());
            break;
            case UNSET:
                if (!var_unset(op.str))
                    fprintf(stderr, "Unable to unset var %s\n", op.str);
            break;
            default:
                fprintf(stderr, "Error: unknown command %s\n", op.str); // not sure...
            break;
        }
    return !quit;
}

