#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <gc/gc.h>
#include <assert.h>
#include "pool.h"
#include "ast.h"

char *enames[] = {
#define X(Y) #Y,
  E_TAGS
#undef X
};

char *unames[] = {
#define X(Y) #Y,
  UOPS
#undef X
};

char *bnames[] = {
#define X(Y) #Y,
  BOPS
#undef X
};

char *blnames[] = {
#define X(Y) #Y,
  BL_VALS
#undef X
};
  
extern int yylineno;

expr *expr_new(etag tag) {
  expr *e = GC_MALLOC(sizeof(expr));
  e->tag = tag;
  e->lineno = yylineno;
  return e;
}

expr *int_const_expr_new(int64_t value) {
  expr *e = expr_new(E_INT_CONST);
  e->intvalue = value;
  return e;
}

expr *bool_const_expr_new(boolval value) {
  expr *e = expr_new(E_BOOL_CONST);
  e->boolvalue = value;
  return e;
}

expr *var_expr_new(string name) {
  expr *e = expr_new(E_VAR);
  e->name = name;
  return e;
}

expr *unary_expr_new(uop uop, expr *rand) {
  expr *e = expr_new(E_UNARY);
  e->uop = uop;
  e->rand = rand;
  return e;
}

expr *binary_expr_new(bop bop, expr *lrand, expr *rrand) {
  expr *e = expr_new(E_BINARY);
  e->bop = bop;
  e->lrand = lrand;
  e->rrand = rrand;
  return e;
}

void print_list(list *lst, void (*pr)(void *)) {
  printf("[");
  if (lst != NULL) {
    pr(car(lst));
    lst = cdr(lst);
    while (lst != NULL) {
      printf(",");
      pr(car(lst));
      lst = cdr(lst);
    }
    printf("]");
  }
}

void print_expr(expr *e) {
  printf("[%s:", enames[e->tag]);
  switch (e->tag) {
  case E_INT_CONST:
    printf("%ld", e->intvalue);
    break;
  case E_BOOL_CONST:
    printf("%s", blnames[e->boolvalue]);
    break;
  case E_VAR:
    printf("%s", e->name);
    break;
  case E_UNARY:
    printf("%s,", unames[e->uop]);
    print_expr(e->rand);
    break;    
  case E_BINARY:
    printf("%s,", bnames[e->bop]);
    print_expr(e->lrand);
    printf(",");
    print_expr(e->rrand);
    break;    
  default: assert(false && "invalid expression tag");
  }
  printf("]");
}

char *snames[] = {
#define X(Y) #Y,
  S_TAGS
#undef X
};

stmt *stmt_new(stag tag) {
  stmt *s = GC_MALLOC(sizeof(stmt));
  s->tag = tag;
  s->lineno = yylineno;
  return s;
}

stmt *assign_stmt_new(string lval, expr *rval) {
  stmt *s = stmt_new(S_ASSIGN);
  s->lval = lval;
  s->rval = rval;
  return s;
}

stmt *write_stmt_new(expr *arg) {
  stmt *s = stmt_new(S_WRITE);
  s->arg = arg;
  return s;
}

stmt *writeln_stmt_new(expr *arg) {
  stmt *s = stmt_new(S_WRITELN);
  s->arg = arg;
  return s;
}

stmt *if_stmt_new(expr *iftest, stmt *ifbody, stmt *elsebody) {
  stmt *s = stmt_new(S_IF);
  s->iftest = iftest;
  s->ifbody = ifbody;
  s->elsebody = elsebody;
  return s;
}

stmt *while_stmt_new(expr *whiletest, stmt *whilebody) {
  stmt *s = stmt_new(S_WHILE);
  s->whiletest = whiletest;
  s->whilebody = whilebody;
  return s;
}

stmt *cmpd_stmt_new(list *cmpdbody) {
  stmt *s = stmt_new(S_CMPD);
  s->cmpdbody = cmpdbody;
  return s;
}

program *program_new(string name, list *type_decls, stmt *body) {
  program *p = GC_MALLOC(sizeof(program));
  p->name = name;
  p->type_decls = type_decls;
  p->body = body;
  return p;
}

type_decl *type_decl_new(list *vars, type *type) {
  type_decl *t = GC_MALLOC(sizeof(type_decl));
  t->vars = vars;
  t->type = type;
  t->lineno = yylineno;
  return t;
}

void print_stmt(stmt *s) {
  printf("[");
  switch(s->tag) {
  case S_ASSIGN:
    printf("ASSIGN,%s,",s->lval);
    print_expr(s->rval);
    printf("]");
    break;
  case S_WRITE:
    printf("WRITE,");
    print_expr(s->arg);
    printf("]");
    break;
  case S_WRITELN:
    printf("WRITELN,");
    if (s->arg != NULL) print_expr(s->arg);
    printf("]");
    break;
  case S_IF:
    printf("IF,");
    print_expr(s->iftest);
    printf(",");
    print_stmt(s->ifbody);
    if (s->elsebody != NULL) {
      printf(",");
      print_stmt(s->elsebody);
    }
    printf("]");
    break;
  case S_WHILE:
    printf("WHILE,");
    print_expr(s->whiletest);
    printf(",");
    print_stmt(s->whilebody);
    printf("]");
    break;
  case S_CMPD:
    printf("CMPD,");
    print_list(s->cmpdbody, (void (*)(void *)) print_stmt);
    printf("]");
    break;
  default:
    assert(false && "invalid statement tag");
  }
}

void print_type(type *t) {
  printf("%s", tnames[t->tag]);
}

void print_string(string s) {
  printf("%s", s);
}

void print_type_decl(type_decl *decl) {
  printf("[TYPE_DECL,");
  print_list(decl->vars, (void (*)(void *)) print_string);
  printf(",");
  print_type(decl->type);
  printf("]");
}

void print_program(program *p) {
  printf("[PROGRAM,");
  printf("%s,\n", p->name);
  print_list(p->type_decls, (void (*)(void *)) print_type_decl);
  print_stmt(p->body);
} 
