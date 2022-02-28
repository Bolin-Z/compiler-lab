%{
    #define PRINT_TK true

    #include "syntax.tab.h"
    #include "decls.h"
    #include <stdio.h>

    void print_tk(char*s);
%}

%{
    #define RETURN(A)      \
        do{                \
            print_tk(#A);  \
            return A;      \
        }while(0)

%}

%option yylineno
relop >|<|>=|<=|==|!=
ws [ \f\r\t\v\n]
%%
{ws} {/* empty */}
struct {RETURN(STRUCT);}
return {RETURN(RETURN);}
if {RETURN(IF);}
else {RETURN(ELSE);}
while {RETURN(WHILE);}
int|float {RETURN(TYPE);}
0|[1-9][0-9]* {RETURN(INT);}
[0-9]+\.[0-9]+ {RETURN(FLOAT);}
[a-zA-Z_][a-zA-Z0-9_]* {RETURN(ID);}
; {RETURN(SEMI);}
, {RETURN(COMMA);}
= {RETURN(ASSIGNOP);}
{relop} {RETURN(RELOP);}
\+ {RETURN(PLUS);}
- {RETURN(MINUS);}
\* {RETURN(STAR);}
\/ {RETURN(DIV);}
&& {RETURN(AND);}
\|\| {RETURN(OR);}
\. {RETURN(DOT);}
! {RETURN(NOT);}
\( {RETURN(LP);}
\) {RETURN(RP);}
\[ {RETURN(LB);}
\] {RETURN(RB);}
\{ {RETURN(LC);}
\} {RETURN(RC);}
<<EOF>> {yyterminate();}
. { error_msg(0,yylineno,"Unknown Symbol");}
%%

void print_tk(char*s){
#if PRINT_TK
    printf("[%d]: %s\n",yylineno,s);
#endif
}