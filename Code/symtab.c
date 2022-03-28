#include "symtab.h"

unsigned int hash_pjw(char* name);

SymbolStack * CreatSymbolStack();
void DestorySymbolStack(SymbolStack* s);
Symbol * PushSymbolStack(SymbolStack * s);
void PopSymbolStack(SymbolStack * s);
int TopIdxOfSymbolStack(SymbolStack * s);

ScopeStack * CreatScopeStack();
void DestoryScopeStack(ScopeStack* s);
Scope * PushScopeStack(ScopeStack * s);
void PopScopeStack(ScopeStack * s);
int TopIdxOfScopeStack(ScopeStack * s);

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
    return st;
}

void DestorySymbolTable(SymbolTable * s){
    DestorySymbolStack(s->symstack);
    DestoryScopeStack(s->scopstack);
    free(s);
}

# define DEFINE_STACK_FUNCTION(type) \
\
    void Destory##type(type* t);                                                        \
\
    type##Stack * Creat##type##Stack(){                                                 \
        type##Stack * s = (type##Stack *)malloc(sizeof(type##Stack));                   \
        if(s){                                                                          \
            s->capacity = type##stacksize;                                              \
            s->numofstack = 1;                                                          \
            s->idx = 0;                                                                 \
            s->curidx = 0;                                                              \
            s->curstack = 0;                                                            \
            s->stack = (type **)malloc(s->numofstack*sizeof(type *));                   \
            if(!s->stack){                                                              \
                free(s); s = NULL;                                                      \
            }else{                                                                      \
                s->stack[0] = (type *)malloc(type##stacksize*sizeof(type));             \
                if(!s->stack[0]){                                                       \
                    free(s->stack); free(s); s = NULL;                                  \
                }                                                                       \
            }                                                                           \
        }                                                                               \
        return s;                                                                       \
    }                                                                                   \
\
    void Destory##type##Stack(type##Stack * s){                                         \
        for(int i = 0;i < s->curstack;i++){                                             \
            for(int j = 0;j < (int)(type##stacksize);j++){                              \
                Destory##type(&(s->stack[i][j]));                                       \
            }                                                                           \
        }                                                                               \
        for(int i = 0;i < s->curidx;i++){                                               \
            Destory##type(&(s->stack[s->curstack][i]));                                 \
        }                                                                               \
        for(int i = 0;i < (int)(s->numofstack);i++){                                    \
            free(s->stack[i]);                                                          \
        }                                                                               \
        free(s->stack);                                                                 \
        free(s);                                                                        \
    }                                                                                   \
\
    type* Push##type##Stack(type##Stack * s){                                           \
        if(s->curidx == (int) (type##stacksize)){                                       \
            if(s->curstack == (int)(s->numofstack - 1)){                                \
                type ** newstack = (type **)malloc((s->numofstack + 1)*sizeof(type *)); \
                if(!newstack) return NULL;                                              \
                for(int i = 0;i <= s->curstack;i++)                                     \
                    newstack[i] = s->stack[i];                                          \
                type * _page = (type *)malloc(type##stacksize*sizeof(type));            \
                if(!_page){                                                             \
                    free(newstack);                                                     \
                    return NULL;                                                        \
                }                                                                       \
                s->numofstack += 1;                                                     \
                newstack[s->curstack + 1] = _page;                                      \
                free(s->stack);                                                         \
                s->stack = newstack;                                                    \
            }                                                                           \
            s->curstack += 1;                                                           \
            s->curidx = 0;                                                              \
        }                                                                               \
        type * r = &(s->stack[s->curstack][s->curidx]);                                 \
        s->curidx += 1;                                                                 \
        s->idx += 1;                                                                    \
        return r;                                                                       \
    }                                                                                   \
\
    void Pop##type##Stack(type##Stack * s){                                             \ 
        if(s->curidx == 0){                                                             \
            if(s->curstack == 0) return;                                                \
            s->curstack -= 1;                                                           \
            s->curidx = (int)(type##stacksize);                                         \
        }                                                                               \
        s->idx -= 1;                                                                    \
        s->curidx -= 1;                                                                 \
        DestorySymbol(&(s->stack[s->curstack][s->curidx]));                             \
    }                                                                                   \
\
    type* Access##type##Stack(type##Stack * s,int index){                               \
        if(index >= s->idx || index < 0) return NULL;                                   \
        int targetstack = (int) (index/(type##stacksize));                              \
        int targetidx = (int) (index%(type##stacksize));                                \
        return &(s->stack[targetstack][targetidx]);                                     \
    }                                                                                   \
\
    int TopIdxOf##type##Stack(type##Stack * s){                                         \
        return s->idx = 0 ? NIL : (s->idx - 1);                                         \
    }


DEFINE_STACK_FUNCTION(Symbol)
DEFINE_STACK_FUNCTION(Scope)

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
    t->scopebeginidx = 0;
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