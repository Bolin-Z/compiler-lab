#include "semantic.h"

#define SA(RETURN,NAME,...) RETURN SemanticAnalysis##NAME (const struct CST_node* n, SymbolTable * systab,##__VA_ARGS__)
SA(void, Program);
SA(void, ExtDefList);
SA(void, ExtDef);
SA(TypeDescriptor*, Specifier);

void SemanticAnalysis(const struct CST_node* root){
    SymbolTable * symtable = CreatSymbolTable();
    CreatTypeSystem();

    SemanticAnalysisProgram(root,symtable);

    DestoryTypeSystem();
    DestorySymbolTable(symtable);
}

/* Program --> ExtDefList */
SA(void, Program){
    OpenScope(systab,"Global_Scope");
    SemanticAnalysisExtDefList(n->child_list[0],systab);
    CloseScope(systab);
}

SA(void,ExtDefList){
    struct CST_node * curExtDefList = n;
    while(curExtDefList->child_cnt != 0){
        SemanticAnalysisExtDef(curExtDefList->child_list[0],systab);
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

/* Creat a TypeDescriptor and return a pointer to it. */
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
                    Symbol*  type = LookUp(systab,id->ID);
                    if(!type){
                        ReportSemanticError(0,0,"structure type was not defined");
                        return BasicError();
                    }else{
                        if(type->attribute.IdClass != TYPENAME){
                            ReportSemanticError(0,0,"name has been used");
                            return BasicError();
                        }else{
                            /* WARNING : Copy or Pointer? */
                            return type->attribute.IdType;
                        }
                    }
                case 5 : /* STRUCT OptTag LC DefList RC */
                default : /* error */
                    return BasicError();
            }
        default : /* error */
            return BasicError();
    }
}