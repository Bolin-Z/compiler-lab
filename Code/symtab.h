#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include"typesys.h"

# define SYMSTACKSIZE 256
# define HASHTABLESIZE 256
# define SCOPESTACKSIZE 16

extern SymbolTable symtab;

typedef struct Symbol{
    /* fields related to Symbol table */
    char* _id;
    size_t pre;
    size_t nxt;
    /* fields contains infomation of the symbol */
    Attribute _attribute;
} Symbol;

typedef struct SymbolStack{
    Symbol * _stack;
    size_t cur;
    size_t _stacksize;
} SymbolStack;

typedef struct SymbolHashTable{
    size_t HashList[HASHTABLESIZE];
} SymbolHashTable;

typedef struct Scope{
    char * ScopeName;
    size_t Scopeidx;
} Scope;

typedef struct ScopeStack{
    Scope * _stack;
    size_t cur;
    size_t _stacksize;
} ScopeStack;

typedef struct SymbolTable{
    SymbolHashTable _hashtable;
    SymbolStack _symstack;
    ScopeStack _scopestack;
} SymbolTable;

typedef struct Attribute{
    enum {ERROR,VARIABLE,FUNCTION,TYPENAME} IdClass;
    TypeDescriptor * _idtype;
    // type
    union{
        int dummy;
        float dummy2;
    };
} Attribute;

/* API */
int initSymbolTable();

size_t OpenScope(char* ScopeName);
void CloseScope();
Scope * CurScope();

Symbol * Insert(char* name);
Symbol * LookUp(char* name, bool curscope);

#endif