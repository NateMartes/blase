#ifndef QUAD_H
#define QUAD_H

#include "addr.h"

#define OPCODES  X(ADD) X(SUB) X(MUL) X(DIV) X(MOD) X(NEG) X(PUT) X(PUTLN) \
                 X(MOV) X(JMP) X(JLT) X(JLE) X(JGT) X(JGE) X(JEQ) X(JNE) \
                 X(NOOP) X(LABEL)

typedef enum {
#define X(Y) OP_##Y,
  OPCODES
#undef X
} opcode;
 
extern char *opcode_names[];

typedef struct quad {
  opcode opcode;
  addr *result;
  addr *left;
  addr *right;
} quad;

void quad_print(quad q);

#endif
