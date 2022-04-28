#include<stdio.h>
#include<stdlib.h>
#include "syntax.tab.h"
#include"decls.h"
#include"error.h"
#include"cst.h"
#include"semantic.h"
#include"ir.h"

extern void yyrestart(FILE*);
extern bool has_error;
struct CST_node * cst_root;

int main(int argc,char* argv[]){
    /* command line arguments */
    if(argc!=3){
        fprintf(ERROR_MSG_2,"Usage: %s *.cmm *.ir\n",argv[0]);
        exit(1);
    }
    FILE * fp = fopen(argv[1],"r");
    if(!fp){
        fprintf(ERROR_MSG_2, "target \"%s\" not exist\n",argv[1]);
        exit(1);
    }
    yyrestart(fp);
    has_error = false;
    cst_root = NULL;
    yyparse();
    fclose(fp);

/* Project 1  
    if(has_error == false){
        if(cst_root != NULL){
            print_CST(cst_root,0);
        }
        destory_tree(cst_root);
    }
*/

/* Project 2 */
    irSystem * irSys = NULL;
    if(!has_error){
        irSys = creatIrSystem();
        SemanticAnalysis(cst_root);
        destory_tree(cst_root);
    }

/* Project 3*/
    if(!has_error){
        // optimize()
        FILE * outputIrCode = fopen(argv[2],"w+");
        if(outputIrCode){
            fprintfIrCode(outputIrCode,irSys);
        }
    }
    destoryIrSystem(irSys);
    
    cst_root = NULL;
    return 0;
}

