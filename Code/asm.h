#ifndef __ASM_H__
#define __ASM_H__

#include "ir.h"

/*  Stack Frame

High Address
    
    |  ARG n  |
    +---------+
    | ARG n-1 |
    +---------+
    |   ...   |
    |   ...   |
    +---------+
    |  ARG 0  |
    +---------+ 0   <-- new $fp
    | cur $ra |
    +---------+ -4
    | old $fp |
    +---------+ -8
    | old $t3 |
    +---------+ -12 <-------+
    | old $s2 |             |
    +---------+ -16         |
    | old $s3 |             |
    +---------+ -20         |
    | old $s4 |             |
    +---------+ -24   callee saved reg
    | old $s5 |             |
    +---------+ -28         |
    | old $s6 |             |
    +---------+ -32         |
    | old $s7 |             |
    +---------+ -36 <-------+
    |  Local  |
    |   Var   |
    |   ...   |
    +---------+ <-- new $t3 $sp
    |  Temp   |
    |   Var   |
    |   in    |
    |  block  |
    +---------+



LOW Address

*/

typedef struct reg {
    char * name;
    bool free;
} reg;

typedef struct tempVarItem {
    int tempVarID;
    bool inReg;
    int storedreg;
    int offset; /* relative to $t3 */
    irCode * lastUsage;
} tempVarItem;

typedef struct asmBlock {
    asmBlock * next, * prev;
    irCode * blockBegin, * blockEnd;

    tempVarItem * tempVarTable;
    int tempVarCount;
    int tempVarStackSize;
} asmBlock;

typedef struct paramItem {
    int paramID;
    int offset; /* relative to $fb */
} paramItem;

typedef struct localVarItem {
    int varID;
    int offset; /* relative to $t3 */
} localVarItem;

typedef struct asmFunction {
    asmFunction * next, * prev;

    asmBlock * blockListHead;
    asmBlock * blockListTail;

    paramItem * paramTable;
    int paramCount;
    localVarItem * localVarTable;
    int localVarCount;

    int size;
} asmFunction;

void translateIRToAsm(FILE* f, irSystem* sys);

#endif