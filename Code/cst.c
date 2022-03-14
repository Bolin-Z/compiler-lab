#include"cst.h"

/* non-terminal node */

struct CST_int_node{
    int compact_type;
    long intval;
};

struct CST_float_node{
    int compact_type;
    float floatval;
};

struct CST_id_node{
    int compact_type;
    char * ID;
};

struct CST_mul_node{
    int compact_type;
    int tktype; 
};

struct CST_uniq_node{
    int compact_type;
};

/* function */

int set_compact_type(int sym_type,int node_type){
    return (sym_type << 4) | node_type;
}

int get_symtype(int compact_type){
    return (compact_type >> 4);
}
int get_nodetype(int compact_type){
    return (compact_type & NODE_MASK);
}

bool is_token(int compact_type){
    return (compact_type & NT_NODE) != NT_NODE;
}

int str2tktype(const char * op){
    size_t op_len = strlen(op);
    switch(op_len){
        case 5 : if(!strncmp("float",op,op_len)) return TK(FLOAT);
        case 3 : if(!strncmp("int",op,op_len))   return TK(INT);
        case 2 : if(!strncmp("<=",op,op_len))    return TK(LTE);
                 if(!strncmp(">=",op,op_len))    return TK(GTE);
                 if(!strncmp("==",op,op_len))    return TK(EQ);
                 if(!strncmp("!=",op,op_len))    return TK(NEQ);
        case 1 : if(op[0] == '>')                return TK(GT);
                 if(op[0] == '<')                return TK(LT);   
    }
    return 0;
}

static inline struct CST_node * creat_mul_node(int comp_type,int val){
    struct CST_mul_node * p = (struct CST_mul_node *)malloc(sizeof(struct CST_mul_node));
    if(!p) return NULL;
    p->compact_type = comp_type;
    p->tktype = val;
    return (struct CST_node *)p;    
}

static inline struct CST_node * creat_uniq_node(int comp_type){
    struct CST_uniq_node * p = (struct CST_uniq_node *)malloc(sizeof(struct CST_uniq_node));
    if(!p) return NULL;
    p->compact_type = comp_type;
    return (struct CST_node *)p;    
}

static inline struct CST_node * creat_int_node(int comp_type,long val){
    struct CST_int_node * p = (struct CST_int_node *)malloc(sizeof(struct CST_int_node));
    if(!p) return NULL;
    p->compact_type = comp_type;
    p->intval = val;
    return (struct CST_node *)p;
}

static inline struct CST_node * creat_float_node(int comp_type,float val){
    struct CST_float_node * p = (struct CST_float_node *)malloc(sizeof(struct CST_float_node));
    if(!p) return NULL;
    p->compact_type = comp_type;
    p->floatval = val;
    return (struct CST_node *)p;
}

static inline struct CST_node * creat_id_node(int comp_type,const char* val){
    struct CST_id_node * p = (struct CST_id_node *)malloc(sizeof(struct CST_id_node));
    if(!p) return NULL;
    p->compact_type = comp_type;
    size_t id_len = strlen(val);
    p->ID = (char*)malloc(id_len+1);
    if(p->ID == NULL){
        free(p);
        return NULL;
    }
    for(size_t i = 0;i < id_len;i++){
        p->ID[i] = val[i];
    }
    p->ID[id_len] = '\0';
    return (struct CST_node *)p;
}

static inline struct CST_node * creat_nt_node(int comp_type,int lineno){
    struct CST_node * p = (struct CST_node *)malloc(sizeof(struct CST_node));
    if(!p) return NULL;
    p->compact_type = comp_type;
    p->lineno = lineno;
    p->child_cnt = 0;
    p->child_list = NULL;
    return p;
}

struct CST_node * creat_node(int sym_type,int node_type,int lineno,const char* lexme){
    struct CST_node * p = NULL;
    size_t lexme_len = (lexme == NULL)? 0 : strlen(lexme);
    char * t = (char *)malloc(lexme_len+1);
    if (!t) return NULL;
    for(size_t i = 0;i < lexme_len;i++){
        t[i] = lexme[i];
    }
    t[lexme_len] = '\0';
    int comp_type = set_compact_type(sym_type,node_type);
    switch(node_type){
        case NT_NODE    : p = creat_nt_node(comp_type,lineno); break;
        case INT_NODE   : p = creat_int_node(comp_type,strtol(t,NULL,0)); break;
        case FLOAT_NODE : p = creat_float_node(comp_type,strtof(t,NULL)); break;
        case ID_NODE    : p = creat_id_node(comp_type,t); break;
        case MUL_NODE   : p = creat_mul_node(comp_type,str2tktype(t)); break;
        case UNIQ_NODE  : p = creat_uniq_node(comp_type); break;
    }
    free(t);
    return p;
}

