#ifndef __DEFS_H__
#define __DEFS_H__

#include<stdlib.h>
#include<stdbool.h>
#include<stddef.h>

/* global marco and type */

// output error msg to
#define ERROR_MSG_2 stderr

#define SYM(a) SYM##_##TYPE##_##a
enum SYM_TYPE{
    SYM(ERROR) = 0,
    /* token type */
    SYM(ID), SYM(LC), SYM(RC), SYM(IF), SYM(OR), SYM(LB), SYM(RB), SYM(LP), SYM(RP),
    SYM(INT), SYM(AND), SYM(DIV), SYM(NOT), SYM(NEG), SYM(DOT),
    SYM(TYPE), SYM(SEMI), SYM(ELSE), SYM(PLUS), SYM(STAR),
    SYM(FLOAT), SYM(COMMA), SYM(WHILE), SYM(RELOP), SYM(MINUS),
    SYM(STRUCT), SYM(RETURN),
    SYM(ASSIGNOP),
    /* non-terminal type */
    SYM(Program), SYM(ExtDefList), SYM(Specifier), SYM(FunDec), SYM(Compst), SYM(VarDec), SYM(ExtDecList),
    SYM(StructSpecifier), SYM(OptTag), SYM(DefList), SYM(Tag),
    SYM(VarList), SYM(ParamDec),
    SYM(StmtList), SYM(Stmt), SYM(Exp),
    SYM(Def), SYM(DecList), SYM(Dec),
    SYM(Args)
};

static const char *const symtype2str[] = {
    "error",
    "ID", "LC", "RC", "IF", "OR", "LB", "RB", "LP", "RP",
    "INT", "AND", "DIV", "NOT", "NEG", "DOT",
    "TYPE", "SEMI", "ELSE", "PLUS", "STAR",
    "FLOAT", "COMMA", "WHILE", "RELOP", "MINUS",
    "STRUCT", "RETURN",
    "ASSIGNOP",
    "Program", "ExtDefList", "Specifier", "FunDec", "Compst", "VarDec", "ExtDecList",
    "StructSpecifier", "OptTag", "DefList", "Tag",
    "VarList", "ParamDec",
    "StmtList", "Stmt", "Exp",
    "Def", "DecList", "Dec", 
    "Args",
    0
};

#define TK(a) TK##_##TYPE##_##a
enum TK_TYPE{
    /* token value is unique */
    TK(UNIQ)=1,
    /* relation operator */
    TK(LT),TK(GT),TK(LTE),TK(GTE),TK(EQ),TK(NEQ),
    /* type */
    TK(INT),TK(FLOAT)
};



/* global variable */
static char Error_Type[] = {'A','B','\0'};

#endif