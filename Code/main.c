#include<stdio.h>
#include<stdlib.h>

int main(int argc,char* argv[]){
    /* command line arguments */
    if(argc!=2){
        fprintf(stderr,"usage: parser *.cmm\n");
        exit(1);
    }
    FILE * fp = NULL;
    fp = fopen(argv[1],"r");
    if(!fp){
        fprintf(stderr, "target \"%s\" not exist\n",argv[1]);
        exit(1);
    }
    
    
    
    fclose(fp);
    fp = NULL;
    return 0;
}

