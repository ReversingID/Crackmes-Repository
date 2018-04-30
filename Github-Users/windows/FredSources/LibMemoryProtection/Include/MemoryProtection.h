// MemoryProtection.h

#ifndef _C_MEMORY_PROTECTION_H_
#define _C_MEMORY_PROTECTION_H_

#include <OS.h>

typedef enum e_ACCESS_MODE {
	READ  = 0x00000000,
	WRITE = 0x00000001
} ACCESS_MODE;

#ifdef __cplusplus
	extern "C"
	{
#endif
	void startMemoryProtection(BOOLEAN (*_AccessControler)(ULONG_PTR,ACCESS_MODE,size_t) ,BOOLEAN (*_UndoAcces)(ULONG_PTR,ACCESS_MODE,size_t),void (*_Read)(ULONG_PTR,size_t),void (*_Write)(ULONG_PTR,size_t));
	void stopMemoryProtection();

#ifdef __cplusplus
	}
#endif

#endif
