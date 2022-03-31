#include "symtab.h"

unsigned int hash_pjw(char* name);
DEFINE_RASTACK_FUNCTION(Symbol)
DEFINE_RASTACK_FUNCTION(Scope)

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
                DestoryTypeDescriptor(t->attribute.Info.Func.ArgTypeList[0]);
            free(t->attribute.Info.Func.ArgTypeList);
            t->attribute.Info.Func.ArgTypeList = NULL;
            t->attribute.Info.Func.Argc = 0;
            t->attribute.Info.Func.defined =false;
            break;
        default : /* Do nothing */ 
            break;
    }
    t->attribute.IdClass = NONE;
    DestoryTypeDescriptor(t->attribute.IdType);
    t->attribute.IdType = NULL;
}

void DestoryScope(Scope* t){
    free(t->scopename);
    t->scopebeginidx = -1;
    t->scopeendidx = -1;
}

Scope * OpenScope(SymbolTable* s, char* ScopeName){
    Scope * newScope = PushScopeStack(s->scopstack);
    if(newScope){
        newScope->scopebeginidx = s->symstack->idx;
        newScope->scopeendidx = newScope->scopebeginidx;
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
    /* always close the top-most scope */
    while(top >= curscope->scopebeginidx){
        Symbol * top_symbol = AccessSymbolStack(s->symstack,top);
        /* The top-most symbol on stack must be the first symbol of one of the hashlists. */
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

/* Look Up 'name' in Symbol table s, curscope indicates that whether to search in current scope. */
Symbol * LookUp(SymbolTable* s, char* name, bool curscope){
    int hashval = (int)hash_pjw(name);
    int checkidx = s->symhtable.hashlist[hashval];
    Scope * cur = CurScope(s);
    while((checkidx != DUMMYIDX) && ((curscope && (checkidx >= cur->scopebeginidx)) || (!curscope))){
        Symbol* check = AccessSymbolStack(s->symstack,checkidx);
        if(0 == strcmp(check->id,name)) break;
        checkidx = check->nxt;
    }
    if(!curscope){
        /* Look up all scope */
        return (checkidx == DUMMYIDX) ? (NULL) : AccessSymbolStack(s->symstack, checkidx);
    }else{
        /* Look up in current scope */
        return (checkidx >= cur->scopebeginidx) ? AccessSymbolStack(s->symstack, checkidx) : (NULL);
    }
}

/* Register symbol with name as key and return a pointer to it. */
Symbol * Insert(SymbolTable* s, char* name){
    int hashval = (int)hash_pjw(name);
    Symbol * newsymbol = PushSymbolStack(s->symstack);
    if(newsymbol){
        newsymbol->id = name;
        int curidx = TopIdxOfSymbolStack(s->symstack);
        newsymbol->pre = hashval - HASHTABLESIZE;
        newsymbol->nxt = s->symhtable.hashlist[hashval];
        if(newsymbol->nxt != DUMMYIDX){
            Symbol * brother = AccessSymbolStack(s->symstack,newsymbol->nxt);
            brother->pre = curidx;
        }
        s->symhtable.hashlist[hashval] = curidx;
        Scope * cur = CurScope(s);
        cur->scopeendidx += 1;
    }
    return newsymbol;
}

/* access symbol by index */
Symbol * Access(SymbolTable * s, int index){
    return AccessSymbolStack(s->symstack,index);
}

int GetIndex(SymbolTable* s, Symbol * id){
    if(id->pre < 0){
        return s->symhtable.hashlist[id->pre + HASHTABLESIZE];
    }else{
        Symbol * brother = Access(s,id->pre);
        return brother->nxt;
    }
}