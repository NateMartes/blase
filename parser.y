%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <gc/gc.h>

  #include "ast.h"
  #define YYMALLOC GC_MALLOC

  extern int yylex();
  extern int yylineno;
  void yyerror(program **prog, char const *s);
%}

%define parse.error verbose
%parse-param { program **prog }
%expect 1

%union {
  expr *expr;
  stmt *stmt;
  list *list;
  type *type;
  type_decl *type_decl;
  uop uop;
  bop bop;
  char *string;
  long constant;
  void *none;
}

%token ASSIGN ":="
%token WRITE "write"
%token WRITELN "writeln"
%token DIV "div"
%token MOD "mod"
%token <string> IDENTIFIER
%token <constant> INT_CONST
%token AND "and"
%token OR "or"
%token NOT "not"
%token TRUE "true"
%token FALSE "false"
%token LE "<="
%token GE ">="
%token NE "<>"
%token '(' ')' ';' '+' '-' '*' '=' '<' '>'
%token WHILE "while"
%token DO "do"
%token IF "if"
%token THEN "then"
%token ELSE "else"
%token BEEGIN "begin"
%token END "end"
%token PROGRAM "program"
%token INTEGER "integer"
%token BOOLEAN "boolean"
%token VAR "var"

%type <none> program
%type <expr> expr simple_expr term factor rel_expr or_expr and_expr
%type <stmt> stmt assign_stmt write_stmt writeln_stmt if_stmt while_stmt cmpd_stmt
%type <type_decl> type_decl
%type <type> type
%type <list> stmt_list type_decls type_decl_list id_list
%type <uop> un_op
%type <bop> add_op mult_op
%%

program :
  "program" IDENTIFIER ';' type_decls cmpd_stmt '.' { *prog = program_new($2, $4, $5); }
;
   
type_decls :
  "var" type_decl_list { $$ = $2; }
| empty { $$ = NULL; }
;

type_decl_list :
  type_decl type_decl_list { $$ = cons($1, $2); }
| type_decl                { $$ = cons($1, NULL); }
;

type_decl :
  id_list ':' type ';' { $$ = type_decl_new($1, $3); }
;

id_list :
  IDENTIFIER ',' id_list { $$ = cons($1, $3); }
| IDENTIFIER             { $$ = cons($1, NULL); }
;

type :
  INTEGER { $$ = type_new(T_INTEGER); }
| BOOLEAN { $$ = type_new(T_BOOLEAN); }
;

stmt_list :
  stmt ';' stmt_list { $$ = cons($1, $3); }
| stmt { $$ = cons($1, NULL); }
;

stmt :
  assign_stmt
| write_stmt
| writeln_stmt
| if_stmt
| while_stmt
| cmpd_stmt
;

assign_stmt :
  IDENTIFIER ":=" expr { $$ = assign_stmt_new($1, $3); }
;

write_stmt :
  "write" '(' expr ')' { $$ = write_stmt_new($3); }
;

writeln_stmt :
  "writeln" '(' expr ')' { $$ = writeln_stmt_new($3); }
| "writeln" '(' ')' { $$ = writeln_stmt_new(NULL); }
;

if_stmt :
  "if" or_expr "then" stmt { $$ = if_stmt_new($2, $4, NULL); }
| "if" or_expr "then" stmt "else" stmt { $$ = if_stmt_new($2, $4, $6); }
;

while_stmt :
  "while" or_expr "do" stmt { $$ = while_stmt_new($2, $4); }
;

cmpd_stmt :
  "begin" stmt_list "end" { $$ = cmpd_stmt_new($2); }
;

expr :
  or_expr
;

rel_expr :
  simple_expr '<' simple_expr { $$ = binary_expr_new(B_LT, $1, $3); }
| simple_expr '>' simple_expr { $$ = binary_expr_new(B_GT, $1, $3); }
| simple_expr "<=" simple_expr { $$ = binary_expr_new(B_LE, $1, $3); }
| simple_expr ">=" simple_expr { $$ = binary_expr_new(B_GE, $1, $3); }
| simple_expr '=' simple_expr { $$ = binary_expr_new(B_EQ, $1, $3); }
| simple_expr "<>" simple_expr { $$ = binary_expr_new(B_NE, $1, $3); }
| simple_expr
;                
                
and_expr:
  and_expr "and" rel_expr { $$ = binary_expr_new(B_AND, $1, $3); }
| rel_expr
;

or_expr :
  or_expr "or" and_expr { $$ = binary_expr_new(B_OR, $1, $3); }
| and_expr
;

simple_expr :
  simple_expr add_op term { $$ = binary_expr_new($2, $1, $3); }
| term
;

add_op :
  '+'      { $$ = B_ADD; }
| '-'      { $$ = B_SUB; }
;

term :
  term mult_op factor { $$ = binary_expr_new($2, $1, $3); }
| factor
;

mult_op :
   '*'     { $$ = B_MUL; }
| "div"    { $$ = B_DIV; }
| "mod"    { $$ = B_MOD; }
;

un_op :
  '+'      { $$ = U_PLUS; }
| '-'      { $$ = U_MINUS; }
| "not"    { $$ = U_NOT; }
;

factor :
  IDENTIFIER   { $$ = var_expr_new($1); }
| INT_CONST     { $$ = int_const_expr_new($1); }
| TRUE         { $$ = bool_const_expr_new(BL_TRUE); }
| FALSE        { $$ = bool_const_expr_new(BL_FALSE); }
| un_op factor { $$ = unary_expr_new($1, $2); }
| '(' expr ')' { $$ = $2; }
;  

empty :
;

%%

void yyerror(program **prog, char const *s) {
  fprintf(stderr, "%s at line %d\n", s, yylineno);
  exit(1);
}
