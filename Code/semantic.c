#include "semantic.h"

#define SA(RETURN,NAME,...) RETURN SemanticAnalysis##NAME (struct CST_node* n, SymbolTable * symtab, irSystem * irSys,##__VA_ARGS__)
SA(void, Program);
SA(void, ExtDefList);
SA(void, ExtDef);
SA(TypeDescriptor*, Specifier);
SA(void, DefList, bool fields);
SA(void, Def, bool field);
SA(Symbol*, VarDec, TypeDescriptor * basetype, bool field);
SA(Symbol*, FunDec, TypeDescriptor * returntype, bool declaration);
SA(TypeDescriptor*, Exp, bool LeftHand);
SA(void, StmtList, TypeDescriptor * returntype);
SA(void, Stmt, TypeDescriptor * returntype); // Working ON

/* Wrap-up function of semantic analysis stage */
void SemanticAnalysis(struct CST_node* root, irSystem * irSys){
    CreatTypeSystem();
    SymbolTable * symtable = CreatSymbolTable();

    SemanticAnalysisProgram(root,symtable,irSys);
    OutputSemanticErrorMessage();

    DestorySymbolTable(symtable);
    DestoryTypeSystem();
}

/* Program := ExtDefList */
SA(void, Program){
    OpenScope(symtab,"Global_Scope");
    
    /* Insert read() and write() */
    Symbol * readFun = Insert(symtab,"read");
    readFun->attribute.IdClass = FUNCTION;
    readFun->attribute.IdType = BasicInt();
    readFun->attribute.Info.Func.Argc = 0;
    readFun->attribute.Info.Func.ArgTypeList = NULL;
    readFun->attribute.Info.Func.defined = UpdateFunctionState(0,"read",true);
    readFun->attribute.irOperand = NULL;
    Symbol * writeFun = Insert(symtab,"write");
    writeFun->attribute.IdClass = FUNCTION;
    writeFun->attribute.IdType = BasicInt();
    writeFun->attribute.Info.Func.Argc = 1;
    writeFun->attribute.Info.Func.ArgTypeList = (TypeDescriptor**)malloc(sizeof(TypeDescriptor*));
    writeFun->attribute.Info.Func.ArgTypeList[0] = BasicInt();
    writeFun->attribute.Info.Func.defined = UpdateFunctionState(0,"write",true);
    writeFun->attribute.irOperand = NULL;

    SemanticAnalysisExtDefList(n->child_list[0],symtab,irSys);
    CloseScope(symtab);
}

SA(void,ExtDefList){
    struct CST_node * curExtDefList = n;
    while(curExtDefList->child_cnt != 0){
        SemanticAnalysisExtDef(curExtDefList->child_list[0],symtab,irSys);
        curExtDefList = curExtDefList->child_list[1];
    }
}

/* ExtDef := Specifier ExtDecList SEMI | Specifier SEMI | Specifier FunDec CompSt | Specifier FunDec SEMI */
SA(void, ExtDef){
    int Production;
    switch(get_symtype(n->child_list[1]->compact_type)){
        case SYM(ExtDecList) : Production = 1; break;
        case SYM(SEMI) : Production = 2; break;
        case SYM(FunDec) : 
            Production = (get_symtype(n->child_list[2]->compact_type) != SYM(SEMI)) ? 3 : 4;
            break;
        default : Production = 0; break;
    }
    switch(Production){
        case 1 : /* Specifier ExtDecList SEMI */
            {
                /* basetype == BasicError() is acceptable, however BasicTypeError will be assigned to IDs. */
                TypeDescriptor * basetype = SemanticAnalysisSpecifier(n->child_list[0],symtab,irSys);
                struct CST_node * curExtDecList = n->child_list[1];
                while(true){
                    SemanticAnalysisVarDec(curExtDecList->child_list[0],symtab,irSys,basetype,false);
                    if(curExtDecList->child_cnt == 1) break;
                    else curExtDecList = curExtDecList->child_list[2];
                }
                break;
            }
        case 2 : /* Specifier SEMI */
            {
                SemanticAnalysisSpecifier(n->child_list[0],symtab,irSys);
                break;
            }
        case 3 : /* Specifier FunDec CompSt */
            {
                TypeDescriptor * rttype = SemanticAnalysisSpecifier(n->child_list[0],symtab,irSys);
                Symbol * newfun = SemanticAnalysisFunDec(n->child_list[1],symtab,irSys,rttype,true);
                /* Semantic Analysis CompSt */
                SemanticAnalysisDefList(n->child_list[2]->child_list[1],symtab,irSys,false);
                SemanticAnalysisStmtList(n->child_list[2]->child_list[2],symtab,irSys,newfun->attribute.IdType);
                CloseScope(symtab);
                break;
            }
        case 4 : /* Specifier FunDec SEMI */
            {
                TypeDescriptor * rttype = SemanticAnalysisSpecifier(n->child_list[0],symtab,irSys);
                SemanticAnalysisFunDec(n->child_list[1],symtab,irSys,rttype,false);
                break;
            }
        default : /* error */
            break;
    }
}

