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
SA(TypeDescriptor*, Exp, bool leftVal, bool leftOfAssign, operand * expIrOperand);
SA(TypeDescriptor*, ExpCondition, operand * labelTrue, operand * labelFalse);
SA(int, ExpProduction);
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
                    Symbol * globalVariable = SemanticAnalysisVarDec(curExtDecList->child_list[0],symtab,irSys,basetype,false);
                    /* Do not generate code for global variable */
                    globalVariable->attribute.irOperand = NULL;

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
                            int structTypeWidth = 0;
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
                                    structTypeWidth += f->FieldType->typeWidth;
                                    if(!head) head = f;
                                    if(pre) pre->NextField = f;
                                    pre = f;
                                }
                            }
                            CloseScope(symtab);
                            TypeDescriptor * newsttype = CreatStructureDescriptor(head,false);
                            newsttype->typeWidth = structTypeWidth;
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

        if(!field){
            /* local variable declaration starts here */
            newsymbol->attribute.irOperand = creatOperand(irSys,IR(VAR),IR(NORMAL));
            if(newsymbol->attribute.IdType->TypeClass == ARRAY || newsymbol->attribute.IdType->TypeClass == STRUCTURE){
                /* allocate space for array and structure */
                operand * sizeOfType = creatOperand(irSys,IR(SIZE),newsymbol->attribute.IdType->typeWidth);
                /* DEC x size */
                generateCode(irSys,IS(DEC),newsymbol->attribute.irOperand,sizeOfType,NULL);
            }
        }

        if(curDec->child_cnt == 3){
            /* VarDec ASSIGNOP Exp */
            operand * srcOperand = creatOperand(irSys,IR(TEMP),IR(NORMAL));
            TypeDescriptor * exptype = SemanticAnalysisExp(curDec->child_list[2],symtab,irSys,false,false,srcOperand);
            if(field){
                /* Initialization of field. */
                ReportSemanticError(curDec->lineno,15,NULL);
            }else{
                if((!IsErrorType(exptype)) && (!IsErrorType(newsymbol->attribute.IdType)) && (!IsEqualType(newsymbol->attribute.IdType,exptype))){
                    /* Type mismatched for assignment. */
                    ReportSemanticError(curDec->lineno,5,NULL);
                }else{
                    /* local variable initialization starts here */
                    if(newsymbol->attribute.IdType->TypeClass == BASIC){
                        /* (int) := (int) | (float) := (float) */
                        /* x := y */
                        generateCode(irSys,IS(ASSIGN),newsymbol->attribute.irOperand,srcOperand,NULL);
                    }else if(newsymbol->attribute.IdType->TypeClass == ARRAY || newsymbol->attribute.IdType->TypeClass == STRUCTURE){
                        /* Tricky: use memory copy here */
                        operand * dstAddr = copyOperand(irSys,newsymbol->attribute.irOperand);
                        dstAddr->info.variable.modifier = IR(ACCESSADDR);
                        generateMemoryCopyCode(irSys,srcOperand,dstAddr,newsymbol->attribute.IdType->typeWidth);
                    }else{
                        /* Error */
                    }
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
        int elemTypeSize = pretype->typeWidth;
        pretype = CreatArrayDescriptor(pretype,arsize,false);
        pretype->typeWidth = elemTypeSize * arsize;
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
/* Only generate code for function definition */
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
    
    /* Assign an operand for function */
    if(oldfun != NULL){
        newfun->attribute.irOperand = oldfun->attribute.irOperand;
    }else{
        /* A new function */
        bool isMain = (strcmp(funid,"main") == 0);
        newfun->attribute.irOperand = creatOperand(irSys,IR(FUN),isMain);
    }
    /* function definition starts here */
    if(definition){
        /* FUNCTION f : */
        generateCode(irSys,IS(FUNCTION),newfun->attribute.irOperand,NULL,NULL);
    }

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
            Symbol * newParam = SemanticAnalysisVarDec(curVarDec,symtab,irSys,basetype,false);            
            newArgc += 1;
            newParam->attribute.irOperand = NULL;
            /* parameter declaration starts here */
            if(definition){
                /* PARAM x */
                /* x stores value of basic type or address of complex type */
                newParam->attribute.irOperand = creatOperand(irSys,IR(PARAM),IR(NORMAL));
                generateCode(irSys,IS(PARAM),newParam->attribute.irOperand,NULL,NULL);
            }

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
    Return the Production number of Exp
*/
SA(int, ExpProduction){
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
    return Production;
}

SA(TypeDescriptor*, ExpCondition, operand * labelTrue, operand * labelFalse){
    int Production = SemanticAnalysisExpProduction(n,symtab,irSys);
    switch(Production){
        case 2 :  /* Exp AND Exp */
            {
                operand * lexpTrueLabel = NULL;
                operand * lexpFalseLabel = (labelFalse) ? labelFalse : creatOperand(irSys,IR(LABEL));
                operand * rexpTrueLabel = labelTrue;
                operand * rexpFalseLabel = labelFalse;
                TypeDescriptor * lexp = SemanticAnalysisExpCondition(n->child_list[0],symtab,irSys,lexpTrueLabel,lexpFalseLabel);
                TypeDescriptor * rexp = SemanticAnalysisExpCondition(n->child_list[2],symtab,irSys,rexpTrueLabel,rexpFalseLabel);
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
                    if(lint && rint){
                        if(labelFalse == NULL){
                            /* LABEL lexpFalseLabel : */
                            generateCode(irSys,IS(LABEL),lexpFalseLabel,NULL,NULL);
                        }
                        return BasicInt();
                    }else return BasicError();
                }
            }
        case 3 :  /* Exp OR Exp */
            {
                operand * lexpTrueLabel = (labelTrue) ? labelTrue : creatOperand(irSys,IR(LABEL));
                operand * lexpFalseLabel = NULL;
                operand * rexpTrueLabel = labelTrue;
                operand * rexpFalseLabel = labelFalse;
                TypeDescriptor * lexp = SemanticAnalysisExpCondition(n->child_list[0],symtab,irSys,lexpTrueLabel,lexpFalseLabel);
                TypeDescriptor * rexp = SemanticAnalysisExpCondition(n->child_list[2],symtab,irSys,rexpTrueLabel,rexpFalseLabel);
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
                    if(lint && rint){
                        if(labelTrue == NULL){
                            /* LABEL lexpTrueLabel : */
                            generateCode(irSys,IS(LABEL),lexpTrueLabel,NULL,NULL);
                        }
                        return BasicInt();
                    }else return BasicError();
                }
            }
        case 4 :  /* Exp RELOP Exp */
            {
                operand * lexpOperand = creatOperand(irSys,IR(TEMP),IR(NORMAL));
                operand * rexpOperand = creatOperand(irSys,IR(TEMP),IR(NORMAL));
                TypeDescriptor * lexp = SemanticAnalysisExp(n->child_list[0],symtab,irSys,false,false,lexpOperand);
                TypeDescriptor * rexp = SemanticAnalysisExp(n->child_list[2],symtab,irSys,false,false,rexpOperand);
                if(IsErrorType(lexp) || IsErrorType(rexp)){
                    return BasicError();
                }else{
                    if(IsEqualType(lexp,rexp)){
                        if(IsEqualType(lexp,BasicInt()) || IsEqualType(lexp,BasicFloat())){
                            /* Relation operation should return boolean value */
                            int relop = ((struct CST_mul_node*)(n->child_list[1]))->tktype;
                            bool ifTrue = (labelTrue != NULL);
                            int instr = 0;
                            switch(relop){
                                case TK(LT)  : instr = (ifTrue) ? IS(LESS) : IS(GREATEREQ); break; 
                                case TK(LTE) : instr = (ifTrue) ? IS(LESSEQ) : IS(GREATER); break;
                                case TK(GT)  : instr = (ifTrue) ? IS(GREATER) : IS(LESSEQ); break;
                                case TK(GTE) : instr = (ifTrue) ? IS(GREATEREQ) : IS(LESS); break;
                                case TK(EQ)  : instr = (ifTrue) ? IS(EQ) : IS(NEQ); break;
                                case TK(NEQ) : instr = (ifTrue) ? IS(NEQ) : IS(EQ); break;
                                default : /* error */
                                    break;
                            }
                            if(labelTrue != NULL && labelFalse != NULL) {
                                /*
                                    IF Exp RELOP Exp GOTO labelTrue
                                    GOTO labelFalse
                                */
                                generateCode(irSys,instr,labelTrue,lexpOperand,rexpOperand);
                                generateCode(irSys,IS(GOTO),labelFalse,NULL,NULL);
                            }else if(labelTrue != NULL){
                                /* IF Exp RELOP Exp GOTO labelTrue */
                                generateCode(irSys,instr,labelTrue,lexpOperand,rexpOperand);
                            }else if(labelFalse != NULL){
                                /* IF False Exp RELOP Exp GOTO labelFalse */
                                generateCode(irSys,instr,labelFalse,lexpOperand,rexpOperand);
                            }else{
                                /* Fall through */
                            }
                            return BasicInt();
                        }else{
                            /* Type mismatched for operands. */
                            ReportSemanticError(n->lineno,7,NULL);
                            return BasicError();
                        }
                    }else{
                        /* Type mismatched for operands. */
                        ReportSemanticError(n->lineno,7,NULL);
                        return BasicError();
                    }
                }
            }
        case 11 : /* NOT Exp */
            {
                /* Switch labelTrue and labelFalse */
                TypeDescriptor * exp = SemanticAnalysisExpCondition(n->child_list[1],symtab,irSys,labelFalse,labelTrue);
                if(IsEqualType(exp,BasicInt()) || IsErrorType(exp)){
                    return exp;
                }else{
                    /* Type mismatched for operands. */
                    ReportSemanticError(n->child_list[1]->lineno,7,NULL);
                    return BasicError();
                }
            }
        case 1 :  /* Exp ASSIGNOP Exp */
        case 5 :  /* Exp PLUS Exp */
        case 6 :  /* Exp MINUS Exp */
        case 7 :  /* Exp STAR Exp */
        case 8 :  /* Exp DIV Exp */
        case 9 :  /* LP Exp RP */
        case 10 : /* MINUS Exp */
        case 12 : /* ID LP Args RP */
        case 13 : /* ID LP RP */
        case 14 : /* Exp LB Exp RB */
        case 15 : /* Exp DOT ID */
            {
                operand * curExpIrOperand = creatOperand(irSys,IR(TEMP),IR(NORMAL));
                TypeDescriptor * curExp = SemanticAnalysisExp(n,symtab,irSys,false,false,curExpIrOperand);
                if(IsEqualType(curExp,BasicInt())){
                    if(labelTrue != NULL && labelFalse != NULL){
                        /*
                            IF Exp != #0 GOTO labelTrue
                            GOTO labelFalse
                        */
                        generateCode(irSys,IS(NEQ),labelTrue,curExpIrOperand,zeroOperand());
                        generateCode(irSys,IS(GOTO),labelFalse,NULL,NULL);
                    }else if(labelTrue != NULL){
                        /* IF Exp != #0 GOTO labelTrue */
                        generateCode(irSys,IS(NEQ),labelTrue,curExpIrOperand,zeroOperand());
                    }else if(labelFalse != NULL){
                        /* IF Exp == #0 GOTO labelFalse */
                        generateCode(irSys,IS(EQ),labelFalse,curExpIrOperand,zeroOperand());
                    }else{
                        /* Fall through */
                    }
                }
                return curExp;
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
                        if(IsEqualType(id->attribute.IdType,BasicInt()) || IsEqualType(id->attribute.IdType,BasicFloat())){
                            if(labelTrue != NULL && labelFalse != NULL){
                                /*
                                    IF ID != #0 GOTO labelTrue
                                    GOTO labelFalse
                                */
                                generateCode(irSys,IS(NEQ),labelTrue,id->attribute.irOperand,zeroOperand());
                                generateCode(irSys,IS(GOTO),labelFalse,NULL,NULL);
                            }else if(labelTrue != NULL){
                                /* IF ID != #0 GOTO labelTrue */
                                generateCode(irSys,IS(NEQ),labelTrue,id->attribute.irOperand,zeroOperand());
                            }else if(labelFalse != NULL){
                                /* IF ID == #0 GOTO labelFalse */
                                generateCode(irSys,IS(EQ),labelFalse,id->attribute.irOperand,zeroOperand());
                            }else{
                                /* Fall through */
                            }
                        }
                        return id->attribute.IdType;
                    }
                }
            }
        case 17 : /* INT */
            {
                int val = ((struct CST_int_node*)n->child_list[0])->intval;
                bool valTrue = (val != 0);
                if(labelTrue != NULL && labelFalse != NULL){
                    if(valTrue){
                        /* GOTO labelTrue */
                        generateCode(irSys,IS(GOTO),labelTrue,NULL,NULL);
                    }else{
                        /* GOTO labelFalse */
                        generateCode(irSys,IS(GOTO),labelFalse,NULL,NULL);
                    }
                }else if(labelTrue != NULL){
                    if(valTrue){
                        /* GOTO labelTrue */
                        generateCode(irSys,IS(GOTO),labelTrue,NULL,NULL);
                    }else{
                        /* Fall through */
                    }
                }else if(labelFalse != NULL){
                    if(valTrue){
                        /* Fall through */
                    }else{
                        /* GOTO labelFalse */
                        generateCode(irSys,IS(GOTO),labelFalse,NULL,NULL);
                    }
                }else{
                    /* Fall through */
                }
                return BasicInt();
            }
        case 18 : /* FLOAT */
            {
                float val = ((struct CST_float_node*)n->child_list[0])->floatval;
                bool valTrue = (val != 0);
                if(labelTrue != NULL && labelFalse != NULL){
                    if(valTrue){
                        /* GOTO labelTrue */
                        generateCode(irSys,IS(GOTO),labelTrue,NULL,NULL);
                    }else{
                        /* GOTO labelFalse */
                        generateCode(irSys,IS(GOTO),labelFalse,NULL,NULL);
                    }
                }else if(labelTrue != NULL){
                    if(valTrue){
                        /* GOTO labelTrue */
                        generateCode(irSys,IS(GOTO),labelTrue,NULL,NULL);
                    }else{
                        /* Fall through */
                    }
                }else if(labelFalse != NULL){
                    if(valTrue){
                        /* Fall through */
                    }else{
                        /* GOTO labelFalse */
                        generateCode(irSys,IS(GOTO),labelFalse,NULL,NULL);
                    }
                }else{
                    /* Fall through */
                }
                return BasicFloat();
            }
        default : /* error */
            return BasicError();
    }
}

