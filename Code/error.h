#ifndef __ERROR_H__
#define __ERROR_H__

#include"defs.h"
#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>
#include<stdbool.h>

/* Syntax error */
void error_msg(char error_type,int line_num,const char * format_msg, ...);

/* Semantic error */
void ReportSemanticError(int line, int errortype, char * funid);
bool UpdateFunctionState(int line, char* funid, bool definition);
void OutputSemanticErrorMessage();

#endif