#include "semantic.h"

#define SA(RETURN,NAME,...) RETURN SemanticAnalysis##NAME (const struct CST_node* n, SymbolTable * symtab,##__VA_ARGS__)
SA(void, Program);
SA(void, ExtDefList);
SA(void, ExtDef);
SA(TypeDescriptor*, Specifier);
SA(void, DefList, bool StructureDefList);

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
    Specifier contains type infomation which use TypeDescriptor to represent.
    If the TypeDescriptor already exists:
            1. the basic type
            2. a structure type that was defined before
        return a pointer to that TypeDescriptor.
    Else if the Specifier defined:
            1. a structure type
            2. a anonymous structure type
        creat a new TypeDescriptor and return a pointer to it.
    Else return Error type to indicate an unexpected error.
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
                            return type->attribute.IdType;
                        }
                    }
                case 5 : /* STRUCT OptTag LC DefList RC */
                    struct CST_node * opttag = st->child_list[1];
                    struct CST_ndoe * deflist = st->child_list[3];
                    Symbol * newst = NULL;
                    bool conflict = false;
                    if(opttag->child_cnt != 0){
                        /* define a new structure type */
                        char * stname = ((struct CST_id_node *)(opttag->child_list[0]))->ID;
                        if(LookUp(symtab,stname,false) != NULL) conflict = true;
                        newst = Insert(symtab,stname);
                        newst->attribute.IdClass = TYPENAME;
                    }
                    /* Construct a structure TypeDescriptor */
                    Scope * newscope = OpenScope(symtab,NULL);
                    TypeDescriptor * newsttype = NULL;
                    SemanticAnalysisDefList(deflist,symtab,true);
                    if(newscope->scopebeginidx == newscope->scopeendidx){
                        /* empty field list */
                        newsttype = CreatStructureDescriptor(NULL,false);
                    }else{
                        
                    }
                    CloseScope(symtab);
                    break;
                default : /* error */
                    return BasicError();
            }
        default : /* error */
            return BasicError();
    }
}

SA(void,DefList, bool StructureDefList){

}