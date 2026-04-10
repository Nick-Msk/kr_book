#include "log.h"
#include "fs_iter.h"
#include "buffer.h"
#include "getword.h"

// probably some configuration is required to  gather tolower, comments, skip_newline
static inline int       skip_spaces(bool get_newline){
    int c;
    while (isspace( c = getch() ) )
        if (get_newline && c == '\n')
            break;
    return c;
}

static int               skip_cl(void){
    int             cnt = 0;        // total count of comments
    int             c, c1;
    bool            comment_and_lines = true;

    while (comment_and_lines){

        c = skip_spaces(false);
        // SKIP comment or literals
        comment_and_lines = false;
        if (c == '/'){
            if ( (c1 = getch() ) == '/'){  // start comment type 1
                cnt++;
                comment_and_lines = true;
                while ( (c1 = getch()) != EOF && c1 != '\n')
                    ;
            } else if (c1 == '*'){  // comment type 2
                cnt++;
                comment_and_lines = true;
                do {
                    while ( (c1 = getch()) != EOF && c1 != '*')
                        ;
                } while ( (c1 = getch() ) != '/' && c1 != EOF);  // end of /* */ /*
            } else  // not a comment! comment_and_lines remains false
                c = c1; // just like ungetch
        } else if (c == '"') {
            comment_and_lines = true;   // line, so setup flag
            while ( (c = getch()) != EOF && c != '"')
                ;
        } else if (c == '\'') {
            comment_and_lines = true;   // anyway!
            while ( (c = getch()) != EOF && c != '\'')
                ;
        }
        //logsimple("comment_and_lines %s", bool_str(comment_and_lines));
    }
    return logsimpleret(c, "[%c], comments %d", c, cnt);
}



// str must have heap alloc
fs                      getword(fs str, bool lower, bool comments, bool get_newline){

    logenter("tolower %s, get_newline %s", bool_str(lower), bool_str(get_newline) );

    fsclear(str);   // reset
    int              c;
    fsnew            iter = fsinew(&str);

    c = comments ? skip_spaces(get_newline) : skip_cl();       // comment and so on are allowed! TODO: probably use flag -c
    if (c != EOF){
        elemnext(iter) = clower(c, lower);
    } else
        elemclear(iter);    // end flag

    if (!isalpha_u(c) ){
        elemend(iter);
        return logret(str, "%c:%d - [%s]", c, str.len, str.v);
    }
    while ( (c = getch()) != EOF){
        if (!isalnum_u(c) ){
            ungetch(c);
            break;
        } else
            elemnext(iter) = clower(c, lower);
    }
    elemend(iter);

    return logret(str, "%d - [%s]", str.len, str.v); // that is probably new str
}

bool                    getlexem(Lexem *lex, bool ign_comments){
    int     c;
    fsnew   iterstr;    // iterator-constructor

    c = ign_comments ? skip_cl() : skip_spaces(false);
    if (c != EOF){
        fsclear(lex->str);  // only if any valuable input
        iterstr = fsinew(&lex->str);
    } else
        return logsimpleret(false, "EOF detected");
    lex->typ = LEXEM_UNK;  // init state
    elemnext(iterstr) = c; // put into fs
    // check the numbers
    if (isdigit_signed(c) ){
        if (c == '+' || c == '-')
            c = getch();
        //
        if (!isdigit(c) ){   // not a number actually
            lex->typ = LEXEM_SYM;
        } else {
            while ( (c = getch()) != EOF && isdigit(c) )
                elemnext(iterstr) = c;
            lex->typ = LEXEM_INT;
        }
    } else if (isalnum_u(c) || c == '\\' ){      // just identifier or so on
        if (c == '\\'){
            lex->typ = LEXEM_CMD;
            iterstr.pos--;  // remove command sign
        } else
            lex->typ = LEXEM_WORD;
        // process the identifier
        while ( (c = getch()) != EOF && isalnum_u(c) )
            elemnext(iterstr) = c;
    } else {
        elemend(iterstr);
        return logsimpleret(true, "Unknown symbol [%c]", c);
    }
    elemend(iterstr);
    ungetch(c);
    return logsimpleret(true, "Parsed %s:%s", Lexemtype_str(lex->typ), lex->str.v );
}

