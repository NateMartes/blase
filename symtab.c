#include <gc/gc.h>
#include "symtab.h"

#define SYMTAB_SIZE 1009

static unsigned int hash(char *s) {
  int h = 0;
  while (*s != '\0') {
    h += *s++ * 117;
  }
  return h;
}

typedef struct node {
  void *key;
  void *value;
  struct node *next;
} node;

struct symtab {
  int size;
  node **chains;
};

symtab *symtab_new() {
  symtab *st = GC_MALLOC(sizeof(symtab));
  st->size = SYMTAB_SIZE;
  st->chains = GC_MALLOC(SYMTAB_SIZE * sizeof(node *));
  return st;
}

void symtab_store(symtab *st, string key, symbol *value) {
  unsigned int h = hash(key) % st->size;
  node *n = st->chains[h];
  while (n != NULL) {
    if (n->key == key) {
      n->value = value;
      return;
    }
    else {
      n = n->next;
    }
  }
  n = GC_MALLOC(sizeof(node));
  n->key = key;
  n->value = value;
  n->next = st->chains[h];
  st->chains[h] = n;
}

symbol *symtab_lookup(symtab *st, string key) {
  unsigned int h = hash(key) % st->size;
  node *n = st->chains[h];
  while (n != NULL) {
    if (n->key == key) {
      return n->value;
    }
    else n = n->next;
  }
  return NULL;
}

void symtab_traverse(symtab *st, void (*visit)(string key, symbol *value)) {
  for (int i = 0; i < st->size; i++) {
    node *n = st->chains[i];
    while (n != NULL) {
      visit(n->key, n->value);
      n = n->next;
    }
  }
}
