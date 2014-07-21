#define ZF_DLL

#include <cdzf.h>
#include <ffi.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "cna.h"

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

typedef void * HANDLE;

void
logger(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  
  #ifdef _WIN32
    /* TODO: write to the RIGHT place */
    FILE *fd = fopen("C:\\Users\\dd\\Dropbox\\is\\CNA\\log.txt", "a");
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
  logger("load_library():\n");
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
  logger("free_library():\n");
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
};

const int ntypes = 16;

inline unsigned char
get_size(enum TYPE type)
{
  switch (type) {
    case CNA_VOID: return 0;
    case CNA_UCHAR: return sizeof(unsigned char);
    case CNA_SCHAR: return sizeof(signed char);
    case CNA_UINT: return sizeof(unsigned int);
    case CNA_INT: return sizeof(int);
    case CNA_USHORT: return sizeof(unsigned short);
    case CNA_SHORT: return sizeof(short);
    case CNA_ULONG: return sizeof(unsigned long);
    case CNA_LONG: return sizeof(long);
    case CNA_UINT64: return sizeof(uint64_t);
    case CNA_INT64: return sizeof(int64_t);
    case CNA_FLOAT: return sizeof(float);
    case CNA_DOUBLE: return sizeof(double);
    case CNA_LONGDOUBLE: return sizeof(long double);
    case CNA_POINTER: return sizeof(void *);
    case CNA_SIZET: return sizeof(size_t);
    default: return 0;
  }
}

int
get_sizes(ZARRAYP retval)
{
  retval->len = ntypes;
  int i;
  for (i = 0; i < ntypes; ++i) {
    retval->data[i] = get_size(i);
  }
  return ZF_SUCCESS;
}

int
call_function(ZARRAYP libID, const char *funcname, ZARRAYP argtypes, ZARRAYP args, ZARRAYP retval)
{
  logger("call_function():\n");

  /* Last value in argtypes and ffi_types is the type of "funcname" return value */ 
  int nargs = argtypes->len - 1;
  ffi_cif cif;
  ffi_type *ffi_types[nargs + 1]; 
  void *ffi_values[nargs];

  int i;
  size_t fullsize = 0, size;
  
  for (i = 0; i < nargs + 1; ++i) {
    switch (argtypes->data[i]) {
      case CNA_UCHAR:
        ffi_types[i] = &ffi_type_uchar;
        break;
      case CNA_SCHAR:
        ffi_types[i] = &ffi_type_schar;
        break;
      case CNA_UINT:
        ffi_types[i] = &ffi_type_uint;
        break;
      case CNA_INT:
        ffi_types[i] = &ffi_type_sint;
        break;
      case CNA_USHORT:
        ffi_types[i] = &ffi_type_ushort;
        break;
      case CNA_SHORT:
        ffi_types[i] = &ffi_type_sshort;
        break;
      case CNA_ULONG:
        ffi_types[i] = &ffi_type_ulong;
        break;
      case CNA_LONG:
        ffi_types[i] = &ffi_type_slong;
        break;
      case CNA_UINT64:
        ffi_types[i] = &ffi_type_uint64;
        break;
      case CNA_INT64:
        ffi_types[i] = &ffi_type_sint64;
        break;
      case CNA_FLOAT:
        ffi_types[i] = &ffi_type_float;
        break;
      case CNA_DOUBLE:
        ffi_types[i] = &ffi_type_double;
        break;
      case CNA_LONGDOUBLE:
        ffi_types[i] = &ffi_type_longdouble;
        break;
      case CNA_POINTER:
        ffi_types[i] = &ffi_type_pointer;
        break;
      case CNA_SIZET:
        switch (get_size(CNA_SIZET)) {
          case 1: ffi_types[i] = &ffi_type_uint8;  break;
          case 2: ffi_types[i] = &ffi_type_uint16; break;
          case 4: ffi_types[i] = &ffi_type_uint32; break;
          case 8: ffi_types[i] = &ffi_type_uint64; break;
          default: logger("Unsupported size of size_t\n"); return ZF_FAILURE;
        }
        break;
      case CNA_VOID: 
        if (i == nargs) {
          ffi_types[i] = &ffi_type_void;
          break;
        }
        /* There is no brake statement for purpose */
      default:
        logger("Unknown data type\n");
        return ZF_FAILURE;
    }
    size = get_size(argtypes->data[i]);
    ffi_values[i] = args->data + fullsize;
    fullsize += size;
  }

  retval->len = size;

  if (fullsize != args->len + retval->len) {
    logger("Wrong size of ZARRAYP\n\tfullsize: %u\tZARRAYP + retsize: %u\n", fullsize, args->len + retval->len);
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

  void *funcpointer = FIND_ENTRY(handle, funcname);

  if (!funcpointer) {
    logger("FIND_ENTRY() failed\n\thandle:%d\tfuncname:%s\n", handle, funcname);
    return ZF_FAILURE;
  }

  ffi_call(&cif, funcpointer, retval->data, ffi_values);

  /* TODO: handle error */

  return ZF_SUCCESS;
}

int
string_to_pointer(char *s, ZARRAYP p)
{
  char *copy = (char *)malloc(strlen(s) * sizeof(char));
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
  logger("pointer_set_at():\n\t%u\n", *((unsigned *)value->data));
  void *array;
  if (ztype->len != sizeof(unsigned char)) {
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
  logger("0x%x: %u\n",  address, *((unsigned *)address));
  return ZF_SUCCESS;
}

int
pointer_get_at(ZARRAYP p, ZARRAYP ztype, ZARRAYP index, ZARRAYP value)
{
  logger("pointer_get_at():\n");
  void *array;
  if (ztype->len != sizeof(unsigned char)) {
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
  memcpy(value->data, address, size);
  logger("0x%x: %u\n",  address, *((unsigned *)address));
  return ZF_SUCCESS;
}

ZFBEGIN
ZFENTRY("get_sizes", "B", get_sizes)
ZFENTRY("call_function", "bcbbB", call_function)
ZFENTRY("load_library", "cB", load_library)
ZFENTRY("free_library", "b", free_library)
ZFENTRY("string_to_pointer", "cB", string_to_pointer)
ZFENTRY("free_pointer", "b", free_pointer)
ZFENTRY("pointer_set_at", "bbbb", pointer_set_at)
ZFENTRY("pointer_get_at", "bbbB", pointer_get_at)
ZFEND
