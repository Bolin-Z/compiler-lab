#include "semantic.h"

#define SA(RETURN,NAME,...) RETURN SemanticAnalysis##NAME (const struct CST_node* n, SymbolTable * symtab,##__VA_ARGS__)
SA(void, Program);
SA(void, ExtDefList);
SA(void, ExtDef);
SA(TypeDescriptor*, Specifier);
SA(void, DefList, bool fields);
SA(void, Def, bool field);
SA(Symbol*, VarDec, TypeDescriptor * basetype, bool field, bool curscope);
SA(Symbol*, FunDec, TypeDescriptor * returntype, bool declaration);
SA(TypeDescriptor*, Exp, bool LeftHand);
SA(void, StmtList, TypeDescriptor * returntype);
SA(void, Stmt, TypeDescriptor * returntype); // Working ON

/* wrap-up function of semantic analysis stage */
void SemanticAnalysis(const struct CST_node* root){
    CreatTypeSystem();
    SymbolTable * symtable = CreatSymbolTable();

    SemanticAnalysisProgram(root,symtable);

    DestorySymbolTable(symtable);
    DestoryTypeSystem();
}

/* Program --> ExtDefList */
SA(void, Program){
    OpenScope(symtab,"Global_Scope");
    SemanticAnalysisExtDefList(n->child_list[0],symtab);
    CloseScope(symtab);
}

SA(void,ExtDefList){
    struct CST_node * curExtDefList = n;
    while(curExtDefList->child_cnt != 0){
        SemanticAnalysisExtDef(curExtDefList->child_list[0],symtab);
        curExtDefList = curExtDefList->child_list[1];
    }
}

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
                TypeDescriptor * basetype = SemanticAnalysisSpecifier(n->child_list[0],symtab);
                struct CST_node * curExtDecList = n->child_list[1];
                while(true){
                    SemanticAnalysisVarDec(curExtDecList->child_list[0],symtab,basetype,false,true);
                    if(curExtDecList->child_cnt == 1) break;
                    else curExtDecList = curExtDecList->child_list[2];
                }
                break;
            }
        case 2 : /* Specifier SEMI */
            {
                SemanticAnalysisSpecifier(n->child_list[0],symtab);
                break;
            }
        case 3 : /* Specifier FunDec CompSt */
            {
                TypeDescriptor * rttype = SemanticAnalysisSpecifier(n->child_list[0],symtab);
                Symbol * newfun = SemanticAnalysisFunDec(n->child_list[1],symtab,rttype,false);
                /* Semantic Analysis CompSt */
                SemanticAnalysisDefList(n->child_list[2]->child_list[1], symtab, false);
                SemanticAnalysisStmtList(n->child_list[2]->child_list[2], symtab, newfun->attribute.IdType);
                CloseScope(symtab);
                break;
            }
        case 4 : /* Specifier FunDec SEMI */
            {
                TypeDescriptor * rttype = SemanticAnalysisSpecifier(n->child_list[0],symtab);
                SemanticAnalysisFunDec(n->child_list[1],symtab,rttype,true);
                break;
            }
        default : /* error */
            break;
    }
}

