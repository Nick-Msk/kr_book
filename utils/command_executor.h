#ifndef _COMMAND_EXECUTOR_H
#define _COMMAND_EXECUTOR_H

// PROTOTYPE

typedef             (*process_unit)(stringhash *h);
typedef struct {
    const char      *name;
    int             shortlen;
    process_unit    proc;
} Command;
#define         CommandNull(...) (Command){.name = 0, .proc = 0, ##__VA_ARGS__ }
#define         CommandsInit(...) (const Command[]) {__VA_ARGS__, CommandNull() }

typedef struct {
    Command *cmd;
} Commandlist;

#define         commandlistInit(...) (

// load + ordering (???), it's not possible to order that
bool    reg_commands(Commandlist list){
    while(list.cmd.name){
        
    }
}

#endif /* !_COMMAND_EXECUTOR_H */
