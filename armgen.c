#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "list.h"
#include "ast.h"
#include "symtab.h"
#include "quadtab.h"

void emit0(char *opcode) {
  printf("%8s%-8s\n", "", opcode);
}

void emit1(char *opcode, char *rand1) {
  printf("%8s%-8s%s\n", "", opcode, rand1);
}

void emit2(char *opcode, char *rand1, char *rand2) {
  printf("%8s%-8s%s, %s\n", "", opcode, rand1, rand2);
}

void emit2imm(char *opcode, char *rand1, char *rand2) {
  printf("%8s%-8s%s, =%s\n", "", opcode, rand1, rand2);
}

void emit3(char *opcode, char *rand1, char *rand2, char *rand3) {
  printf("%8s%-8s%s, %s, %s\n", "", opcode, rand1, rand2, rand3);
}

void emitl(string label) {
  printf("%s:\n", label);
}

static void gen_prologue() {
  emit1(".arch", "armv8-a");
  emit1(".global", "main");
  emit1(".text", "");
  emit1(".align", "2");
  emitl("main");
  emit3("stp", "fp", "lr", "[sp,-16]!");
  emit2("mov", "fp", "sp");
}

static void allocate_variable(string name, symbol *sym) {
  if (sym->type->tag != T_LABEL) {
    int len = strlen(name);
    printf("%-s:%*s%s\n", name, (int) (7 - len), "", ".quad 0");
  }
}

static void gen_epilogue(symtab *st) {
  emit2("mov", "x0", "#0");
  emit3("ldp", "fp", "lr", "[sp], 16");
  emit0("ret");
  emit1(".align", "3");
  emit0(".data");
  symtab_traverse(st, allocate_variable);
  emit1(".section", " .rodata");
  printf("wfmt:   .string \"%%8ld\"\n");
  printf("wlfmt:  .string \"\\n\"\n");
}

static void gen_binary(quad *q) {
  switch (q->left->mode) {
  case A_MEM:
    emit2("adr", "x10", q->left->rand);
    emit2("ldr", "x10", "[x10]");
    break;
  default:
    assert(false && "invalid address mode for binary operator left operand");
  }
  switch (q->right->mode) {
  case A_MEM:
    emit2("adr", "x11", q->right->rand);
    emit2("ldr", "x11", "[x11]");
    break;
  case A_IMM:
    emit2imm("ldr", "x11", q->right->rand);
    break;
  default:
    assert(false && "invalid address mode for binary operator right operand");
  }
  switch (q->opcode) {
  case OP_ADD: emit3("add", "x12", "x10", "x11"); break;
  case OP_SUB: emit3("sub", "x12", "x10", "x11"); break;
  case OP_MUL: emit3("mul", "x12", "x10", "x11"); break;
  case OP_DIV: emit3("sdiv", "x12", "x10", "x11");break;
  case OP_MOD:
    emit3("sdiv", "x12", "x10", "x11");
    emit3("mneg", "x13", "x11", "x12");
    emit3("add", "x12", "x10", "x13");
    break;
  case OP_JLT: emit2("cmp", "x10", "x11"); emit1("blt", q->result->rand); return;
  case OP_JLE: emit2("cmp", "x10", "x11"); emit1("ble", q->result->rand); return;
  case OP_JGT: emit2("cmp", "x10", "x11"); emit1("bgt", q->result->rand); return;
  case OP_JGE: emit2("cmp", "x10", "x11"); emit1("bge", q->result->rand); return;
  case OP_JNE: emit2("cmp", "x10", "x11"); emit1("bne", q->result->rand); return;
  case OP_JEQ: emit2("cmp", "x10", "x11"); emit1("beq", q->result->rand); return;
  default: assert(false && "invalid opcode");
  }
  switch (q->result->mode) {
  case A_MEM:
    emit2("adr", "x9", q->result->rand);
    break;
  default:
    assert(false && "invalid address mode for binary operator result");
  }
  emit2("str", "x12", "[x9]");
}

static void gen_unary(quad *q) {
  switch (q->opcode) {
  case OP_NEG:
    switch (q->left->mode) {
    case A_MEM:
      emit2("adr", "x10", q->left->rand);
      emit2("ldr", "x10", "[x10]");
      break;
    case A_IMM:
      emit2imm("ldr", "x10", q->left->rand);
      break;
    default:
      assert(false && "invalid address mode for unary operator left operand");
    }
    emit2("neg", "x10", "x10");
    switch (q->result->mode) {
    case A_MEM:
      emit2("adr", "x9", q->result->rand);
      break;
    default:
      assert(false && "invalid address mode for binary operator result");
    }
    emit2("str", "x10", "[x9]");
    break;
  default:
    assert(false && "invalid op code");
  }
}

static void gen_mov(quad *q) {
  switch (q->result->mode) {
  case A_MEM:
    emit2("adr", "x9", q->result->rand);
    break;
  default:
    assert(false && "invalid address mode for binary operator result");
  }
  switch (q->left->mode) {
  case A_MEM:
    emit2("adr", "x10", q->left->rand);
    emit2("ldr", "x10", "[x10]");
    break;
  case A_IMM:
    emit2imm("ldr", "x10", q->left->rand);
    break;
  default:
    assert(false && "invalid address mode for unary operator left operand");
  }
  emit2("str", "x10", "[x9]");
}

static void gen_put(quad *q) {
  emit2("adr", "x0", "wfmt");
  switch (q->left->mode) {
  case A_MEM:
    emit2("adr", "x1", q->left->rand);
    emit2("ldr", "x1", "[x1]");
    break;
  case A_IMM:
    emit2imm("ldr", "x1", q->left->rand);
    break;
  default:
    assert(false && "invalid address mode for put");
  }
  emit1("bl", "printf");
}

static void gen_putln(quad *q) {
  emit2("adr", "x0", "wlfmt");
  emit1("bl", "printf");
}

static void gen_jmp(quad *q) {
  emit1("b", q->result->rand);
}

static void gen_label(quad *q) {
  emitl(q->result->rand);
}

static void gen_quad(quad *q) {
  switch(q->opcode) {
  case OP_LABEL: gen_label(q); break;
  default:
    printf("        # "); quad_print(*q);
    switch (q->opcode) {
    case OP_ADD: case OP_SUB:  case OP_MUL:  case OP_DIV:  case OP_MOD:
    case OP_JLT: case OP_JLE:  case OP_JGT:  case OP_JGE:  case OP_JNE:
    case OP_JEQ:   gen_binary(q); break;
    case OP_JMP:   gen_jmp(q); break;
    case OP_NEG:   gen_unary(q);  break;
    case OP_PUT:   gen_put(q);  break;
    case OP_PUTLN: gen_putln(q);  break;
    case OP_MOV:   gen_mov(q);    break;
    case OP_NOOP: break;
    default:       assert(false && "invalid opcode");
    }
  }
}

static void gen_quads(quadtab *qt) {
  for (int i = 0; i < qt->count; i++) {
    gen_quad(&qt->quads[i]);
  }
}

void gen_program(quadtab *qt, symtab *st) {
  gen_prologue();
  gen_quads(qt);
  gen_epilogue(st);
}
