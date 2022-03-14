%{
    #define PRINT_TK false
    #define YYSTYPE struct CST_node *
    
    #include "syntax.tab.h"
    #include "decls.h"
    #include <stdio.h>
    #include "cst.h"
    
    //extern struct CST_node * creat_node(int sym_type,int node_type,int lineno,const char* lexme);
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
{STRUCT}     {yylval = creat_node(SYM(STRUCT), UNIQ_NODE, yylineno, NULL); return STRUCT;}
{RETURN}     {yylval = creat_node(SYM(RETURN), UNIQ_NODE, yylineno, NULL); return RETURN;}
{IF}         {yylval = creat_node(SYM(IF), UNIQ_NODE, yylineno, NULL); return IF;}
{ELSE}       {yylval = creat_node(SYM(ELSE), UNIQ_NODE, yylineno, NULL); return ELSE;}
{WHILE}      {yylval = creat_node(SYM(WHILE), UNIQ_NODE, yylineno, NULL); return WHILE;}
{TYPE}       {yylval = creat_node(SYM(TYPE), UNIQ_NODE, yylineno, NULL); return TYPE;}
{EXPONENT}   { yylval = creat_node(SYM(FLOAT), FLOAT_NODE, yylineno, yytext); 
		//printf("EXPONENT:%s\n",yytext);
		return FLOAT;}
{HEX}        { yylval = creat_node(SYM(INT), INT_NODE, yylineno, yytext);
		//printf("HEX:%s\n",yytext); 
		return INT;}
{OCT}        { yylval = creat_node(SYM(INT), INT_NODE, yylineno, yytext);
		//printf("OCT:%s\n",yytext); 
		return INT;}
{FLOAT}      { yylval = creat_node(SYM(FLOAT), FLOAT_NODE, yylineno, yytext);
		//printf("FLOAT:%s\n",yytext);
		return FLOAT;}
{INT}        { yylval = creat_node(SYM(INT), INT_NODE, yylineno, yytext);
		//printf("Integer value %d\n", atoi(yytext)); 
		return INT;}
{ID}         {yylval = creat_node(SYM(ID), ID_NODE, yylineno, yytext); return ID;}
{SEMI}       {yylval = creat_node(SYM(SEMI), UNIQ_NODE, yylineno, NULL); return SEMI;}
{COMMA}      {yylval = creat_node(SYM(COMMA), UNIQ_NODE, yylineno, NULL); return COMMA;}
{ASSIGNOP}   {yylval = creat_node(SYM(ASSIGNOP), UNIQ_NODE, yylineno, NULL); return ASSIGNOP;}
{RELOP}      {yylval = creat_node(SYM(RELOP), MUL_NODE, yylineno, NULL); return RELOP;}
{PLUS}       {yylval = creat_node(SYM(PLUS), UNIQ_NODE, yylineno, NULL); return PLUS;}
{MINUS}      {yylval = creat_node(SYM(MINUS), UNIQ_NODE, yylineno, NULL); return MINUS;}
{STAR}       {yylval = creat_node(SYM(STAR), UNIQ_NODE, yylineno, NULL); return STAR;}
{DIV}        {yylval = creat_node(SYM(DIV), UNIQ_NODE, yylineno, NULL); return DIV;}
{AND}        {yylval = creat_node(SYM(AND), UNIQ_NODE, yylineno, NULL); return AND;}
{OR}         {yylval = creat_node(SYM(OR), UNIQ_NODE, yylineno, NULL); return OR;}
{DOT}        {yylval = creat_node(SYM(DOT), UNIQ_NODE, yylineno, NULL); return DOT;}
{NOT}        {yylval = creat_node(SYM(NOT), UNIQ_NODE, yylineno, NULL); return NOT;}
{LP}         {yylval = creat_node(SYM(LP), UNIQ_NODE, yylineno, NULL); return LP;}
{RP}         {yylval = creat_node(SYM(RP), UNIQ_NODE, yylineno, NULL); return RP;}
{LB}         {yylval = creat_node(SYM(LB), UNIQ_NODE, yylineno, NULL); return LB;}
{RB}         {yylval = creat_node(SYM(RB), UNIQ_NODE, yylineno, NULL); return RB;}
{LC}         {yylval = creat_node(SYM(LC), UNIQ_NODE, yylineno, NULL); return LC;}
{RC}         {yylval = creat_node(SYM(RC), UNIQ_NODE, yylineno, NULL); return RC;}
{COMMENT}    {
		char c = input(); 
		while(c!='\n'){c=input();}
		//printf("COMMETN\n");
              }
{LCOMMENT}   {
		//printf("LCOMMENT\n");
		
		char last, cur;
		int init = 1, flag = 0;
		last = input();
		cur = input();
		while(cur!='\0'){
		    if(init==0) {
		        //printf("Bad Match!\n");
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
		else error_msg('A',yylineno,"Didn't match /* !");
	      }

{RCOMMENT}   { error_msg('A',yylineno,yytext);}
<<EOF>>      {yyterminate();}
.            { error_msg('A',yylineno,yytext);}
%%
void print_tk(char*s){
#if PRINT_TK
    printf("[%d]: %s\n",yylineno,s);
#endif
}
