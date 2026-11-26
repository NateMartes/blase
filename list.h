#ifndef LIST_H
#define LIST_H

typedef struct list list;

list *cons(void *car, struct list *cdr);
void *car(list *lst);
void *cdr(list *lst);

#endif
