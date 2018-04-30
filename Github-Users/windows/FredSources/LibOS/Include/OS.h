// OS.h

#ifndef _C_OS_H_
#define _C_OS_H_

#include <string.h>
#include <stdio.h>

#define PAGE_SIZE 0x1000

/**********************************************************/
/* Mise en commun des Includes                            */ 
/**********************************************************/
#if ( defined(WIN32) || defined(WIN64) || defined(__MINGW32__) || defined(__MINGW64__) )
	#include <assert.h>
	#include "Windows.h"
	#include <process.h>
	#include <winsock.h>
	#if (!defined(__MINGW32__) && !defined (__MINGW64__))
		#pragma comment(lib, "wsock32.lib")
	#endif
#else 
	#include <stdlib.h>
	#ifdef _DEBUG
        #ifndef assert
            #define assert(test)  if (!(test)) { fprintf(stderr,"Assertion error (%s)",#test); abort();}
        #endif
	#else
        #ifndef assert
            #define assert(test)
        #endif
	#endif
	#include <sys/timeb.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <netinet/in.h> 
	#include <arpa/inet.h>
	#include <fcntl.h>
	#include <pthread.h>
	#include <unistd.h>
	#include <errno.h>
	#include <strings.h>
	#include <sys/socket.h>
	#include <netinet/tcp.h>
	#include <sys/mman.h>
	#include <limits.h>    /* for PAGESIZE */
#endif

#ifdef __cplusplus
	extern "C"
	{
#endif
/**********************************************************/
/* Mise en commun des types                               */ 
/**********************************************************/
#if ((defined(LIN32) || defined (LIN64)) &&  (!defined (__MINGW32__)&&  !defined (__MINGW64__)))
    void Sleep(unsigned long ul_millisecond);
	typedef unsigned int SOCKET ;
    typedef unsigned long DWORD ;
	typedef  void * LPVOID ;
	typedef  unsigned long * ULONG_PTR;
	typedef  unsigned long ULONG;
	typedef  unsigned int BOOLEAN;
    typedef void* (*PTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
	typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
    #define LPSECURITY_ATTRIBUTES pthread_attr_t *
	#define closesocket close
	#define ZeroMemory bzero
	#define CopyMemory memcpy
	#define SOCKET_ERROR -1

	typedef  long LONG;
	typedef  void * HWND;
	typedef  unsigned int UINT;
	typedef  unsigned long WPARAM;
	typedef  unsigned long LPARAM;

	#ifndef TRUE
		#define TRUE 1
	#endif // TRUE
	#ifndef FALSE
		#define FALSE 0
	#endif // FALSE
	#define INFINITE  0xFFFFFFFF
	#ifndef ETIMEDOUT 
		#define ETIMEDOUT 110
	#endif
    #define HANDLE int
	#define NO_ERROR 0
#else
	#ifndef socklen_t
		typedef int socklen_t;
	#endif
	#define	MSG_DONTWAIT 0
	#define pthread_t DWORD 
#endif


/**********************************************************/
/* Gestion des mutex                                      */ 
/**********************************************************/
#define OS_WAIT_TIMEOUT  0xFFFFFFFF
#define OS_WAIT_ERROR    0xEEEEEEEE
#define OS_WAIT_OK       0x00000000

#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) )
	typedef HANDLE OS_mutex;
#else
	typedef pthread_mutex_t OS_mutex ;
#endif

void OS_CreateMutex(OS_mutex * s_mutex);
void OS_DestroyMutex(OS_mutex * s_mutex);
void OS_LockMutex(OS_mutex * s_mutex);
void OS_UnlockMutex(OS_mutex * s_mutex);

/**********************************************************/
/* Gestion des evenements                                 */ 
/**********************************************************/
#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
	typedef HANDLE OS_event;
#else
	typedef struct
	{
		pthread_mutex_t condition_mutex;
		pthread_cond_t  condition_cond ; 
		BOOLEAN  triggered;
	} OS_event ;
#endif

void OS_CreateEvent(OS_event * s_event);
void OS_DestroyEvent(OS_event * s_event);
void OS_SetEvent(OS_event * s_event);
void OS_ResetEvent(OS_event * s_event);
int  OS_WaitEvent(OS_event * s_event,unsigned long ul_timeout);

/**********************************************************/
/* Gestion des Threads                                    */ 
/**********************************************************/
HANDLE OS_CreateThread(LPSECURITY_ATTRIBUTES lpsa, unsigned long dwStackSize, LPTHREAD_START_ROUTINE pFunction, void* argument, unsigned long dwCreationFlags, pthread_t* pThreadId);
unsigned long OS_TerminateThread(pthread_t * pThreadId);
unsigned long OS_JoinThread(pthread_t * pThreadId);

/**********************************************************/
/* Gestion des Sockets                                    */ 
/**********************************************************/
void OS_WSAStartup();
void OS_WSACleanup();

/**********************************************************/
/* Gestion des MessagesThread                             */ 
/**********************************************************/
#if ((defined(LIN32) || defined (LIN64)) &&  (!defined (__MINGW32__)&&  !defined (__MINGW64__)))
	typedef struct tagPOINT
	{
		LONG  x;
		LONG  y;
	} POINT;

	typedef struct tagMSG {
		HWND        hwnd;
		UINT        message;
		WPARAM      wParam;
		LPARAM      lParam;
		DWORD       time;
		POINT       pt;
	} MSG,*LPMSG;

	#define WM_USER                         0x0400
	/*
	 * PeekMessage() Options
	 */
	#define QS_KEY              0x0001
	#define QS_MOUSEMOVE        0x0002
	#define QS_MOUSEBUTTON      0x0004
	#define QS_POSTMESSAGE      0x0008
	#define QS_TIMER            0x0010
	#define QS_PAINT            0x0020
	#define QS_SENDMESSAGE      0x0040
	#define QS_HOTKEY           0x0080
	#define QS_ALLPOSTMESSAGE   0x0100
	#define QS_RAWINPUT         0x0400

	#define QS_MOUSE           (QS_MOUSEMOVE     | \
								QS_MOUSEBUTTON)

	#define QS_INPUT           (QS_MOUSE         | \
								QS_KEY           | \
								QS_RAWINPUT)

	#define QS_ALLEVENTS       (QS_INPUT         | \
								QS_POSTMESSAGE   | \
								QS_TIMER         | \
								QS_PAINT         | \
								QS_HOTKEY)

	#define QS_ALLINPUT        (QS_INPUT         | \
								QS_POSTMESSAGE   | \
								QS_TIMER         | \
								QS_PAINT         | \
								QS_HOTKEY        | \
								QS_SENDMESSAGE)


	#define PM_NOREMOVE         0x0000
	#define PM_REMOVE           0x0001
	#define PM_NOYIELD          0x0002

	#define PM_QS_INPUT         (QS_INPUT << 16)
	#define PM_QS_POSTMESSAGE   ((QS_POSTMESSAGE | QS_HOTKEY | QS_TIMER) << 16)
	#define PM_QS_PAINT         (QS_PAINT << 16)
	#define PM_QS_SENDMESSAGE   (QS_SENDMESSAGE << 16)
