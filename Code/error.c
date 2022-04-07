#include"error.h"

bool has_error;


void error_msg(char error_type,int line_num,const char * format_msg, ...){
    has_error = true;
    va_list ap;
    va_start(ap,format_msg);
    /* defined the out put stream of error msg */
    FILE * output = ERROR_MSG_2;

    fprintf(output,"Error type %c at Line %d: ",error_type,line_num);
    vfprintf(output,format_msg,ap);
    fprintf(output, "\n");

    va_end(ap);
}

/* semantic error */

static const char *const semanticerror[]= {
/* 0 */   "No error",
/* 1 */   "Use undefined variable.",
/* 2 */   "Use undefined function.",
/* 3 */   "Duplicate definition of variable.",
/* 4 */   "Redefinition of function.",
/* 5 */   "Type mismatched for assignment.",
/* 6 */   "Appearance of rvalue on left hand side of assignment.",
/* 7 */   "Type mismatched for operands.",
/* 8 */   "Type mismatched for return type.",
/* 9 */   "Arguments number or type mismatch.",
/* 10 */  "Use array access operand on non-array type variable.",
/* 11 */  "Use function call operand on non-function identifier.",
/* 12 */  "Expect interger inside array access operand.",
/* 13 */  "Use field access operand on non-struct type variable.",
/* 14 */  "Access to undefined field.",
/* 15 */  "Redefinition or initialization of field.",
/* 16 */  "Duplicate definition of struct name.",
/* 17 */  "Use undefined struct type.",
/* 18 */  "Function was declared but not defined.",
/* 19 */  "Function declaration or definition conflict.",
          0
};

typedef struct SemanticErrorMsg{
    SemanticErrorMsg * next;
    int line;
    int semanticerrortype;
    SemanticErrorMsg * nextudfunmsg;
} SemanticErrorMsg;

typedef struct FunctionState{
    FunctionState * next;
    char * functionname;
    bool defined;
    SemanticErrorMsg * udfunmsglist;
} FunctionState;

SemanticErrorMsg * CreatMsg(int l, int type);
void DestoryMsg(SemanticErrorMsg * msg);
SemanticErrorMsg * AppendMsg(int l, int type);
FunctionState * CreatFunctionState(char * funid, bool def);
FunctionState * FindFunctionState(char * funid);
void DestoryFunctionState(FunctionState * s);

static SemanticErrorMsg * MsgListHead = NULL;
static SemanticErrorMsg * MsgListTail = NULL;
static FunctionState * FunList = NULL;

SemanticErrorMsg * CreatMsg(int l, int type){
    SemanticErrorMsg * msg = (SemanticErrorMsg*)malloc(sizeof(SemanticErrorMsg));
    if(msg){
        msg->next = NULL;
        msg->line = l;
        msg->semanticerrortype = type;
        msg->nextudfunmsg = NULL;
    }
    return msg;
}

void DestoryMsg(SemanticErrorMsg * msg){
    free(msg);
}

SemanticErrorMsg * AppendMsg(int l, int type){
    SemanticErrorMsg * newmsg = CreatMsg(l,type);
    if(MsgListTail == NULL){
        MsgListHead = newmsg;
        MsgListTail = newmsg;
    }else{
        MsgListTail->next = newmsg;
        MsgListTail = newmsg;
    }
    return newmsg;
}

FunctionState * CreatFunctionState(char * funid, bool def){
    FunctionState * s = (FunctionState*)malloc(sizeof(FunctionState));
    if(s){
        s->next = NULL;
        s->udfunmsglist = NULL;
        s->functionname = funid;
        s->defined = def;
    }
    return s;
}

FunctionState * FindFunctionState(char * funid){
    FunctionState * curCheck = FunList;
    while(curCheck != NULL){
        if(strcmp(funid,curCheck->functionname) == 0) break;
        curCheck = curCheck->next;
    }
    return curCheck;
}

void DestoryFunctionState(FunctionState * s){
    free(s);
}

bool UpdateFunctionState(int line, char * funid, bool definition){
    FunctionState * find = FindFunctionState(funid);
    if(find){
        if(!find->defined){
            if(definition){
                find->defined = true;
                SemanticErrorMsg * curmsg = find->udfunmsglist;
                while(curmsg != NULL){
                    curmsg->semanticerrortype = 0;
                    curmsg = curmsg->nextudfunmsg;
                }
            }else{
                /* Function was declared but not defined. */
                SemanticErrorMsg * msg = AppendMsg(line,18);
                msg->nextudfunmsg = find->udfunmsglist;
                find->udfunmsglist = msg;
            }
        }
        return find->defined;
    }else{
        FunctionState * newfun = CreatFunctionState(funid, definition);
        if(!definition){
            /* Function was declared but not defined. */
            newfun->udfunmsglist = AppendMsg(line,18);
        }
        newfun->next = FunList;
        FunList = newfun;
        return newfun->defined;
    }
}

void ReportSemanticError(int line, int errortype, char * funid){
    SemanticErrorMsg * newmsg = AppendMsg(line, errortype);
    if((errortype == 2) && (funid != NULL)){
        FunctionState * f = FindFunctionState(funid);
        newmsg->nextudfunmsg = f->udfunmsglist;
        f->udfunmsglist = newmsg;
    }
}

void OutputSemanticErrorMessage(){
    FunctionState * curfunstate = FunList;
    while(curfunstate != NULL){
        FunctionState * nextfun = curfunstate->next;
        DestoryFunctionState(curfunstate);
        curfunstate = nextfun;
    }
    SemanticErrorMsg * curmsg = MsgListHead;
    while(curmsg != NULL){
        SemanticErrorMsg * nextmsg = curmsg->next;

        if(curmsg->semanticerrortype != 0){
            fprintf(ERROR_MSG_2,"Error type %d at Line %d: %s\n",curmsg->semanticerrortype,curmsg->line,semanticerror[curmsg->semanticerrortype]);
            has_error = true;
        }
        
        DestoryMsg(curmsg);
        curmsg = nextmsg;
    }
}