/*
    Creat a TypeDescriptor and return a pointer to it.
*/
SA(TypeDescriptor*, Specifier){
    switch(get_symtype(n->child_list[0]->compact_type)){
        case SYM(TYPE) : 
            struct CST_mul_node * cur = (struct CST_mul_node * )(n->child_list[0]);
            switch(cur->tktype){
                case TK(INT) : return BasicInt();
                case TK(FLOAT) : return BasicFloat();
                default : /* error */
                    return BasicError();
            }
        case SYM(StructSpecifier) :
            struct CST_node * st = n->child_list[0];
            switch(st->child_cnt){
                case 2 : /* STRUCT Tag */
                    struct CST_node * tag = st->child_list[1];
                    struct CST_id_node * id = (struct CST_id_node *)(tag->child_list[0]);
                    Symbol*  type = LookUp(symtab,id->ID,false);
                    if(!type){
                        ReportSemanticError(0,0,"structure type was not defined");
                        return BasicError();
                    }else{
                        if(type->attribute.IdClass != TYPENAME){
                            ReportSemanticError(0,0,"Not a structure type name");
                            return BasicError();
                        }else{
                            return CopyTypeDescriptor(type->attribute.IdType);
                        }
                    }
                case 5 : /* STRUCT OptTag LC DefList RC */
                    struct CST_node * opttag = st->child_list[1];
                    struct CST_ndoe * deflist = st->child_list[3];
                    Symbol * newst = NULL;
                    if(opttag->child_cnt != 0){
                        /* define a new structure type */
                        char * stname = ((struct CST_id_node *)(opttag->child_list[0]))->ID;
                        /* structure name is defined globally */
                        if(LookUp(symtab,stname,false) != NULL)
                            ReportSemanticError(0,0,"Structure name is conflict with existing name");
                        newst = Insert(symtab,stname);
                        newst->attribute.IdClass = TYPENAME;
                    }
                    /* Construct a structure TypeDescriptor */
                    /* Type name is in the outer-scope */
                    Scope * newscope = OpenScope(symtab,NULL);
                    SemanticAnalysisDefList(deflist,symtab,true);
                    /* Construct FieldList */
                    FieldList * head = NULL;
                    FieldList * pre = NULL;
                    for(int i = newscope->scopebeginidx;i < newscope->scopeendidx;i++){
                        Symbol * tempsym = Access(symtab,i);
                        switch(tempsym->attribute.IdClass){
                            case VARIABLE : /* field */
                                FieldList * f = CreatField();
                                f->FieldName = tempsym->id;
                                f->FieldType = CopyTypeDescriptor(tempsym->attribute.IdType);
                                if(!head) head = f;
                                if(!pre) pre->NextField = f;
                                pre = f;
                                break;
                            default : /* ignore TYPENAME */ 
                                break;
                        }
                    }
                    CloseScope(symtab);
                    TypeDescriptor * newsttype = CreatStructureDescriptor(head,false);
                    if(newst) newst->attribute.IdType = newsttype;
                    return newsttype;
                    break;
                default : /* error */
                    return BasicError();
            }
        default : /* error */
            return BasicError();
    }
}

SA(void, DefList, bool fields){
    struct CST_node * curDefList = n;
    while(curDefList->child_cnt != 0){
        SemanticAnalysisDef(n->child_list[0],symtab, fields);
        curDefList = curDefList->child_list[1];
    }
}

/* Def --> Specifier DecList SEMI */
SA(void, Def, bool field){
    /* basetype == BasicError() is acceptable, however BasicTypeError will be assigned to IDs. */
    TypeDescriptor * basetype = SemanticAnalysisSpecifier(n->child_list[0],symtab);
    struct CST_node * curDecList = n->child_list[1];
    while(true){
        struct CST_node * curDec = curDecList->child_list[0];
        Symbol * newsymbol = SemanticAnalysisVarDec(curDec->child_list[0],symtab,basetype,field,true);
        if(curDec->child_cnt == 3){
            /* VarDec ASSIGNOP Exp */
            TypeDescriptor * exptype = SemanticAnalysisExp(curDec->child_list[2],symtab,false);
            if(field) ReportSemanticError(0,0,"Field can not be initialized");
            else{
                if((!IsErrorType(exptype)) && (!IsErrorType(newsymbol->attribute.IdType)) && (!IsEqualType(newsymbol->attribute.IdType,exptype)))
                    ReportSemanticError(0,0,"Type mismatched for assignment");
            }
        }
        if(curDecList->child_cnt == 1) break;
        else curDecList = curDecList->child_list[2];
    }
}

SA(Symbol*, VarDec, TypeDescriptor * basetype, bool field, bool curscope){
    struct CST_node * curVarDec = n;
    TypeDescriptor * pretype = basetype;
    while(curVarDec->child_cnt != 1){
        if(!IsErrorType(pretype)){
            int arsize = ((struct CST_int_node *)(curVarDec->child_list[2]))->intval;
            pretype = CreatArrayDescriptor(pretype,arsize,false);
        }
        curVarDec = curVarDec->child_list[0];
    }
    char * varname = ((struct CST_id_node*)(curVarDec->child_list[0]))->ID;
    if(LookUp(symtab,varname,curscope) != NULL){
        if(field){
            ReportSemanticError(0,0,"Field name conflict");
        }else{
            ReportSemanticError(0,0,"Variable name conflict");
        }
    }
    Symbol * newsymbol = Insert(symtab,varname);
    newsymbol->attribute.IdClass = VARIABLE;
    newsymbol->attribute.IdType = pretype;
    return newsymbol;
}

