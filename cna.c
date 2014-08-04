#define ZF_DLL

#include <cdzf.h>
#include <ffi.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <limits.h>

#include "storage.h"

#ifdef _WIN32
  #include <windows.h>
  
  #define DEFAULT_LOAD_OPTS LOAD_WITH_ALTERED_SEARCH_PATH
  #define LOAD_LIBRARY(NAME,OPTS) (NAME ? LoadLibraryEx(NAME, NULL, OPTS) : GetModuleHandle(NULL))
  #define FREE_LIBRARY(HANDLE) (((HANDLE)==GetModuleHandle(NULL) || FreeLibrary(HANDLE))?0:-1)
  #define FIND_ENTRY(HANDLE, NAME) GetProcAddress(HANDLE, NAME)
#else /* UNIX */
  #include <dlfcn.h>
  
  #define DEFAULT_LOAD_OPTS (RTLD_LAZY|RTLD_GLOBAL)
  #define LOAD_LIBRARY(NAME,OPTS) dlopen(NAME, OPTS)
  #define FREE_LIBRARY(HANDLE) dlclose(HANDLE)
  #define FIND_ENTRY(HANDLE, NAME) dlsym(HANDLE, NAME)
#endif /* _WIN32 */

void
logger(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  
  #ifdef _WIN32
    char path[MAX_PATH];
    HMODULE hm;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR) &logger, &hm);
    GetModuleFileNameA(hm, path, sizeof(path));
    strcpy(strrchr(path, '\\') + 1, "log.txt");

    FILE *fd = fopen(path, "a");
    time_t now;
    time(&now);
    fprintf(fd, "%s\t", ctime(&now));
    vfprintf(fd, format, args);
    fclose(fd);
  #else
    vprintf(format, args);
  #endif /* _WIN32 */
  
  va_end(args);
}


int
assign_pointer_to_ZARRAYP(ZARRAYP a, void *p)
{
  a->len = sizeof(p);
  memcpy(a->data, (void *)&p, a->len);
  return 0;
}

int
assign_ZARRAYP_to_pointer(void **p, ZARRAYP a)
{
  if (a->len != sizeof(*p)) {
    logger("ZARRAYP size must be equal to pointer size\n");
    return 1;
  }
  memcpy(p, a->data, a->len);
  return 0;
}

int
load_library(const char *libname, ZARRAYP retval)
{
  void *handle = (void *)LOAD_LIBRARY(libname, DEFAULT_LOAD_OPTS);
  if (!handle) {
    logger("LOAD_LIBRARY failed\n");
    return ZF_FAILURE;
  }
  assign_pointer_to_ZARRAYP(retval, handle);
  return ZF_SUCCESS;
}

int
free_library(ZARRAYP libID)
{
  void *handle;
  if (assign_ZARRAYP_to_pointer(&handle, libID)) {
    return ZF_FAILURE;
  }
  if (FREE_LIBRARY(handle)) {
    logger("FREE_LIBRARY failed\n");\
    return ZF_FAILURE;
  }
  return ZF_SUCCESS;
}

enum TYPE {
  CNA_VOID = 0,
  CNA_UCHAR = 1,
  CNA_SCHAR = 2,
  CNA_UINT = 3,
  CNA_INT = 4,
  CNA_USHORT = 5,
  CNA_SHORT = 6,
  CNA_ULONG = 7,
  CNA_LONG = 8, 
  CNA_UINT64 = 9,
  CNA_INT64 = 10,
  CNA_FLOAT = 11,
  CNA_DOUBLE = 12,
  CNA_LONGDOUBLE = 13,
  CNA_POINTER = 14,
  CNA_SIZET = 15,
  CNA_STRUCT = 16
};

static const size_t ntypes = 17;

