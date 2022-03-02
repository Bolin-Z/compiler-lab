#ifndef __DEFS_H__
#define __DEFS_H__

#include<stdlib.h>
#include<stdbool.h>
#include<stddef.h>

/* global marco and type */
#define IDLEN 32
// output error msg to
#define ERROR_MSG_2 stderr

#define L(a) LEX##_##a
enum{
    /* Not a lexical unit */
    L(NOT),
    /* relation operator */
    L(LT),L(GT),L(LTE),L(GTE),L(EQ),L(NEQ),
    /* type */
    L(INT),L(FLOAT)
};

/* global variable */
static char Error_Type[] = {'A','B','\0'};

/* data structure */

// YYSTYPE
/* Type of token */
struct CST_tk_node{
    int symtype;
    union symval{
        int intval;
        float floatval;
        int tktype;
        char ID[IDLEN];
    }_symval;
};

/* Type of non-token */

struct CST_nt_node{
    int symtype;
    int lineno;
    size_t child_cnt;
    struct CST_nt_node ** child_list; 
};

union YYSTYPE {
    struct CST_tk_node tk_node;
    struct CST_nt_node nt_node;
};

#endif