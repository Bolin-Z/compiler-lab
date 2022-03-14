%{  
    #include "syntax.tab.h"
    #include "decls.h"
	#include "defs.h"
    #include <stdio.h>
    #include "cst.h"
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
	#define page_size 1024
	typedef struct c_stack{
		bool empty;
		size_t cur_page;
		size_t cur_pos;
		char ** page_list;
	}c_stack;

	c_stack * creat_stack();
	void free_stack(c_stack* s);
	void push_c(c_stack* s,char c);
	char pop_c(c_stack* s);
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
ERRORINT [0-9]+[a-wA-Wy-zY-Z]+[0-9a-dA-Df-zF-Z]*|[0-9]+[0-9]+[a-dA-Df-zF-Z]+[0-9]*[0-9a-dA-Df-zF-Z]*
HEX 0[X|x][0-9a-fA-F]+
ERRORHEX 0[X|x][0-9a-fA-F]*[g-zG-Z]+[0-9a-zA-Z]*
OCT 0[0-7]*
ERROROCT 0[0-7]*[8-9]+[0-9]*
ID [a-zA-Z_][a-zA-Z0-9_]*

%%
\n   { yycolumn = 1;}
{ws} {/* empty */}
{STRUCT}     {yylval = creat_node(SYM(STRUCT),   UNIQ_NODE,  yylineno, NULL);   return STRUCT;}
{RETURN}     {yylval = creat_node(SYM(RETURN),   UNIQ_NODE,  yylineno, NULL);   return RETURN;}
{IF}         {yylval = creat_node(SYM(IF),       UNIQ_NODE,  yylineno, NULL);   return IF;}
{ELSE}       {yylval = creat_node(SYM(ELSE),     UNIQ_NODE,  yylineno, NULL);   return ELSE;}
{WHILE}      {yylval = creat_node(SYM(WHILE),    UNIQ_NODE,  yylineno, NULL);   return WHILE;}
{TYPE}       {yylval = creat_node(SYM(TYPE),     MUL_NODE,   yylineno, yytext); return TYPE;}
{EXPONENT}   {yylval = creat_node(SYM(FLOAT),    FLOAT_NODE, yylineno, yytext); return FLOAT;}
{HEX}        {yylval = creat_node(SYM(INT),      INT_NODE,   yylineno, yytext); return INT;}
{ERRORHEX}   {error_msg('A',yylineno,"Illegal hexadecimal number \'%s\'", yytext); yylval = 0; return INT;} 
{OCT}        {yylval = creat_node(SYM(INT),      INT_NODE,   yylineno, yytext); return INT;}
{ERROROCT}   {error_msg('A',yylineno,"Illegal octal number \'%s\'", yytext); yylval = 0; return INT;}
{FLOAT}      {yylval = creat_node(SYM(FLOAT),    FLOAT_NODE, yylineno, yytext); return FLOAT;}
{INT}        {yylval = creat_node(SYM(INT),      INT_NODE,   yylineno, yytext); return INT;}
{ERRORINT}   {error_msg('A',yylineno,"Illegal number \'%s\'", yytext); yylval = 0; return INT;}
{ID}         {yylval = creat_node(SYM(ID),       ID_NODE,    yylineno, yytext); return ID;}
{SEMI}       {yylval = creat_node(SYM(SEMI),     UNIQ_NODE,  yylineno, NULL);   return SEMI;}
{COMMA}      {yylval = creat_node(SYM(COMMA),    UNIQ_NODE,  yylineno, NULL);   return COMMA;}
{ASSIGNOP}   {yylval = creat_node(SYM(ASSIGNOP), UNIQ_NODE,  yylineno, NULL);   return ASSIGNOP;}
{RELOP}      {yylval = creat_node(SYM(RELOP),    MUL_NODE,   yylineno, yytext); return RELOP;}
{PLUS}       {yylval = creat_node(SYM(PLUS),     UNIQ_NODE,  yylineno, NULL);   return PLUS;}
{MINUS}      {yylval = creat_node(SYM(MINUS),    UNIQ_NODE,  yylineno, NULL);   return MINUS;}
{STAR}       {yylval = creat_node(SYM(STAR),     UNIQ_NODE,  yylineno, NULL);   return STAR;}
{DIV}        {yylval = creat_node(SYM(DIV),      UNIQ_NODE,  yylineno, NULL);   return DIV;}
{AND}        {yylval = creat_node(SYM(AND),      UNIQ_NODE,  yylineno, NULL);   return AND;}
{OR}         {yylval = creat_node(SYM(OR),       UNIQ_NODE,  yylineno, NULL);   return OR;}
{DOT}        {yylval = creat_node(SYM(DOT),      UNIQ_NODE,  yylineno, NULL);   return DOT;}
{NOT}        {yylval = creat_node(SYM(NOT),      UNIQ_NODE,  yylineno, NULL);   return NOT;}
{LP}         {yylval = creat_node(SYM(LP),       UNIQ_NODE,  yylineno, NULL);   return LP;}
{RP}         {yylval = creat_node(SYM(RP),       UNIQ_NODE,  yylineno, NULL);   return RP;}
{LB}         {yylval = creat_node(SYM(LB),       UNIQ_NODE,  yylineno, NULL);   return LB;}
{RB}         {yylval = creat_node(SYM(RB),       UNIQ_NODE,  yylineno, NULL);   return RB;}
{LC}         {yylval = creat_node(SYM(LC),       UNIQ_NODE,  yylineno, NULL);   return LC;}
{RC}         {yylval = creat_node(SYM(RC),       UNIQ_NODE,  yylineno, NULL);   return RC;}
{COMMENT}    {
				char c;
				while(true){
					c = input();
					yycolumn++;
					if(c == '\n'){
						yycolumn = 1;
						break;
					}else if(c == EOF){
						unput(c);
						break;
					}
				}
			 }
{LCOMMENT}   {
				int last_column = yycolumn;
				int last_line = yylineno;
				bool match = false;
				char last = input();
				char cur = input();
				if(last == '\0' || cur == '\0'){
					error_msg('A',last_line,"Unmatched \"/*\".");
					unput(cur); unput(last);
					yycolumn = last_column;
				}else{
					yycolumn += 2;
					c_stack * buf = creat_stack();
					push_c(buf,last); push_c(buf,cur);
					while(true){
						if(last == '*' && cur == '/'){
							match = true;
							break;
						}
						last = cur; cur = input(); yycolumn++;
						if(cur == '\n') yycolumn = 1;
						else if(cur == '\0'){
							while(buf->empty == false){	
								unput(pop_c(buf));
							}
							yycolumn = last_column;
							break;
						}
						push_c(buf,cur);
					}
					if(!match){
						error_msg('A',last_line,"Unmatched \"/*\".");
					}
					free_stack(buf);
				}
	      	}

{RCOMMENT}   { error_msg('A',yylineno,"Unmatched \"*/\".");}
<<EOF>>      { yyterminate();}
.            { error_msg('A',yylineno,"Mysterious character \"%s\".",yytext);}
%%

c_stack * creat_stack(){
	c_stack * p = (c_stack*)malloc(sizeof(c_stack));
	if(p){
		p->cur_pos = 0;
		p->cur_page = 0;
		p->page_list = (char**)malloc(sizeof(char*));
		p->page_list[p->cur_page] = (char*)malloc(page_size*sizeof(char));
		p->empty = true;
	}
	return p;
}

void free_stack(c_stack* s){
	size_t p = 0;
	for(;p <= s->cur_page;p++)
		free(s->page_list[p]);
	free(s->page_list);
	free(s);
}

void push_c(c_stack* s,char c){
	if(s->cur_pos == page_size){
		char ** new_list = (char**)malloc((s->cur_page+2)*sizeof(char*));
		size_t p = 0;
		for(;p <= s->cur_page;p++)
			new_list[p] = s->page_list[p];
		new_list[p] = (char*)malloc(page_size*sizeof(char));
		free(s->page_list);
		s->page_list = new_list;
		s->cur_page += 1;
		s->cur_pos = 0;
	}
	s->page_list[s->cur_page][s->cur_pos] = c;
	s->cur_pos += 1;
	if(s->empty) s->empty = false;
}

char pop_c(c_stack* s){
	char c = 0;
	if(!s->empty){
		if(s->cur_pos == 0){
			char** new_list = (char**) malloc((s->cur_page)*sizeof(char*));
			size_t p = 0;
			for(;p < s->cur_page;p++)
				new_list[p] = s->page_list[p];
			free(s->page_list[s->cur_page]);
			free(s->page_list);
			s->page_list = new_list;
			s->cur_page -= 1;
			s->cur_pos = page_size;
		}
		s->cur_pos -= 1;
		c = s->page_list[s->cur_page][s->cur_pos];
		if(s->cur_page == 0 && s->cur_pos == 0)
			s->empty = true;
	}
	return c;
}