inline unsigned char
get_size(enum TYPE type)
{
  switch (type) {
    case CNA_VOID:       return 0;
    case CNA_UCHAR:      return sizeof(unsigned char);
    case CNA_SCHAR:      return sizeof(signed char);
    case CNA_UINT:       return sizeof(unsigned int);
    case CNA_INT:        return sizeof(int);
    case CNA_USHORT:     return sizeof(unsigned short);
    case CNA_SHORT:      return sizeof(short);
    case CNA_ULONG:      return sizeof(unsigned long);
    case CNA_LONG:       return sizeof(long);
    case CNA_UINT64:     return sizeof(uint64_t);
    case CNA_INT64:      return sizeof(int64_t);
    case CNA_FLOAT:      return sizeof(float);
    case CNA_DOUBLE:     return sizeof(double);
    case CNA_LONGDOUBLE: return sizeof(long double);
    case CNA_POINTER:    return sizeof(void *);
    case CNA_SIZET:      return sizeof(size_t);
    default:             return 0;
  }
}

int
get_sizes(ZARRAYP retval)
{
  retval->len = ntypes;
  size_t i;
  for (i = 0; i < ntypes; ++i) {
    retval->data[i] = get_size(i);
  }
  return ZF_SUCCESS;
}


ffi_type *
create_ffi_struct(ZARRAYP args, int *i, storage *mem);


ffi_type *
get_ffi_type(ZARRAYP types, int *i, storage *mem)
{
  if (*i >= types->len) {
    logger("Invalid index in get_ffi_type()\n");
    return NULL;
  }
  switch (types->data[*i]) {
    case CNA_VOID:       return &ffi_type_void;
    case CNA_UCHAR:      return &ffi_type_uchar;     
    case CNA_SCHAR:      return &ffi_type_schar;     
    case CNA_UINT:       return &ffi_type_uint;     
    case CNA_INT:        return &ffi_type_sint;     
    case CNA_USHORT:     return &ffi_type_ushort;     
    case CNA_SHORT:      return &ffi_type_sshort;     
    case CNA_ULONG:      return &ffi_type_ulong;     
    case CNA_LONG:       return &ffi_type_slong;     
    case CNA_UINT64:     return &ffi_type_uint64;     
    case CNA_INT64:      return &ffi_type_sint64;     
    case CNA_FLOAT:      return &ffi_type_float;     
    case CNA_DOUBLE:     return &ffi_type_double;     
    case CNA_LONGDOUBLE: return &ffi_type_longdouble;     
    case CNA_POINTER:    return &ffi_type_pointer;     
    case CNA_STRUCT:     return create_ffi_struct(types, i, mem);
    case CNA_SIZET:
      switch (get_size(CNA_SIZET)) {
        case 1: return &ffi_type_uint8; 
        case 2: return &ffi_type_uint16;
        case 4: return &ffi_type_uint32;
        case 8: return &ffi_type_uint64;
        default: logger("Unsupported size of size_t\n"); return NULL;
      }

    default:
      logger("Unknown data type\n");
      return NULL;
    }
}


ffi_type *
create_ffi_struct(ZARRAYP args, int *i, storage *mem)
{

  ffi_type *st = alloc(mem, sizeof(ffi_type));
  
  if (*i >= args->len - 1) {
    logger("Invalid index\n");
    return NULL;
  }
  
  ++*i;

  unsigned char j, n = args->data[*i];
  
  st->type = FFI_TYPE_STRUCT;
  st->size = 0;
  st->alignment = 0;
  st->elements = alloc(mem, sizeof(ffi_type *) * (n + 1));

  ++*i;

  for (j = 0; j < n; ++j, ++*i) {
    st->elements[j] = get_ffi_type(args, i, mem);
    if (!st->elements[j]) {
      return NULL;
    }
    if (st->elements[j] == &ffi_type_void) {
      logger("You can not use CNA_VOID in structures\n");
      return NULL;
    }
  }

  --*i;

  st->elements[j] = NULL;
  return st;
}

int
double_to_float(ZARRAYP val, ZARRAYP retval)
{
  if (val->len != sizeof(double)) {
    logger("Wrong size of input ZARRAYP\n");
    return ZF_FAILURE;
  }
  retval->len = sizeof(float);
  *((float *)retval->data) = *((double *)val->data);
  return ZF_SUCCESS;
}

