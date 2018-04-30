#ifndef _C_MEMORY_PROTECTION_INTERNAL_H_
#define _C_MEMORY_PROTECTION_INTERNAL_H_

#include <MemoryProtection.h>
#include <signal.h>

#ifdef __cplusplus
	extern "C"
	{
#endif

#if ( defined(WIN32) || defined(WIN64) || defined(__MINGW32__) || defined(__MINGW64__) )
#include <windows.h>

#else
	#include <ucontext.h>
#endif

#include <types.h>
#include <extern.h>
#include <itab.h>


BOOLEAN __UndoAcces___default(ULONG_PTR _ProtectedMemoryAdress,ACCESS_MODE mode,size_t memorySize);
BOOLEAN __AccessControler___default(ULONG_PTR _ProtectedMemoryAdress,ACCESS_MODE mode,size_t memorySize);


#if ((defined(WIN32) ||  defined(WIN64)) && (!defined(__MINGW32__) && !defined(__MINGW64__)) )
	#if defined(WIN64)
		#define StartMagicFunction _StartMagicFunction
		#define EndMagicFunction   _EndMagicFunction
		#define restauration       _restauration
		#define getTrueEIP         _getTrueEIP
		#define getBuffer_FPU      _getBuffer_FPU
	#endif

	#define _CALL_C_DECL_ __cdecl
	#define STATIC_TLS	  __declspec( thread ) static

	typedef unsigned __int64 uint64_t;
	typedef unsigned __int32 uint32_t;
	typedef  __int64 int64_t;
	typedef  __int32 int32_t;

#elif (defined(LIN32) || defined (LIN64))
	//#ifdef LIN64
		#define StartMagicFunction _StartMagicFunction
		#define EndMagicFunction   _EndMagicFunction
		#define restauration       _restauration
		#define getTrueEIP         _getTrueEIP
		#define getBuffer_FPU      _getBuffer_FPU
	//#endif

	#ifdef LIN32
		#define _CALL_C_DECL_ __attribute__((__cdecl__))
	#else
		#define _CALL_C_DECL_
	#endif
	#define STATIC_TLS	static __thread
#else
	#error Un des flags : WIN64, LIN64, WIN32 , LIN32 doit etre défini
#endif

#if (defined(WIN64) || defined (LIN64))
	typedef  uint64_t uintPtrSize ;
	#define asm_used 64
#elif (defined(WIN32) || defined (LIN32))
	typedef  uint32_t uintPtrSize ;
	#define asm_used 32
#else
	#error Un des flags : WIN64, LIN64, WIN32 , LIN32 doit etre défini
#endif


void _CALL_C_DECL_  StartMagicFunction(void);
void _CALL_C_DECL_  EndMagicFunction(void);
void _CALL_C_DECL_  restauration(void);
ULONG_PTR _CALL_C_DECL_ getTrueEIP(void);
ULONG_PTR _CALL_C_DECL_ getBuffer_FPU(void);

#if ((defined(WIN32) ||  defined(WIN64)) || defined(__MINGW32__) || defined(__MINGW64__) )
	LONG WINAPI StructuredSignalHandlerSegmentViolation(PEXCEPTION_POINTERS pExceptionPtrs);
#else
	void StructuredSignalHandlerSegmentViolation (int sig, siginfo_t *siginfo, void *context);
#endif

extern BOOLEAN (*__AccessControler__)(ULONG_PTR,ACCESS_MODE,size_t) ;
extern BOOLEAN (*__UndoAcces__)(ULONG_PTR,ACCESS_MODE,size_t) ;
extern void (*__Read__)(ULONG_PTR,size_t) ;
extern void (*__Write__)(ULONG_PTR,size_t) ;
extern OS_mutex __LibMemProtect_mutex__ ;

#ifdef __cplusplus
	}
#endif

#endif
