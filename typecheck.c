#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "assert.h"
#include "ast.h"
#include "symtab.h"
#include "type.h"
#include "typecheck.h"

static bool error = false;

static void declare_symbols(symtab *st, list *decls) {
  while (decls != NULL) {
    type_decl *decl = car(decls);
    list *vars = decl->vars;
    type *type = decl->type;
    while (vars != NULL) {
      string var = car(vars);
      symbol *sym = symtab_lookup(st, var);
      if (sym == NULL) {
        symtab_store(st, var, symbol_new(var, type, mem_addr_new(var)));
      }
      else {
        fprintf(stderr, "duplicate type declaration %s at line %d\n", var, decl->lineno);
        error = true;
      }
      vars = cdr(vars);
    }
    decls = cdr(decls);
  }
}

static type *typecheck_expr(symtab *st, expr *e);

static type *typecheck_int_const_expr(symtab *st, expr *e) {
  return e->type = type_new(T_INTEGER);
}

static type *typecheck_bool_const_expr(symtab *st, expr *e) {
  return e->type = type_new(T_BOOLEAN);
}

static type *typecheck_var_expr(symtab *st, expr *e) {
  symbol *s = symtab_lookup(st, e->name);
  return e->type = s->type;
}

static type *typecheck_binary_expr(symtab *st, expr *e) {
  type *ltype = typecheck_expr(st, e->lrand);
  type *rtype = typecheck_expr(st, e->rrand);
  switch (e->bop) {
  case B_ADD:
  case B_SUB:
  case B_MUL:
  case B_DIV:
  case B_MOD:
    if (ltype->tag != T_INTEGER || rtype->tag != T_INTEGER) {
      fprintf(stderr, "type mismatch at line %d\n", e->lineno);
      error = true;
    }
    e->type = type_new(T_INTEGER);
    return e->type;
  case B_LT:
  case B_LE:
  case B_GT:
  case B_GE:
  case B_NE:
  case B_EQ:
    if (ltype->tag != T_INTEGER || rtype->tag != T_INTEGER) {
      fprintf(stderr, "type mismatch at line %d\n", e->lineno);
      error = true;
    }
    return e->type = type_new(T_BOOLEAN);
  case B_AND:
  case B_OR:
    if (ltype->tag != T_BOOLEAN || rtype->tag != T_BOOLEAN) {
      fprintf(stderr, "type mismatch at line %d\n", e->lineno);
      error = true;
    }
    return e->type = type_new(T_BOOLEAN);
  default:
    assert(false && "invalid binary operator");
  }
}

static type *typecheck_unary_expr(symtab *st, expr *e) {
  type *t = typecheck_expr(st, e->rand);
  switch (e->uop) {
  case U_PLUS: 
  case U_MINUS:
    if (t->tag != T_INTEGER) {
      fprintf(stderr, "type mismatch at line %d\n", e->lineno);
      error = true;
    }
    return e->type = type_new(T_INTEGER);
  case U_NOT:
    if (t->tag != T_BOOLEAN) {
      fprintf(stderr, "type mismatch at line %d\n", e->lineno);
      error = true;
      return e->type = type_new(T_BOOLEAN);
    }
    else {
      return type_new(T_BOOLEAN);
    }
  default:
    assert(false && "invalid unary operator");
  }
}

static type *typecheck_expr(symtab *st, expr *e) {
  switch (e->tag) {
  case E_INT_CONST: return typecheck_int_const_expr(st, e);
  case E_BOOL_CONST: return typecheck_bool_const_expr(st, e);
  case E_VAR: return typecheck_var_expr(st, e);
  case E_BINARY: return typecheck_binary_expr(st, e);
  case E_UNARY: return typecheck_unary_expr(st, e);
  default: assert(false && "invalid expression tag");
  }
}

static void typecheck_stmt(symtab *st, stmt *s);

static void typecheck_assign_stmt(symtab *st, stmt *s) {
  type *t = typecheck_expr(st, s->rval);
  symbol *sym = symtab_lookup(st, s->lval);
  if (t->tag != sym->type->tag) {
    fprintf(stderr, "type mismatch at line %d\n", s->lineno);
    error = true;
  }
}

static void typecheck_write_stmt(symtab *st, stmt *s) {
  type *t = typecheck_expr(st, s->arg);
  if (t->tag != T_INTEGER) {
    fprintf(stderr, "type mismatch at line %d\n", s->lineno);
    error = true;
  }
}

static void typecheck_writeln_stmt(symtab *st, stmt *s) {
  if (s->arg != NULL) typecheck_write_stmt(st, s);
}

static void typecheck_if_stmt(symtab *st, stmt *s) {
  type *t = typecheck_expr(st, s->iftest);
  if (t->tag != T_BOOLEAN) {
    fprintf(stderr, "type mismatch at line %d\n", s->lineno);
    error = true;
  }
  typecheck_stmt(st, s->ifbody);
  if (s->elsebody != NULL) {
    typecheck_stmt(st, s->elsebody);
  }    
}

static void typecheck_while_stmt(symtab *st, stmt *s) {
  type *t = typecheck_expr(st, s->whiletest);
  if (t->tag != T_BOOLEAN) {
    fprintf(stderr, "type mismatch at line %d\n", s->lineno);
    error = true;
  }
  typecheck_stmt(st, s->whilebody);
}

static void typecheck_stmt_list(symtab *st, list *stmts) {
  while (stmts != NULL) {
    typecheck_stmt(st, car(stmts));
    stmts = cdr(stmts);
  }
}

static void typecheck_cmpd_stmt(symtab *st, stmt *s) {
  typecheck_stmt_list(st, s->cmpdbody);
}

static void typecheck_stmt(symtab *st, stmt *s) {
  switch (s->tag) {
  case S_ASSIGN: typecheck_assign_stmt(st, s); break;
  case S_WRITE: typecheck_write_stmt(st, s); break;
  case S_WRITELN: typecheck_writeln_stmt(st, s); break;
  case S_IF: typecheck_if_stmt(st, s); break;
  case S_WHILE: typecheck_while_stmt(st, s); break;
  case S_CMPD: typecheck_cmpd_stmt(st, s); break;
  default: assert(false && "invalid statement tag");
  }
}

bool typecheck_prog(symtab *st, program *prog) {
  declare_symbols(st, prog->type_decls);
  typecheck_stmt(st, prog->body);
  return !error;
}