bool add_child(struct CST_node* father,size_t cnt,...){
    if(!father) return false;
    if(get_nodetype(father->compact_type)!= NT_NODE) return false;
    va_list children;
    va_start(children,cnt);
    size_t new_cnt = father->child_cnt + cnt;
    struct CST_node ** new_list = (struct CST_node **)malloc(sizeof(struct CST_node *)*new_cnt);
    size_t cptr = 0;
    if(!new_list){
        va_end(children);
        return false;
    }
    /* copy the old children ptr */
    for(;cptr < father->child_cnt;cptr++){
        new_list[cptr] = father->child_list[cptr];
    }
    /* add new children */
    for(;cptr < new_cnt;cptr++){
        new_list[cptr] = va_arg(children,struct CST_node *);
    }
    father->child_cnt = new_cnt;
    free(father->child_list);
    father->child_list = new_list;
    va_end(children);
    return true;
}

void destory_node(struct CST_node * cur){
    if(cur){
        switch(get_nodetype(cur->compact_type)){
            case NT_NODE : 
                if(cur->child_list) 
                    free(cur->child_list);
                break;
            case ID_NODE :
                if(((struct CST_id_node * )cur)->ID) 
                    free(((struct CST_id_node * )cur)->ID);
                break;
            default : 
                break;
        }
        free(cur);
    }
}

void destory_tree(struct CST_node * root){
    if(root){
        /* if root is a NT_NODE and contains child nodes */
        if(get_nodetype(root->compact_type) == NT_NODE){
            if(root->child_cnt!=0){
                for(size_t i = 0;i < root->child_cnt;i++){
                    destory_tree(root->child_list[i]);
                }
            }
        }
        /* delete cur itself */
        destory_node(root);
    }
}

struct CST_node * copy_node(struct CST_node * rhs){
    struct CST_node * lhs = NULL;
    if(rhs){
        switch(get_nodetype(rhs->compact_type)){
            case NT_NODE : 
                lhs = creat_nt_node(rhs->compact_type,rhs->lineno);
                lhs->child_cnt = rhs->child_cnt;
                lhs->child_list = (struct CST_node**)malloc(sizeof(rhs->child_list));
                if(!lhs->child_list){
                    free(lhs);
                    return NULL;
                }
                for(size_t i = 0;i < rhs->child_cnt;i++)
                    lhs->child_list[i] = rhs->child_list[i];
                break;
            case INT_NODE :
                lhs = creat_int_node(((struct CST_int_node *)rhs)->compact_type,((struct CST_int_node *)rhs)->intval);
                break;
            case FLOAT_NODE :
                lhs = creat_float_node(((struct CST_float_node *)rhs)->compact_type,((struct CST_float_node *)rhs)->floatval);
                break;
            case ID_NODE :
                lhs = creat_id_node(((struct CST_id_node *)rhs)->compact_type,((struct CST_id_node *)rhs)->ID);
                break;
            case MUL_NODE :
                lhs = creat_mul_node(((struct CST_mul_node *)rhs)->compact_type,((struct CST_mul_node *)rhs)->tktype);
                break;
            case UNIQ_NODE :
                lhs = creat_uniq_node(((struct CST_uniq_node *)rhs)->compact_type);
                break;
            default :
                break;
        }
    }
    return lhs;
}

void print_CST(struct CST_node * cur,int depth){
    if(cur!=NULL){
        if(is_token(cur->compact_type)){
            for(int i = 0;i < depth;i++) printf("  ");
            printf("%s",symtype2str[get_symtype(cur->compact_type)]);
            switch(get_nodetype(cur->compact_type)){
                case ID_NODE    : printf(": %s",((struct CST_id_node *)cur)->ID); break;
                case MUL_NODE   : 
                    if(get_symtype(cur->compact_type) == SYM(TYPE)){
                        if(((struct CST_mul_node *)cur)->tktype == TK(INT)){
                            printf(": int");
                        }else if(((struct CST_mul_node *)cur)->tktype == TK(FLOAT)){
                            printf(": float");
                        }else printf(": error!");
                    }
                    break;
                case INT_NODE   : printf(": %ld",((struct CST_int_node *)cur)->intval); break;
                case FLOAT_NODE : printf(": %f",((struct CST_float_node *)cur)->floatval); break;
                default         : break;
            }
            printf("\n");
        }else{
            if(cur->child_cnt != 0){
                for(int i = 0;i < depth;i++) printf("  ");
                printf("%s",symtype2str[get_symtype(cur->compact_type)]);
                printf(" (%d)\n",cur->lineno);
                for(size_t i = 0;i < cur->child_cnt;i++){
                    print_CST(cur->child_list[i],depth+1);
                }
            }
        }
    }
}