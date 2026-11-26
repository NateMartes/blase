#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <gc/gc.h>
#include <assert.h>
#include "pool.h"
#include "quadtab.h"
#include "ast.h"
#include "list.h"
#include "type.h"

static symbol *gen_temp(symtab *st) {
  static int next = 0;
  int len = snprintf(NULL, 0, "%d", next);
  char *name = GC_MALLOC(len + 3);
  snprintf(name, len + 3, "T_%d", next);
  next++;
  name = intern(name);
  symbol *sym = symbol_new(name, type_new(T_INTEGER), mem_addr_new(name));
  symtab_store(st, name, sym);
  return sym;
}

static symbol *gen_label(symtab *st) {
  static int next = 0;
  int len = snprintf(NULL, 0, "%d", next);
  char *name = GC_MALLOC(len + 3);
  snprintf(name, len + 3, "L_%d", next);
  next++;
  name = intern(name);
  symbol *sym = symbol_new(name, type_new(T_LABEL), lab_addr_new(name));
  symtab_store(st, name, sym);
  return sym;
}

#define INITIAL_QUADTAB_SIZE 1024

quadtab *quadtab_new() {
  quadtab *qt = GC_MALLOC(sizeof(quadtab));
  qt->quads = GC_MALLOC(INITIAL_QUADTAB_SIZE * sizeof(quad));
  qt->size = INITIAL_QUADTAB_SIZE;
  qt->count = 0;
  return qt;
}

static void add_quad(quadtab *qt, opcode oc, addr *result, addr *left, addr *right) {
  if (qt->count == qt->size) {
    qt->size = 2 * qt->size;
    qt->quads = GC_REALLOC(qt->quads, qt->size * sizeof(quad));
  }
  qt->quads[qt->count].opcode = oc;
  qt->quads[qt->count].result = result;
  qt->quads[qt->count].left = left;
  qt->quads[qt->count].right = right;
  qt->count++;
}

static void gen_int_const_expr(quadtab *qt, symtab *st, expr *e, addr *result);
static void gen_var_expr(quadtab *qt, symtab *st, expr *e, addr *result);
static void gen_binary_expr(quadtab *qt, symtab *st, expr *e, addr *result);
static void gen_unary_expr(quadtab *qt, symtab *st, expr *e, addr *result);

static void gen_expr(quadtab *qt, symtab *st, expr *e, addr *result) {
  switch(e->tag) {
  case E_INT_CONST: gen_int_const_expr(qt, st, e, result); break;
  case E_BINARY: gen_binary_expr(qt, st, e, result); break;
  case E_UNARY: gen_unary_expr(qt, st, e, result); break;
  case E_VAR: gen_var_expr(qt, st, e, result); break;
  default: assert(false && "invalid expression tag");
  }
}
    
static void gen_int_const_expr(quadtab *qt, symtab *st, expr *e, addr *result) {
  addr *value = imm_addr_new(e->intvalue);
  add_quad(qt, OP_MOV, result, value, NULL);
}

static void gen_var_expr(quadtab *qt, symtab *st, expr *e, addr *result) {
  symbol *var = symtab_lookup(st, e->name);
  add_quad(qt, OP_MOV, result, var->addr, NULL);
}

static void gen_unary_expr(quadtab *qt, symtab *st, expr *e, addr *result) {
  symbol *sym = gen_temp(st);
  addr *rand = mem_addr_new(sym->name);
  gen_expr(qt, st, e->rand, rand);
  opcode oc;
  switch (e->uop) {
  case U_MINUS: oc = OP_NEG; break;
  case U_PLUS: oc = OP_MOV; break;
  default: assert(false && "invalid unary operator");
  }
  add_quad(qt, oc, result, rand, NULL);
}

static void gen_binary_expr(quadtab *qt, symtab *st, expr *e, addr *result) {
  symbol *lsym = gen_temp(st);
  addr *lrand = mem_addr_new(lsym->name);
  symbol *rsym = gen_temp(st);
  addr *rrand = mem_addr_new(rsym->name);
  gen_expr(qt, st, e->lrand, lrand);
  gen_expr(qt, st, e->rrand, rrand);
  opcode oc;
  switch (e->bop) {
  case B_ADD: oc = OP_ADD; break;
  case B_SUB: oc = OP_SUB; break;
  case B_MUL: oc = OP_MUL; break;
  case B_DIV: oc = OP_DIV; break;
  case B_MOD: oc = OP_MOD; break;
  default: assert(false && "invalid binary operator");
  }
  add_quad(qt, oc, result, lrand, rrand);
}

