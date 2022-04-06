#ifndef __ERROR_H__
#define __ERROR_H__

#include"defs.h"
#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>
#include<stdbool.h>

void error_msg(char error_type,int line_num,const char * format_msg, ...);

void ReportSemanticError(int error_num,int line_num,char * tempmsg);
bool UpdateFunctionState(int line_num, char* funid, bool declaration);
void CreatSemanticErrorSystem();
void DestorySemanticErrorSystem();

#endif