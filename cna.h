/* Cache Native Access */
#ifndef CNA_H
#define CNA_H

int call_function_from_dynamic_library(const char *libname, const char *funcname, int nargs, int *args, int *retval);

#endif /* CNA_H */
