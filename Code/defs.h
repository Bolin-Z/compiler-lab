#ifndef __DEFS_H__
#define __DEFS_H__

#include<stdlib.h>
#include<stdbool.h>

/* global marco and type */
#define IDLEN 32
// output error msg to
#define ERROR_MSG_2 stdout

#define LEX(a) LEX##_##a
enum{
    /* Not a lexical unit */
    LEX(NOT),
    /* relation operator */
    LEX(LT),LEX(GT),LEX(LTE),LEX(GTE),LEX(EQ),LEX(NEQ),
    /* type */
    LEX(INT),LEX(FLOAT)
};

/* global variable */
static char Error_Type[] = {'A','B','\0'};

/* data structure */

// YYSTYPE
typedef union tokenlval{
    char ID[IDLEN];
    int lextype;
    int intval;
    float floatval;
}tokenlval;

struct CSTnode{
    int tokentype;
    int lineno;
    bool empty_str;
};

#endif