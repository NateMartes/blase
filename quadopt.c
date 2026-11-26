#include <stdbool.h>

#include "symtab.h"
#include "symbol.h"
#include "quad.h"
#include "quadtab.h"
#include "addr.h"
#include "quadopt.h"

bool changed;

bool is_jump(opcode oc) {
  switch (oc) {
  case OP_JLT: case OP_JLE: case OP_JGT: case OP_JGE:
  case OP_JEQ: case OP_JNE: case OP_JMP:
      return true;
  default:
    return false;
  }
}

void find_labels(quadtab *qt, symtab *st) {
  for (int i = 0; i < qt->count; i++) {
    if (qt->quads[i].opcode == OP_LABEL) {
      qt->quads[i].result->index = i;
    }
  }
}

static void opt_unused_labels(quadtab *qt, symtab *st) {
  for (int i = 0; i < qt->count; i++) {
    if (qt->quads[i].opcode == OP_LABEL) {
      qt->quads[i].result->used = false;
    }
  }
  for (int i = 0; i < qt->count; i++) {
    if (is_jump(qt->quads[i].opcode)) {
      qt->quads[i].result->used = true;
    }
  }
  for (int i = 0; i < qt->count; i++) {
    if (qt->quads[i].opcode == OP_LABEL && !qt->quads[i].result->used) {
      qt->quads[i].opcode = OP_NOOP;
      changed = true;
    }
  }
}

static void opt_jumps_to_next(quadtab *qt, symtab *st) {
  for (int i = 0; i < qt->count; i++) {
    if (is_jump(qt->quads[i].opcode)) {
      int j = i + 1;
      while (j < qt->count && qt->quads[j].opcode == OP_NOOP) {
        j++;
      }
      if (j < qt->count && qt->quads[j].opcode == OP_LABEL
                        && qt->quads[i].result == qt->quads[j].result) {
        qt->quads[i].opcode = OP_NOOP;
        changed = true;
      }
    }
  }
}

static void opt_jumps_to_jumps(quadtab *qt, symtab *st) {
  for (int i = 0; i < qt->count; i++) {
    if (is_jump(qt->quads[i].opcode)) {
      symbol *sym = symtab_lookup(st, qt->quads[i].result->rand);
      int target = sym->addr->index + 1;
      if (qt->quads[target].opcode == OP_JMP && i != target) {
        qt->quads[i].result = qt->quads[target].result;
        changed = true;
      }
    }
  }
}

static void opt_dead_code(quadtab *qt, symtab *st) {
  for (int i = 0; i < qt->count; i++) {
    if (qt->quads[i].opcode == OP_JMP) {
      i++;
      while (i < qt->count && qt->quads[i].opcode != OP_LABEL) {
        if (qt->quads[i].opcode != OP_NOOP) {
          changed = true;
          qt->quads[i].opcode = OP_NOOP;
        }
        i++;
      }
    }
  }
}

void opt_jumps(quadtab *qt, symtab *st) {
  do {
    changed = false;
    opt_jumps_to_next(qt, st);
    opt_jumps_to_jumps(qt, st);
    opt_dead_code(qt, st);
    opt_unused_labels(qt, st);
  } while (changed);
}
