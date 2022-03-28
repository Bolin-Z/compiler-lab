#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include"typesys.h"

/* Macro related to hashlist */
# define HASHTABLESIZE 1024 // 0x400
# define NIL -1
# define DUMMYIDX 0

# define DEFINE_STACK(type,size) \
\
    typedef struct type##Stack{ \
        type ** stack;          \
        int idx;                \
        int curidx;             \
        int curstack;           \
        size_t capacity;        \
        size_t numofstack;      \
    } type##Stack;              \
    const size_t type##stacksize = size;

typedef struct Symbol{
    char* id; // point to the id field of cst_id_node
    int pre,nxt;
    Attribute attribute;
} Symbol;

typedef struct Scope{
    char * scopename;
    int scopebeginidx;
} Scope;

DEFINE_STACK(Symbol,256)
DEFINE_STACK(Scope,256)

typedef struct SymbolTable{
    SymbolHashTable symhtable;
    SymbolStack * symstack;
    ScopeStack * scopstack;
} SymbolTable;

/* point to the idx of symbol in stack */
typedef struct SymbolHashTable{
    int hashlist[HASHTABLESIZE];
} SymbolHashTable;

/* TODO() */
typedef struct Attribute{
    enum {NONE,VARIABLE,FUNCTION,TYPENAME} IdClass;
    TypeDescriptor * _idtype;
    union{
        /* IdClass == VARIABLE */
        /* empty */
        /* IdClass == FUNCTION */
        struct{int Argc; TypeDescriptor** ArgTypeList; bool defined;} Func;
        /* IdClass == TYPENAME */
        /* empty */
    } Info;
} Attribute;

/* API */

/* Creat and initialize SymbolTable object, return NULL if failed. */
SymbolTable * CreatSymbolTable();
void DestorySymbolTable(SymbolTable* s);

Scope * OpenScope(SymbolTable* s, char* ScopeName);
void CloseScope(SymbolTable* s);
Scope * CurScope(SymbolTable* s);

Symbol * Insert(SymbolTable* s, char* name);
Symbol * LookUp(SymbolTable* s, char* name);

#endif