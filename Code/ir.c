#include "ir.h"

/* Private interface */

operandPool * creatOperandPool(operandPool * oldPoolList);
void destoryOperandPoolList(operandPool * poolList);
void destoryIrCodeList(irCode * codeListHead);
void fprintfOperand(FILE * f, operand * op);

/* Implementation */

/* Creat an ir translation system */
irSystem * creatIrSystem(){
    irSystem * sys = (irSystem *)malloc(sizeof(irSystem));
    if(sys){
        sys->codeListHead = sys->codeListTail = NULL;
        sys->poolList = creatOperandPool(sys->poolList);
        sys->counter.function = 1;
        sys->counter.variable = 0;
        sys->counter.tempVar = 0;
        sys->counter.label = 0;
    }
    return sys;
}

/* Delete the ir translation system */
void * destoryIrSystem(irSystem * sys){
    destoryOperandPoolList(sys->poolList);
    destoryIrCodeList(sys->codeListHead);
    free(sys);
}

/* Add a new operandPool to poolList and return the new list */
operandPool * creatOperandPool(operandPool * oldPoolList){
    operandPool * newPoolList = oldPoolList;
    operandPool * newPool = (operandPool*)malloc(sizeof(operandPool)); 
    if(newPool){
        newPool->curEmpty = 0;
        newPool->next = oldPoolList;
        newPoolList = newPool;
    }
    return newPoolList;
}

void destoryOperandPoolList(operandPool * poolList){
    operandPool * curPool = poolList;
    while(curPool){
        operandPool * nextPool = curPool->next;
        free(curPool);
        curPool = nextPool;
    }
}

void destoryIrCodeList(irCode * codeListHead){
    irCode * curCode = codeListHead;
    while(curCode){
        irCode * nextCode = curCode->next;
        free(curCode);
        curCode = nextCode;
    }
}

/* 
    Creat a new operand and return a pointer to it
    Immediate int operand : creatOperand(irSystem * sys, int operandClass, int val)
    Immediate float operand : creatOperand(irSystem * sys, int operandClass, float val)
    Funciton operand : creatOperand(irSystem * sys, int operandClass, bool isMain)
    Size operand : creatOperand(irSystem * sys, int operandClass, int val)
    Other operand : creatOperand(irSystem * sys, int operandClass)
*/
operand * creatOperand(irSystem * sys, int operandClass, ...){
    if(sys->poolList->curEmpty == POOLSIZE){
        sys->poolList = creatOperandPool(sys->poolList);
        if(sys->poolList->curEmpty != 0) return NULL;
    }
    operand * newOperand = &sys->poolList->operands[sys->poolList->curEmpty];
    sys->poolList->curEmpty += 1;

    newOperand->operandClass = operandClass;
    va_list ap;
    va_start(ap,operandClass);
    switch(operandClass){
        case IR(VAR) : {
            newOperand->info.variableTag = sys->counter.variable;
            sys->counter.variable += 1;
        } break;
        case IR(TEMP) : {
            newOperand->info.tempVarTag = sys->counter.tempVar;
            sys->counter.tempVar += 1;
        } break;
        case IR(INT) : {
            newOperand->info.integerVal = va_arg(ap,int);
        }break;
        case IR(FLOAT) : {
            newOperand->info.floatVal = va_arg(ap,float);
        } break;
        case IR(LABEL) : {
            newOperand->info.labelTag = sys->counter.label;
            sys->counter.label += 1;
        } break;
        case IR(SIZE) : {
            newOperand->info.sizeVal = va_arg(ap,int);
        }
        case IR(FUN) : {
            bool isMain = va_arg(ap,bool);
            if(isMain) newOperand->info.funTag = 0;
            else {
                newOperand->info.funTag = sys->counter.function;
                sys->counter.function += 1;
            }
        } break;
        default : {
            /* Invalid operandclass */
            newOperand = NULL;
            sys->poolList->curEmpty -= 1;
        }
    }
    va_end(ap);

    return newOperand;
}

irCode * generateCode(irSystem * sys, int instruction, operand * result, operand * arg1, operand * arg2){
    irCode * newCode = (irCode*)malloc(sizeof(irCode));
    if(newCode){
        newCode->next = NULL;
        newCode->prev = sys->codeListTail;
        if(sys->codeListHead == NULL) sys->codeListHead = newCode;
        if(sys->codeListTail != NULL) sys->codeListTail->next = newCode;
        sys->codeListTail = newCode;

        newCode->code.instr = instruction;
        newCode->code.result = result;
        newCode->code.arg1 = arg1;
        newCode->code.arg2 = arg2;
    }
    return newCode;
}

