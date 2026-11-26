#ifndef TYPE_H
#define TYPE_H

#define T_TAGS X(INTEGER) X(BOOLEAN) X(LABEL)

typedef enum {
#define X(Y) T_##Y,
  T_TAGS
#undef X
} ttag;

extern char *tnames[];

typedef struct {
  ttag tag;
} type;

type *type_new(ttag tag);

#endif
