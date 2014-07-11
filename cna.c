#define ZF_DLL

#include <cdzf.h>
#include <ffi.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cna.h"

#ifdef _WIN32
  #include <windows.h>
  
  #define DEFAULT_LOAD_OPTS LOAD_WITH_ALTERED_SEARCH_PATH
  #define LOAD_LIBRARY(NAME,OPTS) (NAME ? LoadLibraryExW(NAME, NULL, OPTS) : GetModuleHandleW(NULL))
  #define FREE_LIBRARY(HANDLE) (((HANDLE)==GetModuleHandleW(NULL) || FreeLibrary(HANDLE))?0:-1)
  #define FIND_ENTRY(HANDLE, NAME) GetProcAddress(HANDLE, NAME)
#else /* UNIX */
  #include <dlfcn.h>
  
  #define DEFAULT_LOAD_OPTS (RTLD_LAZY|RTLD_GLOBAL)
  #define LOAD_LIBRARY(NAME,OPTS) dlopen(NAME, OPTS)
  #define FREE_LIBRARY(HANDLE) dlclose(HANDLE)
  #define FIND_ENTRY(HANDLE, NAME) dlsym(HANDLE, NAME)
#endif /* _WIN32 */

typedef void * HANDLE;

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
    fprintf(stderr, "ZARRAYP size must be equal to pointer size\n");
    return 1;
  }
  memcpy(p, a->data, a->len);
  return 0;
}

int
load_library(const char *libname, ZARRAYP retval)
{
  printf("load_library():\n");
  void *handle = (void *)LOAD_LIBRARY(libname, DEFAULT_LOAD_OPTS);
  if (!handle) {
    fprintf(stderr, "LOAD_LIBRARY failed\n");
    return ZF_FAILURE;
  }
  assign_pointer_to_ZARRAYP(retval, handle);
  return ZF_SUCCESS;
}

int
free_library(ZARRAYP libID)
{
  printf("free_library():\n");
  void *handle;
  if (assign_ZARRAYP_to_pointer(&handle, libID)) {
    return ZF_FAILURE;
  }
  if (FREE_LIBRARY(handle)) {
    fprintf(stderr, "FREE_LIBRARY failed\n");\
    return ZF_FAILURE;
  }
  return ZF_SUCCESS;
}

enum TYPE {
  VOID = 0,
  UCHAR = 1,
  UINT = 2,
  USHORT = 3,
  ULONG = 4,
  INT64 = 5,
  FLOAT = 6,
  DOUBLE = 7,
  LONGDOUBLE = 8
};

int
call_function(ZARRAYP libID, const char *funcname, ZARRAYP argtypes, ZARRAYP args, ZARRAYP retval)
{
  printf("call_function():\n");

  /* Last value in argtypes and ffi_types is the type of "funcname" return value */ 
  int nargs = argtypes->len - 1;
  ffi_cif cif;
  ffi_type *ffi_types[nargs + 1]; 
  void *ffi_values[nargs];

  int i;
  size_t fullsize = 0, size;
  
  for (i = 0; i < nargs + 1; ++i) {
    switch (argtypes->data[i]) {
      case UCHAR:
        size = sizeof(unsigned char);
        ffi_types[i] = &ffi_type_uchar;
        break;
      case UINT:
        size = sizeof(unsigned int);
        ffi_types[i] = &ffi_type_uint;
        break;
      case USHORT:
        size = sizeof(unsigned short);
        ffi_types[i] = &ffi_type_ushort;
        break;
      case ULONG:
        size = sizeof(unsigned long);
        ffi_types[i] = &ffi_type_ulong;
        break;
      case INT64:
        size = sizeof(int64_t);
        ffi_types[i] = &ffi_type_sint64;
        break;
      case FLOAT:
        size = sizeof(float);
        ffi_types[i] = &ffi_type_float;
        break;
      case DOUBLE:
        size = sizeof(double);
        ffi_types[i] = &ffi_type_double;
        break;
      case LONGDOUBLE:
        size = sizeof(long double);
        ffi_types[i] = &ffi_type_longdouble;
        break;
      case VOID: 
        if (i == nargs) {
          size = 0;
          ffi_types[i] = &ffi_type_void;
          break;
        }
        /* There is no brake statement for purpose */
      default:
        fprintf(stderr, "Unknown data type\n");
        return ZF_FAILURE;
    }
    ffi_values[i] = args->data + fullsize;
    fullsize += size;
  }

  retval->len = size;

  if (fullsize != args->len + retval->len) {
    fprintf(stderr, "Wrong size of ZARRAYP\n");
    return ZF_FAILURE;
  }

  void *handle;
  if (assign_ZARRAYP_to_pointer(&handle, libID)) {
    return ZF_FAILURE;
  }

  if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, nargs, ffi_types[nargs], ffi_types) != FFI_OK) {
    fprintf(stderr, "ffi_prep_cif() failed\n");
    return ZF_FAILURE;
  }

  void *funcpointer = dlsym(handle, funcname);

  if (!funcpointer) {
    fprintf(stderr, "dlsym() failed\n");
    return ZF_FAILURE;
  }

  ffi_call(&cif, funcpointer, retval->data, ffi_values);

  /* TODO: handle error */

  return ZF_SUCCESS;
}

ZFBEGIN
ZFENTRY("call_function", "bcbbB", call_function)
ZFENTRY("load_library", "cB", load_library)
ZFENTRY("free_library", "b", free_library)
ZFEND
