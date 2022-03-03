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
//    | error ExtDefList { fprintf(stderr,"ExtDefList\n");}
    ;
ExtDef : Specifier ExtDecList SEMI
    | Specifier SEMI 
    | Specifier FunDec CompSt
//    | error ExtDecList SEMI { fprintf(stderr,"Line[%d]: specifier error\n",yylineno);}
//    | error FunDec CompSt
//    | error SEMI
//    | error { fprintf(stderr,"Line[%d]: stmt error\n",yylineno);}
    ;
ExtDecList : VarDec
    | VarDec COMMA ExtDecList
 //   | error COMMA ExtDecList { fprintf(stderr,"ExtDecList\n");}
    ;

/* Specifiers */
Specifier : TYPE
    | StructSpecifier
//    | error { fprintf(stderr,"Line[%d]: specifier error\n",yylineno);}
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
//    | VarDec error RB { fprintf(stderr,"Line[%d]: RB error\n",yylineno);}
    ;
FunDec : ID LP VarList RP
    | ID LP RP
//    | error RP
    ;
VarList : ParamDec COMMA VarList
    | ParamDec
//   | error COMMA VarList { fprintf(stderr,"VarList\n");}
    ;
ParamDec : Specifier VarDec
    ;

/* Statements */
CompSt : LC DefList StmtList RC
//    | error RC
    ;
StmtList : Stmt StmtList
    | /* empty */
//    | error StmtList { fprintf(stderr,"StmList\n");}
    ;
Stmt : Exp SEMI
    | CompSt
    | RETURN Exp SEMI
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
    | IF LP Exp RP Stmt ELSE Stmt
    | WHILE LP Exp RP Stmt
//    | error SEMI { fprintf(stderr,"Line[%d]: stmt error\n",yylineno);}
    ;

/* Local Definitions */
DefList : Def DefList
    | /* empty */
//    | error DefList { fprintf(stderr,"DefList\n");}
    ;
Def : Specifier DecList SEMI
    ;
DecList : Dec
    | Dec COMMA DecList
//    | error COMMA DecList { fprintf(stderr,"Wrong DecList\n");}
    ;
Dec : VarDec
    | VarDec ASSIGNOP Exp
    ;

/* Expressions */
Exp : Exp ASSIGNOP Exp
    | Exp AND Exp
    | Exp OR Exp
    | Exp RELOP Exp
    | Exp PLUS Exp
    | Exp MINUS Exp
    | Exp STAR Exp
    | Exp DIV Exp
    | LP Exp RP
    | MINUS Exp %prec NEG
    | NOT Exp
    | ID LP Args RP
    | ID LP RP
    | Exp LB Exp RB
    | Exp DOT ID
    | ID
    | INT
    | FLOAT
//    | error { fprintf(stderr, "Line[%d]: Experr\n",yylineno);}
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