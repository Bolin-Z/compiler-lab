#include "semantic.h"

#define SA(X) void SemanticAnalysis##X (const struct CST_node* n, SymbolTable * systab)
SA(Program);
SA(ExtDefList);
SA(ExtDef);

void SemanticAnalysis(const struct CST_node* root){
    SymbolTable * symtable = CreatSymbolTable();
    CreatTypeSystem();

    SemanticAnalysisProgram(root,symtable);

    DestoryTypeSystem();
    DestorySymbolTable(symtable);
}

SA(Program){
    OpenScope(systab,"Global_Scope");
    for(int i = 0;i < (int)(n->child_cnt);i++){
        SemanticAnalysisExtDefList(n->child_list[i],systab);
    }
    CloseScope(systab);
}

SA(ExtDefList){
    for(int i = 0;i < (int)(n->child_cnt);i++){
        switch(get_symtype(n->child_list[i]->compact_type)){
            case SYM(ExtDef) :
                SemanticAnalysisExtDef(n->child_list[i],systab);
                break;
            case SYM(ExtDefList) :
                SemanticAnalysisExtDefList(n->child_list[i],systab);
                break;
            default :
                break;
        }
    }
}

SA(ExtDef){
    int Production = 0;
    TODO;
}