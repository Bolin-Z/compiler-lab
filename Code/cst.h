#ifndef __CST_H__
#define __CST_H__

#include<stddef.h>
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdarg.h>
#include"defs.h"

/* Data Structure for CST */
/*
   The lower 4 bits of compact_type is used to store extra message
   
   31             4           0
    +-------------+-----------+
    | symbol type | node type |
    + ------------+-----------+
*/
/* 
    +------------+----------------------------------------------+
    |                          node type                        |          
    +------------+----------------------------------------------+
    | NT_NODE    | non-terminal node                            |
    +------------+----------------------------------------------+
    | INT_NODE   | token contains int val                       |
    +------------+----------------------------------------------+
    | FLOAT_NODE | token contains float val                     |
    +------------+----------------------------------------------+
    | ID_NODE    | token contains ID                            |
    +------------+----------------------------------------------+
    | MUL_NODE   | token that has multiple type vals e.g. RELOP |
    +------------+----------------------------------------------+
    | UNIQ_NODE  | token that has unique type val e.g. SEMI     |
    +------------+----------------------------------------------+

*/

enum{
    NT_NODE  = 8, 
    INT_NODE = 0, FLOAT_NODE = 1, ID_NODE = 2, MUL_NODE = 3, UNIQ_NODE = 4,
    NODE_MASK = 0xf
};

struct CST_node{
    int compact_type;
    int lineno;
    size_t child_cnt;
    struct CST_node ** child_list;
};

/* Interface */

/* Manipulation */
struct CST_node * creat_node(int sym_type,int node_type,int lineno,const char* lexme);
bool add_child(struct CST_node* father,size_t cnt,...);
void destory_node(struct CST_node *);
void destory_tree(struct CST_node *);
struct CST_node * copy_node(struct CST_node *);

/* Utility */
int get_symtype(int compact_type);
int get_nodetype(int compact_type);
bool is_token(int compact_type);
int str2tktype(const char * op);

/* Action */
void print_CST(struct CST_node *,int);

#endif