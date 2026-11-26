#ifndef AST_H
#define AST_H

#include <stdint.h>
#include "pool.h"
#include "type.h"

#define E_TAGS X(INT_CONST) X(BOOL_CONST) X(VAR) X(BINARY) X(UNARY)

typedef enum {
#define X(Y) E_##Y, 
  E_TAGS
#undef X
} etag;

extern char *enames[];

#define UOPS X(PLUS) X(MINUS) X(NOT)

typedef enum {
#define X(Y) U_##Y,
  UOPS
#undef X
} uop;

extern char *unames[];

#define BOPS X(ADD) X(SUB) X(MUL) X(DIV) X(MOD) X(LT) X(LE) X(GT) X(GE) X(EQ) X(NE) X(AND) X(OR)

typedef enum {
#define X(Y) B_##Y,
  BOPS
#undef X
} bop;

extern char *bnames[];

#define BL_VALS X(FALSE) X(TRUE)

typedef enum {
#define X(Y) BL_##Y,
  BL_VALS
#undef X
} boolval;

extern char *blnames[];

typedef struct expr {
  etag tag;
  int lineno;
  type *type;
  union {
    struct {
      int64_t intvalue;
    };
    struct {
      boolval boolvalue;
    };
    struct {
      string name;
    };
    struct {
      uop uop;
      struct expr *rand;
    };
    struct {
      bop bop;
      struct expr *lrand;
      struct expr *rrand;
    };
  };
} expr;

expr *int_const_expr_new(int64_t value);
expr *bool_const_expr_new(boolval value);
expr *var_expr_new(string name);
expr *unary_expr_new(uop uop, expr *rand);
expr *binary_expr_new(bop bop, expr *lrand, expr *rrand);
void print_expr(expr *e);

#define S_TAGS X(ASSIGN) X(WRITE) X(WRITELN) X(IF) X(WHILE) X(CMPD)

typedef enum {
#define X(Y) S_##Y,
  S_TAGS
#undef X
} stag;

typedef struct list {
  void *car;
  struct list *cdr;
} list;

list *cons(void *car, struct list *cdr);
void *car(list *lst);
void *cdr(list *lst);

extern char *snames[];

typedef struct stmt {
  stag tag;
  int lineno;
  union {
    // assignment
    struct {
      string lval;
      expr *rval;
    };
    // write or writeln
    struct {
      expr *arg;
    };
    // if
    struct {
      expr *iftest;
      struct stmt *ifbody;
      struct stmt *elsebody;
    };
    // while
    struct {
      expr *whiletest;
      struct stmt *whilebody;
    };
    // compound
    struct {
      list *cmpdbody;
    };
  };
} stmt;

typedef struct {
  string name;
  list *type_decls;
  stmt *body;
} program;

typedef struct {
  list *vars;
  type *type;
  int lineno;
} type_decl;

void print_stmt(stmt *s);
void print_stmt_list(list *lst);
void print_program(program *p);
program *program_new(string name, list *type_decls, stmt *body);
type_decl *type_decl_new(list *vars, type *type);
stmt *assign_stmt_new(string lval, expr *rval);
stmt *write_stmt_new(expr *arg);
stmt *writeln_stmt_new(expr *arg);
stmt *if_stmt_new(expr *iftest, stmt *ifbody, stmt *elsebody);
stmt *while_stmt_new(expr *whiletest, stmt *whilebody);
stmt *cmpd_stmt_new(list *cmpdbody);

#endif
