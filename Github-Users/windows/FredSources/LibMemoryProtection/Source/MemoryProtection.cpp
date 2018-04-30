#include <MemoryProtection_internal.h>
#include <signal.h>

BOOLEAN (*__AccessControler__)(ULONG_PTR,ACCESS_MODE,size_t) ;
BOOLEAN (*__UndoAcces__)(ULONG_PTR,ACCESS_MODE,size_t) ;
void (*__Read__)(ULONG_PTR,size_t) ;
void (*__Write__)(ULONG_PTR,size_t) ;

#if (defined(__MINGW32__) || defined(__MINGW64__))
extern "C"
{
	PVOID WINAPI AddVectoredExceptionHandler(
		ULONG FirstHandler,
		LONG WINAPI (*VectoredHandler)(PEXCEPTION_POINTERS)
	);

	ULONG WINAPI RemoveVectoredExceptionHandler(
			LONG WINAPI (*VectoredHandler)(PEXCEPTION_POINTERS)
	);
}
#endif

#if ((defined(LIN32) || defined(LIN64)) && (!defined(__MINGW32__) && !defined(__MINGW64__)) )
static struct sigaction old_act;
#endif

void startMemoryProtection(BOOLEAN (*_AccessControler)(ULONG_PTR,ACCESS_MODE,size_t) ,BOOLEAN (*_UndoAcces)(ULONG_PTR,ACCESS_MODE,size_t),void (*_Read)(ULONG_PTR,size_t),void (*_Write)(ULONG_PTR,size_t))
{
	#if ((defined(WIN32) ||  defined(WIN64)) || defined(__MINGW32__) || defined(__MINGW64__) )
		AddVectoredExceptionHandler(SIGSEGV,StructuredSignalHandlerSegmentViolation);
	#else
		struct sigaction act;
		memset (&act, '\0', sizeof(act));
		/* Use the sa_sigaction field because the handles has two additional parameters */
		act.sa_sigaction = &StructuredSignalHandlerSegmentViolation;
		/* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
		act.sa_flags = SA_SIGINFO;
		sigaction(SIGSEGV, &act, &old_act);
	#endif
	if (_AccessControler == NULL)
	{
		__AccessControler__ = __AccessControler___default;
	}
	else
	{
		__AccessControler__ = _AccessControler;
	}
	if (_UndoAcces == NULL)
	{
		__UndoAcces__ = __UndoAcces___default;
	}
	else
	{
		__UndoAcces__ = _UndoAcces;
	}
	__Read__  = _Read;
	__Write__ = _Write;
}

void stopMemoryProtection()
{
	#if ((defined(WIN32) ||  defined(WIN64)) || defined(__MINGW32__) || defined(__MINGW64__) )
		RemoveVectoredExceptionHandler(StructuredSignalHandlerSegmentViolation);
	#else
		sigaction(SIGSEGV, &old_act, NULL);
		memset (&old_act, '\0', sizeof(old_act));
	#endif
	__AccessControler__ = __AccessControler___default;
	__UndoAcces__ = __UndoAcces___default;
	__Read__  = NULL;
	__Write__ = NULL;
}