/* 
    Return a pointer to a TypeDescriptor that describes Exp.
    No new TypeDescriptor will be created.
*/
SA(TypeDescriptor*, Exp, bool leftVal, bool leftOfAssign, operand * expIrOperand){
    int Production = SemanticAnalysisExpProduction(n,symtab,irSys);
    switch(Production){
        case 1 : /* Exp ASSIGNOP Exp */
            {   
                operand * rvalOperand = expIrOperand;
                TypeDescriptor * rexp = SemanticAnalysisExp(n->child_list[2],symtab,irSys,false,false,rvalOperand);
                TypeDescriptor * lexp = SemanticAnalysisExp(n->child_list[0],symtab,irSys,true,true,rvalOperand);
                if(IsErrorType(lexp) || IsErrorType(rexp)){
                    /* lexp or rexp contains error */
                    return BasicError();
                }else{
                    if(!IsEqualType(lexp,rexp)){
                        /* Type mismatched for assignment */
                        ReportSemanticError(n->lineno,5,NULL);
                        return BasicError();
                    }else{
                        return lexp;
                    }
                } 
            }
        case 2 : /* Exp AND Exp */
            {
                if(leftVal) {
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                } else {
                    /* Exp := #0 */
                    generateCode(irSys,IS(ASSIGN),expIrOperand,zeroOperand(),NULL);
                    operand * newlabelFalse = creatOperand(irSys,IR(LABEL));
                    /* IF False Exp AND Exp GOTO newlabelFalse */
                    TypeDescriptor * curExp = SemanticAnalysisExpCondition(n,symtab,irSys,NULL,newlabelFalse);
                    if(IsEqualType(curExp,BasicInt())){
                        /*
                                Exp := #1
                            LABEL newlabelFalse :
                        */
                        generateCode(irSys,IS(ASSIGN),expIrOperand,oneOperand(),NULL);
                        generateCode(irSys,IS(LABEL),newlabelFalse,NULL,NULL);
                    }
                    return curExp;
                }
            }
        case 3 : /* Exp OR Exp */
            {
                if(leftVal) {
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                } else {
                    /* Exp := #1 */
                    generateCode(irSys,IS(ASSIGN),expIrOperand,oneOperand(),NULL);
                    operand * newlabelTrue = creatOperand(irSys,IR(LABEL));
                    /* IF Exp OR Exp GOTO newlabelTrue */
                    TypeDescriptor * curExp = SemanticAnalysisExpCondition(n,symtab,irSys,newlabelTrue,NULL);
                    if(IsEqualType(curExp,BasicInt())){
                        /*
                                Exp := #0
                            LABEL newlabelTrue :
                        */
                        generateCode(irSys,IS(ASSIGN),expIrOperand,zeroOperand(),NULL);
                        generateCode(irSys,IS(LABEL),newlabelTrue,NULL,NULL);
                    }
                    return curExp;
                }
            }
        case 4 : /* Exp RELOP Exp */
            {
                if(leftVal) {
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                } else {
                    /* Exp := #1 */
                    generateCode(irSys,IS(ASSIGN),expIrOperand,oneOperand(),NULL);
                    operand * newlabelTrue = creatOperand(irSys,IR(LABEL));
                    /* IF Exp RELOP Exp GOTO newlabelTrue */
                    TypeDescriptor * curExp = SemanticAnalysisExpCondition(n,symtab,irSys,newlabelTrue,NULL);
                    if(IsEqualType(curExp,BasicInt())){
                        /*
                                Exp := #0
                            LABEL newlabelTrue :
                        */
                        generateCode(irSys,IS(ASSIGN),expIrOperand,zeroOperand(),NULL);
                        generateCode(irSys,IS(LABEL),newlabelTrue,NULL,NULL);
                    }
                    return curExp;
                }
            }
        case 5 : /* Exp PLUS Exp */
        case 6 : /* Exp MINUS Exp */
        case 7 : /* Exp STAR Exp */
        case 8 : /* Exp DIV Exp */
            {
                if(leftVal){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                }else{
                    operand * lexpOperand = creatOperand(irSys,IR(TEMP),IR(NORMAL));
                    operand * rexpOperand = creatOperand(irSys,IR(TEMP),IR(NORMAL));
                    TypeDescriptor * lexp = SemanticAnalysisExp(n->child_list[0],symtab,irSys,false,false,lexpOperand);
                    TypeDescriptor * rexp = SemanticAnalysisExp(n->child_list[2],symtab,irSys,false,false,rexpOperand);
                    if(IsErrorType(lexp) || IsErrorType(rexp)){
                        return BasicError();
                    }else{
                        if(IsEqualType(lexp,rexp)){
                            if(IsEqualType(lexp,BasicInt()) || IsEqualType(lexp,BasicFloat())){
                                int instr = 0;
                                switch(Production){
                                    case 5 : instr = IS(PLUS); break;
                                    case 6 : instr = IS(MINUS); break;
                                    case 7 : instr = IS(MUL); break;
                                    case 8 : instr = IS(DIV); break;
                                    default : /* error */
                                        break;
                                }
                                /* Exp := lexp OP rexp */
                                generateCode(irSys,instr,expIrOperand,lexpOperand,rexpOperand);
                                return lexp;
                            }else{
                                /* Type mismatched for operands. */
                                ReportSemanticError(n->lineno,7,NULL);
                                return BasicError();
                            }
                        }else{
                            /* Type mismatched for operands. */
                            ReportSemanticError(n->lineno,7,NULL);
                            return BasicError();
                        }
                    }
                }
            }
        case 9 : /* LP Exp RP */
            {
                return SemanticAnalysisExp(n->child_list[1],symtab,irSys,leftVal,leftOfAssign,expIrOperand);
            }
        case 10 : /* MINUS Exp */
            {
                if(leftVal){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                }else{
                    operand * subExpOperand = creatOperand(irSys,IR(TEMP),IR(NORMAL));
                    TypeDescriptor * exp = SemanticAnalysisExp(n->child_list[1],symtab,irSys,false,false,subExpOperand);
                    if(IsErrorType(exp)){
                        return BasicError();
                    }else if(IsEqualType(exp,BasicInt()) || IsEqualType(exp,BasicFloat())){
                        /* Exp := #0 - subExp */
                        generateCode(irSys,IS(MINUS),expIrOperand,zeroOperand(),subExpOperand);
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
                if(leftVal){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                } else {
                    /* Exp := #1 */
                    generateCode(irSys,IS(ASSIGN),expIrOperand,oneOperand(),NULL);
                    operand * newlabelTrue = creatOperand(irSys,IR(LABEL));
                    /* IF NOT Exp GOTO newlabelTrue */
                    TypeDescriptor * curExp = SemanticAnalysisExpCondition(n,symtab,irSys,newlabelTrue,NULL);
                    if(IsEqualType(curExp,BasicInt())){
                        /*
                                Exp := #0
                            LABEL newlabelTrue :
                        */
                        generateCode(irSys,IS(ASSIGN),expIrOperand,zeroOperand(),NULL);
                        generateCode(irSys,IS(LABEL),newlabelTrue,NULL,NULL);
                    }
                    return curExp;
                }
            }
        case 12 : /* ID LP Args RP */
            {
                if(leftVal){
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

                        operand ** argList = (operand**)malloc(expectargnum*sizeof(operand*));

                        for(int i = 0;i < expectargnum;i++){
                            struct CST_node * curExp = curArg->child_list[0];
                            argList[i] = creatOperand(irSys,IR(TEMP),IR(NORMAL));
                            TypeDescriptor * curExptype = SemanticAnalysisExp(curExp,symtab,irSys,false,false,argList[i]);
                            TypeDescriptor * expecttype = fun->attribute.Info.Func.ArgTypeList[i];
                            if(IsErrorType(curExptype)){
                                free(argList);
                                return BasicError();
                            }
                            if(!IsEqualType(expecttype,curExptype)){
                                /* Arguments number or type mismatch. */
                                ReportSemanticError(curExp->lineno,9,NULL);
                                free(argList);
                                return BasicError();
                            }
                            bool lastexparg = (i == (expectargnum - 1));
                            bool lastrealarg = (curArg->child_cnt == 1);
                            if((lastexparg && (!lastrealarg)) || ((!lastexparg) && lastrealarg)){
                                /* Arguments number or type mismatch. */
                                ReportSemanticError(n->lineno,9,NULL);
                                free(argList);
                                return BasicError();
                            }
                            curArg = curArg->child_list[2];
                        }

                        if(strcmp("write",funid) == 0){
                            /* 
                                WRITE x 
                                Exp := #0
                            */
                            generateCode(irSys,IS(WRITE),argList[0],NULL,NULL);
                            generateCode(irSys,IS(ASSIGN),expIrOperand,zeroOperand(),NULL);
                        }else{
                            for(int i = expectargnum - 1;i >= 0;i--){
                                /* ARG x */
                                generateCode(irSys,IS(ARG),argList[i],NULL,NULL);
                            }
                            /* Exp := CALL f */
                            generateCode(irSys,IS(CALL),expIrOperand,fun->attribute.irOperand,NULL);
                        }

                        free(argList);

                        return fun->attribute.IdType;
                    }
                }
            }
        case 13 : /* ID LP RP */
            {
                if(leftVal){
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
                        if(fun->attribute.Info.Func.Argc != 0){
                            /* Arguments number or type mismatch. */
                            ReportSemanticError(n->lineno,9,NULL);
                            return BasicError();
                        }else{
                            if(strcmp("read",funid) == 0){
                                /* READ Exp */
                                generateCode(irSys,IS(READ),expIrOperand,NULL,NULL);
                            }else{
                                /* Exp := CALL f */
                                generateCode(irSys,IS(CALL),expIrOperand,fun->attribute.irOperand,NULL);
                            }
                            return fun->attribute.IdType;
                        }
                    }
                }
            }
        case 14 : /* Exp LB Exp RB */
            {
                operand * lexpOperand = creatOperand(irSys,IR(TEMP),IR(NORMAL));
                operand * rexpOperand = creatOperand(irSys,IR(TEMP),IR(NORMAL));
                TypeDescriptor * lexp = SemanticAnalysisExp(n->child_list[0],symtab,irSys,true,false,lexpOperand);
                TypeDescriptor * rexp = SemanticAnalysisExp(n->child_list[2],symtab,irSys,false,false,rexpOperand);
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
                if(!rterror){
                    int elemTypeSize = lexp->Array.elem->typeWidth;
                    operand * sizeOperand = creatOperand(irSys,IR(INT),elemTypeSize);
                    /*
                        rexp := rexp * elemsize
                        lexp := lexp + rexp
                    */
                    generateCode(irSys,IS(MUL),rexpOperand,rexpOperand,sizeOperand);
                    generateCode(irSys,IS(PLUS),lexpOperand,lexpOperand,rexpOperand);
                    if(leftOfAssign) {
                        if (lexp->Array.elem->TypeClass == BASIC) {
                            /* *lexp := rvalue */
                            generateCode(irSys,IS(SETVAL),lexpOperand,expIrOperand,NULL);
                        } else if (lexp->Array.elem->TypeClass == STRUCTURE || lexp->Array.elem->TypeClass == ARRAY) {
                            /* use memory copy here */
                            generateMemoryCopyCode(irSys,expIrOperand,lexpOperand,lexp->Array.elem->typeWidth);
                        } else {
                            /* error */
                        }
                    } else {
                        if(lexp->Array.elem->TypeClass == BASIC){
                            /* Exp := *lexp */
                            generateCode(irSys,IS(GETVAL),expIrOperand,lexpOperand,NULL);
                        }else{
                            /* Exp := lexp */
                            generateCode(irSys,IS(ASSIGN),expIrOperand,lexpOperand,NULL);
                        }
                    }
                }
                return (rterror) ? BasicError() : lexp->Array.elem;
            }
        case 15 : /* Exp DOT ID */
            {
                operand * curExpOperand = creatOperand(irSys,IR(TEMP),IR(NORMAL));
                TypeDescriptor * exp = SemanticAnalysisExp(n->child_list[0],symtab,irSys,true,false,curExpOperand);
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
                        int offset = 0;
                        while(curField != NULL){
                            if(strcmp(fieldid,curField->FieldName) == 0){
                                operand * offsetOperand = creatOperand(irSys,IR(INT),offset);
                                /* curExp := curExp + offset */
                                generateCode(irSys,IS(PLUS),curExpOperand,curExpOperand,offsetOperand);
                                if(leftOfAssign){
                                    if(curField->FieldType->TypeClass == BASIC) {
                                        /* curExp := rvalue */
                                        generateCode(irSys,IS(SETVAL),curExpOperand,expIrOperand,NULL);
                                    } else if (curField->FieldType->TypeClass == ARRAY || curField->FieldType->TypeClass == STRUCTURE) {
                                        /* Tricky: use memory copy here */
                                        generateMemoryCopyCode(irSys,expIrOperand,curExpOperand,curField->FieldType->typeWidth);
                                    } else {
                                        /* error */
                                    }
                                } else {
                                    if(curField->FieldType->TypeClass == BASIC){
                                        /* Exp := *curExp */
                                        generateCode(irSys,IS(GETVAL),expIrOperand,curExpOperand,NULL);
                                    }else{
                                        /* Exp := curExp */
                                        generateCode(irSys,IS(ASSIGN),expIrOperand,curExpOperand,NULL);
                                    }
                                }
                                return curField->FieldType;
                            }
                            offset += curField->FieldType->typeWidth;
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
                        if (leftOfAssign) {
                            if(id->attribute.IdType->TypeClass == BASIC){
                                /* ID := rvalue */
                                generateCode(irSys,IS(ASSIGN),id->attribute.irOperand,expIrOperand,NULL);
                            } else if(id->attribute.IdType->TypeClass == ARRAY || id->attribute.IdType->TypeClass == STRUCTURE){
                                /* ADDR := &ID */
                                operand * tempOperand = creatOperand(irSys,IR(TEMP),IR(NORMAL));
                                generateCode(irSys,IS(GETADDR),tempOperand,id->attribute.irOperand,NULL);
                                generateMemoryCopyCode(irSys,expIrOperand,tempOperand,id->attribute.IdType->typeWidth);
                            }
                        } else {
                            if(id->attribute.IdType->TypeClass == BASIC){
                                /* Exp := ID */
                                generateCode(irSys,IS(ASSIGN),expIrOperand,id->attribute.irOperand,NULL);
                            }else if(id->attribute.IdType->TypeClass == ARRAY || id->attribute.IdType->TypeClass == STRUCTURE){
                                if(id->attribute.irOperand->operandClass == IR(VAR)){
                                    /* Exp := &ID */
                                    generateCode(irSys,IS(GETADDR),expIrOperand,id->attribute.irOperand,NULL);
                                }else if(id->attribute.irOperand->operandClass == IR(PARAM)){
                                    /* Exp := PARAM */
                                    generateCode(irSys,IS(ASSIGN),expIrOperand,id->attribute.irOperand,NULL);
                                }else{
                                    /* error */
                                }
                            }else{
                                /* error */
                            }
                        }
                        return id->attribute.IdType;
                    }
                }
            }
        case 17 : /* INT */
            {
                if(leftVal){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                }else{
                    int val = ((struct CST_int_node*)n->child_list[0])->intval;
                    operand * intVal = creatOperand(irSys,IR(INT),val);
                    /* Exp := INT */
                    generateCode(irSys,IS(ASSIGN),expIrOperand,intVal,NULL);
                    return BasicInt();
                }
            }
        case 18 : /* FLOAT */
            {
                if(leftVal){
                    /* Appearance of rvalue on left hand side of assignment. */
                    ReportSemanticError(n->lineno,6,NULL);
                    return BasicError();
                }else{
                    float val = ((struct CST_float_node*)n->child_list[0])->floatval;
                    operand * floatVal = creatOperand(irSys,IR(FLOAT),val);
                    generateCode(irSys,IS(ASSIGN),expIrOperand,floatVal,NULL);
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
                /* Top-most level of exp */
                operand * expOperand = creatOperand(irSys,IR(TEMP),IR(NORMAL));
                SemanticAnalysisExp(n->child_list[0],symtab,irSys,false,false,expOperand);
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
                operand * expOperand = creatOperand(irSys,IR(TEMP),IR(NORMAL));
                TypeDescriptor * exptype = SemanticAnalysisExp(n->child_list[1],symtab,irSys,false,false,expOperand);
                if(!IsEqualType(exptype,returntype)){
                    /* Type mismatched for return type. */
                    ReportSemanticError(n->child_list[1]->lineno,8,NULL);
                }else{
                    /* Always return a basic type value */
                    /* RETURN x */
                    generateCode(irSys,IS(RETURN),expOperand,NULL,NULL);
                }

                break;
            }
        case 4 : /* IF LP Exp RP Stmt */
            {
                operand * labelTrue = NULL;
                operand * labelFalse = creatOperand(irSys,IR(LABEL));
                
                /*
                        IF False Exp GOTO labelFalse
                        Stmt.code
                    LABEL labelFalse :
                */
                
                TypeDescriptor * exptype = SemanticAnalysisExpCondition(n->child_list[2],symtab,irSys,labelTrue,labelFalse);
                if(!IsEqualType(exptype,BasicInt())){
                    /* Type mismatched for operands. */
                    ReportSemanticError(n->child_list[2]->lineno,7,NULL);
                }
                SemanticAnalysisStmt(n->child_list[4],symtab,irSys,returntype);                
                generateCode(irSys,IS(LABEL),labelFalse,NULL,NULL);
                break;
            }
        case 5 : /* IF LP Exp RP Stmt ELSE Stmt */
            {
                operand * labelTrue = NULL;
                operand * labelFalse = creatOperand(irSys,IR(LABEL));
                operand * labelNext = creatOperand(irSys,IR(LABEL));

                /*
                        IF False Exp GOTO labelFalse
                        Stmt1.code
                        GOTO labelNext
                    LABEL labelFalse :
                        Stmt2.code
                    LABEL labelNext :
                */

                TypeDescriptor * exptype = SemanticAnalysisExpCondition(n->child_list[2],symtab,irSys,labelTrue,labelFalse);
                if(!IsEqualType(exptype,BasicInt())){
                    /* Type mismatched for operands. */
                    ReportSemanticError(n->child_list[2]->lineno,7,NULL);
                }
                SemanticAnalysisStmt(n->child_list[4],symtab,irSys,returntype);
                generateCode(irSys,IS(GOTO),labelNext,NULL,NULL);
                generateCode(irSys,IS(LABEL),labelFalse,NULL,NULL);
                SemanticAnalysisStmt(n->child_list[6],symtab,irSys,returntype);
                generateCode(irSys,IS(LABEL),labelNext,NULL,NULL);
                break;
            }
        case 6 : /* WHILE LP Exp RP Stmt */
            {
                operand * labelBegin = creatOperand(irSys,IR(LABEL));
                operand * labelTrue = NULL;
                operand * labelFalse = creatOperand(irSys,IR(LABEL));
                
                /*
                    LABEL labelBegin :
                        IF False Exp GOTO labelFalse
                        Stmt.code
                        GOTO labelBegin
                    LABEL labelFalse :
                */

               generateCode(irSys,IS(LABEL),labelBegin,NULL,NULL);
                TypeDescriptor * exptype = SemanticAnalysisExpCondition(n->child_list[2],symtab,irSys,labelTrue,labelFalse);
                if(!IsEqualType(exptype,BasicInt())){
                    /* Type mismatched for operands. */
                    ReportSemanticError(n->child_list[2]->lineno,7,NULL);
                }
                SemanticAnalysisStmt(n->child_list[4],symtab,irSys,returntype);
                generateCode(irSys,IS(GOTO),labelBegin,NULL,NULL);
                generateCode(irSys,IS(LABEL),labelFalse,NULL,NULL);
                break;
            }
        default : break;
    }
}