void gen_bool_expr(quadtab *qt, symtab *st, expr *e, addr *tofalse, addr *totrue);

void gen_bool_rel_expr(quadtab *qt, symtab *st, expr *e, addr *tofalse, addr *totrue) {
  symbol *lsym = gen_temp(st);
  addr *lrand = mem_addr_new(lsym->name);
  symbol *rsym = gen_temp(st);
  addr *rrand = mem_addr_new(rsym->name);
  gen_expr(qt, st, e->lrand, lrand);
  gen_expr(qt, st, e->rrand, rrand);
  opcode oc;
  switch (e->bop) {
  case B_LT: oc = OP_JLT; break;
  case B_LE: oc = OP_JLE; break;
  case B_GT: oc = OP_JGT; break;
  case B_GE: oc = OP_JGE; break;
  case B_NE: oc = OP_JNE; break;
  case B_EQ: oc = OP_JEQ; break;
  default: assert(false && "invalid binary operator");
  }
  add_quad(qt, oc, totrue, lrand, rrand);
  add_quad(qt, OP_JMP, tofalse, NULL, NULL);
}

void gen_bool_binary_expr(quadtab *qt, symtab *st, expr *e, addr *tofalse, addr *totrue) {
  symbol *maybe = gen_label(st);
  switch (e->bop) {
  case B_OR:
    gen_bool_expr(qt, st, e->lrand, maybe->addr, totrue); 
    add_quad(qt, OP_LABEL, maybe->addr, NULL, NULL);
    gen_bool_expr(qt, st, e->rrand, tofalse, totrue);
    break;
  case B_AND:
    gen_bool_expr(qt, st, e->lrand, tofalse, maybe->addr); 
    add_quad(qt, OP_LABEL, maybe->addr, NULL, NULL);
    gen_bool_expr(qt, st, e->rrand, tofalse, totrue);
    break;
  case B_LT:
  case B_LE:
  case B_GT:
  case B_GE:
  case B_NE:
  case B_EQ:
    gen_bool_rel_expr(qt, st, e, tofalse, totrue);
    break;
  default:
    assert(false && "invalid boolean unary operator");
  }
}

void gen_bool_unary_expr(quadtab *qt, symtab *st, expr *e, addr *tofalse, addr *totrue) {
  switch (e->uop) {
  case U_NOT: gen_bool_expr(qt, st, e->rand, totrue, tofalse); break;
  default: assert(false && "invalid boolean unary operator");
  }
}

void gen_bool_const_expr(quadtab *qt, symtab *st, expr *e, addr *tofalse, addr *totrue) {
  switch (e->boolvalue) {
  case BL_TRUE: add_quad(qt, OP_JMP, totrue, NULL, NULL); break;
  case BL_FALSE: add_quad(qt, OP_JMP, tofalse, NULL, NULL); break;
  default: assert(false && "invalid boolean value");
  }
}

void gen_bool_var_expr(quadtab *qt, symtab *st, expr *e, addr *tofalse, addr *totrue) {
  symbol *sym = symtab_lookup(st, e->name);
  add_quad(qt, OP_JEQ, tofalse, sym->addr, imm_addr_new(0));
  add_quad(qt, OP_JMP, totrue, NULL, NULL);
}

void gen_bool_expr(quadtab *qt, symtab *st, expr *e, addr *tofalse, addr *totrue) {
  switch (e->tag) {
  case E_BOOL_CONST: gen_bool_const_expr(qt, st, e, tofalse, totrue); break;
  case E_VAR: gen_bool_var_expr(qt, st, e, tofalse, totrue); break;
  case E_UNARY: gen_bool_unary_expr(qt, st, e, tofalse, totrue); break;
  case E_BINARY: gen_bool_binary_expr(qt, st, e, tofalse, totrue); break;
  default: assert(false && "invalid expression tag");
  }
}

static void gen_stmt(quadtab *qt, symtab *st, stmt *s);
static void gen_stmt_list(quadtab *qt, symtab *st, list *stmts);

