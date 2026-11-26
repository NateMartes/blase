#include <stdio.h>
#include <string.h>
#include <gc/gc.h>
#include "pool.h"

#define SIZE 1013

typedef struct node {
  string atom;
  struct node *next;
} node;

node *chains[SIZE] = { NULL };

static unsigned int hash(char *s) {
  unsigned int h = 0;
  while (*s) {
    h = h * 117 + (*s++);
  }
  return h % SIZE;
}

string intern(char *s) {
  unsigned int h = hash(s);
  node *n = chains[h];
  while (n != NULL) {
    if (strcmp(n->atom, s) == 0) {
      return n->atom;
    }
    else {
      n = n->next;
    }
  }
  chains[h] = GC_MALLOC(sizeof(node));
  chains[h]->atom = GC_MALLOC(strlen(s) + 1);
  strcpy(chains[h]->atom, s);
  return chains[h]->atom;
}
      
