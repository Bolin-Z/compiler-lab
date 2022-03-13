%{
    #define PRINT_TK false

    #include "syntax.tab.h"
    #include "decls.h"
    #include <stdio.h>
    #include "cst.h"
    
    extern struct CST_node * creat_node(int sym_type,int node_type,int lineno,const char* lexme);
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

ws [ \f\r\t\v\n]
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
TYPE int|float
COMMENT "//"
LCOMMENT \/\*
RCOMMENT \*\/
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
FLOAT [0-9]+\.[0-9]+
BASE \.[0-9]+|[0-9]+\.|[0-9]+\.[0-9]+
EXPONENT {BASE}[Ee][+-]?[0-9]+
INT 0|[1-9][0-9]*
OCT 0[0-7]*
HEX 0[X|x][0-9a-fA-F]+
ID [a-zA-Z_][a-zA-Z0-9_]*

%%
\n   { yycolumn = 1;}
{ws} {/* empty */}
{STRUCT}     {creat_node(STRUCT, UNIQ_NODE, yylineno, NULL); return STRUCT;}
{RETURN}     {creat_node(RETURN, UNIQ_NODE, yylineno, NULL); return RETURN;}
{IF}         {creat_node(IF, UNIQ_NODE, yylineno, NULL); return IF;}
{ELSE}       {creat_node(ELSE, UNIQ_NODE, yylineno, NULL); return ELSE;}
{WHILE}      {creat_node(WHILE, UNIQ_NODE, yylineno, NULL); return WHILE;}
{TYPE}       {creat_node(TYPE, UNIQ_NODE, yylineno, NULL); return TYPE;}
{EXPONENT}   { creat_node(FLOAT, FLOAT_NODE, yylineno, yytext); 
		printf("EXPONENT:%s\n",yytext);
		return FLOAT;}
{HEX}        { creat_node(INT, INT_NODE, yylineno, yytext);
		printf("HEX:%s\n",yytext); 
		return INT;}
{OCT}        { creat_node(INT, INT_NODE, yylineno, yytext);
		printf("OCT:%s\n",yytext); 
		return INT;}
{FLOAT}      { creat_node(FLOAT, FLOAT_NODE, yylineno, yytext);
		printf("FLOAT:%s\n",yytext);
		return FLOAT;}
{INT}        { creat_node(INT, INT_NODE, yylineno, yytext);
		printf("Integer value %d\n", atoi(yytext)); RETURN(INT);}
{ID}         {creat_node(ID, ID_NODE, yylineno, yytext); return ID;}
{SEMI}       {creat_node(SEMI, UNIQ_NODE, yylineno, NULL); return SEMI;}
{COMMA}      {creat_node(COMMA, UNIQ_NODE, yylineno, NULL); return COMMA;}
{ASSIGNOP}   {creat_node(ASSIGNOP, UNIQ_NODE, yylineno, NULL); return ASSIGNOP;}
{RELOP}      {creat_node(RELOP, MUL_NODE, yylineno, NULL); return RELOP;}
{PLUS}       {creat_node(PLUS, UNIQ_NODE, yylineno, NULL); return PLUS;}
{MINUS}      {creat_node(MINUS, UNIQ_NODE, yylineno, NULL); return MINUS;}
{STAR}       {creat_node(STAR, UNIQ_NODE, yylineno, NULL); return STAR;}
{DIV}        {creat_node(DIV, UNIQ_NODE, yylineno, NULL); return DIV;}
{AND}        {creat_node(AND, UNIQ_NODE, yylineno, NULL); return AND;}
{OR}         {creat_node(OR, UNIQ_NODE, yylineno, NULL); return OR;}
{DOT}        {creat_node(DOT, UNIQ_NODE, yylineno, NULL); return DOT;}
{NOT}        {creat_node(NOT, UNIQ_NODE, yylineno, NULL); return NOT;}
{LP}         {creat_node(LP, UNIQ_NODE, yylineno, NULL); return LP;}
{RP}         {creat_node(RP, UNIQ_NODE, yylineno, NULL); return RP;}
{LB}         {creat_node(LB, UNIQ_NODE, yylineno, NULL); return LB;}
{RB}         {creat_node(RB, UNIQ_NODE, yylineno, NULL); return RB;}
{LC}         {creat_node(LC, UNIQ_NODE, yylineno, NULL); return LC;}
{RC}         {creat_node(RC, UNIQ_NODE, yylineno, NULL); return RC;}
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
