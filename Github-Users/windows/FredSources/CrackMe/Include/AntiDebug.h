#ifndef __ANTI_DEBUG__H__
#define __ANTI_DEBUG__H__

#include "Singleton.h"
#include "MemoryProtection.h"

#if ((defined(WIN32) || defined (WIN64)) && (!defined(__MINGW32__) && !defined(__MINGW64__) ))
#pragma region SECTION_SECURE
#pragma section(".Secure")
#pragma section(".Secure$")
#pragma section(".Secure$ZZZ")
#pragma endregion 
#define MEMORY_PROTECT __declspec(allocate(".Secure$")) __declspec(align(PAGE_SIZE))
#ifdef WIN64
#define checkDebugger _checkDebugger
#endif

extern "C" unsigned int __cdecl  checkDebugger(void);

#else
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#ifdef LIN32
	#define _CALL_C_DECL_ __attribute__((__cdecl__))
#else
	#define _CALL_C_DECL_
#endif

#if (!defined(__MINGW32__) && !defined(__MINGW64__))
	unsigned int _CALL_C_DECL_ IsDebuggerPresent(void);
#endif
unsigned int _CALL_C_DECL_ checkDebugger(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#define MEMORY_PROTECT  __attribute__((section("Secure"))) __attribute__((aligned(PAGE_SIZE)))

#endif

typedef enum e_DebuggerStatus
{
	NO_DEBUGGER_INIT = 0x2F ,
	NO_DEBUGGER_INIT_VERROU  = 0xAF ,
	DEBUGGER_NO_INIT = 0x83 ,
	DEBUGGER_INIT = 0x18
} DebuggerStatus ;

typedef enum e_ACCESS_TIME {
	DO = 0x00000000,
	UNDO = 0x00000001
} ACCESS_TIME;

class AntiDebug : public Singleton_Static<AntiDebug>
{
	// Debut singleton
	friend class Singleton_Static<AntiDebug>;
	private :
		AntiDebug();
		clock_t RDTSC;
	public :
		~AntiDebug();
	// Fin singleton

	private :
		void decode_data( ACCESS_TIME time, ACCESS_MODE mode, ULONG_PTR adresseVar, size_t memorySize);

	protected :
		DebuggerStatus debuggerStatut;

	public :
		DebuggerStatus start();
		unsigned int inline check(unsigned int nbcheck=4);

		unsigned int check_memory(void);
		unsigned int various_rdtsc(void);
		void debugger();
		void noDebugger();

		BOOLEAN UndoAcces(ULONG_PTR _ProtectedMemoryAdress,ACCESS_MODE mode,size_t memorySize);
		BOOLEAN AccessControler(ULONG_PTR _ProtectedMemoryAdress,ACCESS_MODE mode,size_t memorySize);
		void readMemory(ULONG_PTR adresseVar,size_t memorySize);
		void writeMemory(ULONG_PTR adresseVar, size_t memorySize);
};

#endif
