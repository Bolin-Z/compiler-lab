%{
    #define PRINT_TK false

    #include "syntax.tab.h"
    #include "decls.h"
    #include <stdio.h>
%}

%option yylineno

%{
    int yycolumn = 1;
    #define YY_USER_ACTION                               \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn;                  \
        yylloc.last_column = yycolumn + yyleng - 1;      \
        yycolumn += yyleng;
%}

%{
    void print_tk(char*s);
    #define RETURN(A)      \
        do{                \
            print_tk(#A);  \
            return A;      \
        }while(0)

%}

relop >|<|>=|<=|==|!=
ws [ \f\r\t\v\n]

%%
\n   { yycolumn = 1;}
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
. { error_msg(0,yylineno,"Mysterious character \"%c\".",yytext[0]);}
%%

void print_tk(char*s){
#if PRINT_TK
    printf("[%d]: %s\n",yylineno,s);
#endif
}