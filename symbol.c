#include <gc/gc.h>
#include "symbol.h"

symbol *symbol_new(string s, type *type, addr *addr) {
  symbol *sym = GC_MALLOC(sizeof(symbol));
  sym->name = s;
  sym->type = type;
  sym->addr = addr;
  return sym;
}
