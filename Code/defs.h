#ifndef __DEFS_H__
#define __DEFS_H__

#include<stdlib.h>
#include<stdbool.h>
#include<stddef.h>

/* global marco and type */

// output error msg to
#define ERROR_MSG_2 stderr

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