/* Specifier := TYPE | StrucSpecifier */
/*
    Creat a TypeDescriptor and return a pointer to it.
*/
SA(TypeDescriptor*, Specifier){
    switch(get_symtype(n->child_list[0]->compact_type)){
        case SYM(TYPE) :
            {
                struct CST_mul_node * cur = (struct CST_mul_node * )(n->child_list[0]);
                switch(cur->tktype){
                    case TK(INT) : return BasicInt();
                    case TK(FLOAT) : return BasicFloat();
                    default : /* error */
                        return BasicError();
                }
            }
        case SYM(StructSpecifier) :
            {
                struct CST_node * st = n->child_list[0];
                switch(st->child_cnt){
                    case 2 : /* STRUCT Tag */
                        {
                            struct CST_node * tag = st->child_list[1];
                            struct CST_id_node * id = (struct CST_id_node *)(tag->child_list[0]);
                            Symbol*  type = LookUp(symtab,id->ID,false);
                            if(!type || type->attribute.IdClass != TYPENAME){
                                /* Use undefined struct type. */
                                ReportSemanticError(tag->lineno,17,NULL);
                                return BasicError();
                            }else{
                                return CopyTypeDescriptor(type->attribute.IdType);
                            }
                        }
                    case 5 : /* STRUCT OptTag LC DefList RC */
                        {
                            struct CST_node * opttag = st->child_list[1];
                            struct CST_node * deflist = st->child_list[3];
                            Symbol * newst = NULL;
                            if(opttag->child_cnt != 0){
                                char * stname = ((struct CST_id_node *)(opttag->child_list[0]))->ID;
                                /* structure name is defined globally */
                                if(LookUp(symtab,stname,false) != NULL){
                                    /* Duplicate definition of struct name. */
                                    ReportSemanticError(opttag->lineno,16,NULL);
                                }
                                newst = Insert(symtab,stname);
                                newst->attribute.IdClass = TYPENAME;
                            }
                            /* Construct a structure TypeDescriptor */
                            Scope * newscope = OpenScope(symtab,"Struct_Field");
                            SemanticAnalysisDefList(deflist,symtab,irSys,true);
                            /* Construct FieldList */
                            FieldList * head = NULL;
                            FieldList * pre = NULL;
                            for(int i = newscope->scopebeginidx;i < newscope->scopeendidx;i++){
                                Symbol * tempsym = Access(symtab,i);
                                if(tempsym->attribute.IdClass == VARIABLE){
                                    FieldList * f = CreatField();
                                    f->FieldName = tempsym->id;
                                    f->FieldType = CopyTypeDescriptor(tempsym->attribute.IdType);
                                    if(!head) head = f;
                                    if(pre) pre->NextField = f;
                                    pre = f;
                                }
                            }
                            CloseScope(symtab);
                            TypeDescriptor * newsttype = CreatStructureDescriptor(head,false);
                            if(newst) newst->attribute.IdType = newsttype;
                            return newsttype;
                        }
                    default : /* error */
                        return BasicError();
                }
            }
        default : /* error */
            return BasicError();
    }
}

SA(void, DefList, bool fields){
    struct CST_node * curDefList = n;
    while(curDefList->child_cnt != 0){
        SemanticAnalysisDef(curDefList->child_list[0],symtab,irSys,fields);
        curDefList = curDefList->child_list[1];
    }
}

