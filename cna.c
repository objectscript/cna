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
    fprintf(stderr, "ZARRAYP size must be equal to pointer size\n");
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
    fprintf(stderr, "LOAD_LIBRARY failed\n");
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
    fprintf(stderr, "FREE_LIBRARY failed\n");\
    return ZF_FAILURE;
  }
  return ZF_SUCCESS;
}


// int
// call_function_from_dynamic_library(const char *libname, const char *funcname, int nargs, int *args, int *retval)
// {

//   ffi_cif cif;
//   ffi_type *argtypes[nargs];
//   void *argvalues[nargs];

//   int i;

//   for (i = 0; i < nargs; ++i) {
//     argtypes[i] = &ffi_type_sint;
//     argvalues[i] = &args[i];
//   }

//   void *libhandle = dlopen(libname, RTLD_LAZY);

//   if (!libhandle) {
//     fprintf(stderr, "dlopen() failed.");
//     return ZF_FAILURE;
//   } 

//   if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, nargs, &ffi_type_sint, argtypes) != FFI_OK) {
//     fprintf(stderr, "ffi_prep_cif() failed.");
//     return ZF_FAILURE;
//   }

//   void *funcpointer = dlsym(libhandle, funcname);

//   if (!funcpointer) {
//     fprintf(stderr, "dlsym() failed.");
//     return ZF_FAILURE;
//   }

//   ffi_call(&cif, funcpointer, retval, argvalues);

//   dlclose(libhandle);

//   return ZF_SUCCESS;
// }

ZFBEGIN
ZFENTRY("load_library", "cB", load_library)
ZFENTRY("free_library", "b", free_library)
ZFEND
