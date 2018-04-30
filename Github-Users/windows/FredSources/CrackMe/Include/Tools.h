#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <stdio.h>

#define REALLOC_STEP 16

size_t my_fgets(FILE * fichier,char ** buffer);

#ifdef OBFUSCATED

#define my_fgets_ my_fgets
#undef my_fgets
#define my_fgets(...) (OBFUSCATED_CALL_RET(my_fgets_,__VA_ARGS__))

#endif

#endif
