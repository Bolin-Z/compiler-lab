#include "semantic.h"

#define SA(RETURN,NAME,...) RETURN SemanticAnalysis##NAME (const struct CST_node* n, SymbolTable * symtab,##__VA_ARGS__)
SA(void, Program);
SA(void, ExtDefList);
SA(void, ExtDef);
SA(TypeDescriptor*, Specifier);
SA(void, DefList, bool fields);
SA(void, Def, bool field);
SA(TypeDescriptor*, Exp, bool LeftHand);

void SemanticAnalysis(const struct CST_node* root){
    SymbolTable * symtable = CreatSymbolTable();
    CreatTypeSystem();

    SemanticAnalysisProgram(root,symtable);

    DestoryTypeSystem();
    DestorySymbolTable(symtable);
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
        case 2 : /* Specifier SEMI */
        case 3 : /* Specifier FunDec CompSt */
        case 4 : /* Specifier FunDec SEMI */
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
    TypeDescriptor * basetype = SemanticAnalysisSpecifier(n->child_list[0],symtab);
    struct CST_node * curDecList = n->child_list[1];
    while(1){
        struct CST_node * curDec = curDecList->child_list[0];
        /* Semantic Analysis VarDec */
        struct CST_node * curVarDec = curDec->child_list[0];
        TypeDescriptor * pretype = basetype;
        while(curVarDec->child_cnt != 1){
            int arsize = ((struct CST_int_node *)(curVarDec->child_list[2]))->intval;
            pretype = CreatArrayDescriptor(pretype,arsize,false);
            curVarDec = curVarDec->child_list[0];
        }
        char * varname = ((struct CST_id_node *)(curVarDec->child_list[0]))->ID;
        if(LookUp(symtab,varname,true) != NULL){
            if(field) ReportSemanticError(0,0,"Field name conflict");
            else ReportSemanticError(0,0,"Variable name conflict");
        }
        Symbol * newsymboal = Insert(symtab,varname);
        newsymboal->attribute.IdClass = VARIABLE;
        newsymboal->attribute.IdType = pretype;
        if(curDec->child_cnt == 3){
            /* VarDec ASSIGNOP Exp */
            TypeDescriptor * exptype = SemanticAnalysisExp(curDec->child_list[2],symtab,false);
            if(field) ReportSemanticError(0,0,"Field can not be initialized");
            else{
                if(!IsEqualType(newsymboal->attribute.IdType,exptype))
                    ReportSemanticError(0,0,"Type mismatched for assignment");
            }
        }
        if(curDecList->child_cnt == 1) break;
        else curDecList = curDecList->child_list[2];
    }
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
                if(IsEqualType(lexp,BasicError()) || IsEqualType(rexp,BasicError())){
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
                    if(IsEqualType(lexp,BasicError()) || IsEqualType(rexp,BasicError())){
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
                    if(IsEqualType(lexp,BasicError()) || IsEqualType(rexp,BasicError())){
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
                    if(IsEqualType(exp,BasicInt()) || IsEqualType(exp,BasicFloat()))
                        return exp;
                    else{
                        ReportSemanticError(0,0,"Only int and float types can do arithmetic operation");
                        return BasicError();
                    }
                }
            }
        case 11 : /* NOT Exp */
            {
                if(LeftHand){
                    ReportSemanticError(0,0,"The left-hand side of assignment must be left value");
                }
            }
        default : /* error */
            return BasicError();
    }
}