/* Def := Specifier DecList SEMI */
SA(void, Def, bool field){
    /* basetype == BasicError() is acceptable, however BasicTypeError will be assigned to IDs. */
    TypeDescriptor * basetype = SemanticAnalysisSpecifier(n->child_list[0],symtab,irSys);
    struct CST_node * curDecList = n->child_list[1];
    while(true){
        struct CST_node * curDec = curDecList->child_list[0];
        struct CST_node * curVarDec = curDec->child_list[0];
        Symbol * newsymbol = SemanticAnalysisVarDec(curVarDec,symtab,irSys,basetype,field);
        if(curDec->child_cnt == 3){
            /* VarDec ASSIGNOP Exp */
            TypeDescriptor * exptype = SemanticAnalysisExp(curDec->child_list[2],symtab,irSys,false);
            if(field){
                /* Initialization of field. */
                ReportSemanticError(curDec->lineno,15,NULL);
            }else{
                if((!IsErrorType(exptype)) && (!IsErrorType(newsymbol->attribute.IdType)) && (!IsEqualType(newsymbol->attribute.IdType,exptype))){
                    /* Type mismatched for assignment. */
                    ReportSemanticError(curDec->lineno,5,NULL);
                }
            }
        }
        if(curDecList->child_cnt == 1) break;
        else curDecList = curDecList->child_list[2];
    }
}

/* VarDec := ID | VarDec LB INT RB */
SA(Symbol*, VarDec, TypeDescriptor * basetype, bool field){
    struct CST_node * curVarDec = n;
    TypeDescriptor * pretype = basetype;
    while(curVarDec->child_cnt != 1){
        int arsize = ((struct CST_int_node *)(curVarDec->child_list[2]))->intval;
        pretype = CreatArrayDescriptor(pretype,arsize,false);
        curVarDec = curVarDec->child_list[0];
    }
    char * varname = ((struct CST_id_node*)(curVarDec->child_list[0]))->ID;
    Symbol * checkglobal = LookUp(symtab,varname,false);
    Symbol * checklocal = LookUp(symtab,varname,true);
    if(checklocal != NULL){
        if(field){
            /* Redefinition of field. */
            ReportSemanticError(curVarDec->lineno,15,NULL);
        }else{
            /* Duplicate definition of variable */
            ReportSemanticError(curVarDec->lineno,3,NULL);
        }
    }else{
        if(checkglobal != NULL){
            if(checkglobal->attribute.IdClass != VARIABLE){
                /* Duplicate definition of variable */
                ReportSemanticError(curVarDec->lineno,3,NULL);
            }
        }
    }
    Symbol * newsymbol = Insert(symtab,varname);
    newsymbol->attribute.IdClass = VARIABLE;
    newsymbol->attribute.IdType = pretype;
    return newsymbol;
}

/* FunDec := ID LP RP | ID LP VarList RP */
SA(Symbol*, FunDec, TypeDescriptor * returntype, bool definition){
    char * funid = ((struct CST_id_node*)(n->child_list[0]))->ID;
    Symbol * newfun = NULL;
    Symbol * oldfun = LookUp(symtab,funid,false);
    if((oldfun != NULL) && (oldfun->attribute.IdClass != FUNCTION)){
        /* Duplicate definition of variable. */
        ReportSemanticError(n->lineno,3,NULL);
        oldfun = NULL;
    }

    if(oldfun != NULL){
        if(oldfun->attribute.Info.Func.defined && definition){
            /* Redefinition of function. */
            ReportSemanticError(n->lineno,4,NULL);
            oldfun = NULL;
        }else{
            if(!IsEqualType(returntype,oldfun->attribute.IdType)){
                /* Function declaration or definition conflict. */
                ReportSemanticError(n->lineno,19,NULL);
            }
        }
    }
    /* always insert */
    newfun = Insert(symtab,funid);
    newfun->attribute.IdClass = FUNCTION;
    newfun->attribute.IdType = returntype;
    newfun->attribute.Info.Func.defined = UpdateFunctionState(n->lineno,funid,definition);

    Scope * funscope = OpenScope(symtab,newfun->id);

    int newArgc = 0;
    TypeDescriptor ** newArgTypeList = NULL;
    if(n->child_cnt == 4){
        struct CST_node * curVarList = n->child_list[2];
        while(true){
            struct CST_node * curParamDec = curVarList->child_list[0];
            struct CST_node * curSpecifier = curParamDec->child_list[0];
            struct CST_node * curVarDec = curParamDec->child_list[1];
            TypeDescriptor * basetype = SemanticAnalysisSpecifier(curSpecifier,symtab,irSys);
            SemanticAnalysisVarDec(curVarDec,symtab,irSys,basetype,false);
            newArgc += 1;
            if(curVarList->child_cnt == 1) break;
            else curVarList = curVarList->child_list[2];
        }
    }
    newArgTypeList = (newArgc == 0) ? NULL : (TypeDescriptor**)malloc(newArgc*sizeof(TypeDescriptor*));
    for(int idx = funscope->scopebeginidx, i = 0;idx < funscope->scopeendidx;idx++){
        Symbol * cursymbol = Access(symtab,idx);
        if(cursymbol->attribute.IdClass == VARIABLE){
            newArgTypeList[i] = CopyTypeDescriptor(cursymbol->attribute.IdType);
            i++;
        }
    }

    if(oldfun != NULL){
        if(oldfun->attribute.Info.Func.Argc != newArgc){
            /* Function declaration or definition conflict. */
            ReportSemanticError(n->lineno,19,NULL);
        }else{
            for(int i = 0;i < newArgc;i++){
                if(!IsEqualType(newArgTypeList[i],oldfun->attribute.Info.Func.ArgTypeList[i])){
                    /* Function declaration or definition conflict. */
                    ReportSemanticError(n->lineno,19,NULL);
                    break;
                }
            }
        }
    }

    newfun->attribute.Info.Func.Argc = newArgc;
    newfun->attribute.Info.Func.ArgTypeList = newArgTypeList;

    if(!definition) CloseScope(symtab);
    return newfun;
}

