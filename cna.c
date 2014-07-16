#define ZF_DLL

#include <cdzf.h>
#include <ffi.h>
#include <stdio.h>
#include <stdint.h>
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
assign_pointer_to_ZARRAYP(ZARRAYP a, const void *p)
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
  CNA_UINT = 2,
  CNA_USHORT = 3,
  CNA_ULONG = 4,
  CNA_INT64 = 5,
  CNA_FLOAT = 6,
  CNA_DOUBLE = 7,
  CNA_LONGDOUBLE = 8,
  CNA_POINTER = 9
};

const int ntypes = 10;

inline unsigned char
get_sizeof(enum TYPE type)
{
  switch (type) {
    case CNA_VOID: return 0;
    case CNA_UCHAR: return sizeof(unsigned char);
    case CNA_UINT: return sizeof(unsigned int);
    case CNA_USHORT: return sizeof(unsigned short);
    case CNA_ULONG: return sizeof(unsigned long);
    case CNA_INT64: return sizeof(int64_t);
    case CNA_FLOAT: return sizeof(float);
    case CNA_DOUBLE: return sizeof(double);
    case CNA_LONGDOUBLE: return sizeof(long double);
    case CNA_POINTER: return sizeof(void *);
    default: return 0;
  }
}

int
get_sizes(ZARRAYP retval)
{
  retval->len = ntypes;
  int i;
  for (i = 0; i < ntypes; ++i) {
    retval->data[i] = get_sizeof(i);
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
      case CNA_UINT:
        ffi_types[i] = &ffi_type_uint;
        break;
      case CNA_USHORT:
        ffi_types[i] = &ffi_type_ushort;
        break;
      case CNA_ULONG:
        ffi_types[i] = &ffi_type_ulong;
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
    size = get_sizeof(argtypes->data[i]);
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

ZFBEGIN
ZFENTRY("get_sizes", "B", get_sizes)
ZFENTRY("call_function", "bcbbB", call_function)
ZFENTRY("load_library", "cB", load_library)
ZFENTRY("free_library", "b", free_library)
ZFEND