SA(Symbol*, FunDec, TypeDescriptor * returntype, bool declaration){
    /* FunDec := ID LP RP | ID LP VarList RP  */
    char * funid = ((struct CST_id_node*)(n->child_list[0]))->ID;
    Symbol * newfun = NULL;
    Symbol * oldfun = LookUp(symtab,funid,false);
    if((oldfun != NULL) && (oldfun->attribute.IdClass != FUNCTION)){
        ReportSemanticError(0,0,"Function name conflicts with variable or structure name");
        oldfun = NULL;
    }

    if(oldfun != NULL){
        if(oldfun->attribute.Info.Func.defined && (!declaration)){
            ReportSemanticError(4,0,"Redefined of function");
        }else{
            if(!IsEqualType(returntype,oldfun->attribute.IdType))
                ReportSemanticError(19,0,"Function declaration/definition conflict: return type");
        }
    }
    /* always insert */
    newfun = Insert(symtab,funid);
    newfun->attribute.IdClass = FUNCTION;
    newfun->attribute.IdType = returntype;
    newfun->attribute.Info.Func.defined = UpdateFunctionState(0,funid,declaration);

    Scope * funscope = OpenScope(symtab,newfun->id);

    int newArgc = 0;
    TypeDescriptor ** newArgTypeList = NULL;
    if(n->child_cnt == 4){
        struct CST_node * curVarList = n->child_list[2];
        while(true){
            struct CST_node * curParamDec = curVarList->child_list[0];
            struct CST_node * curSpecifier = curParamDec->child_list[0];
            struct CST_node * curVarDec = curParamDec->child_list[1];
            TypeDescriptor * basetype = SemanticAnalysisSpecifier(curSpecifier,symtab);
            SemanticAnalysisVarDec(curVarDec,symtab,basetype,false,true);
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
            ReportSemanticError(19,0,"Function definition/declaration conflict: arguments number");
        }else{
            for(int i = 0;i < newArgc;i++){
                if(!IsEqualType(newArgTypeList[i],oldfun->attribute.Info.Func.ArgTypeList[i])){
                    ReportSemanticError(19,0,"Function definition/declaration conflict: argument type");
                    break;
                }
            }
        }
    }

    newfun->attribute.Info.Func.Argc = newArgc;
    newfun->attribute.Info.Func.ArgTypeList = newArgTypeList;

    if(declaration) CloseScope(symtab);
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
                TypeDescriptor * lexp = SemanticAnalysisExp(n->child_list[0], symtab, true);
                TypeDescriptor * rexp = SemanticAnalysisExp(n->child_list[2], symtab, LeftHand);
                if(IsErrorType(lexp) || IsErrorType(rexp)){
                    /* lexp or rexp contains error */
                    return BasicError();
                }else{
                    if(!IsEqualType(lexp,rexp)){
                        ReportSemanticError(0,0,"Type mismatched for assignment");
                        return BasicError();
                    }
                    return lexp;
                }
            }
        case 2 : /* Exp AND Exp */
        case 3 : /* Exp OR Exp */
        case 4 : /* Exp RELOP Exp */
            {
                if(LeftHand){
                    ReportSemanticError(0,0,"The left-hand side of an assignment must be left value");
                    return BasicError();
                }else{
                    TypeDescriptor * lexp = SemanticAnalysisExp(n->child_list[0],symtab,false);
                    TypeDescriptor * rexp = SemanticAnalysisExp(n->child_list[2],symtab,false);
                    if(IsErrorType(lexp) || IsErrorType(rexp)){
                        /* lexp or rexp contains error */
                        return BasicError();
                    }else{
                        bool lint = IsEqualType(lexp,BasicInt());
                        bool rint = IsEqualType(rexp,BasicInt());
                        if(!lint) ReportSemanticError(0,0,"Expected int value in lexp");
                        if(!rint) ReportSemanticError(0,0,"Expected int value in rexp");
                        if(lint && rint) return BasicInt();
                        else return BasicError();
                    }
                }
            }
        case 5 : /* Exp PLUS Exp */
        case 6 : /* Exp MINUS Exp */
        case 7 : /* Exp STAR Exp */
        case 8 : /* Exp DIV Exp */
            {
                if(LeftHand){
                    ReportSemanticError(0,0,"The left-hand side of assignment must be left value");
                    return BasicError();
                }else{
                    TypeDescriptor * lexp = SemanticAnalysisExp(n->child_list[0],symtab,false);
                    TypeDescriptor * rexp = SemanticAnalysisExp(n->child_list[2],symtab,false);
                    if(IsErrorType(lexp) || IsErrorType(rexp)){
                        return BasicError();
                    }else{
                        if(IsEqualType(lexp,rexp)){
                            if(IsEqualType(lexp,BasicInt()) || IsEqualType(lexp,BasicFloat()))
                                return lexp;
                            else{
                                ReportSemanticError(0,0,"Only int and float types can do arithmetic operation");
                                return BasicError();
                            }                            
                        }
                        else{
                            ReportSemanticError(0,0,"Type mismatched for operands");
                            return BasicError();
                        }
                    }
                }
            }
        case 9 : /* LP Exp RP */
            return SemanticAnalysisExp(n->child_list[1],symtab,LeftHand);
        case 10 : /* MINUS Exp */
            {
                if(LeftHand){
                    ReportSemanticError(0,0,"The left-hand side of assignment must be left value");
                    return BasicError();
                }else{
                    TypeDescriptor * exp = SemanticAnalysisExp(n->child_list[1],symtab,false);
                    if(IsEqualType(exp,BasicInt()) || IsEqualType(exp,BasicFloat()) || IsErrorType(exp)){
                        return exp;
                    }else{
                        ReportSemanticError(0,0,"Only int and float types can do arithmetic operation");
                        return BasicError();
                    }
                }
            }
        case 11 : /* NOT Exp */
            {
                if(LeftHand){
                    ReportSemanticError(0,0,"The left-hand side of assignment must be left value");
                    return BasicError();
                }else{
                    TypeDescriptor * exp = SemanticAnalysisExp(n->child_list[1],symtab,false);
                    if(IsEqualType(exp,BasicInt()) || IsErrorType(exp)){
                        return exp;
                    }else{
                        ReportSemanticError(0,0,"Only int types can do logical operation");
                        return BasicError();
                    }
                }
            }
        case 12 : /* ID LP Args RP */
            {
                if(LeftHand){
                    ReportSemanticError(0,0,"The left-hand side of assignment must be left value");
                    return BasicError();
                }else{
                    char * funid = ((struct CST_id_node *)(n->child_list[0]))->ID;
                    Symbol * fun = LookUp(symtab,funid,false);
                    if(fun == NULL){
                        ReportSemanticError(2,0,"Function was used before it was defined");
                        return BasicError();
                    }else if(fun->attribute.IdClass != FUNCTION){
                        ReportSemanticError(11,0,"Use () operator on non-function id");
                        return BasicError();
                    }else{
                        struct CST_node * curArg = n->child_list[2];
                        int expectargnum = fun->attribute.Info.Func.Argc;
                        for(int i = 0;i < expectargnum;i++){
                            struct CST_node * curExp = curArg->child_list[0];
                            TypeDescriptor * curExptype = SemanticAnalysisExp(curExp,symtab,false);
                            TypeDescriptor * expecttype = fun->attribute.Info.Func.ArgTypeList[i];
                            if(IsErrorType(curExp)){
                                return BasicError();
                            }
                            if(!IsEqualType(expecttype,curExptype)){
                                ReportSemanticError(9,0,"Argument type unmatched");
                                return BasicError();
                            }
                            bool lastexparg = (i == expectargnum - 1);
                            bool lastrealarg = (curArg->child_cnt == 1);
                            if((lastexparg && (!lastrealarg)) || ((!lastexparg) && lastrealarg)){
                                ReportSemanticError(9,0,"Argument number unmatched");
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
                    ReportSemanticError(0,0,"The left-hand side of assignment must be left value");
                    return BasicError();
                }else{
                    char * funid = ((struct CST_id_node *)(n->child_list[0]))->ID;
                    Symbol * fun = LookUp(symtab,funid,false);
                    if(fun == NULL){
                        ReportSemanticError(2,0,"Function was used before it was defined");
                        return BasicError();
                    }else if(fun->attribute.IdClass != FUNCTION){
                        ReportSemanticError(11,0,"Use () operator on non-function id");
                        return BasicError();
                    }else{
                        if(fun->attribute.Info.Func.Argc != 0){
                            ReportSemanticError(9,0,"Argument number unmatched");
                            return BasicError();
                        }else{
                            return fun->attribute.IdType;
                        }
                    }
                }
            }
        case 14 : /* Exp LB Exp RB */
            {
                /* WARING: LeftHand ? */
                TypeDescriptor * lexp = SemanticAnalysisExp(n->child_list[0],symtab,LeftHand);
                TypeDescriptor * rexp = SemanticAnalysisExp(n->child_list[2],symtab,false);
                bool rterror = false;
                if(IsErrorType(lexp)){
                    rterror = true;
                }else{
                    if(lexp->TypeClass != ARRAY){
                        ReportSemanticError(10,0,"Use [] operator on non-array type variable");
                        rterror = true;
                    }
                }
                if(IsErrorType(rexp)){
                    rterror = true;
                }else{
                    if(!IsEqualType(rexp,BasicInt())){
                        ReportSemanticError(12,0,"Expect int value inside []");
                        rterror = true;
                    }
                }
                return (rterror) ? BasicError() : lexp->Array.elem;
            }
        case 15 : /* Exp DOT ID */
            {
                TypeDescriptor * exp = SemanticAnalysisExp(n->child_list[0],symtab,LeftHand);
                if(IsErrorType(exp)){
                    return BasicError();
                }else{
                    if(exp->TypeClass != STRUCTURE){
                        ReportSemanticError(13,0,"Use . operator on non-structure type variable");
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
                        ReportSemanticError(14,0,"Field 'fieldid' was not defined");
                        return BasicError();
                    }
                }         
            }
        case 16 : /* ID */
            {
                char * idname = ((struct CST_id_node*)(n->child_list[0]))->ID;
                Symbol * id = LookUp(symtab,idname,false);
                if(id == NULL){
                    ReportSemanticError(1,0,"Use undefined variable");
                    return BasicError();
                }else{
                    if(id->attribute.IdClass != VARIABLE){
                        ReportSemanticError(0,0,"Id is not a variable");
                        return BasicError();
                    }else{
                        return id->attribute.IdType;
                    }
                }
            }
        case 17 : /* INT */
            {
                if(LeftHand){
                    ReportSemanticError(0,0,"The left-hand side of assignment must be left value");
                    return BasicError();
                }else{
                    return BasicInt();
                }
            }
        case 18 : /* FLOAT */
            {
                if(LeftHand){
                    ReportSemanticError(0,0,"The left-hand side of assignment must be left value");
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
        SemanticAnalysisStmt(curStmtList->child_list[0],symtab,returntype);
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
                SemanticAnalysisExp(n->child_list[0],symtab,false);
                break;
            }
        case 2 : /* CompSt */
            {
                OpenScope(symtab,"CompSt");
                SemanticAnalysisDefList(n->child_list[0]->child_list[1],symtab,false);
                SemanticAnalysisStmtList(n->child_list[0]->child_list[2],symtab,returntype);
                CloseScope(symtab);
                break;
            }
        case 3 : /* RETURN Exp SEMI */
            {
                TypeDescriptor * exptype = SemanticAnalysisExp(n->child_list[1],symtab,false);
                if(!IsEqualType(exptype,returntype))
                    ReportSemanticError(8,0,"Unmatch function return type");
                break;
            }
        case 4 : /* IF LP Exp RP Stmt */
            {
                TypeDescriptor * exptype = SemanticAnalysisExp(n->child_list[2],symtab,false);
                if(!IsEqualType(exptype,BasicInt()))
                    ReportSemanticError(0,0,"Expect int value expression");
                SemanticAnalysisStmt(n->child_list[4],symtab,returntype);
                break;
            }
        case 5 : /* IF LP Exp RP Stmt ELSE Stmt */
            {
                TypeDescriptor * exptype = SemanticAnalysisExp(n->child_list[2],symtab,false);
                if(!IsEqualType(exptype,BasicInt()))
                    ReportSemanticError(0,0,"Expect int value expression");
                SemanticAnalysisStmt(n->child_list[4],symtab,returntype);
                SemanticAnalysisStmt(n->child_list[6],symtab,returntype);
                break;
            }
        case 6 : /* WHILE LP Exp RP Stmt */
            {
                TypeDescriptor * exptype = SemanticAnalysisExp(n->child_list[2],symtab,false);
                if(!IsEqualType(exptype,BasicInt()))
                    ReportSemanticError(0,0,"Expect int value expression");
                SemanticAnalysisStmt(n->child_list[4],symtab,returntype);
                break;
            }
        default : break;
    }
}