#ifndef __IR_H__
#define __IR_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

/* Avoid name conflict */
#define IR(X) IRNAME##_##X
#define IS(X) INSTRUCTION_##X

typedef struct operand {
    enum {IR(VAR), IR(TEMP), IR(INT), IR(FLOAT), IR(LABEL), IR(SIZE), IR(FUN)} operandClass;
    union{
        int integerVal;
        float floatVal;
        int labelTag, funTag;
        struct {
            int Tag;
            enum {IR(NORMAL),IR(ACCESSADDR),IR(ACCESSVAL)} modifier;
        } variable, tempVar;
        int sizeVal;
    } info;
} operand;

#define POOLSIZE 256
/* Store empty operand object */
typedef struct operandPool {
    /* Singly Linked List */
    struct operandPool * next;
    operand operands[POOLSIZE];
    int curEmpty;
} operandPool;

/* Three-Address Code */
typedef struct quadruple {
    enum {
    /*  Operator           INSTRUCTION         RESULT        ARG1        ARG2 */
        IS(LABEL),     //  LABEL x :           x
        IS(FUNCTION),  //  FUNCTION f :        f
        IS(ASSIGN),    //  x := y              x             y
        IS(PLUS),      //  x := y + z          x             y           z
        IS(MINUS),     //  x := y - z          x             y           z
        IS(MUL),       //  x := y * z          x             y           z
        IS(DIV),       //  x := y / z          x             y           z
        IS(GETADDR),   //  x := &y             x             y
        IS(GETVAL),    //  x := *y             x             y
        IS(SETVAL),    //  *x := y             x             y
        IS(GOTO),      //  GOTO x              x
        IS(EQ),        //  IF x == y GOTO z    z             x           y
        IS(NEQ),       //  IF x != y GOTO z    z             x           y
        IS(LESS),      //  IF x < y GOTO z     z             x           y
        IS(LESSEQ),    //  IF x <= y GOTO z    z             x           y
        IS(GREATER),   //  IF x > y GOTO z     z             x           y
        IS(GREATEREQ), //  IF x >= y GOTO z    z             x           y
        IS(RETURN),    //  RETURN x            x
        IS(DEC),       //  DEC x [size]        x             size
        IS(ARG),       //  ARG x               x
        IS(CALL),      //  x := CALL f         x             f
        IS(PARAM),     //  PARAM x             x
        IS(READ),      //  READ x              x
        IS(WRITE)      //  WRITE x             x
    } instr;
    operand *result, *arg1, *arg2;
} quadruple;

typedef struct irCode {
    /* Double Linked List */
    struct irCode *prev, *next;
    quadruple code;
} irCode;

/* Data Structure to store all information */
typedef struct irSystem {
    irCode *codeListHead, *codeListTail;
    operandPool *poolList;
    struct {
        int function; // f0 = 'main'
        int variable, tempVar, label;
    } counter;
} irSystem;

/* Public interface */

irSystem * creatIrSystem();
void destoryIrSystem(irSystem * sys);
operand * creatOperand(irSystem * sys, int operandClass, ...);
operand * copyOperand(irSystem * sys, operand * src);
irCode * generateCode(irSystem * sys, int instruction, operand * result, operand * arg1, operand * arg2);
void fprintfIrCode(FILE * f, irSystem * sys);

/* Some frequently used operand. Do not modify. */
operand * zeroOperand();
operand * oneOperand();
operand * minTypeWidthOperand();

#endif