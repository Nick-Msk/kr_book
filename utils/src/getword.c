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

// parse only LEXEM_STR or LEXEM_CMD!
bool                    getstring(Lexem *lex){

    int     c;
    fsnew   iter;

    c = skip_spaces(true);
    logsimple("[%c]", c);
    if (c == '\\')
        lex->typ = LEXEM_CMD;  // if command, then just return line without init '\'
    else {
        elemnext(iter) = c;
        lex->typ = LEXEM_STR;
    }
    while ( (c = getch()) != EOF && c != '\n')
        elemnext(iter) = c;
    elemend(iter);
    // no need to ungetch

    return logsimpleret(true, "Parsed as str %s:%s", Lexemtype_str(lex->typ), lex->str.v);
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

// -------------------------------Testing --------------------------
#ifdef GETWORDTESTING

#include "test.h"
#include "checker.h"
#include "fs_array.h"

//types for testing


// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: read input by getstring()", ++subnum);
    {
        const char  fname[] = "res/getword_test_file_getstring.dat";
        const char *pts[] = { "Fiest line xxx", "Second line yyyyy", "Third line oooooo", 0};
        FILE        *f = fopen(fname, "w+");
        if (!f)
            return logerr(TEST_FAILED, "Unable to open %s for w+", fname);

        fsarray     fa = fsarr_fromarr(pts, 0);
        fsarr_fsavelines(f, &fa, 0);  // save ONLY lines, divided by '\n'

        rewind(f);
        buffer_set(f);  // read from f instead of stdin

        Lexem       lex = lexeminit();
        int         i = 0;
        while (getstring(&lex) ){
            test_validatefree(lex.typ = LEXEM_STR, (lexemfree(lex), fsarrfree(fa), fclose(f) ),
                "Lexem type must be LEXEM_STR but not %s", Lexemtype_str(lex.typ) );
            test_validatefree(lexem_cmp(&lex, pts[i]) == 0, (lexemfree(lex), fsarrfree(fa), fclose(f) ),
                "String must be [%s] but not [%s]", lexemstr(lex), pts[i] );
            i++;
        }

        lexemfree(lex);
        fsarrfree(fa);
        fclose(f);
    }

    test_sub("subtest %d: read input by with command getstring()", ++subnum);
    {
        const char  fname[] = "res/getword_test_file_getstring2.dat";
        // odd - LEXEM_STR
        const char *pts[] = { "Command line 1", "\\Fiest line xxx", "Second line yyyyy", "\\Third line oooooo", "Command line 2"};
        FILE        *f = fopen(fname, "w+");
        if (!f)
            return logerr(TEST_FAILED, "Unable to open %s for w+", fname);

        fsarray     fa = fsarr_fromarr(pts, 0);
        fsarr_fsavelines(f, &fa, 0);  // save ONLY lines, divided by '\n'

        rewind(f);
        buffer_set(f);  // read from f instead of stdin

        Lexem       lex = lexeminit();
        int         i = 0;
        while (getstring(&lex) ){
            Lexemtype typ = (i % 2 == 0) ? LEXEM_STR : LEXEM_CMD;
            test_validatefree(lex.typ = LEXEM_STR, (lexemfree(lex), fsarrfree(fa), fclose(f) ),
                "Lexem type must be %s but not %s", Lexemtype_str(typ), Lexemtype_str(lex.typ) );
            test_validatefree(lexem_cmp(&lex, pts[i] + i % 2) == 0, (lexemfree(lex), fsarrfree(fa), fclose(f) ),
                "String must be [%s] but not [%s]", lexemstr(lex), pts[i] + i % 2 );
            i++;
        }

        lexemfree(lex);
        fsarrfree(fa);
        fclose(f);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main( /* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 = tf1,  .num =  1, .name = "getstring() simple file test"                     , .desc=""                , .mandatory=true)

);

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* GETWORDTESTING */

