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