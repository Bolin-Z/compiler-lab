#include<stdio.h>
#include<stdlib.h>
#include "syntax.tab.h"
#include"decls.h"
#include"error.h"
#include"cst.h"
#include"semantic.h"

extern void yyrestart(FILE*);
extern bool has_error;
struct CST_node * cst_root;

int main(int argc,char* argv[]){
    /* command line arguments */
    if(argc!=2){
        fprintf(ERROR_MSG_2,"Usage: %s *.cmm\n",argv[0]);
        exit(1);
    }
    FILE * fp = NULL;
    fp = fopen(argv[1],"r");
    if(!fp){
        fprintf(ERROR_MSG_2, "target \"%s\" not exist\n",argv[1]);
        exit(1);
    }
    yyrestart(fp);
    has_error = false;
    cst_root = NULL;
    yyparse();
    fclose(fp);
    fp = NULL;
/* Project 1  
    if(has_error == false){
        if(cst_root != NULL){
            print_CST(cst_root,0);
        }
        destory_tree(cst_root);
    }
*/

/* Project 2 */
    if(!has_error){
        SemanticAnalysis(cst_root);
        destory_tree(cst_root);
    }

    cst_root = NULL;
    return 0;
}

