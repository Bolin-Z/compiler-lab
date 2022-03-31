#include "semantic.h"

#define SA(RETURN,NAME,...) RETURN SemanticAnalysis##NAME (const struct CST_node* n, SymbolTable * symtab,##__VA_ARGS__)
SA(void, Program);
SA(void, ExtDefList);
SA(void, ExtDef);
SA(TypeDescriptor*, Specifier);
SA(void, DefList, bool fields);
SA(void, Def, bool field);
SA(TypeDescriptor*, Exp);

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
            TypeDescriptor * exptype = SemanticAnalysisExp(curDec->child_list[2],symtab);
            if(field) ReportSemanticError(0,0,"Field can not be initialized");
            else{
                if(!IsEqualType(newsymboal->attribute.IdType,exptype))
                    ReportSemanticError(0,0,"Unmatch Exp type");
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
SA(TypeDescriptor*, Exp){

}