/* 
    Return a pointer to a TypeDescriptor that describes Exp.
    No new TypeDescriptor will be created.
*/
SA(TypeDescriptor*, Exp, bool LeftHand){
    int Production;
    switch(n->child_cnt){
        case 1 :
            switch(get_symtype(n->child_list[0]->compact_type)){
                case SYM(ID) : Production = 16; break;
                case SYM(INT) : Production = 17; break;
                case SYM(FLOAT) : Production = 18; break;
                default : Production = 0; break;
            }
            break;
        case 2 :
            switch(get_symtype(n->child_list[0]->compact_type)){
                case SYM(MINUS) : Production = 10; break;
                case SYM(NOT) : Production = 11; break;
                default : Production = 0; break;
            }
            break;
        case 3 :
            switch(get_symtype(n->child_list[1]->compact_type)){
                case SYM(ASSIGNOP) : Production = 1; break;
                case SYM(AND) : Production = 2; break;
                case SYM(OR) : Production = 3; break;
                case SYM(RELOP) : Production = 4; break;
                case SYM(PLUS) : Production = 5; break;
                case SYM(MINUS) : Production = 6; break;
                case SYM(STAR) : Production = 7; break;
                case SYM(DIV) : Production = 8; break;
                case SYM(Exp) : Production = 9; break;
                case SYM(LP) : Production = 13; break;
                case SYM(DOT) : Production = 15; break;
                default : Production = 0; break;
            }
            break;
        case 4 :
            switch(get_symtype(n->child_list[1]->compact_type)){
                case SYM(LP) : Production = 12; break;
                case SYM(LB) : Production = 14; break;
                default : Production = 0; break;
            }
            break;
        default : Production = 0; break;
    }
    switch(Production){
        case 1 : /* Exp ASSIGNOP Exp */
            {
                TypeDescriptor * lexp = SemanticAnalysisExp(n->child_list[0],symtab,irSys,true);
                TypeDescriptor * rexp = SemanticAnalysisExp(n->child_list[2],symtab,irSys,LeftHand);
                if(IsErrorType(lexp) || IsErrorType(rexp)){
                    /* lexp or rexp contains error */
                    return BasicError();
                }else{
                    if(!IsEqualType(lexp,rexp)){
                        /* Type mismatched for assignment. */
                        ReportSemanticError(n->lineno,5,NULL);
                        return BasicError();
                    }
                    return lexp;
                }
            }
        case 2 : /* Exp AND Exp */
        case 3 : /* Exp OR Exp */
            {
                if(LeftHand){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                }else{
                    TypeDescriptor * lexp = SemanticAnalysisExp(n->child_list[0],symtab,irSys,false);
                    TypeDescriptor * rexp = SemanticAnalysisExp(n->child_list[2],symtab,irSys,false);
                    if(IsErrorType(lexp) || IsErrorType(rexp)){
                        /* lexp or rexp contains error */
                        return BasicError();
                    }else{
                        bool lint = IsEqualType(lexp,BasicInt());
                        bool rint = IsEqualType(rexp,BasicInt());
                        if(!lint){
                            /* Type mismatched for operands. */
                            ReportSemanticError(n->child_list[0]->lineno,7,NULL);
                        }
                        if(!rint){
                            /* Type mismatched for operands. */
                            ReportSemanticError(n->child_list[2]->lineno,7,NULL);
                        }
                        if(lint && rint) return BasicInt();
                        else return BasicError();
                    }
                }
            }
        case 4 : /* Exp RELOP Exp */
            {
                if(LeftHand){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                }else{
                    TypeDescriptor * lexp = SemanticAnalysisExp(n->child_list[0],symtab,irSys,false);
                    TypeDescriptor * rexp = SemanticAnalysisExp(n->child_list[2],symtab,irSys,false);
                    if(IsErrorType(lexp) || IsErrorType(rexp)){
                        return BasicError();
                    }else{
                        if(IsEqualType(lexp,rexp)){
                            if(IsEqualType(lexp,BasicInt()) || IsEqualType(lexp,BasicFloat())){
                                /* Relation operation should return boolean value */
                                return BasicInt();
                            }
                            else{
                                /* Type mismatched for operands. */
                                ReportSemanticError(n->lineno,7,NULL);
                                return BasicError();
                            }                            
                        }
                        else{
                            /* Type mismatched for operands. */
                            ReportSemanticError(n->lineno,7,NULL);
                            return BasicError();
                        }
                    }
                }
            }
        case 5 : /* Exp PLUS Exp */
        case 6 : /* Exp MINUS Exp */
        case 7 : /* Exp STAR Exp */
        case 8 : /* Exp DIV Exp */
            {
                if(LeftHand){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                }else{
                    TypeDescriptor * lexp = SemanticAnalysisExp(n->child_list[0],symtab,irSys,false);
                    TypeDescriptor * rexp = SemanticAnalysisExp(n->child_list[2],symtab,irSys,false);
                    if(IsErrorType(lexp) || IsErrorType(rexp)){
                        return BasicError();
                    }else{
                        if(IsEqualType(lexp,rexp)){
                            if(IsEqualType(lexp,BasicInt()) || IsEqualType(lexp,BasicFloat()))
                                return lexp;
                            else{
                                /* Type mismatched for operands. */
                                ReportSemanticError(n->lineno,7,NULL);
                                return BasicError();
                            }                            
                        }
                        else{
                            /* Type mismatched for operands. */
                            ReportSemanticError(n->lineno,7,NULL);
                            return BasicError();
                        }
                    }
                }
            }
        case 9 : /* LP Exp RP */
            return SemanticAnalysisExp(n->child_list[1],symtab,irSys,LeftHand);
        case 10 : /* MINUS Exp */
            {
                if(LeftHand){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                }else{
                    TypeDescriptor * exp = SemanticAnalysisExp(n->child_list[1],symtab,irSys,false);
                    if(IsEqualType(exp,BasicInt()) || IsEqualType(exp,BasicFloat()) || IsErrorType(exp)){
                        return exp;
                    }else{
                        /* Type mismatched for operands. */
                        ReportSemanticError(n->child_list[1]->lineno,7,NULL);
                        return BasicError();
                    }
                }
            }
        case 11 : /* NOT Exp */
            {
                if(LeftHand){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                }else{
                    TypeDescriptor * exp = SemanticAnalysisExp(n->child_list[1],symtab,irSys,false);
                    if(IsEqualType(exp,BasicInt()) || IsErrorType(exp)){
                        return exp;
                    }else{
                        /* Type mismatched for operands. */
                        ReportSemanticError(n->child_list[1]->lineno,7,NULL);
                        return BasicError();
                    }
                }
            }
        case 12 : /* ID LP Args RP */
            {
                if(LeftHand){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                }else{
                    char * funid = ((struct CST_id_node *)(n->child_list[0]))->ID;
                    Symbol * fun = LookUp(symtab,funid,false);
                    if(fun == NULL){
                        /* Use undefined function. */
                        ReportSemanticError(n->lineno,2,NULL);
                        return BasicError();
                    }else if(fun->attribute.IdClass != FUNCTION){
                        /* Use function call operand on non-function identifier. */
                        ReportSemanticError(n->lineno,11,NULL);
                        return BasicError();
                    }else{
                        if(fun->attribute.Info.Func.defined == false){
                            /* Function was not defined yet. */
                            ReportSemanticError(n->lineno,2,fun->id);
                        }
                        struct CST_node * curArg = n->child_list[2];
                        int expectargnum = fun->attribute.Info.Func.Argc;
                        if(expectargnum == 0){
                            /* Arguments number or type mismatch. */
                            ReportSemanticError(n->lineno,9,NULL);
                            return BasicError();
                        }
                        for(int i = 0;i < expectargnum;i++){
                            struct CST_node * curExp = curArg->child_list[0];
                            TypeDescriptor * curExptype = SemanticAnalysisExp(curExp,symtab,irSys,false);
                            TypeDescriptor * expecttype = fun->attribute.Info.Func.ArgTypeList[i];
                            if(IsErrorType(curExptype)){
                                return BasicError();
                            }
                            if(!IsEqualType(expecttype,curExptype)){
                                /* Arguments number or type mismatch. */
                                ReportSemanticError(curExp->lineno,9,NULL);
                                return BasicError();
                            }
                            bool lastexparg = (i == (expectargnum - 1));
                            bool lastrealarg = (curArg->child_cnt == 1);
                            if((lastexparg && (!lastrealarg)) || ((!lastexparg) && lastrealarg)){
                                /* Arguments number or type mismatch. */
                                ReportSemanticError(n->lineno,9,NULL);
                                return BasicError();
                            }
                            curArg = curArg->child_list[2];
                        }
                        return fun->attribute.IdType;
                    }
                }
            }
        case 13 : /* ID LP RP */
            {
                if(LeftHand){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                }else{
                    char * funid = ((struct CST_id_node *)(n->child_list[0]))->ID;
                    Symbol * fun = LookUp(symtab,funid,false);
                    if(fun == NULL){
                        /* Use undefined function. */
                        ReportSemanticError(n->lineno,2,NULL);
                        return BasicError();
                    }else if(fun->attribute.IdClass != FUNCTION){
                        /* Use function call operand on non-function identifier. */
                        ReportSemanticError(n->lineno,11,NULL);
                        return BasicError();
                    }else{
                        if(fun->attribute.Info.Func.Argc != 0){
                            /* Arguments number or type mismatch. */
                            ReportSemanticError(n->lineno,9,NULL);
                            return BasicError();
                        }else{
                            return fun->attribute.IdType;
                        }
                    }
                }
            }
        case 14 : /* Exp LB Exp RB */
            {
                TypeDescriptor * lexp = SemanticAnalysisExp(n->child_list[0],symtab,irSys,LeftHand);
                TypeDescriptor * rexp = SemanticAnalysisExp(n->child_list[2],symtab,irSys,false);
                bool rterror = false;
                if(IsErrorType(lexp)){
                    rterror = true;
                }else{
                    if(lexp->TypeClass != ARRAY){
                        /* Use array access operand on non-array type variable. */
                        ReportSemanticError(n->lineno,10,NULL);
                        rterror = true;
                    }
                }
                if(IsErrorType(rexp)){
                    rterror = true;
                }else{
                    if(!IsEqualType(rexp,BasicInt())){
                        /* Expect interger inside array access operand. */
                        ReportSemanticError(n->child_list[2]->lineno,12,NULL);
                        rterror = true;
                    }
                }
                return (rterror) ? BasicError() : lexp->Array.elem;
            }
        case 15 : /* Exp DOT ID */
            {
                TypeDescriptor * exp = SemanticAnalysisExp(n->child_list[0],symtab,irSys,LeftHand);
                if(IsErrorType(exp)){
                    return BasicError();
                }else{
                    if(exp->TypeClass != STRUCTURE){
                        /* Use field access operand on non-struct type variable. */
                        ReportSemanticError(n->lineno,13,NULL);
                        return BasicError();
                    }else{
                        FieldList * curField = exp->Structure;
                        char * fieldid = ((struct CST_id_node *)(n->child_list[2]))->ID;
                        while(curField != NULL){
                            if(strcmp(fieldid,curField->FieldName) == 0){
                                return curField->FieldType;
                            }
                            curField = curField->NextField;
                        }
                        /* Access to undefined field. */
                        ReportSemanticError(n->lineno,14,NULL);
                        return BasicError();
                    }
                }         
            }
        case 16 : /* ID */
            {
                char * idname = ((struct CST_id_node*)(n->child_list[0]))->ID;
                Symbol * id = LookUp(symtab,idname,false);
                if(id == NULL){
                    /* Use undefined variable. */
                    ReportSemanticError(n->lineno,1,NULL);
                    return BasicError();
                }else{
                    if(id->attribute.IdClass != VARIABLE){
                        /* Use undefined variable. */
                        ReportSemanticError(n->lineno,1,NULL);
                        return BasicError();
                    }else{
                        return id->attribute.IdType;
                    }
                }
            }
        case 17 : /* INT */
            {
                if(LeftHand){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                }else{
                    return BasicInt();
                }
            }
        case 18 : /* FLOAT */
            {
                if(LeftHand){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                }else{
                    return BasicFloat();
                }
            }
        default : /* error */
            return BasicError();
    }
}

