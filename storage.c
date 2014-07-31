#include "storage.h"

#include <stdlib.h>
#include <string.h> 

void
init_storage(storage *x)
{
  x->n = 0;
  x->allocated = 10;
  x->data = malloc(sizeof(void *) * x->allocated);
}

void *
alloc(storage *x, size_t size)
{
  if (x->n == x->allocated) {
    x->allocated *= 2;
    x->data = realloc(x->data, sizeof(void *) * x->allocated);
  }

  return x->data[x->n++] = malloc(size);
}

void
free_storage(storage *x)
{ 
  size_t i;
  for (i = 0; i < x->n; ++i) {
    free(x->data[i]);
  }
  free(x->data);
  x->n = 0;
  x->allocated = 0;
}