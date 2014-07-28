#ifndef STORAGE_H
#define STORAGE_H

#include <stdlib.h>

typedef struct _storage {
  void **data;
  size_t n;
  size_t allocated;
} storage;

void
init_storage(storage *x);

void *
alloc(storage *x, size_t size);

void
free_storage(storage *x);
  
#endif