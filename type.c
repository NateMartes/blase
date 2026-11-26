#include <gc/gc.h>
#include "type.h"

char *tnames[] = {
  #define X(Y) #Y,
  T_TAGS
  #undef X
};

type *type_new(ttag tag) {
  type *t = GC_MALLOC(sizeof(type));
  t->tag = tag;
  return t;
}
