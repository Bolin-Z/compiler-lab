#include"decls.h"

extern bool has_error;


void error_msg(char error_type,int line_num,const char * format_msg, ...){
    has_error = true;
    va_list ap;
    va_start(ap,format_msg);
    /* defined the out put stream of error msg */
    FILE * output = ERROR_MSG_2;

    fprintf(output,"Error type %c at Line %d: ",error_type,line_num);
    vfprintf(output,format_msg,ap);
    fprintf(output, "\n");

    va_end(ap);
}