#ifndef __RASTACK_H__
#define __RASTACK_H__

// Use multiple pages to simulate a stack that can be accessed randomly.
//
// Arguments
// type    : the object type stack contains
// maxsize : the maximum size of each page
//
// The mapped index of object in the stack : idx --> (curpage, curidx) The physical index of the object
// typeStack[idx] = typeStack->pages[curpage][curidx]
//
// Methods
// void Destorytype(type* t)                      : method provided by user to clean the object
// typeStack * CreattypeStack()                   : creat a new stack contains object of type 'type'
// void DestorytypeStack(typeStack * s)           : destory the stack object
// type* PushtypeStack(typeStack * s)             : push an object into the stack and return a pointer to it
// void PoptypeStack(typeStack * s)               : pop the top-most object of the stack
// type* AccesstypeStack(typeStack * s,int index) : access the object
// int TopIdxOftypeStack(type##Stack * s)         : the index of the top-most object, -1 if the stack is empty

# define DEFINE_RASTACK(type,maxsize) \
\
    typedef struct type##Stack{            \
        type ** pages;                     \
        int idx;                           \
        int curidx;                        \
        int curpage;                       \
        size_t capacity;                   \
        size_t numofpages;                 \
    } type##Stack;                         \
    const size_t type##pagesize = maxsize; \
\
    void Destory##type(type* t);                \
\
    type##Stack * Creat##type##Stack();         \
\
    void Destory##type##Stack(type##Stack * s); \
\
    type* Push##type##Stack(type##Stack * s);   \
\
    void Pop##type##Stack(type##Stack * s);     \
\
    type* Access##type##Stack(type##Stack * s,int index); \
\
    int TopIdxOf##type##Stack(type##Stack * s);


# define DEFINE_RASTACK_FUNCTION(type) \
\
    type##Stack * Creat##type##Stack(){                                                 \
        type##Stack * s = (type##Stack *)malloc(sizeof(type##Stack));                   \
        if(s){                                                                          \
            s->capacity = type##pagesize;                                               \
            s->numofstack = 1;                                                          \
            s->idx = 0;                                                                 \
            s->curidx = 0;                                                              \
            s->curpage = 0;                                                             \
            s->pages = (type **)malloc(s->numofpages*sizeof(type *));                   \
            if(!s->pages){                                                              \
                free(s); s = NULL;                                                      \
            }else{                                                                      \
                s->pages[0] = (type *)malloc(type##pagesize*sizeof(type));              \
                if(!s->pages[0]){                                                       \
                    free(s->pages); free(s); s = NULL;                                  \
                }                                                                       \
            }                                                                           \
        }                                                                               \
        return s;                                                                       \
    }                                                                                   \
\
    void Destory##type##Stack(type##Stack * s){                                         \
        for(int i = 0;i < s->curpage;i++){                                              \
            for(int j = 0;j < (int)(type##pagesize);j++){                               \
                Destory##type(&(s->pages[i][j]));                                       \
            }                                                                           \
        }                                                                               \
        for(int i = 0;i < s->curidx;i++){                                               \
            Destory##type(&(s->stack[s->curpage][i]));                                  \
        }                                                                               \
        for(int i = 0;i < (int)(s->numofpages);i++){                                    \
            free(s->pages[i]);                                                          \
        }                                                                               \
        free(s->pages);                                                                 \
        free(s);                                                                        \
    }                                                                                   \
\
    type* Push##type##Stack(type##Stack * s){                                           \
        if(s->curidx == (int) (type##pagesize)){                                        \
            if(s->curpage == (int)(s->numofpages - 1)){                                 \
                type ** newpages = (type **)malloc((s->numofpages + 1)*sizeof(type *)); \
                if(!newpages) return NULL;                                              \
                for(int i = 0;i <= s->curpage;i++)                                      \
                    newpages[i] = s->pages[i];                                          \
                type * _page = (type *)malloc(type##pagesize*sizeof(type));             \
                if(!_page){                                                             \
                    free(newpages);                                                     \
                    return NULL;                                                        \
                }                                                                       \
                s->numofpages += 1;                                                     \
                newpages[s->curpage + 1] = _page;                                       \
                free(s->pages);                                                         \
                s->pages = newpages;                                                    \
            }                                                                           \
            s->curpage += 1;                                                            \
            s->curidx = 0;                                                              \
        }                                                                               \
        type * r = &(s->pages[s->curpage][s->curidx]);                                  \
        s->curidx += 1;                                                                 \
        s->idx += 1;                                                                    \
        return r;                                                                       \
    }                                                                                   \
\
    void Pop##type##Stack(type##Stack * s){                                             \ 
        if(s->curidx == 0){                                                             \
            if(s->curpage == 0) return;                                                 \
            s->curpage -= 1;                                                            \
            s->curidx = (int)(type##pagesize);                                          \
        }                                                                               \
        s->idx -= 1;                                                                    \
        s->curidx -= 1;                                                                 \
        DestorySymbol(&(s->pages[s->curpage][s->curidx]));                              \
    }                                                                                   \
\
    type* Access##type##Stack(type##Stack * s,int index){                               \
        if(index >= s->idx || index < 0) return NULL;                                   \
        int targetpage = (int) (index/(type##pagesize));                                \
        int targetidx = (int) (index%(type##pagesize));                                 \
        return &(s->pages[targetpage][targetidx]);                                      \
    }                                                                                   \
\
    int TopIdxOf##type##Stack(type##Stack * s){                                         \
        return s->idx - 1;                                                              \
    }

#endif