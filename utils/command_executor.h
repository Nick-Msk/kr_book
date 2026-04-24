#ifndef _COMMAND_EXECUTOR_H
#define _COMMAND_EXECUTOR_H

typedef struct Context Context;     // just to use a pointer

typedef int         (*process_unit)(Context *c);   // function, process context

typedef struct Command {
    const char *    name;
    int             shortlen;
    const char *    desc;
    process_unit    proc;
} Command;

#define             CommandInit(...) (Command) {.name = 0, .proc = 0, .desc = 0, .shortlen = 0, ##__VA_ARGS__}

extern bool         process_command(const char *restrict name, Command *restrict cmd, Context *restrict ctx);

#endif /* !_COMMAND_EXECUTOR_H */