/* Output all IrCode to file */
void * fprintfIrCode(FILE * f, irSystem * sys){
    irCode * curCode = sys->codeListHead;
    while(curCode){
        quadruple * c = & curCode->code;
        switch(c->instr){
            case IS(LABEL)     : fprintf(f,"LABEL "); fprintfOperand(f,c->result); fprintf(f," :"); break;
            case IS(FUNCTION)  : fprintf(f,"FUNCTION "); fprintfOperand(f,c->result); fprintf(f," :"); break;
            case IS(ASSIGN)    : fprintfOperand(f,c->result); fprintf(f," := "); fprintfOperand(f,c->arg1); fprintf(f,"\n"); break;
            case IS(PLUS)      : fprintfOperand(f,c->result); fprintf(f," := "); fprintfOperand(f,c->arg1); fprintf(f," + "); fprintfOperand(f,c->arg2); break;
            case IS(MINUS)     : fprintfOperand(f,c->result); fprintf(f," := "); fprintfOperand(f,c->arg1); fprintf(f," - "); fprintfOperand(f,c->arg2); break;
            case IS(MUL)       : fprintfOperand(f,c->result); fprintf(f," := "); fprintfOperand(f,c->arg1); fprintf(f," * "); fprintfOperand(f,c->arg2); break;
            case IS(DIV)       : fprintfOperand(f,c->result); fprintf(f," := "); fprintfOperand(f,c->arg1); fprintf(f," / "); fprintfOperand(f,c->arg2); break;
            case IS(GETADDR)   : fprintfOperand(f,c->result); fprintf(f," := &"); fprintfOperand(f,c->arg1); break;
            case IS(GETVAL)    : fprintfOperand(f,c->result); fprintf(f," := *"); fprintfOperand(f,c->arg1); break;
            case IS(SETVAL)    : fprintf(f,"*"); fprintfOperand(f,c->result); fprintf(f," := "); fprintfOperand(f,c->arg1); break;
            case IS(GOTO)      : fprintf(f,"GOTO "); fprintfOperand(f,c->result); break;
            case IS(EQ)        : fprintf(f,"IF "); fprintfOperand(f,c->arg1); fprintf(f," == "); fprintfOperand(f,c->arg2); fprintf(f," GOTO "); fprintfOperand(f,c->result); break;
            case IS(NEQ)       : fprintf(f,"IF "); fprintfOperand(f,c->arg1); fprintf(f," != "); fprintfOperand(f,c->arg2); fprintf(f," GOTO "); fprintfOperand(f,c->result); break;
            case IS(LESS)      : fprintf(f,"IF "); fprintfOperand(f,c->arg1); fprintf(f," < "); fprintfOperand(f,c->arg2); fprintf(f," GOTO "); fprintfOperand(f,c->result); break;
            case IS(LESSEQ)    : fprintf(f,"IF "); fprintfOperand(f,c->arg1); fprintf(f," <= "); fprintfOperand(f,c->arg2); fprintf(f," GOTO "); fprintfOperand(f,c->result); break;
            case IS(GREATER)   : fprintf(f,"IF "); fprintfOperand(f,c->arg1); fprintf(f," > "); fprintfOperand(f,c->arg2); fprintf(f," GOTO "); fprintfOperand(f,c->result); break;
            case IS(GREATEREQ) : fprintf(f,"IF "); fprintfOperand(f,c->arg1); fprintf(f," >= "); fprintfOperand(f,c->arg2); fprintf(f," GOTO "); fprintfOperand(f,c->result); break;
            case IS(RETURN)    : fprintf(f,"RETURN "); fprintfOperand(f,c->result); break;
            case IS(DEC)       : fprintf(f,"DEC "); fprintfOperand(f,c->result); fprintf(f," "); fprintfOperand(f,c->arg1); break;
            case IS(ARG)       : fprintf(f,"ARG "); fprintfOperand(f,c->result); break;
            case IS(CALL)      : fprintfOperand(f,c->result); fprintf(f," := CALL "); fprintfOperand(f,c->arg1); break;
            case IS(PARAM)     : fprintf(f,"PARAM "); fprintfOperand(f,c->result); break;
            case IS(READ)      : fprintf(f,"READ "); fprintfOperand(f,c->result); break;
            case IS(WRITE)     : fprintf(f,"WRITE "); fprintfOperand(f,c->result); break;
            default : /* Invalid instruction */ 
                fprintf(f,"INVALID INSTRUCTION !!!");
                break;
        }
        fprintf(f,"\n");
        curCode = curCode->next;
    }
}

/* Output an operand to file */
void fprintfOperand(FILE * f, operand * op){
    if(op){
        switch(op->operandClass){
            case IR(VAR)   : fprintf(f,"v%d",op->info.variableTag); break;
            case IR(TEMP)  : fprintf(f,"t%d",op->info.tempVarTag); break;
            case IR(INT)   : fprintf(f,"#%d",op->info.integerVal); break;
            case IR(FLOAT) : fprintf(f,"#%f",op->info.floatVal); break;
            case IR(LABEL) : fprintf(f,"label%d",op->info.labelTag); break;
            case IR(SIZE)  : fprintf(f,"%d",op->info.sizeVal); break;
            case IR(FUN)   : 
                if(op->info.funTag == 0) fprintf(f,"main");
                else fprintf(f,"f%d",op->info.funTag);
                break;
            default : break;
        }
    }
}