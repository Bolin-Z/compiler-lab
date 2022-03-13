%{
    #include "lex.yy.c"
    #include "decls.h"
    #include "defs.h"
    #include "cst.h"

    void yyerror(const char*msg);
    extern struct CST_node* cst_root;
    extern bool has_error;
%}

%define parse.error detailed
%locations

%define api.value.type {struct CST_node *}


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

%destructor {destory_node($$); $$ = NULL; } TYPE ID INT FLOAT SEMI COMMA LC RC STRUCT RETURN IF WHILE
%destructor {destory_node($$); $$ = NULL; } ELSE ASSIGNOP OR AND RELOP PLUS MINUS STAR DIV NOT NEG DOT LB RB LP RP
%destructor {destory_tree($$); $$ = NULL; } Program ExtDefList ExtDef Specifier FunDec CompSt VarDec ExtDecList
%destructor {destory_tree($$); $$ = NULL; } StructSpecifier OptTag DefList Tag
%destructor {destory_tree($$); $$ = NULL; } VarList ParamDec StmtList Stmt Exp
%destructor {destory_tree($$); $$ = NULL; } Def DecList Dec Args

%%
/* High-level Definitions */
Program : ExtDefList { $$ = creat_node(SYM(Program),NT_NODE,@$.first_line,NULL); add_child($$,1,$1); cst_root = $$;}
    ;
ExtDefList : ExtDef ExtDefList { $$ = creat_node(SYM(ExtDefList),NT_NODE,@$.first_line,NULL); add_child($$,2,$1,$2);}
    | /* empty */ { $$ = creat_node(SYM(ExtDefList),NT_NODE,@$.first_line,NULL); }
    ;
ExtDef : Specifier ExtDecList SEMI { $$ = creat_node(SYM(ExtDef),NT_NODE,@$.first_line,NULL); add_child($$,3,$1,$2,$3);}
    | Specifier SEMI { $$ = creat_node(SYM(ExtDef),NT_NODE,@$.first_line,NULL); add_child($$,2,$1,$2);}
    | Specifier FunDec CompSt { $$ = creat_node(SYM(ExtDef),NT_NODE,@$.first_line,NULL); add_child($$,3,$1,$2,$3);}
    /* error recovery*/
    | Specifier ExtDecList error { $$ = NULL; destory_tree($1); destory_tree($2);}
    | Specifier error SEMI { $$ = NULL; destory_tree($1); destory_node($3);}
    | error SEMI { $$ = NULL; destory_node($2); }
    | error FunDec CompSt { $$ = NULL; destory_tree($2); destory_tree($3);}
    | Specifier error CompSt { $$ = NULL; destory_tree($1); destory_tree($3);}
    ;
ExtDecList : VarDec { $$ = creat_node(SYM(ExtDecList),NT_NODE,@$.first_line,NULL); add_child($$,1,$1);}
    | VarDec COMMA ExtDecList { $$ = creat_node(SYM(ExtDecList),NT_NODE,@$.first_line,NULL); add_child($$,3,$1,$2,$3);}
    ;

/* Specifiers */
Specifier : TYPE { $$ = creat_node(SYM(Specifier),NT_NODE,@$.first_line,NULL); add_child($$,1,$1);}
    | StructSpecifier { $$ = creat_node(SYM(Specifier),NT_NODE,@$.first_line,NULL); add_child($$,1,$1);}
    ;
StructSpecifier : STRUCT OptTag LC DefList RC { $$ = creat_node(SYM(StructSpecifier),NT_NODE,@$.first_line,NULL); add_child($$,5,$1,$2,$3,$4,$5);}
    | STRUCT Tag { $$ = creat_node(SYM(StructSpecifier),NT_NODE,@$.first_line,NULL); add_child($$,2,$1,$2);}
    ;
OptTag : ID { $$ = creat_node(SYM(OptTag),NT_NODE,@$.first_line,NULL); add_child($$,1,$1);}
    | /* empty */ { $$ = creat_node(SYM(OptTag),NT_NODE,@$.first_line,NULL);}
    ;
Tag : ID { $$ = creat_node(SYM(Tag),NT_NODE,@$.first_line,NULL); add_child($$,1,$1);}
    ;

/* Declarators */
VarDec : ID { $$ = creat_node(SYM(VarDec),NT_NODE,@$.first_line,NULL); add_child($$,1,$1);}
    | VarDec LB INT RB { $$ = creat_node(SYM(VarDec),NT_NODE,@$.first_line,NULL); add_child($$,4,$1,$2,$3,$4);}
    /* error recovery */
    | VarDec LB error RB { $$ = NULL; destory_tree($1); destory_node($2); destory_node($4);}
    | VarDec LB INT error { $$ = NULL; destory_tree($1); destory_node($2); destory_node($3);}
    ;
