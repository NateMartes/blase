#ifndef QUADTAB_H
#define QUADTAB_H

#include "addr.h"
#include "symtab.h"
#include "quad.h"
#include "list.h"
#include "ast.h"

typedef struct {
  int size;
  int count;
  quad *quads;
} quadtab;

quadtab *quadtab_new();
void quadtab_gen(quadtab *qt, symtab *st, program *prog);
void quadtab_print(quadtab *qt);


#endif
