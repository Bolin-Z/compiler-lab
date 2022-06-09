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
    +---------+ <-- new $fb
    | cur $ra |
    +---------+
    | old $fb |
    +---------+
    | old $t3 |
    +---------+
    | callee  |
    |  saved  |
    |   reg   |
    +---------+
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

    asmBlock * blocks;

    paramItem * paramTable;
    int paramCount;
    localVarItem * localValTable;
    int localValCount;

    int size;
} asmFunction;

void translateIRToAsm(FILE* f, irSystem* sys);

#endif