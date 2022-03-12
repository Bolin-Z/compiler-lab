%{
    #include "lex.yy.c"
    #include "decls.h"
    #include "cst.h"

    void yyerror(const char*msg);
%}

%define parse.error detailed
//%define parse.error custom
%locations

%define api.value.type {struct CST_nt_node *}


%token  TYPE
%token  ID
%token  INT FLOAT
%token  SEMI
%token  COMMA
%token  LC RC
%token  STRUCT RETURN IF WHILE

%nonassoc  LOWER_THAN_ELSE
%nonassoc  ELSE

%right  ASSIGNOP
%left   OR
%left   AND
%left   RELOP
%left   PLUS MINUS
%left   STAR DIV
%right  NOT NEG
%left   DOT LB RB LP RP

%nterm  Program ExtDefList ExtDef Specifier FunDec CompSt VarDec ExtDecList
%nterm  StructSpecifier OptTag DefList Tag
%nterm  VarList ParamDec
%nterm  StmtList Stmt Exp
%nterm  Def DecList Dec
%nterm  Args

%%
/* High-level Definitions */
Program : ExtDefList
    ;
ExtDefList : ExtDef ExtDefList
    | /* empty */
    ;
ExtDef : Specifier ExtDecList SEMI
    | Specifier ExtDecList error
    | Specifier error SEMI
    | Specifier SEMI 
    | error SEMI
    | Specifier FunDec CompSt
    | error FunDec CompSt
    | Specifier error CompSt
    ;
ExtDecList : VarDec
    | VarDec COMMA ExtDecList
    ;

/* Specifiers */
Specifier : TYPE
    | StructSpecifier
    ;
StructSpecifier : STRUCT OptTag LC DefList RC
    | STRUCT Tag
    ;
OptTag : ID
    | /* empty */
    ;
Tag : ID
    ;

/* Declarators */
VarDec : ID
    | VarDec LB INT RB
    | VarDec LB error RB
    | VarDec LB INT error
    ;
FunDec : ID LP VarList RP
    | ID LP RP
    | ID LP error RP
    | ID LP VarList error
    ;
VarList : ParamDec COMMA VarList
    | ParamDec
    ;
ParamDec : Specifier VarDec
    ;

/* Statements */
CompSt : LC DefList StmtList RC
    ;
StmtList : Stmt StmtList
    | /* empty */
    ;
Stmt : Exp SEMI
    | error SEMI
    | Exp error
    | CompSt
    | RETURN Exp SEMI
    | RETURN Exp error
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE
    | IF LP Exp error Stmt %prec LOWER_THAN_ELSE
    | IF LP Exp RP Stmt ELSE Stmt
    | IF LP error RP Stmt ELSE Stmt
    | IF LP Exp RP error ELSE Stmt
    | IF LP Exp error  ELSE Stmt
    | WHILE LP Exp RP Stmt
    | WHILE LP error RP Stmt
    | WHILE LP Exp error Stmt
    ;

/* Local Definitions */
DefList : Def DefList
    | /* empty */
    ;
Def : Specifier DecList SEMI
    | Specifier error SEMI
    | Specifier DecList error
    ;
DecList : Dec
    | Dec COMMA DecList
    ;
Dec : VarDec
    | VarDec ASSIGNOP Exp
    ;

/* Expressions */
Exp : Exp ASSIGNOP Exp
    | Exp ASSIGNOP error
    | Exp AND Exp
    | Exp AND error
    | Exp OR Exp
    | Exp OR error
    | Exp RELOP Exp
    | Exp RELOP error
    | Exp PLUS Exp
    | Exp PLUS error
    | Exp MINUS Exp
    | Exp MINUS error
    | Exp STAR Exp
    | Exp STAR error
    | Exp DIV Exp
    | Exp DIV error
    | LP Exp RP
    | LP error RP
    | LP Exp error
    | MINUS Exp %prec NEG
    | NOT Exp
    | ID LP Args RP
    | ID LP Args error
    | ID LP RP
    | ID LP error RP
    | Exp LB Exp RB
    | Exp LB error RB
    | Exp LB Exp error
    | Exp DOT ID
    | ID
    | INT
    | FLOAT
    ;
Args : Exp COMMA Args
    | Exp
    ;
%%

void yyerror(const char*msg){
    error_msg(1,yylineno,"near \"%s\". %s",yytext,msg);
}
/*
static int yyreport_syntax_error(const yypcontext_t *ctx){
    yyerror("find error");
    return 0;
}
*/