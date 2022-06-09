#include "asm.h"

reg regs[32] = {
    {"$zero", true}, {"$at"  , true}, {"$v0"  , true}, {"$v1"  , true},
    {"$a0"  , true}, {"$a1"  , true}, {"$a2"  , true}, {"$a3"  , true},

    {"$t0"  , true}, {"$t1"  , true}, {"$t2"  , true}, {"$t3"  , true},

    {"$t4"  , true}, {"$t5"  , true}, {"$t6"  , true}, {"$t7"  , true},
    {"$s0"  , true}, {"$s1"  , true}, {"$s2"  , true}, {"$s3"  , true},
    {"$s4"  , true}, {"$s5"  , true}, {"$s6"  , true}, {"$s7"  , true},
    
    {"$t8"  , true}, {"$t9"  , true}, {"$k0"  , true}, {"$k1"  , true},
    
    {"$gp"  , true}, {"$sp"  , true}, {"$fp"  , true}, {"$ra"  , true}
};

asmFunction * analysisIRcode(irSystem * sys);
void init(FILE* f);
void translateFunctionToAsm(FILE * f, asmFunction * function);

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
    "   _prompt: .asciiz \"Enter an integer:\"\n"
    "   _ret: .asciiz \"\\n\"\n"
    "\n"
    ".text\n"
    ".global main\n"
    "\n"
    "read:\n"
    "   li      $v0, 4\n"
    "   la      $a0, _prompt\n"
    "   syscall\n"
    "   li      $v0, 5\n"
    "   syscall\n"
    "   jr      $ra\n"
    "\n"
    "write:\n"
    "   li      $v0, 1\n"
    "   syscall\n"
    "   li      $v0, 4\n"
    "   la      $a0, _ret\n"
    "   syscall\n"
    "   move    $v0, $0\n"
    "   jr      $ra\n"
    "\n";
    fprintf(f,"%s",initCode);
}

asmFunction * analysisIRcode(irSystem * sys){
    asmFunction * functionListHead = NULL;
    asmFunction * curFunction = functionListHead;
    asmBlock * curBlock = NULL;
    irCode * curCode = sys->codeListHead;
    while(curCode){
        switch(curCode->code.instr){
            case IS(FUNCTION) : {

            }
            case IS(LABEL) : {

                break;
            }
        }
        curCode = curCode->next;
    }

    return functionListHead;
}