FunDec : ID LP VarList RP { $$ = creat_node(SYM(FunDec),NT_NODE,@$.first_line,NULL); add_child($$,4,$1,$2,$3,$4);}
    | ID LP RP { $$ = creat_node(SYM(FunDec),NT_NODE,@$.first_line,NULL); add_child($$,3,$1,$2,$3);}
    /* error recovery */
    | ID LP error RP { $$ = NULL; destory_node($1); destory_node($2); destory_node($4);}
    | ID LP VarList error { $$ = NULL; destory_node($1); destory_node($2); destory_tree($3);}
    ;
VarList : ParamDec COMMA VarList { $$ = creat_node(SYM(VarList),NT_NODE,@$.first_line,NULL); add_child($$,3,$1,$2,$3);}
    | ParamDec { $$ = creat_node(SYM(VarList),NT_NODE,@$.first_line,NULL); add_child($$,1,$1);}
    ;
ParamDec : Specifier VarDec { $$ = creat_node(SYM(ParamDec),NT_NODE,@$.first_line,NULL); add_child($$,2,$1,$2);}
    ;

/* Statements */
CompSt : LC DefList StmtList RC { $$ = creat_node(SYM(CompSt),NT_NODE,@$.first_line,NULL); add_child($$,4,$1,$2,$3,$4);}
    ;
StmtList : Stmt StmtList { $$ = creat_node(SYM(StmtList),NT_NODE,@$.first_line,NULL); add_child($$,2,$1,$2);}
    | /* empty */ { $$ = creat_node(SYM(StmtList),NT_NODE,@$.first_line,NULL);}
    ;
Stmt : Exp SEMI { $$ = creat_node(SYM(Stmt),NT_NODE,@$.first_line,NULL); add_child($$,2,$1,$2);} 
    | CompSt { $$ = creat_node(SYM(Stmt),NT_NODE,@$.first_line,NULL); add_child($$,1,$1);}
    | RETURN Exp SEMI { $$ = creat_node(SYM(Stmt),NT_NODE,@$.first_line,NULL); add_child($$,3,$1,$2,$3);}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { $$ = creat_node(SYM(Stmt),NT_NODE,@$.first_line,NULL); add_child($$,5,$1,$2,$3,$4,$5);}
    | IF LP Exp RP Stmt ELSE Stmt { $$ = creat_node(SYM(Stmt),NT_NODE,@$.first_line,NULL); add_child($$,7,$1,$2,$3,$4,$5,$6,$7);}
    | WHILE LP Exp RP Stmt { $$ = creat_node(SYM(Stmt),NT_NODE,@$.first_line,NULL); add_child($$,5,$1,$2,$3,$4,$5);}
    /* error recovery */
    | error SEMI { $$ = NULL; destory_node($2);}
    | Exp error { $$ = NULL; destory_tree($1);}
    | RETURN Exp error { $$ = NULL; destory_node($1); destory_tree($2);}
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE { $$ = NULL; destory_node($1); destory_node($2); destory_node($4); destory_tree($5);}
    | IF LP Exp error Stmt %prec LOWER_THAN_ELSE { $$ = NULL; destory_node($1); destory_node($2); destory_tree($3); destory_tree($5);}
    | IF LP error RP Stmt ELSE Stmt { $$ = NULL; destory_node($1); destory_node($2); destory_node($4); destory_tree($5); destory_node($6); destory_tree($7);}
    | IF LP Exp RP error ELSE Stmt { $$ = NULL; destory_node($1); destory_node($2); destory_tree($3); destory_node($4); destory_node($6); destory_tree($7);}
    | IF LP Exp error ELSE Stmt { $$ = NULL; destory_node($1); destory_node($2); destory_tree($3); destory_node($5); destory_tree($6);}
    | WHILE LP error RP Stmt { $$ = NULL; destory_node($1); destory_node($2); destory_node($4); destory_tree($5);}
    | WHILE LP Exp error Stmt { $$ = NULL; destory_node($1); destory_node($2); destory_tree($3); destory_tree($5);}
    ;

/* Local Definitions */
DefList : Def DefList { $$ = creat_node(SYM(DefList),NT_NODE,@$.first_line,NULL); add_child($$,2,$1,$2);}
    | /* empty */ { $$ = creat_node(SYM(DefList),NT_NODE,@$.first_line,NULL);}
    ;
Def : Specifier DecList SEMI { $$ = creat_node(SYM(Def),NT_NODE,@$.first_line,NULL); add_child($$,3,$1,$2,$3);}
    /* error recovery */
    | Specifier error SEMI { $$ = NULL; destory_tree($1); destory_node($3);}
    | Specifier DecList error { $$ = NULL; destory_tree($1); destory_tree($2);}
    ;
DecList : Dec { $$ = creat_node(SYM(DecList),NT_NODE,@$.first_line,NULL); add_child($$,1,$1);}
    | Dec COMMA DecList { $$ = creat_node(SYM(DecList),NT_NODE,@$.first_line,NULL); add_child($$,3,$1,$2,$3);}
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
    has_error = true;
    error_msg('B',yylineno,msg);
}