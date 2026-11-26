#include <stdio.h>
#include <string.h>
#include <gc/gc.h>
#include <assert.h>
#include "symtab.h"
#include "addr.h"

char *addrmode_names[] = {
#define X(Y) #Y,
  ADDR_MODES
#undef X
};

addr *addr_new(addrmode mode) {
  addr *a = GC_MALLOC(sizeof(addr));
  a->mode = mode;
  return a;
}

addr *mem_addr_new(string rand) {
  addr *a = addr_new(A_MEM);
  a->rand = rand;
  return a;
}

addr *imm_addr_new(int val) {
  addr *a = addr_new(A_IMM);
  int len = snprintf(NULL, 0, "%d", val);
  char *s = GC_MALLOC(len + 1);
  snprintf(s, len + 1, "%d", val);
  a->rand = intern(s);
  return a;
}

addr *stk_addr_new(int offset) {
  addr *a = addr_new(A_IMM);
  int len = snprintf(NULL, 0, "%d", offset);
  char *s = GC_MALLOC(len + 1);
  snprintf(s, len + 1, "%d", offset);
  a->rand = intern(s);
  return a;
}

addr *lab_addr_new(string label) {
  addr *a = addr_new(A_LABEL);
  a->rand = label;
  return a;
}

string addr_tostr(addr *a) {
  return a->rand;
}
