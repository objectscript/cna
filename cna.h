/* Cache Native Access */

#ifndef CNA_H
#define CNA_H

#define ZF_DLL
#include <cdzf.h>

int
load_library(const char *libname, ZARRAYP retval);

int
free_library(ZARRAYP libID);

int
call_function(ZARRAYP libID, const char *funcname, ZARRAYP argtypes, ZARRAYP args, ZARRAYP retval);


#endif /* CNA_H */
