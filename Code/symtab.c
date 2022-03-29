#include "symtab.h"

unsigned int hash_pjw(char* name);

/* Hash Function Designed by P.J. Weinberger */
unsigned int hash_pjw(char* name){
    unsigned int val = 0, i;
    for(; *name; ++name){
        val = (val << 2) + *name;
        if (i = val & ~HASHTABLESIZE) 
            val = (val ^ (i >> 12)) & HASHTABLESIZE;
    }
    return val;
}

SymbolTable * CreatSymbolTable(){
    SymbolTable * st = (SymbolTable *)malloc(sizeof(SymbolTable));
    if(st){
        st->symstack = CreatSymbolStack();
        st->scopstack = CreatScopeStack();
        if((!st->symstack)||(!st->scopstack)){
            DestorySymbolStack(st->symstack);
            DestoryScopeStack(st->scopstack);
            free(st); st = NULL;
        }
    }
    for(int i = 0;i < HASHTABLESIZE;i++)
        st->symhtable.hashlist[i] = DUMMYIDX;
    Symbol* dummy = PushSymbolStack(st);
    CreatTypeSystem();
    return st;
}

void DestorySymbolTable(SymbolTable * s){
    DestorySymbolStack(s->symstack);
    DestoryScopeStack(s->scopstack);
    DestoryTypeSystem();
    free(s);
}

void DestorySymbol(Symbol* t){
    t->id = NULL;
    t->pre = t->nxt = DUMMYIDX;
    switch(t->attribute.IdClass){
        case FUNCTION :
            for(int i = 0;i < t->attribute.Info.Func.Argc;i++)
                DestoryTypeDescriptor(t->attribute.Info.Func.ArgTypeList[i]);
            free(t->attribute.Info.Func.ArgTypeList);
            t->attribute.Info.Func.Argc = 0;
            break;
        default : /* Do nothing */ 
            break;
    }
    t->attribute.IdClass = NONE;
    DestoryTypeDescriptor(t->attribute._idtype);
}

void DestoryScope(Scope* t){
    free(t->scopename);
    t->scopebeginidx = NIL;
}

Scope * OpenScope(SymbolTable* s, char* ScopeName){
    Scope * newScope = PushScopeStack(s->scopstack);
    if(newScope){
        newScope->scopebeginidx = s->symstack->idx;
        if(ScopeName){
            int len = (int)(strlen(ScopeName)+1);
            newScope->scopename = (char*)malloc(len*sizeof(char));
            if(newScope->scopename){
                for(int i = 0;i < len - 1;i++)
                    newScope->scopename[i] = ScopeName[i];
                newScope->scopename[len-1] = '\0';
            }
        }
    }
    return newScope;
}

void CloseScope(SymbolTable* s){
    Scope * curscope = CurScope(s);
    int top = TopIdxOfSymbolStack(s->symstack);
    while(top >= curscope->scopebeginidx){
        Symbol * top_symbol = AccessSymbolStack(s->symstack,top);
        /* The top-most symbol on stack must be the first symbol of one of the hashlist */
        if(top_symbol->nxt != DUMMYIDX){
            Symbol * sibiling = AccessSymbolStack(s->symstack,top_symbol->nxt);
            sibiling->pre = top_symbol->pre;
        }
        s->symhtable.hashlist[top_symbol->pre + HASHTABLESIZE] = top_symbol->nxt;
        PopSymbolStack(s->symstack);
        top--;
    }
    PopScopeStack(s->scopstack);
}

Scope * CurScope(SymbolTable* s){
    return AccessScopeStack(s->scopstack,TopIdxOfScopeStack(s->scopstack));
}

Symbol * LookUp(SymbolTable* s, char* name){
    int hashval = (int)hash_pjw(name);
    int checkidx = s->symhtable.hashlist[hashval];
    while(checkidx != DUMMYIDX){
        Symbol* check = AccessSymbolStack(s->symstack,checkidx);
        if(0 == strcmp(check->id,name)) break;
        checkidx = check->nxt;
    }
    return (checkidx == DUMMYIDX) ? (NULL) : (AccessSymbolStack(s->symstack,checkidx));
}


Symbol * Insert(SymbolTable* s, char* name){
    int hashval = (int)hash_pjw(name);
    Scope * cur = CurScope(s);
    int checkidx = s->symhtable.hashlist[hashval];
    bool conflict = false;
    while((checkidx != DUMMYIDX)&&(checkidx >= cur->scopebeginidx)){
        Symbol* check = AccessSymbolStack(s->symstack,checkidx);
        if(0 == strcmp(check->id,name)){
            conflict = true;
            break;
        }
        checkidx = check->nxt;
    }
    if(conflict) return NULL;
    Symbol * newsymbol = PushSymbolStack(s->symstack);
    int curidx = TopIdxOfSymbolStack(s->symstack);
    newsymbol->pre = hashval - HASHTABLESIZE;
    if(s->symhtable.hashlist[hashval] == DUMMYIDX){
        newsymbol->nxt = DUMMYIDX;
    }else{
        int nxtidx = s->symhtable.hashlist[hashval];
        Symbol * nxtsymbol = AccessSymbolStack(s->symstack,nxtidx);
        nxtsymbol->pre = curidx;
        newsymbol->nxt = nxtidx;
    }
    s->symhtable.hashlist[hashval] = curidx;
    return newsymbol;
}