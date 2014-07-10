#define ZF_DLL

#include <cdzf.h>
#include <ffi.h>
#include <dlfcn.h>
#include <stdio.h>

#include "cna.h"

int call_function_from_dynamic_library(const char *libname, const char *funcname, int nargs, int *args, int *retval)
{

	ffi_cif cif;
	ffi_type *argtypes[nargs];
	void *argvalues[nargs];

	int i;

	for (i = 0; i < nargs; ++i) {
		argtypes[i] = &ffi_type_sint;
		argvalues[i] = &args[i];
	}

	void *libhandle = dlopen(libname, RTLD_LAZY);

	if (!libhandle) {
		fprintf(stderr, "dlopen() failed.");
		return ZF_FAILURE;
	} 

	if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, nargs, &ffi_type_sint, argtypes) != FFI_OK) {
		fprintf(stderr, "ffi_prep_cif() failed.");
		return ZF_FAILURE;
	}

	void *funcpointer = dlsym(libhandle, funcname);

	if (!funcpointer) {
		fprintf(stderr, "dlsym() failed.");
		return ZF_FAILURE;
	}

	ffi_call(&cif, funcpointer, retval, argvalues);

	dlclose(libhandle);

	return ZF_SUCCESS;
}

ZFBEGIN
ZFENTRY("call", "ccipP", call_function_from_dynamic_library)
ZFEND
