#include"decls.h"

void error_msg(int error_type,int line_num,char * format_msg, ...){
    va_list ap;
    va_start(ap,format_msg);
    /* defined the out put stream of error msg */
    FILE * output = ERROR_MSG_2;

    fprintf(output,"Error type %c at Line %d: ",Error_Type[error_type],line_num);
    vfprintf(output,format_msg,ap);
    fprintf(output, "\n");

    va_end(ap);
}