int
double_to_long_double(ZARRAYP val, ZARRAYP retval)
{
  if (val->len != sizeof(double)) {
    logger("Wrong size of input ZARRAYP\n");
    return ZF_FAILURE;
  }
  retval->len = sizeof(long double);
  *((long double *)retval->data) = (long double)*((double *)val->data);
  return ZF_SUCCESS;
}

int
float_to_double(ZARRAYP val, ZARRAYP retval)
{
  if (val->len != sizeof(float)) {
    logger("Wrong size of input ZARRAYP\n");
    return ZF_FAILURE;
  }
  retval->len = sizeof(double);
  *((double *)retval->data) = *((float *)val->data);
  return ZF_SUCCESS;
}

int
long_double_to_double(ZARRAYP val, ZARRAYP retval)
{
  if (val->len != sizeof(long double)) {
    logger("Wrong size of input ZARRAYP\n");
    return ZF_FAILURE;
  }
  retval->len = sizeof(double);
  *((double *)retval->data) = *((long double *)val->data);
  return ZF_SUCCESS;
}

int
call_function(ZARRAYP libID, const char *funcname, ZARRAYP argtypes, ZARRAYP args, ZARRAYP retval)
{
  /* Last value in argtypes and ffi_types is the type of "funcname" return value */ 
  int maxargs = argtypes->len - 1, nargs;
  ffi_cif cif;
  ffi_type *ffi_types[maxargs + 1]; 
  void *ffi_values[maxargs];

  int i, j;
  size_t fullsize = 0, size;
  storage mem;
  init_storage(&mem);


  for (i = 0, j = 0; i < maxargs + 1; ++j, ++i) {
    size = get_size(argtypes->data[i]);
    ffi_types[j] = get_ffi_type(argtypes, &i, &mem);
    if (!ffi_types[j]) {
      return ZF_FAILURE;
    }
    if (ffi_types[j] == &ffi_type_void && i != maxargs) {
      logger("CNA_VOID type may be used only for return value\n");
      return ZF_FAILURE;
    }
    if (size == 0 && ffi_types[j] != &ffi_type_void && i != maxargs) {
      size = *((size_t *)(args->data + fullsize));
      fullsize += sizeof(size_t);
    }
    if (i != maxargs) {
      ffi_values[j] = args->data + fullsize;
    }
    fullsize += size;
  }

  retval->len = size;
  nargs = j - 1;

  if (fullsize != args->len + retval->len) {
    logger("Wrong size of ZARRAYP\n\tfullsize: %u\tZARRAYP args: %u\tretsize: %u\n", fullsize, args->len, retval->len);
    return ZF_FAILURE;
  }

  void *handle;
  if (assign_ZARRAYP_to_pointer(&handle, libID)) {
    return ZF_FAILURE;
  }

  if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, nargs, ffi_types[nargs], ffi_types) != FFI_OK) {
    logger("ffi_prep_cif() failed\n");
    return ZF_FAILURE;
  }
  
  if (retval->len == 0 && ffi_types[nargs] != &ffi_type_void) {
    retval->len = cif.arg_types[nargs]->size;
  }

  void *funcpointer = FIND_ENTRY(handle, funcname);

  if (!funcpointer) {
    logger("FIND_ENTRY() failed\n\thandle:%d\tfuncname:%s\n", handle, funcname);
    return ZF_FAILURE;
  }

  ffi_call(&cif, funcpointer, retval->data, ffi_values);

  /* TODO: handle error */

  free_storage(&mem);

  return ZF_SUCCESS;
}

int
string_to_pointer(char *s, ZARRAYP p)
{
  char *copy = (char *)malloc((strlen(s) + 1) * sizeof(char));
  strcpy(copy, s);
  assign_pointer_to_ZARRAYP(p, copy);
  return ZF_SUCCESS;
}