#endif

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	pthread_t OS_GetCurrentThreadID();
	BOOLEAN OS_PostThreadMessage(pthread_t idThread,UINT Msg,WPARAM wParam,LPARAM lParam);
	BOOLEAN OS_GetMessage(LPMSG lpMsg, HWND hWnd, unsigned int wMsgFilterMin, unsigned int wMsgFilterMax);
	BOOLEAN OS_PeekMessage(LPMSG lpMsg,HWND hWnd, unsigned int wMsgFilterMin,unsigned int wMsgFilterMax,unsigned int wRemoveMsg);

#ifdef __cplusplus
}
#endif //__cplusplus




/**********************************************************/
/* Gestion des pages mémoires                             */ 
/**********************************************************/
#ifndef PAGESIZE
	#define PAGESIZE 4096
#endif
#if ((defined(LIN32) || defined (LIN64)) &&  (!defined (__MINGW32__)&&  !defined (__MINGW64__)))
	#define OS_PAGE_NOACCESS			PROT_NONE               
	#define OS_PAGE_READONLY			PROT_READ   
	#define OS_PAGE_READWRITE			(PROT_READ | PROT_WRITE)             
	#define OS_PAGE_WRITECOPY			PROT_WRITE   
	#define OS_PAGE_EXECUTE				PROT_EXEC                
	#define OS_PAGE_EXECUTE_READ		(PROT_READ | PROT_EXEC)          
	#define OS_PAGE_EXECUTE_READWRITE   ( PROT_READ | PROT_WRITE | PROT_EXEC)
	#define OS_PAGE_EXECUTE_WRITECOPY	( PROT_WRITE | PROT_EXEC )

	#define OS_MEM_COMMIT               0x1
	#define OS_MEM_RESERVE		        0x2
	#define OS_MEM_RESET                0x4
	#define OS_MEM_RESET_UNDO           0x8
#else
	#define OS_PAGE_NOACCESS			PAGE_NOACCESS               
	#define OS_PAGE_READONLY			PAGE_READONLY   
	#define OS_PAGE_READWRITE			PAGE_READWRITE              
	#define OS_PAGE_WRITECOPY			PAGE_WRITECOPY   
	#define OS_PAGE_EXECUTE				PAGE_EXECUTE                
	#define OS_PAGE_EXECUTE_READ		PAGE_EXECUTE_READ           
	#define OS_PAGE_EXECUTE_READWRITE   PAGE_EXECUTE_READWRITE
	#define OS_PAGE_EXECUTE_WRITECOPY	PAGE_EXECUTE_WRITECOPY 

	#define OS_MEM_COMMIT		MEM_COMMIT
	#define OS_MEM_RESERVE		MEM_RESERVE
	#define OS_MEM_RESET		MEM_RESET
	#define OS_MEM_RESET_UNDO	MEM_RESET_UNDO
#endif
int OS_MemoryProtect(LPVOID lpAddress, size_t dwSize, DWORD prot,DWORD *old_prot);
LPVOID OS_MemoryAllocation(LPVOID lpAddress, size_t dwSize, DWORD flAllocationType, DWORD flProtect);
int OS_MemoryRelease(LPVOID lpAddress, size_t dwSize);


#ifdef __cplusplus
	}
#endif
#endif
