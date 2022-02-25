#include<stdio.h>
#include<stdlib.h>
#include"decls.h"

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
    
    fclose(fp);
    fp = NULL;
    return 0;
}

