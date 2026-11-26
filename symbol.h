#ifndef SYMBOL_H
#define SYMBOL_H

#include "pool.h"
#include "type.h"
#include "addr.h"

typedef struct symbol {
  string name;
  type *type;
  addr *addr;
} symbol;

symbol *symbol_new(string s, type *type, addr *addr);

#endif
