#ifndef SYMTAB_H
#define SYMTAB_H

#include "pool.h"
#include "type.h"
#include "addr.h"
#include "symbol.h"

typedef struct symtab symtab;

symtab *symtab_new();
void symtab_store(symtab *table, string key, symbol *value);
symbol *symtab_lookup(symtab *table, string key);
void symtab_traverse(symtab *table, void (*visit)(string, symbol *));

#endif
