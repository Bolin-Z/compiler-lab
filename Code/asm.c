#include "asm.h"

static const int _zero = 0;
static const int _v0 = 2;
static const int _v1 = 3;
static const int _a0 = 4;
static const int _a1 = 5;
static const int _a2 = 6;
static const int _a3 = 7;
static const int _t0 = 8;
static const int _t1 = 9;
static const int _t2 = 10;

#define NOTEXIST -1

static reg regs[32] = {
    /* reserved for special use */
    {"$zero", true}, {"$at"  , true}, {"$v0"  , true}, {"$v1"  , true},
    {"$a0"  , true}, {"$a1"  , true}, {"$a2"  , true}, {"$a3"  , true},
    /* reserved for ir code */    
    {"$t0"  , true}, {"$t1"  , true}, {"$t2"  , true}, 
    /* temp variabble stack bottom pointer*/
    {"$t3"  , true},
    /* used for temp variable */
    {"$t4"  , true}, {"$t5"  , true}, {"$t6"  , true}, {"$t7"  , true},
    {"$s0"  , true}, {"$s1"  , true}, 
    {"$s2"  , true}, {"$s3"  , true}, {"$s4"  , true}, {"$s5"  , true}, 
    {"$s6"  , true}, {"$s7"  , true},
    /* reserved for special use */
    {"$t8"  , true}, {"$t9"  , true}, {"$k0"  , true}, {"$k1"  , true},
    {"$gp"  , true}, {"$sp"  , true}, {"$fp"  , true}, {"$ra"  , true}
};

#define reg(ID) &regs[ID]

asmFunction * analysisIRcode(irSystem * sys);
void init(FILE* f);
void translateFunctionToAsm(FILE * f, asmFunction * function);

tempVarItem * getTempVarItemByID(asmBlock * b, int ID);
localVarItem * getLocalVarItemByID(asmFunction * f, int ID);
paramItem * getParamItemByID(asmFunction * f, int ID);

int getReg(FILE * f, asmBlock * curBlock);
void LoadToReg(operand * src, reg * dst, FILE * f, asmFunction * curFunction, asmBlock * curBlock, irCode * curCode);
void StoreFromReg(reg * src, operand * dst, FILE * f, asmFunction * curFunction, asmBlock * curBlock, irCode * curCode);

void exitBlock(FILE * f,asmBlock * curBlock);

void translateIRToAsm(FILE* f, irSystem* sys){
    /* seperate ir code into blocks */
    asmFunction * asmFunctionList = analysisIRcode(sys);

    /* translate */
    init(f);
    asmFunction * curFunction = asmFunctionList;
    while(curFunction){
        translateFunctionToAsm(f,curFunction);
        curFunction = curFunction->next;
    }
    
}

void init(FILE* f){
    const char * initCode = 
    ".data\n"
    "     _prompt: .asciiz \"Enter an integer:\"\n"
    "     _ret: .asciiz \"\\n\"\n"
    "\n"
    ".text\n"
    ".global main\n"
    "\n"
    "read:\n"
    "     li      $v0, 4\n"
    "     la      $a0, _prompt\n"
    "     syscall\n"
    "     li      $v0, 5\n"
    "     syscall\n"
    "     jr      $ra\n"
    "\n"
    "write:\n"
    "     li      $v0, 1\n"
    "     syscall\n"
    "     li      $v0, 4\n"
    "     la      $a0, _ret\n"
    "     syscall\n"
    "     move    $v0, $0\n"
    "     jr      $ra\n"
    "\n";
    fprintf(f,"%s",initCode);
}

