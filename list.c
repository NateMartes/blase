#include <gc/gc.h>
#include "list.h"

struct list {
  void *car;
  struct list *cdr;
};

list *cons(void *car, struct list *cdr) {
  list *lst = GC_MALLOC(sizeof(list));
  lst->car = car;
  lst->cdr = cdr;
  return lst;
}

void *car(list *lst) {
  return lst->car;
}

void *cdr(list *lst) {
  return lst->cdr;
}
