/* Cache Native Access */
#ifndef CNA_H
#define CNA_H

#include <cdzf.h>

int
load_library(const char *libname, ZARRAYP retval);

int
free_library(ZARRAYP libID);

#endif /* CNA_H */
