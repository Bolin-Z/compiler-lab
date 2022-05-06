#include "ir.h"

/* Private interface */

operandPool * creatOperandPool(operandPool * oldPoolList);
void destoryOperandPoolList(operandPool * poolList);
void destoryIrCodeList(irCode * codeListHead);
void fprintfOperand(FILE * f, operand * op);

static operand * zeroIrOperand;
static operand * oneIROperand;
static operand * minTypeWidthIROperand;

operand * zeroOperand(){return zeroIrOperand;}
operand * oneOperand(){return oneIROperand;}
operand * minTypeWidthOperand(){return minTypeWidthIROperand;}


/* Implementation */

/* Creat an ir translation system */
irSystem * creatIrSystem(){
    irSystem * sys = (irSystem *)malloc(sizeof(irSystem));
    if(sys){
        sys->codeListHead = sys->codeListTail = NULL;
        sys->poolList = creatOperandPool(NULL);
        sys->counter.function = 1;
        sys->counter.variable = 0;
        sys->counter.tempVar = 0;
        sys->counter.parameter = 0;
        sys->counter.label = 0;
        zeroIrOperand = creatOperand(sys,IR(INT),0);
        oneIROperand = creatOperand(sys,IR(INT),1);
        minTypeWidthIROperand = creatOperand(sys,IR(INT),4);
    }
    return sys;
}

/* Delete the ir translation system */
void destoryIrSystem(irSystem * sys){
    if(sys){
        destoryOperandPoolList(sys->poolList);
        destoryIrCodeList(sys->codeListHead);
        free(sys);
    }
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
    Immediate int               : creatOperand(irSystem * sys, IR(INT), int val)
    Immediate float             : creatOperand(irSystem * sys, IR(FLOAT), float val)
    Funciton operand            : creatOperand(irSystem * sys, IR(FUN), bool isMain)
    Label operand               : creatOperand(irSystem * sys, IR(LABEL))
    Size                        : creatOperand(irSystem * sys, IR(SIZE), int size)
    Variable | Temp | Parameter : creatOperand(irSystem * sys, IR(TEMP) | IR(VAR) | IR(PARAM), int modifier) 
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
            newOperand->info.variable.Tag = sys->counter.variable;
            sys->counter.variable += 1;
            newOperand->info.variable.modifier = va_arg(ap,int);
        } break;
        case IR(TEMP) : {
            newOperand->info.tempVar.Tag = sys->counter.tempVar;
            sys->counter.tempVar += 1;
            newOperand->info.tempVar.modifier = va_arg(ap,int);
        } break;
        case IR(PARAM) : {
            newOperand->info.parameter.Tag = sys->counter.parameter;
            sys->counter.parameter += 1;
            newOperand->info.parameter.modifier = va_arg(ap,int);
        }
        case IR(INT) : {
            newOperand->info.integerVal = va_arg(ap,int);
        } break;
        case IR(FLOAT) : {
            newOperand->info.floatVal = (float)(va_arg(ap,double));
        } break;
        case IR(LABEL) : {
            newOperand->info.labelTag = sys->counter.label;
            sys->counter.label += 1;
        } break;
        case IR(SIZE) : {
            newOperand->info.sizeVal = va_arg(ap,int);
        } break;
        case IR(FUN) : {
            bool isMain = (bool)(va_arg(ap,int));
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

/*
    Creat a new operand same as src and return a pointer to it
*/
operand * copyOperand(irSystem * sys, operand * src){
    operand * dst = NULL;
    if(src){
        if(sys->poolList->curEmpty == POOLSIZE){
            sys->poolList = creatOperandPool(sys->poolList);
            if(sys->poolList->curEmpty != 0) return NULL;
        }
        dst = &sys->poolList->operands[sys->poolList->curEmpty];
        sys->poolList->curEmpty += 1;
        
        dst->operandClass = src->operandClass;
        switch(src->operandClass){
            case IR(VAR) : {
                dst->info.variable.Tag = src->info.variable.Tag;
                dst->info.variable.modifier = src->info.variable.modifier;
                break;
            }
            case IR(TEMP) : {
                dst->info.tempVar.Tag = src->info.tempVar.Tag;
                dst->info.tempVar.modifier = src->info.tempVar.modifier;
                break;
            }
            case IR(PARAM) : {
                dst->info.parameter.Tag = src->info.parameter.Tag;
                dst->info.parameter.modifier = src->info.parameter.modifier;
                break;
            }
            case IR(INT)   : dst->info.integerVal = src->info.integerVal; break;
            case IR(FLOAT) : dst->info.floatVal = src->info.floatVal; break;
            case IR(LABEL) : dst->info.labelTag = src->info.labelTag; break;
            case IR(SIZE)  : dst->info.sizeVal = src->info.sizeVal; break;
            case IR(FUN)   : dst->info.funTag = src->info.funTag; break;        
            default : {
                /* Invalid operandclass */
                dst = NULL;
                sys->poolList->curEmpty -= 1;
            }
        }
    }
    return dst;
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
void fprintfIrCode(FILE * f, irSystem * sys){
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
            case IR(VAR)   : {
                switch(op->info.variable.modifier){
                    case IR(ACCESSADDR) : fprintf(f,"&"); break;
                    case IR(ACCESSVAL)  : fprintf(f,"*"); break;
                    default : break;
                }
                fprintf(f,"v%d",op->info.variable.Tag); 
                break;
            }
            case IR(TEMP)  : {
                switch(op->info.tempVar.modifier){
                    case IR(ACCESSADDR) : fprintf(f,"&"); break;
                    case IR(ACCESSVAL)  : fprintf(f,"*"); break;
                    default : break;
                }
                fprintf(f,"t%d",op->info.tempVar.Tag); 
                break;
            }
            case IR(PARAM) : {
                switch(op->info.parameter.modifier){
                    case IR(ACCESSADDR) : fprintf(f,"&"); break;
                    case IR(ACCESSVAL)  : fprintf(f,"*"); break;
                    default : break;
                }
                fprintf(f,"p%d",op->info.parameter.Tag);
                break;
            }
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