#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "symtab.h"
#include "quad.h"

char *opcode_names[] = {
#define X(Y) #Y,
  OPCODES
#undef X
}; 


void quad_print(quad q) {
  switch (q.opcode) {
  case OP_MOV:   printf("%s := %s\n", addr_tostr(q.result), addr_tostr(q.left)); break;
  case OP_ADD:   printf("%s := %s + %s\n", addr_tostr(q.result), addr_tostr(q.left), addr_tostr(q.right)); break;
  case OP_SUB:   printf("%s := %s - %s\n", addr_tostr(q.result), addr_tostr(q.left), addr_tostr(q.right)); break;
  case OP_MUL:   printf("%s := %s * %s\n", addr_tostr(q.result), addr_tostr(q.left), addr_tostr(q.right)); break;
  case OP_DIV:   printf("%s := %s div %s\n", addr_tostr(q.result), addr_tostr(q.left), addr_tostr(q.right)); break;
  case OP_MOD:   printf("%s := %s mod %s\n", addr_tostr(q.result), addr_tostr(q.left), addr_tostr(q.right)); break;
  case OP_JLT:   printf("if %s < %s jmp %s\n", addr_tostr(q.left), addr_tostr(q.right), addr_tostr(q.result)); break;
  case OP_JLE:   printf("if %s <= %s jmp %s\n", addr_tostr(q.left), addr_tostr(q.right), addr_tostr(q.result)); break;
  case OP_JGT:   printf("if %s > %s jmp %s\n", addr_tostr(q.left), addr_tostr(q.right), addr_tostr(q.result)); break;
  case OP_JGE:   printf("if %s >= %s jmp %s\n", addr_tostr(q.left), addr_tostr(q.right), addr_tostr(q.result)); break;
  case OP_JEQ:   printf("if %s == %s jmp %s\n", addr_tostr(q.left), addr_tostr(q.right), addr_tostr(q.result)); break;
  case OP_JNE:   printf("if %s != %s jmp %s\n", addr_tostr(q.left), addr_tostr(q.right), addr_tostr(q.result)); break;
  case OP_JMP:   printf("jmp %s\n", addr_tostr(q.result)); break;
  case OP_NEG:   printf("%s := - %s\n", addr_tostr(q.result), addr_tostr(q.left)); break;
  case OP_LABEL: printf("%s:\n", addr_tostr(q.result)); break;
  case OP_PUT:   printf("put %s\n", addr_tostr(q.left)); break;
  case OP_PUTLN: printf("putln\n"); break;
  case OP_NOOP:  printf("noop\n"); break;
  default: assert(false && "invalid opcode");
  }
}