SA(void, StmtList, TypeDescriptor * returntype){
    struct CST_node * curStmtList = n;
    while(curStmtList->child_cnt != 0){
        SemanticAnalysisStmt(curStmtList->child_list[0],symtab,irSys,returntype);
        curStmtList = curStmtList->child_list[1];
    }
}

SA(void, Stmt, TypeDescriptor * returntype){
    int Production;
    switch(n->child_cnt){
        case 1 : Production = 2; break;
        case 2 : Production = 1; break;
        case 3 : Production = 3; break;
        case 5 : {
            switch(get_symtype(n->child_list[0]->compact_type)){
                case SYM(IF) : Production = 4; break;
                case SYM(WHILE) : Production = 6; break;
                default : Production = 0; break;
            }
            break;
        }
        case 7 : Production = 5; break;
        default : Production = 0; break;
    }
    switch(Production){
        case 1 : /* Exp SEMI */
            {
                SemanticAnalysisExp(n->child_list[0],symtab,irSys,false);
                break;
            }
        case 2 : /* CompSt */
            {
                OpenScope(symtab,"CompSt");
                SemanticAnalysisDefList(n->child_list[0]->child_list[1],symtab,irSys,false);
                SemanticAnalysisStmtList(n->child_list[0]->child_list[2],symtab,irSys,returntype);
                CloseScope(symtab);
                break;
            }
        case 3 : /* RETURN Exp SEMI */
            {
                TypeDescriptor * exptype = SemanticAnalysisExp(n->child_list[1],symtab,irSys,false);
                if(!IsEqualType(exptype,returntype))
                    /* Type mismatched for return type. */
                    ReportSemanticError(n->child_list[1]->lineno,8,NULL);
                break;
            }
        case 4 : /* IF LP Exp RP Stmt */
            {
                TypeDescriptor * exptype = SemanticAnalysisExp(n->child_list[2],symtab,irSys,false);
                if(!IsEqualType(exptype,BasicInt())){
                    /* Type mismatched for operands. */
                    ReportSemanticError(n->child_list[2]->lineno,7,NULL);
                }
                SemanticAnalysisStmt(n->child_list[4],symtab,irSys,returntype);
                break;
            }
        case 5 : /* IF LP Exp RP Stmt ELSE Stmt */
            {
                TypeDescriptor * exptype = SemanticAnalysisExp(n->child_list[2],symtab,irSys,false);
                if(!IsEqualType(exptype,BasicInt())){
                    /* Type mismatched for operands. */
                    ReportSemanticError(n->child_list[2]->lineno,7,NULL);
                }
                SemanticAnalysisStmt(n->child_list[4],symtab,irSys,returntype);
                SemanticAnalysisStmt(n->child_list[6],symtab,irSys,returntype);
                break;
            }
        case 6 : /* WHILE LP Exp RP Stmt */
            {
                TypeDescriptor * exptype = SemanticAnalysisExp(n->child_list[2],symtab,irSys,false);
                if(!IsEqualType(exptype,BasicInt())){
                    /* Type mismatched for operands. */
                    ReportSemanticError(n->child_list[2]->lineno,7,NULL);
                }
                SemanticAnalysisStmt(n->child_list[4],symtab,irSys,returntype);
                break;
            }
        default : break;
    }
}