asmFunction * analysisIRcode(irSystem * sys){
    asmFunction * functionListHead = NULL;
    asmFunction * functionListTail = NULL;
    irCode * curCode = sys->codeListHead;
    /* seperate into blocks */
    while(curCode){
        switch(curCode->code.instr){
            switch(curCode->code.instr){
                case IS(FUNCTION) : {
                    /* begin of new function */
                    asmFunction * newFunction = (asmFunction *)malloc(sizeof(asmFunction));
                    newFunction->next = NULL; newFunction->prev = functionListTail;
                    newFunction->paramCount = 0;
                    newFunction->localVarCount = 0;
                    newFunction->size = 0;

                    if(!functionListHead) functionListHead = newFunction;
                    if(!functionListTail) functionListTail = newFunction;
                    else {
                        /* not the first function */
                        functionListTail->next = newFunction;
                        /* close last block */
                        functionListTail->blockListTail->blockEnd = curCode->prev;
                        functionListTail = newFunction;
                    }
                    /* begin of new block */
                    asmBlock * newBlock = (asmBlock *)malloc(sizeof(asmBlock));
                    newBlock->blockBegin = curCode; newBlock->blockEnd = NULL;
                    newBlock->tempVarCount = 0; newBlock->tempVarStackSize = 0;
                    /* must be the first block in function */
                    newBlock->prev = NULL; newBlock->next = NULL;
                    functionListTail->blockListHead = newBlock;
                    functionListTail->blockListTail = newBlock;
                    break;
                }
                case IS(GOTO) :
                case IS(EQ) :
                case IS(NEQ) :
                case IS(LESS) :
                case IS(LESSEQ) :
                case IS(GREATER) :
                case IS(GREATEREQ) : {
                    /* end of block */
                    irCode * nextCode = curCode->next;
                    if(!nextCode) {
                        /* curCode is the last code */
                        asmFunction * curFunction = functionListTail;
                        curFunction->blockListTail->blockEnd = curCode;
                        break;
                    }
                    curCode = nextCode;
                    /* fall through */
                }
                case IS(LABEL) : {
                    /* begin of new block */
                    asmBlock * newBlock = (asmBlock *)malloc(sizeof(asmBlock));
                    newBlock->blockBegin = curCode; newBlock->blockEnd = NULL;
                    newBlock->tempVarCount = 0; newBlock->tempVarStackSize = 0;
                    /* can not be the first block in function*/
                    asmFunction * curFunction = functionListTail;
                    asmBlock * prevBlock = curFunction->blockListTail;
                    /* set the last code of previous block */
                    prevBlock->blockEnd = curCode->prev;
                    /* connect blocklists of function */
                    newBlock->prev = prevBlock; prevBlock->next = newBlock;
                    curFunction->blockListTail = newBlock;
                    break;
                }
            }
        }
        curCode = curCode->next;
    }

    /* calculate variable, parameter and temp variable */
    asmFunction * curFunction = functionListHead;
    bool * localVar = (bool *)malloc(sizeof(bool)*(sys->counter.variable));
    bool * parameter = (bool *)malloc(sizeof(bool)*(sys->counter.parameter));
    bool * tempVar = (bool *)malloc(sizeof(bool)*(sys->counter.tempVar));
    
    asmFunction * curFunction;
    for(;;){
        asmBlock * curBlock = curFunction->blockListHead;
        for(;;){
            irCode * curCode = curBlock->blockBegin;
            for(;;){
                operand * test = curCode->code.result;
                for(int i = 0;i < 3;i++){
                    if(test){
                        switch(test->operandClass){
                            case IR(VAR) : {
                                if(!localVar[test->info.variable.Tag]){
                                    curFunction->localVarCount += 1;
                                    localVar[test->info.variable.Tag] = true;
                                }
                                break;
                            }
                            case IR(PARAM) : {
                                if(!parameter[test->info.parameter.Tag]){
                                    curFunction->paramCount += 1;
                                    parameter[test->info.parameter.Tag] = true;
                                }
                                break;
                            }
                            case IR(TEMP) : {
                                if(!tempVar[test->info.tempVar.Tag]){
                                    curBlock->tempVarCount += 1;
                                    tempVar[test->info.tempVar.Tag] = true;
                                }
                                break;
                            }
                        }
                    }
                    test = (i == 0) ? (curCode->code.arg1) : (curCode->code.arg2);
                }
                if(curCode == curBlock->blockEnd) break;
                else curCode = curCode->next;
            }
            curBlock->tempVarTable = (tempVarItem *)malloc(sizeof(tempVarItem)*curBlock->tempVarCount);
            if(curBlock == curFunction->blockListTail) break;
            else curBlock = curBlock->next;
        }
        curFunction->localVarTable = (localVarItem *)malloc(sizeof(localVarItem)*curFunction->localVarCount);
        curFunction->paramTable = (paramItem*)malloc(sizeof(paramItem)*curFunction->paramCount);
        if(curFunction == functionListTail) break;
        else curFunction = curFunction->next;
    }

    /* fill the table */
    curFunction = functionListHead;
    for(;;){
        int localVarPushed = 0; int parameterPushed = 0;
        int parameterOffset = 0;
        asmBlock * curBlock = curFunction->blockListHead;
        for(;;){
            int tempVarPushed = 0;
            irCode * curCode = curBlock->blockBegin;
            for(;;){
                switch(curCode->code.instr){
                    case IS(PLUS) : 
                    case IS(MINUS) :
                    case IS(MUL) :
                    case IS(DIV) : {
                        operand * test = curCode->code.result;
                        for(int i = 0;i < 3;i++){
                            switch(test->operandClass){
                                case IR(VAR) : {
                                    if(localVar[test->info.variable.Tag]){
                                        localVarItem * v = &(curFunction->localVarTable[localVarPushed]);
                                        localVarPushed++;
                                        v->varID = test->info.variable.Tag;
                                        v->offset = curFunction->size;
                                        curFunction->size += 4;
                                        localVar[test->info.variable.Tag] = false;
                                    }
                                    break;
                                }
                                case IR(PARAM) : {
                                    /* first appear in declaration */
                                    break;
                                }
                                case IR(TEMP) : {
                                    if(tempVar[test->info.tempVar.Tag]){
                                        tempVarItem * t = &(curBlock->tempVarTable[tempVarPushed]);
                                        tempVarPushed++;
                                        t->tempVarID = test->info.tempVar.Tag;
                                        t->inReg = false;
                                        t->storedreg = NOTEXIST;
                                        t->offset = NOTEXIST;
                                        t->lastUsage = curCode;
                                        tempVar[test->info.tempVar.Tag] = false;
                                    } else {
                                        tempVarItem * t = getTempVarItemByID(curBlock,test->info.tempVar.Tag);
                                        t->lastUsage = curCode;
                                    }
                                    break;
                                }
                            }
                            test = (i == 0) ? (curCode->code.arg1) : (curCode->code.arg2);
                        }
                        break;
                    }
                    case IS(ASSIGN) : 
                    case IS(GETADDR) :
                    case IS(GETVAL) :
                    case IS(SETVAL) : {
                        operand * test = curCode->code.result;
                        for(int i = 0;i < 2;i++){
                            switch(test->operandClass){
                                case IR(VAR) : {
                                    if(localVar[test->info.variable.Tag]){
                                        localVarItem * v = &(curFunction->localVarTable[localVarPushed]);
                                        localVarPushed++;
                                        v->varID = test->info.variable.Tag;
                                        v->offset = curFunction->size;
                                        curFunction->size += 4;
                                        localVar[test->info.variable.Tag] = false;
                                    }
                                    break;
                                }
                                case IR(PARAM) : {
                                    /* first appear in declaration */
                                    break;
                                }
                                case IR(TEMP) : {
                                    if(tempVar[test->info.tempVar.Tag]){
                                        tempVarItem * t = &(curBlock->tempVarTable[tempVarPushed]);
                                        tempVarPushed++;
                                        t->tempVarID = test->info.tempVar.Tag;
                                        t->inReg = false;
                                        t->storedreg = NOTEXIST;
                                        t->offset = NOTEXIST;
                                        t->lastUsage = curCode;
                                        tempVar[test->info.tempVar.Tag] = false;
                                    } else {
                                        tempVarItem * t = getTempVarItemByID(curBlock,test->info.tempVar.Tag);
                                        t->lastUsage = curCode;
                                    }
                                    break;
                                }
                            }
                            test = curCode->code.arg1;
                        }
                        break;
                    }
                    case IS(EQ) :
                    case IS(NEQ) :
                    case IS(LESS) :
                    case IS(LESSEQ) :
                    case IS(GREATER) :
                    case IS(GREATEREQ) : {
                        operand * test = curCode->code.arg1;
                        for(int i = 0;i < 2;i++){
                            switch(test->operandClass){
                                case IR(VAR) : {
                                    if(localVar[test->info.variable.Tag]){
                                        localVarItem * v = &(curFunction->localVarTable[localVarPushed]);
                                        localVarPushed++;
                                        v->varID = test->info.variable.Tag;
                                        v->offset = curFunction->size;
                                        curFunction->size += 4;
                                        localVar[test->info.variable.Tag] = false;
                                    }
                                    break;
                                }
                                case IR(PARAM) : {
                                    /* first appear in declaration */
                                    break;
                                }
                                case IR(TEMP) : {
                                    if(tempVar[test->info.tempVar.Tag]){
                                        tempVarItem * t = &(curBlock->tempVarTable[tempVarPushed]);
                                        tempVarPushed++;
                                        t->tempVarID = test->info.tempVar.Tag;
                                        t->inReg = false;
                                        t->storedreg = NOTEXIST;
                                        t->offset = NOTEXIST;
                                        t->lastUsage = curCode;
                                        tempVar[test->info.tempVar.Tag] = false;
                                    } else {
                                        tempVarItem * t = getTempVarItemByID(curBlock,test->info.tempVar.Tag);
                                        t->lastUsage = curCode;
                                    }
                                    break;
                                }
                            }
                            test = curCode->code.arg2;
                        }
                        break;
                    }
                    case IS(RETURN) :
                    case IS(ARG) :
                    case IS(CALL) :
                    case IS(READ) :
                    case IS(WRITE) : {
                        operand * test = curCode->code.result;
                        switch(test->operandClass){
                            case IR(VAR) : {
                                if(localVar[test->info.variable.Tag]){
                                    localVarItem * v = &(curFunction->localVarTable[localVarPushed]);
                                    localVarPushed++;
                                    v->varID = test->info.variable.Tag;
                                    v->offset = curFunction->size;
                                    curFunction->size += 4;
                                    localVar[test->info.variable.Tag] = false;
                                }
                                break;
                            }
                            case IR(PARAM) : {
                                /* first appear in declaration */
                                break;
                            }
                            case IR(TEMP) : {
                                if(tempVar[test->info.tempVar.Tag]){
                                    tempVarItem * t = &(curBlock->tempVarTable[tempVarPushed]);
                                    tempVarPushed++;
                                    t->tempVarID = test->info.tempVar.Tag;
                                    t->inReg = false;
                                    t->storedreg = NOTEXIST;
                                    t->offset = NOTEXIST;
                                    t->lastUsage = curCode;
                                    tempVar[test->info.tempVar.Tag] = false;
                                } else {
                                    tempVarItem * t = getTempVarItemByID(curBlock,test->info.tempVar.Tag);
                                    t->lastUsage = curCode;
                                }
                                break;
                            }
                        }
                        break;
                    }
                    case IS(PARAM) : {
                        operand * p = curCode->code.result;
                        if(parameter[p->info.parameter.Tag]){
                            paramItem * param = &(curFunction->paramTable[parameterPushed]);
                            parameterPushed++;
                            param->paramID = p->info.parameter.Tag; 
                            param->offset = parameterOffset;
                            parameterOffset += 4;
                            parameter[p->info.parameter.Tag] = false;
                        }
                        break;
                    }
                    case IS(DEC) : {
                        operand * v = curCode->code.result;
                        int varSize = curCode->code.arg1->info.sizeVal;
                        if(localVar[v->info.variable.Tag]){
                            localVarItem * var = &(curFunction->localVarTable[localVarPushed]);
                            localVarPushed++;
                            var->varID = v->info.variable.Tag;
                            var->offset = curFunction->size;
                            curFunction->size += varSize;
                            localVar[v->info.variable.Tag] = false;
                        }
                        break;
                    }
                }
                if(curCode == curBlock->blockEnd) break;
                else curCode = curCode->next;
            }
            if(curBlock == curFunction->blockListTail) break;
            else curBlock = curBlock->next;
        }
        curFunction->size += 36;
        if(curFunction == functionListTail) break;
        else curFunction = curFunction->next;
    }

    free(localVar);
    free(parameter);
    free(tempVar);

    return functionListHead;
}

