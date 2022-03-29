#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include"typesys.h"
#include"rastack.h"

/* Macro related to hashlist */
# define HASHTABLESIZE 1024 // 0x400
# define DUMMYIDX 0

/* Structure to realize the Store and Search Functions of symbol table */
typedef struct SymbolTable{
    SymbolHashTable symhtable;
    SymbolStack * symstack;
    ScopeStack * scopstack;
} SymbolTable;

DEFINE_RASTACK(Symbol,256)
DEFINE_RASTACK(Scope,256)

/* Structure to realize the hash-list */
typedef struct SymbolHashTable{
    int hashlist[HASHTABLESIZE];
} SymbolHashTable;

typedef struct Symbol{
    char* id; // point to the id field of cst_id_node
    int pre,nxt;
    Attribute attribute;
} Symbol;

/* Attribute infomation of the identifier */
typedef struct Attribute{
    enum {NONE,VARIABLE,FUNCTION,TYPENAME} IdClass;
    TypeDescriptor * IdType;
    union{
        /* IdClass == VARIABLE */
        /* IdType stores the type of id */
        /* IdClass == FUNCTION */
        /* IdType stores the type of return value */
        struct{int Argc; TypeDescriptor** ArgTypeList; bool defined;} Func;
        /* IdClass == TYPENAME */
        /* IdType stores the type that the typename defined */
    } Info;
} Attribute;

/* Structure to realize nested scope functions */
typedef struct Scope{
    char * scopename;
    int scopebeginidx;
} Scope;

SymbolTable * CreatSymbolTable();
void DestorySymbolTable(SymbolTable* s);

Scope * OpenScope(SymbolTable* s, char* ScopeName);
void CloseScope(SymbolTable* s);
Scope * CurScope(SymbolTable* s);

Symbol * Insert(SymbolTable* s, char* name);
Symbol * LookUp(SymbolTable* s, char* name);

//                              The image of SymbolTable
//
//                 | .  |
//    +---+        + .  +   
//    |   |---+    | .  |   
//    +---+   |    +----+   
//    |   |   + +->| id |-----+     |  . |
//    +---+   | |  +----+     |     +  . +
//    |   |   +--->| id |---+ |     |  . |
//    +---+     |  +----+   | |     +----+
//    |   |-----+  | id |<--+ |  +--| S2 |
//    +---+        +----+<----|--+  +----+
//    |   |        |    |     | +---| S1 |
//    +---+        +----+     v |   +----+
//   HashList    SymbolStack    v  ScopeStack
//

#endif