static void gen_assign_stmt(quadtab *qt, symtab *st, stmt *s) {
  symbol *lval = symtab_lookup(st, s->lval);
  switch (lval->type->tag) {
  case T_BOOLEAN:
    symbol *no = gen_label(st);
    symbol *yes = gen_label(st);
    symbol *done = gen_label(st);
    gen_bool_expr(qt, st, s->rval, no->addr, yes->addr);
    add_quad(qt, OP_LABEL, yes->addr, NULL, NULL);
    add_quad(qt, OP_MOV, lval->addr, imm_addr_new(1), NULL);
    add_quad(qt, OP_JMP, done->addr, NULL, NULL);
    add_quad(qt, OP_LABEL, no->addr, NULL, NULL);
    add_quad(qt, OP_MOV, lval->addr, imm_addr_new(0), NULL);
    add_quad(qt, OP_LABEL, done->addr, NULL, NULL);
    break;
  case T_INTEGER: gen_expr(qt, st, s->rval, lval->addr); break;
  default: assert(false && "invalid type tag");
  }
}

static void gen_write_stmt(quadtab *qt, symtab *st, stmt *s) {
  symbol *sym = gen_temp(st);
  gen_expr(qt, st, s->arg, sym->addr);
  add_quad(qt, OP_PUT, NULL, sym->addr,NULL);
}

static void gen_writeln_stmt(quadtab *qt, symtab *st, stmt *s) {
  if (s->arg != NULL) {
    gen_write_stmt(qt, st, s);
  }
  add_quad(qt, OP_PUTLN, NULL, NULL, NULL);
}

static void gen_if_stmt(quadtab *qt, symtab *st, stmt *s) {
  symbol *yes = gen_label(st);
  symbol *no = gen_label(st);
  if (s->elsebody == NULL) {
    gen_bool_expr(qt, st, s->iftest, no->addr, yes->addr);
    add_quad(qt, OP_LABEL, yes->addr, NULL, NULL);
    gen_stmt(qt, st, s->ifbody);
    add_quad(qt, OP_LABEL, no->addr, NULL, NULL);
  }
  else {
    symbol *end = gen_label(st);
    gen_bool_expr(qt, st, s->iftest, no->addr, yes->addr);
    add_quad(qt, OP_LABEL, yes->addr, NULL, NULL);
    gen_stmt(qt, st, s->ifbody);
    add_quad(qt, OP_JMP, end->addr, NULL, NULL);
    add_quad(qt, OP_LABEL, no->addr, NULL, NULL);
    gen_stmt(qt, st, s->elsebody);
    add_quad(qt, OP_LABEL, end->addr, NULL, NULL);
  }
}
    
static void gen_while_stmt(quadtab *qt, symtab *st, stmt *s) {
  symbol *loop = gen_label(st);
  symbol *yes = gen_label(st);
  symbol *no = gen_label(st);
  add_quad(qt, OP_LABEL, loop->addr, NULL, NULL);
  gen_bool_expr(qt, st, s->whiletest, no->addr, yes->addr);
  add_quad(qt, OP_LABEL, yes->addr, NULL, NULL);
  gen_stmt(qt, st, s->whilebody);
  add_quad(qt, OP_JMP, loop->addr, NULL, NULL);
  add_quad(qt, OP_LABEL, no->addr, NULL, NULL);
}

static void gen_cmpd_stmt(quadtab *qt, symtab *st, stmt *s) {
  gen_stmt_list(qt, st, s->cmpdbody);
}

static void gen_stmt(quadtab *qt, symtab *st, stmt *s) {
  switch(s->tag) {
  case S_ASSIGN: gen_assign_stmt(qt, st, s); break;
  case S_WRITE: gen_write_stmt(qt, st, s); break;
  case S_WRITELN: gen_writeln_stmt(qt, st, s); break;
  case S_IF: gen_if_stmt(qt, st, s); break;
  case S_WHILE: gen_while_stmt(qt, st, s); break;
  case S_CMPD: gen_cmpd_stmt(qt, st, s); break;
  default: assert(false && "Invalid statement tag");
  }
}

static void gen_stmt_list(quadtab *qt, symtab *st, list *stmts) {
  while (stmts != NULL) {
    gen_stmt(qt, st, car(stmts));
    stmts = cdr(stmts);
  }
}  

void quadtab_gen(quadtab *qt, symtab *st, program *prog) {
  gen_stmt(qt, st, prog->body);
}

void quadtab_print(quadtab *qt) {
  for (int i = 0; i < qt->count; i++) {
    quad_print(qt->quads[i]);
  }
}