int
free_pointer(ZARRAYP p)
{
  void *s;
  if (assign_ZARRAYP_to_pointer(&s, p)) {
    return ZF_FAILURE;
  }
  free(s);
  return ZF_SUCCESS;
}

int
pointer_set_at(ZARRAYP p, ZARRAYP ztype, ZARRAYP index, ZARRAYP value)
{
  void *array;
  if (p->len != sizeof(void *)) {
    logger("Wrong size of ZARRAY 'pointer'\n");
    return ZF_FAILURE;
  }
  assign_ZARRAYP_to_pointer(&array, p);
  
  if (ztype->len != sizeof(unsigned char)) {
    logger("Wrong size of ZARRAY 'type'\n");
    return ZF_FAILURE;
  }
  unsigned char type = *((unsigned char *)ztype->data);
  size_t size = get_size(type);
  if (value->len != size) {
    logger("Wrong size of ZARRAY 'value'\n");
    return ZF_FAILURE;
  }

  if (index->len != sizeof(size_t)) {
    logger("Wrong size of ZARRAY index\n");
    return ZF_FAILURE;
  }  


  void *address = array + (*((size_t *)index->data)) * size;
  memcpy(address, value->data, size);
  return ZF_SUCCESS;
}

int
pointer_get_at(ZARRAYP p, ZARRAYP ztype, ZARRAYP index, ZARRAYP value)
{
  void *array;
  if (p->len != sizeof(void *)) {
    logger("Wrong size of ZARRAY 'pointer'\n");
    return ZF_FAILURE;
  }
  assign_ZARRAYP_to_pointer(&array, p);
  
  if (ztype->len != sizeof(unsigned char)) {
    logger("Wrong size of ZARRAY 'type'\n");
    return ZF_FAILURE;
  }
  unsigned char type = *((unsigned char *)ztype->data);
  size_t size = get_size(type);

  if (index->len != sizeof(size_t)) {
    logger("Wrong size of ZARRAY 'index'\n");
    return ZF_FAILURE;
  }  


  void *address = array + (*((size_t *)index->data)) * size;
  value->len = size;
  memcpy(value->data, address, size);  return ZF_SUCCESS;
}

int
append_to_pointer(ZARRAYP pointer, ZARRAYP n, char *string, ZARRAYP retval)
{
  void *mem;
  if (pointer->len != sizeof(void *)) {
    logger("Wrong size of ZARRAY 'pointer'\n");
    return ZF_FAILURE;
  }
  assign_ZARRAYP_to_pointer(&mem, pointer);
  if (n->len != sizeof(size_t)) {
    logger("Wrong size of ZARRAY 'n'\n");
    return ZF_FAILURE;
  }
  
  size_t oldsize = *((size_t *)n->data);
  
  mem = realloc(mem, oldsize + strlen(string) + 1);
  if (!mem) {
    logger("Can't allocate memory for appending to pointer");
    return ZF_FAILURE;
  }
  strcpy(mem + oldsize, string);

  retval->len = sizeof(void *);
  assign_pointer_to_ZARRAYP(retval, mem);
  return ZF_SUCCESS;
}


ZFBEGIN
ZFENTRY("get_sizes", "B", get_sizes)

ZFENTRY("double_to_float", "bB", double_to_float)
ZFENTRY("double_to_long_double", "bB", double_to_long_double)
ZFENTRY("float_to_double", "bB", float_to_double)
ZFENTRY("long_double_to_double", "bB", long_double_to_double)

ZFENTRY("call_function", "bcbbB", call_function)
ZFENTRY("load_library", "cB", load_library)
ZFENTRY("free_library", "b", free_library)

ZFENTRY("string_to_pointer", "cB", string_to_pointer)
ZFENTRY("append_to_pointer", "bbcB", append_to_pointer)
ZFENTRY("free_pointer", "b", free_pointer)
ZFENTRY("pointer_set_at", "bbbb", pointer_set_at)
ZFENTRY("pointer_get_at", "bbbB", pointer_get_at)

ZFEND