void translateFunctionToAsm(FILE * f, asmFunction * function){
    asmBlock * curBlock = function->blockListHead;
    while(curBlock){
        irCode * curCode = curBlock->blockBegin;
        while(1){
            bool lastCode = (curCode == curBlock->blockEnd);
            switch(curCode->code.instr){
                case IS(FUNCTION) : {
                    /* FUNCTION f : */
                    int funcID = curCode->code.result->info.funTag;
                    if(funcID == 0) fprintf(f,"main :\n");
                    else            fprintf(f,"f%d :\n",funcID);

                    /* initialize stack frame */
                    {
                        fprintf(f,"    addi    $sp, $sp, %d\n",(-function->size));
                        fprintf(f,"    sw      $ra, %d($sp)\n",function->size - 4);
                        fprintf(f,"    sw      $fp, %d($sp)\n",function->size - 8);
                        fprintf(f,"    sw      $t3, %d($sp)\n",function->size - 12);
                    }
                    /* save callee saved regs */
                    {
                        fprintf(f,"    sw      $s2, %d($sp)\n",function->size - 16);
                        fprintf(f,"    sw      $s3, %d($sp)\n",function->size - 20);
                        fprintf(f,"    sw      $s4, %d($sp)\n",function->size - 24);
                        fprintf(f,"    sw      $s5, %d($sp)\n",function->size - 28);
                        fprintf(f,"    sw      $s6, %d($sp)\n",function->size - 32);
                        fprintf(f,"    sw      $s7, %d($sp)\n",function->size - 36);
                    }
                    /* set $fp and $t3 */
                    {
                        fprintf(f,"    addi    $fp, $sp, %d\n",function->size);
                        fprintf(f,"    move    $t3, $sp\n");
                    }
                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(RETURN) : {
                    /* RETURN x */
                    /* set return value */
                    LoadToReg(curCode->code.result, reg(_v0), f, function, curBlock, curCode);
                    /* restore callee saved regs */
                    {
                        fprintf(f,"    lw      $s2, %d($fp)\n", (-16));
                        fprintf(f,"    lw      $s3, %d($fp)\n", (-20));
                        fprintf(f,"    lw      $s4, %d($fp)\n", (-24));
                        fprintf(f,"    lw      $s5, %d($fp)\n", (-28));
                        fprintf(f,"    lw      $s6, %d($fp)\n", (-32));
                        fprintf(f,"    lw      $s7, %d($fp)\n", (-36));
                    }
                    /* restore $t3 $ra $fp and $sp */
                    {
                        fprintf(f,"    lw      $t3, %d($fp)\n", (-12));
                        fprintf(f,"    lw      $ra, %d($fp)\n", (-4));
                        fprintf(f,"    move    $sp, $fp\n");
                        fprintf(f,"    lw      $fp, %d($fp)\n", (-8));
                    }
                    /* return to caller */
                    fprintf(f,"    jr      $ra\n");
                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(CALL) : {
                    /* function without argument */                    
                    /* save caller saved registers */
                    {
                        fprintf(f,"    addi    $sp, $sp, %d\n", (-24));
                        fprintf(f,"    sw      $t4, %d($sp)\n", 0);
                        fprintf(f,"    sw      $t5, %d($sp)\n", 4);
                        fprintf(f,"    sw      $t6, %d($sp)\n", 8);
                        fprintf(f,"    sw      $t7, %d($sp)\n", 12);
                        fprintf(f,"    sw      $s0, %d($sp)\n", 16);
                        fprintf(f,"    sw      $s1, %d($sp)\n", 20);
                    }

                    /* transfer control flow */
                    int functionID = curCode->code.arg1->info.funTag;
                    if(functionID) fprintf(f,"    jal     main\n");
                    else           fprintf(f,"    jal     f%d\n",functionID);

                    /* restore caller saved registers */
                    {
                        fprintf(f,"    lw      $t4, %d($sp)\n", 0);
                        fprintf(f,"    lw      $t5, %d($sp)\n", 4);
                        fprintf(f,"    lw      $t6, %d($sp)\n", 8);
                        fprintf(f,"    lw      $t7, %d($sp)\n", 12);
                        fprintf(f,"    lw      $s0, %d($sp)\n", 16);
                        fprintf(f,"    lw      $s1, %d($sp)\n", 20);
                        fprintf(f,"    addi    $sp, $sp, %d\n", 24);
                    }

                    /* save return value */
                    StoreFromReg(reg(_v0),curCode->code.result, f, function, curBlock, curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(ARG) : {
                    /* function with arguments */
                    /* save caller saved registers */
                    {
                        fprintf(f,"    addi    $sp, $sp, %d\n", (-24));
                        fprintf(f,"    sw      $t4, %d($sp)\n", 0);
                        fprintf(f,"    sw      $t5, %d($sp)\n", 4);
                        fprintf(f,"    sw      $t6, %d($sp)\n", 8);
                        fprintf(f,"    sw      $t7, %d($sp)\n", 12);
                        fprintf(f,"    sw      $s0, %d($sp)\n", 16);
                        fprintf(f,"    sw      $s1, %d($sp)\n", 20);
                    }

                    /* set arguments */
                    int argCount = 0;
                    while(curCode->code.instr == IS(ARG)){
                        operand * res = curCode->code.result;
                        /* load arg val to $t0 */
                        LoadToReg(res,reg(_t0),f,function,curBlock,curCode);
                        fprintf(f,"    addi    $sp, $sp, %d\n", (-4));
                        fprintf(f,"    sw      $t0, %d($sp)\n", 0);
                        argCount++;
                        curCode = curCode->next;
                    }

                    /* curCode = x := CALL f */
                    /* transfer control flow */
                    int functionID = curCode->code.arg1->info.funTag;
                    if(functionID) fprintf(f,"    jal     main\n");
                    else           fprintf(f,"    jal     f%d\n",functionID);

                    /* pop arguments */
                    fprintf(f,"    addi    $sp, $sp, %d\n", argCount * 4);

                    /* restore caller saved registers */
                    {
                        fprintf(f,"    lw      $t4, %d($sp)\n", 0);
                        fprintf(f,"    lw      $t5, %d($sp)\n", 4);
                        fprintf(f,"    lw      $t6, %d($sp)\n", 8);
                        fprintf(f,"    lw      $t7, %d($sp)\n", 12);
                        fprintf(f,"    lw      $s0, %d($sp)\n", 16);
                        fprintf(f,"    lw      $s1, %d($sp)\n", 20);
                        fprintf(f,"    addi    $sp, $sp, %d\n", 24);
                    }

                    /* save return value */
                    operand * res = curCode->code.result;
                    StoreFromReg(reg(_v0),res,f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(LABEL) : {
                    fprintf(f,"  label%d :\n",curCode->code.result->info.labelTag);
                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(ASSIGN) : {
                    /* x := y */
                    operand * src = curCode->code.arg1;
                    operand * dst = curCode->code.result;
                    LoadToReg(src,reg(_t0),f,function,curBlock,curCode);
                    StoreFromReg(reg(_t0),dst,f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(PLUS) : {
                    /* x := y + z */
                    operand * arg1 = curCode->code.arg1;
                    operand * arg2 = curCode->code.arg2;
                    operand * dst = curCode->code.result;
                    LoadToReg(arg1,reg(_t1),f,function,curBlock,curCode);
                    LoadToReg(arg2,reg(_t2),f,function,curBlock,curCode);
                    fprintf(f,"    add     $t0, $t1, $t2\n");
                    StoreFromReg(reg(_t0),dst,f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(MINUS) : {
                    /* x := y - z */
                    operand * arg1 = curCode->code.arg1;
                    operand * arg2 = curCode->code.arg2;
                    operand * dst = curCode->code.result;
                    LoadToReg(arg1,reg(_t1),f,function,curBlock,curCode);
                    LoadToReg(arg2,reg(_t2),f,function,curBlock,curCode);
                    fprintf(f,"    sub     $t0, $t1, $t2\n");
                    StoreFromReg(reg(_t0),dst,f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(MUL) : {
                    /* x := y * z */
                    operand * arg1 = curCode->code.arg1;
                    operand * arg2 = curCode->code.arg2;
                    operand * dst = curCode->code.result;
                    LoadToReg(arg1,reg(_t1),f,function,curBlock,curCode);
                    LoadToReg(arg2,reg(_t2),f,function,curBlock,curCode);
                    fprintf(f,"    mul     $t0, $t1, $t2\n");
                    StoreFromReg(reg(_t0),dst,f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(DIV) : {
                    /* x := y / z */
                    operand * arg1 = curCode->code.arg1;
                    operand * arg2 = curCode->code.arg2;
                    operand * dst = curCode->code.result;
                    LoadToReg(arg1,reg(_t1),f,function,curBlock,curCode);
                    LoadToReg(arg2,reg(_t2),f,function,curBlock,curCode);
                    fprintf(f,"    div     $t1, $t2\n");
                    fprintf(f,"    mflo    $t0\n");
                    StoreFromReg(reg(_t0),dst,f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(GETADDR) : {
                    /* x := &y */
                    operand * src = curCode->code.arg1;
                    operand * dst = curCode->code.result;
                    switch(src->operandClass){
                        case IR(VAR) : {
                            localVarItem * v = getLocalVarItemByID(function,src->info.variable.Tag);
                            fprintf(f,"    addi    $t0, $t3, %d\n",v->offset);
                            break;
                        }
                        case IR(PARAM) : {
                            paramItem * p = getParamItemByID(function,src->info.parameter.Tag);
                            fprintf(f,"    addi    $t0, $fp, %d\n",p->offset);
                            break;
                        }
                        case IR(TEMP) : {
                            tempVarItem * t = getTempVarItemByID(curBlock,src->info.tempVar.Tag);
                            if(t->offset != NOTEXIST){
                                if(t->inReg){
                                    fprintf(f,"    sw      %s, %d($t3)\n", regs[t->storedreg].name ,t->offset);
                                    t->inReg = false; regs[t->storedreg].free = true; t->storedreg = NOTEXIST;
                                }
                                fprintf(f,"    addi    $t0, $t3, %d\n",t->offset);                                    
                            }else{
                                if(t->inReg){
                                    fprintf(f,"    addi    $sp, $sp, %d\n",(-4));
                                    fprintf(f,"    sw      %s, 0($sp)\n",regs[t->storedreg].name);
                                    curBlock->tempVarStackSize += 4;
                                    t->offset = (-curBlock->tempVarStackSize);
                                    t->inReg = false; regs[t->storedreg].free = true; t->storedreg = NOTEXIST;
                                    fprintf(f,"    addi    $t0, $t3, %d\n",t->offset);
                                }else{
                                    /* error */
                                }
                            }
                            break;
                        }
                    }
                    StoreFromReg(reg(_t0),dst,f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(GETVAL) : {
                    /* x := *y */
                    operand * src = curCode->code.arg1;
                    operand * dst = curCode->code.result;
                    LoadToReg(src,reg(_t1),f,function,curBlock,curCode);
                    fprintf(f,"    lw      $t0, 0($t1)\n");
                    StoreFromReg(reg(_t0),dst,f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(SETVAL) : {
                    /* *x := y */
                    operand * src = curCode->code.arg1;
                    operand * dst = curCode->code.result;
                    LoadToReg(dst,reg(_t0),f,function,curBlock,curCode);
                    LoadToReg(src,reg(_t1),f,function,curBlock,curCode);
                    fprintf(f,"    sw      $t1, 0($t0)\n");
                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(GOTO) : {
                    /* GOTO x */
                    operand * label = curCode->code.result;
                    if(lastCode) exitBlock(f,curBlock);
                    fprintf(f,"    j       label%d\n",label->info.labelTag);
                    break;
                }
                case IS(EQ) : {
                    /* IF x == y GOTO z */
                    operand * argx = curCode->code.arg1;
                    operand * argy = curCode->code.arg2;
                    operand * label = curCode->code.result;
                    LoadToReg(argx,reg(_t0),f,function,curBlock,curCode);
                    LoadToReg(argy,reg(_t1),f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    fprintf(f,"    beq     $t0, $t1, label%d\n",label->info.labelTag);
                    break;
                }
                case IS(NEQ) : {
                    /* IF x != y GOTO z */
                    operand * argx = curCode->code.arg1;
                    operand * argy = curCode->code.arg2;
                    operand * label = curCode->code.result;
                    LoadToReg(argx,reg(_t0),f,function,curBlock,curCode);
                    LoadToReg(argy,reg(_t1),f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    fprintf(f,"    bne     $t0, $t1, label%d\n",label->info.labelTag);
                    break;
                }
                case IS(LESS) : {
                    /* IF x < y GOTO z */
                    operand * argx = curCode->code.arg1;
                    operand * argy = curCode->code.arg2;
                    operand * label = curCode->code.result;
                    LoadToReg(argx,reg(_t0),f,function,curBlock,curCode);
                    LoadToReg(argy,reg(_t1),f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    fprintf(f,"    blt     $t0, $t1, label%d\n",label->info.labelTag);
                    break;
                }
                case IS(LESSEQ) : {
                    /* IF x <= y GOTO z */
                    operand * argx = curCode->code.arg1;
                    operand * argy = curCode->code.arg2;
                    operand * label = curCode->code.result;
                    LoadToReg(argx,reg(_t0),f,function,curBlock,curCode);
                    LoadToReg(argy,reg(_t1),f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    fprintf(f,"    ble     $t0, $t1, label%d\n",label->info.labelTag);
                    break;
                }
                case IS(GREATER) : {
                    /* IF x > y GOTO z */
                    operand * argx = curCode->code.arg1;
                    operand * argy = curCode->code.arg2;
                    operand * label = curCode->code.result;
                    LoadToReg(argx,reg(_t0),f,function,curBlock,curCode);
                    LoadToReg(argy,reg(_t1),f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    fprintf(f,"    bgt     $t0, $t1, label%d\n",label->info.labelTag);
                    break;
                }
                case IS(GREATEREQ) : {
                    /* IF x >= y GOTO z */
                    operand * argx = curCode->code.arg1;
                    operand * argy = curCode->code.arg2;
                    operand * label = curCode->code.result;
                    LoadToReg(argx,reg(_t0),f,function,curBlock,curCode);
                    LoadToReg(argy,reg(_t1),f,function,curBlock,curCode);
                    if(lastCode) exitBlock(f,curBlock);
                    fprintf(f,"    bge     $t0, $t1, label%d\n",label->info.labelTag);
                    break;
                }
                case IS(READ) : {
                    /* READ x */
                    operand * dst = curCode->code.result;
                    /* save caller saved registers */
                    {
                        fprintf(f,"    addi    $sp, $sp, %d\n", (-24));
                        fprintf(f,"    sw      $t4, %d($sp)\n", 0);
                        fprintf(f,"    sw      $t5, %d($sp)\n", 4);
                        fprintf(f,"    sw      $t6, %d($sp)\n", 8);
                        fprintf(f,"    sw      $t7, %d($sp)\n", 12);
                        fprintf(f,"    sw      $s0, %d($sp)\n", 16);
                        fprintf(f,"    sw      $s1, %d($sp)\n", 20);
                    }

                    /* transfer control flow */
                    fprintf(f,"    jal     read\n");

                    /* restore caller saved registers */
                    {
                        fprintf(f,"    lw      $t4, %d($sp)\n", 0);
                        fprintf(f,"    lw      $t5, %d($sp)\n", 4);
                        fprintf(f,"    lw      $t6, %d($sp)\n", 8);
                        fprintf(f,"    lw      $t7, %d($sp)\n", 12);
                        fprintf(f,"    lw      $s0, %d($sp)\n", 16);
                        fprintf(f,"    lw      $s1, %d($sp)\n", 20);
                        fprintf(f,"    addi    $sp, $sp, %d\n", 24);
                    }

                    /* save return value */
                    StoreFromReg(reg(_v0),dst,f,function,curBlock,curCode);

                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
                case IS(WRITE) : {
                    /* WRITE x */
                    operand * src = curCode->code.result;
                    
                    /* set arguments */
                    LoadToReg(src,reg(_a0),f,function,curBlock,curCode);

                    /* save caller saved registers */
                    {
                        fprintf(f,"    addi    $sp, $sp, %d\n", (-24));
                        fprintf(f,"    sw      $t4, %d($sp)\n", 0);
                        fprintf(f,"    sw      $t5, %d($sp)\n", 4);
                        fprintf(f,"    sw      $t6, %d($sp)\n", 8);
                        fprintf(f,"    sw      $t7, %d($sp)\n", 12);
                        fprintf(f,"    sw      $s0, %d($sp)\n", 16);
                        fprintf(f,"    sw      $s1, %d($sp)\n", 20);
                    }

                    /* transfer control flow */
                    fprintf(f,"    jal     write\n");

                    /* restore caller saved registers */
                    {
                        fprintf(f,"    lw      $t4, %d($sp)\n", 0);
                        fprintf(f,"    lw      $t5, %d($sp)\n", 4);
                        fprintf(f,"    lw      $t6, %d($sp)\n", 8);
                        fprintf(f,"    lw      $t7, %d($sp)\n", 12);
                        fprintf(f,"    lw      $s0, %d($sp)\n", 16);
                        fprintf(f,"    lw      $s1, %d($sp)\n", 20);
                        fprintf(f,"    addi    $sp, $sp, %d\n", 24);
                    }

                    if(lastCode) exitBlock(f,curBlock);
                    break;
                }
            }
            if(lastCode) break;
            else curCode = curCode->next;
        }
        curBlock = curBlock->next;
    }
    /* End of Function */
    fprintf(f,"\n");
}

tempVarItem * getTempVarItemByID(asmBlock * b, int ID) {
    tempVarItem * res = NULL;
    for(int i = 0;i < b->tempVarCount;i++){
        if(b->tempVarTable[i].tempVarID == ID){
            res = &(b->tempVarTable[i]);
            break;
        }
    }
    return res;
}

localVarItem * getLocalVarItemByID(asmFunction * f, int ID) {
    localVarItem * res = NULL;
    for(int i = 0;i < f->localVarCount;i++){
        if(f->localVarTable[i].varID == ID){
            res = &(f->localVarTable[i]);
            break;
        }
    }
    return res;
}

paramItem * getParamItemByID(asmFunction * f, int ID) {
    paramItem * res = NULL;
    for(int i = 0;i < f->paramCount;i++){
        if(f->paramTable[i].paramID == ID){
            res = &(f->paramTable[i]);
            break;
        }
    }
    return res;
}

/* Allocate a reg for temp variable and return its ID. */
int getReg(FILE * f, asmBlock * curBlock) {
    int res = 12;
    for(;res < 24;res++){
        /* find empty reg */
        if(regs[res].free) break;
    }
    if(res == 24){
        /* spill one */
        for(int i = 0;i < curBlock->tempVarCount;i++){
            tempVarItem * replace = &(curBlock->tempVarTable[i]);
            if(replace->inReg){
                res = replace->storedreg;
                if(replace->offset != NOTEXIST){
                    fprintf(f,"    sw      %s, %d($t3)\n",regs[res].name,replace->offset);
                }else{
                    fprintf(f,"    addi    $sp, $sp, %d\n",(-4));
                    fprintf(f,"    sw      %s, 0($sp)\n",regs[res].name);
                    curBlock->tempVarStackSize += 4;
                    replace->offset = (-curBlock->tempVarStackSize);
                }
                replace->inReg = false; replace->storedreg = NOTEXIST;
                regs[res].free = true;
                break;
            }
        }
    }
    return res;
}

/* Load the value of operand into target register. Assume the value is valid. */
void LoadToReg(operand * src, reg * dst, FILE * f, asmFunction * curFunction, asmBlock * curBlock, irCode * curCode) {
    switch(src->operandClass) {
        case IR(VAR) : {
            localVarItem * v = getLocalVarItemByID(curFunction,src->info.variable.Tag);
            fprintf(f,"    lw      %s, %d($t3)\n",dst->name,v->offset);
            break;
        }
        case IR(PARAM) : {
            paramItem * p = getParamItemByID(curFunction,src->info.parameter.Tag);
            fprintf(f,"    lw      %s, %d($fp)\n",dst->name,p->offset);
            break;
        }
        case IR(TEMP) : {
            tempVarItem * t = getTempVarItemByID(curBlock,src->info.tempVar.Tag);
            if(t->inReg){
                /* value in register */
                fprintf(f,"    move    %s, %s\n",dst->name,regs[t->storedreg].name);
                if(t->lastUsage == curCode){
                    /* temp name will not be used forever */
                    t->inReg = false; regs[t->storedreg].free = true; t->storedreg = NOTEXIST;
                }
            }else{
                if(t->offset != NOTEXIST){
                    /* temp name in memory */
                    fprintf(f,"    lw      %s, %d($t3)\n",dst->name,t->offset);
                }else{
                    /* error! */
                }
            }
            break;
        }
        case IR(INT) : {
            fprintf(f,"    li      %s, %d\n",dst->name,src->info.integerVal);
            break;
        }
        case IR(FLOAT) : {
            /* not support in lab4 */
            break;
        }
    }
    dst->free = false;
}

/* Store the value in reg into target place. */
void StoreFromReg(reg * src, operand * dst, FILE * f, asmFunction * curFunction, asmBlock * curBlock, irCode * curCode) {
    switch(dst->operandClass) {
        case IR(VAR) : {
            localVarItem * v = getLocalVarItemByID(curFunction,dst->info.variable.Tag);
            fprintf(f,"    sw      %s, %d($t3)\n", src->name, v->offset);
            break;
        }
        case IR(PARAM) : {
            paramItem * p = getParamItemByID(curFunction,dst->info.parameter.Tag);
            fprintf(f,"    sw      %s, %d($fp)\n", src->name, p->offset);
            break;
        }
        case IR(TEMP) : {
            tempVarItem * t = getTempVarItemByID(curBlock,dst->info.tempVar.Tag);
            if(t->lastUsage != curCode){
                /* temp name will be used later */
                if(t->offset != NOTEXIST){
                    /* temp name store in memory */
                    fprintf(f,"    sw      %s, %d($t3)\n", src->name,t->offset);
                }else{
                    /* temp name store in reg */
                    if(!t->inReg){
                        /* temp name first appear */
                        /* allocate a reg */
                        t->storedreg = getReg(f,curBlock);
                        t->inReg = true;
                    }
                    /* temp name is in reg */
                    fprintf(f,"    move    %s, %s\n", regs[t->storedreg].name,src->name);
                    regs[t->storedreg].free = false;
                }
            }
            break;
        }
    }
}

/* exit a block of code */
void exitBlock(FILE * f,asmBlock * curBlock) {
    for(int i = 12;i < 24;i++) regs[i].free = true;
    if(curBlock->tempVarStackSize != 0){
        fprintf(f,"    addi    $sp, $sp, %d\n",curBlock->tempVarStackSize);
    }
}