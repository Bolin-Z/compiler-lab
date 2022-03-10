%{
    #define PRINT_TK true

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
%option yylineno
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
TYPE int|float
COMMENT "//"
LCOMMENT "/*"
RCOMMENT "*/"
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS \+
MINUS -
STAR \*
DIV \/
AND &&
OR \|\|
DOT \.
NOT !
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
FLOAT [0-9]+.[0-9]+
BASE .[0-9]+|[0-9]+.|[0-9]+.[0-9]+
EXPONENT {BASE}[Ee][+-]?[0-9]+
INT 0|[1-9][0-9]*
OCT 0[0-7]*
HEX 0[X|x][0-9a-fA-F]+
ID [a-zA-Z_][a-zA-Z0-9_]*

%%
\n   { yycolumn = 1;}
{ws} {/* empty */}
{STRUCT}     {RETURN(STRUCT);}
{RETURN}     {RETURN(RETURN);}
{IF}         {RETURN(IF);}
{ELSE}       {RETURN(ELSE);}
{WHILE}      {RETURN(WHILE);}
{TYPE}       {RETURN(TYPE);}
{EXPONENT}   {printf("EXPONENT:%s\n",yytext); RETURN(FLOAT);}
{HEX}        {printf("HEX:%s\n",yytext); RETURN(INT);}
{OCT}        {printf("OCT:%s\n",yytext); RETURN(INT);}
{FLOAT}      {RETURN(FLOAT);}
{INT}        {printf("Integer value %d\n", atoi(yytext)); RETURN(INT);}
{ID}         {RETURN(ID);}
{SEMI}       {RETURN(SEMI);}
{COMMA}      {RETURN(COMMA);}
{ASSIGNOP}   {RETURN(ASSIGNOP);}
{RELOP}      {RETURN(RELOP);}
{PLUS}       {RETURN(PLUS);}
{MINUS}      {RETURN(MINUS);}
{STAR}       {RETURN(STAR);}
{DIV}        {RETURN(DIV);}
{AND}        {RETURN(AND);}
{OR}         {RETURN(OR);}
{DOT}        {RETURN(DOT);}
{NOT}        {RETURN(NOT);}
{LP}         {RETURN(LP);}
{RP}         {RETURN(RP);}
{LB}         {RETURN(LB);}
{RB}         {RETURN(RB);}
{LC}         {RETURN(LC);}
{RC}         {RETURN(RC);}
{COMMENT}    {
		char c = input(); 
		while(c!='\n'){c=input();}
		printf("COMMETN\n");
              }
{LCOMMENT}   {
		printf("LCOMMENT\n");
		
		char last, cur;
		int init = 1, flag = 0;
		last = input();
		cur = input();
		while(cur!='\0'){
		    if(init==0) {
		        printf("Bad Match!\n");
		        break;
		    }else{
		        if(last=='*' && cur=='/'){
		            flag = 1;
		            break;
		        }else if(last=='/' && cur=='*'){
		            break;
		        }else if(last=='/' && cur=='/'){
		            break;
		        }
		        last = cur;
		        cur = input();
		    }
		}
		if(flag) printf("Match/* */!\n");
		else error_msg(0,yylineno,"Didn't match /* !");
	      }

{RCOMMENT}   { error_msg(0,yylineno,yytext);}
<<EOF>>      {yyterminate();}
.            { error_msg(0,yylineno,yytext);}
%%
void print_tk(char*s){
#if PRINT_TK
    printf("[%d]: %s\n",yylineno,s);
#endif
}
