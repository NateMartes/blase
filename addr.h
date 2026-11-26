#ifndef ADDR_H
#define ADDR_H

#include <stdbool.h>

#define ADDR_MODES X(MEM) X(IMM) X(STK) X(LABEL)

typedef enum {
#define X(Y) A_##Y,
  ADDR_MODES
#undef X
} addrmode;

extern char *addrmode_names[];

typedef struct addr {
  addrmode mode;
  string rand;
  int index;
  bool used;
} addr;

addr *addr_new(addrmode mode);
addr *mem_addr_new(string mem);
addr *imm_addr_new(int imm);
addr *stk_addr_new(int stk);
addr *lab_addr_new(string label);
char *addr_tostr(addr *a);

#endif
