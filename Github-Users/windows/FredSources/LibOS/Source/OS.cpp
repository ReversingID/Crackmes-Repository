
#include <OS.h>
#include <CTrace.h>
#include <stdint.h>


/**********************************************************/
/* Adaptation de la fonction Sleep Windows pour Unix      */ 
/**********************************************************/
#if ((defined(LIN32) || defined (LIN64)) &&  (!defined (__MINGW32__)&&  !defined (__MINGW64__)))
	void  Sleep(unsigned long ul_millisecond)
	{
		ulong sec  = ul_millisecond / 1000 ;
		sleep(sec);
		usleep(ul_millisecond - ( sec * 1000));
	}
#endif

/**********************************************************/
/* Gestion des mutex                                      */ 
/**********************************************************/
void OS_CreateMutex(OS_mutex * s_mutex)
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
	    // Problème sur les mutex Windows : ils sont non bloquant 
	    // si le processus possede deja le mutex, ici on veut le 
	    // meme conportemant que Linux/Unix
		//*s_mutex=CreateMutex(NULL,FALSE,NULL);
	    *s_mutex=CreateSemaphore(NULL,1,1,NULL);
	#else
		pthread_mutex_init((pthread_mutex_t *)s_mutex,NULL);
	#endif
}

void OS_DestroyMutex(OS_mutex * s_mutex)
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		CloseHandle(*s_mutex);
	#else
		pthread_mutex_destroy((pthread_mutex_t *)s_mutex);
	#endif
}

void OS_LockMutex(OS_mutex * s_mutex)
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		WaitForSingleObject(*s_mutex, INFINITE);
	#else
		pthread_mutex_lock((pthread_mutex_t *)s_mutex);
	#endif
}

void OS_UnlockMutex(OS_mutex * s_mutex)
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		//ReleaseMutex(*s_mutex);
		ReleaseSemaphore(*s_mutex,1,NULL);
	#else
		 pthread_mutex_unlock((pthread_mutex_t *)s_mutex);
	#endif

}

/**********************************************************/
/* Gestion des evenements                                 */ 
/**********************************************************/
void OS_CreateEvent(OS_event * s_event)
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		*s_event=CreateEvent(NULL, FALSE, FALSE, NULL);
	#else
	    pthread_cond_init(&(s_event->condition_cond),NULL) ;
		pthread_mutex_init(&(s_event->condition_mutex),NULL);
		s_event->triggered = FALSE;
	#endif
}

void OS_DestroyEvent(OS_event * s_event)
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		if (*s_event != NULL)
		{
			CloseHandle(*s_event);
		}
	#else
		pthread_cond_destroy(&(s_event->condition_cond)) ;
		s_event->triggered = false;
		pthread_mutex_destroy(&(s_event->condition_mutex));
	#endif
}

void OS_SetEvent(OS_event * s_event)
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		if (*s_event != NULL)
		{
			SetEvent(*s_event);
		}
	#else
        pthread_mutex_lock(&(s_event->condition_mutex));
        pthread_cond_signal(&(s_event->condition_cond));
		s_event->triggered = TRUE;
        pthread_mutex_unlock(&(s_event->condition_mutex));
	#endif
}

void OS_ResetEvent(OS_event * s_event)
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		if (*s_event != NULL)
		{
			ResetEvent(*s_event);
		}
	#else
	    pthread_mutex_lock(&(s_event->condition_mutex));
		s_event->triggered = FALSE;
		pthread_mutex_unlock(&(s_event->condition_mutex));
	#endif
}

int OS_WaitEvent(OS_event * s_event,unsigned long ul_timeout)
{
	int  isReady = OS_WAIT_OK;
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		if (*s_event != NULL)
		{
			DWORD dwRes = 0;
			dwRes = WaitForSingleObject(*s_event, ul_timeout);
			isReady = OS_WAIT_OK;
			if (dwRes == WAIT_TIMEOUT)
			{
				isReady = OS_WAIT_TIMEOUT;
			}
			else if (dwRes != WAIT_OBJECT_0)
			{
				isReady = OS_WAIT_ERROR;
			}
		}
		else
		{
			isReady = OS_WAIT_ERROR;
		}
	#else
		int iRes = 0;
		if (ul_timeout != INFINITE)
		{

		    struct timespec tm;
			struct timeb tp;
			long sec, millisec;

			sec = ul_timeout / 1000;
			millisec = ul_timeout % 1000;
			ftime( &tp );
			tp.time += sec;
			tp.millitm += millisec;
			if( tp.millitm > 999 )
			{
				tp.millitm -= 1000;
				tp.time++;
			}
			tm.tv_sec = tp.time;
			tm.tv_nsec = tp.millitm * 1000000 ;

			pthread_mutex_lock(&(s_event->condition_mutex));
			if  (s_event->triggered == FALSE)
			{
				iRes = pthread_cond_timedwait( &(s_event->condition_cond), &(s_event->condition_mutex),&tm);
			}
			pthread_mutex_unlock(&(s_event->condition_mutex));
			if (iRes == ETIMEDOUT)
			{
				isReady = OS_WAIT_TIMEOUT;
			}
			else if (iRes != 0)
			{
				isReady = OS_WAIT_ERROR;
			}
			else if (iRes == 0)
			{
				isReady = OS_WAIT_OK;
				s_event->triggered = FALSE ;
			}
		}
		else
		{
			pthread_mutex_lock(&(s_event->condition_mutex));
			if (!s_event->triggered)
			{
				iRes = pthread_cond_wait(&(s_event->condition_cond), &(s_event->condition_mutex));
			}
			pthread_mutex_unlock(&(s_event->condition_mutex));
			if (iRes != 0)
			{
				isReady = OS_WAIT_ERROR;
			}
			else
			{
				isReady = OS_WAIT_OK;
				s_event->triggered = FALSE ;
			}
		}
	#endif
	return isReady;
}

/**********************************************************/
/* Gestion des Threads                                    */ 
/**********************************************************/

HANDLE OS_CreateThread(LPSECURITY_ATTRIBUTES lpsa, unsigned long dwStackSize, LPTHREAD_START_ROUTINE pFunction, void* argument, unsigned long dwCreationFlags, pthread_t* pThreadId)
{
	HANDLE iRes = 0;
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		iRes = CreateThread(lpsa, (SIZE_T) dwStackSize, (LPTHREAD_START_ROUTINE) pFunction, (LPVOID) argument, (DWORD) dwCreationFlags, pThreadId);
	#else
		pthread_attr_t localAttrib;
		pthread_attr_t * secuLocalAttrib;
		//unsigned long ulCreationFlags = dwCreationFlags;
		if (lpsa == NULL)
		{
			secuLocalAttrib = &localAttrib;
			iRes = pthread_attr_init(secuLocalAttrib);
		}
		else
		{
			secuLocalAttrib = lpsa;
			iRes = 0 ;
		}
		if ((iRes == 0) && (dwStackSize != 0))
		{
			iRes = pthread_attr_setstacksize(secuLocalAttrib, dwStackSize);
		}
		if (iRes == 0)
		{
			iRes = pthread_create(pThreadId, secuLocalAttrib, pFunction, (void *) argument) ;
		}
	#endif
	return iRes;
}

unsigned long OS_TerminateThread(pthread_t * pThreadId)
{
	unsigned long rc ;
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		rc = TerminateThread((HANDLE *)pThreadId, 0);
	#else
		rc = pthread_cancel(*pThreadId);
	#endif
	return rc;
}


unsigned long OS_JoinThread(pthread_t *pThreadId)
{
	unsigned long rc ;
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		rc =  WaitForSingleObject((HANDLE*)pThreadId, INFINITE);
	#else
	    void *status;
		rc = pthread_join(*pThreadId,&status);
	#endif
	return rc;
}



/**********************************************************/
/* Gestion des Sockets                                    */ 
/**********************************************************/
void OS_WSAStartup()
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		WSADATA wsa;
		int err;
		if ((err = WSAStartup(MAKEWORD(1,0), &wsa)) != 0)
		{
			exit(0);
		}
	#endif
}

void OS_WSACleanup()
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) )
		WSACleanup();
	#endif
}

/**********************************************************/
/* Gestion des MessagesThread                             */ 
/**********************************************************/
#if (!defined(WIN32) && !defined (WIN64) && !defined (__MINGW32__) && !defined (__MINGW64__))
#include <list>
class c_Message 
{
	public :
		//thread ID
		pthread_t idThread;
		//ID of the user-defined message.
		UINT message;
		//First message parameter.
		WPARAM wParam;
		//Second message parameter.
		LPARAM   lParam;

	virtual ~c_Message()
	{
		/* Do nothing */
	}

	c_Message()
	{
		idThread=0;
		message=0;
		wParam=0;
		lParam=0;
	}

	c_Message(pthread_t _idThread,UINT _message,WPARAM  _wParam,LPARAM   _lParam)
	{
		idThread=_idThread;
		message=_message;
		wParam=_wParam;
		lParam=_lParam;
	}

    virtual c_Message &operator=(const c_Message &source)
	{
		if (&source != this)
		{
			idThread=source.idThread;
			message=source.message;
			wParam=source.wParam;
			lParam=source.lParam;
		}
		return *this;
	}
} ;

class c_MessageList
{
	private :
		std::list<c_Message>  List ;
		OS_event Event;
		OS_mutex Mutex;

	public :
		c_MessageList()
		{
			OS_CreateEvent(&Event);
			OS_CreateMutex(&Mutex);
		}

		virtual ~c_MessageList()
		{
			OS_DestroyEvent(&Event);
			OS_DestroyMutex(&Mutex);
		}


		bool getMessage(pthread_t idThread,unsigned int iTimeout,c_Message *lpMsg,HWND hWnd, unsigned int wMsgFilterMin,unsigned int wMsgFilterMax,unsigned int  wRemoveMsg)
		{
			size_t size;
			bool ret = false ;

			do 
			{
				OS_LockMutex(&Mutex);
				size = List.size();
				OS_UnlockMutex(&Mutex);
				if (size == 0)
				{
					unsigned int ret = OS_WaitEvent(&Event,iTimeout);
					if (ret == OS_WAIT_TIMEOUT)
					{
						ret = false;
					}
				}
				OS_LockMutex(&Mutex);
				std::list<c_Message>::iterator it = List.begin(); 
				for (; it != List.end() ; ++it) 
				{
					if ((it->idThread == idThread ) && (it->message >= wMsgFilterMin) && ((it->message <= wMsgFilterMax) || (0 == wMsgFilterMax)))
					{
						ret = true;
						*lpMsg = *it ;
						if (wRemoveMsg == PM_REMOVE)
						{
							List.erase(it);
						}
						break;
					}
				}
				OS_UnlockMutex(&Mutex);
				if ((ret == false) && (iTimeout == INFINITE ))
				{
					Sleep(100);
				}
			} while ((ret == false) && (iTimeout == INFINITE ));
			return ret;
		}

		bool putMessage(pthread_t idThread,unsigned int Msg,unsigned int wParam,long lParam)
		{
			OS_LockMutex(&Mutex);
			List.push_back(c_Message(idThread,Msg,wParam,lParam));
			if (List.size() == 1 )
			{
				OS_SetEvent(&Event);
			}
			OS_UnlockMutex(&Mutex);
			return true;
		}
};

static c_MessageList __MessageList;

#endif
 

pthread_t OS_GetCurrentThreadID()
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		DWORD ret = GetCurrentThreadId();
		return (pthread_t)ret ;
	#else
		return pthread_self();
	#endif
}

BOOLEAN OS_PostThreadMessage(pthread_t idThread,UINT Msg,WPARAM wParam,LPARAM lParam)
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		BOOL ret = PostThreadMessage(idThread,(UINT)Msg,(WPARAM) wParam, (LPARAM)lParam);
		if (ret == TRUE) 
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	#else
		return __MessageList.putMessage(idThread,Msg,wParam,lParam)?TRUE:FALSE;;
	#endif
 return TRUE;
}

BOOLEAN OS_GetMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		BOOL ret = GetMessage(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax);
		if (ret == TRUE) 
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	#else
		c_Message pMsg;
		pthread_t pThreadId = OS_GetCurrentThreadID();
		bool ret = __MessageList.getMessage(pThreadId,INFINITE,&pMsg,hWnd,wMsgFilterMin,wMsgFilterMax,PM_REMOVE);
		lpMsg->hwnd    = (HWND)hWnd;
		lpMsg->message = (UINT)pMsg.message;
		lpMsg->wParam  = (WPARAM)pMsg.wParam;
		lpMsg->lParam  = (LPARAM)pMsg.lParam;
		lpMsg->time    = 0 ;
		lpMsg->pt.x    = 0 ;
		lpMsg->pt.y    = 0 ;
		return (ret)?TRUE:FALSE;;
	#endif
}

BOOLEAN OS_PeekMessage(LPMSG lpMsg,HWND hWnd, unsigned int wMsgFilterMin,unsigned int wMsgFilterMax,unsigned int  wRemoveMsg)
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		BOOL ret = PeekMessage(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax,wRemoveMsg);
		if (ret == TRUE) 
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	#else
		c_Message pMsg;
		pthread_t pThreadId = OS_GetCurrentThreadID();
		bool ret = __MessageList.getMessage(pThreadId,100,&pMsg,hWnd,wMsgFilterMin,wMsgFilterMax,wRemoveMsg);
		lpMsg->hwnd    = (HWND)hWnd;
		lpMsg->message = (UINT)pMsg.message;
		lpMsg->wParam  = (WPARAM)pMsg.wParam;
		lpMsg->lParam  = (LPARAM)pMsg.lParam;
		lpMsg->time    = 0 ;
		lpMsg->pt.x    = 0 ;
		lpMsg->pt.y    = 0 ;

		return (ret)?TRUE:FALSE;
	#endif
}

/**********************************************************/
/* Gestion des pages mémoires                             */ 
/**********************************************************/
#if ((defined(LIN32) || defined (LIN64)) &&  (!defined (__MINGW32__)&&  !defined (__MINGW64__)))
#define READ_ACCESS_PAGE 0
#if READ_ACCESS_PAGE == 1
#include <signal.h>
#include <setjmp.h>
static jmp_buf env_getPageAccess;
static char Flag_Read  ;
static char Flag_Write ;
static struct sigaction act;
static struct sigaction old_act;
static DWORD getPageAccess(uintptr_t lpAddress);

static void SignalHandlerSegmentViolation (int sig, siginfo_t *siginfo, void  *contextPtr)
{
	ucontext_t*  context = (ucontext_t *) contextPtr;

	if (sig == SIGSEGV)
	{
		if (context->uc_mcontext.gregs[REG_ERR] & 0x2)
		{
			TRACE_DEFAULT("SignalHandlerSegmentViolation(SIGSEGV - READ)");
			Flag_Read  = 0;
			Flag_Write = 0;
		}
		else
		{
			TRACE_DEFAULT("SignalHandlerSegmentViolation(SIGSEGV - WRITE)");
			Flag_Read  = 1;
			Flag_Write = 0;
		}
	}
	longjmp(env_getPageAccess, 1);
	return ;
}

DWORD getPageAccess(uintptr_t lpAddress)
{
	DWORD prot = OS_PAGE_NOACCESS;
	unsigned char ValTemp;

	TRACE_DEFAULT("getPageAccess(lpAddress=%p)",lpAddress);

	Flag_Read  = 1 ;
	Flag_Write = 1 ;
	memset (&act, '\0', sizeof(act));
	act.sa_sigaction = &SignalHandlerSegmentViolation;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &act, &old_act);

	if (0 == setjmp(env_getPageAccess))
	{
		TRACE_DEFAULT("@SignalHandlerSegmentViolation = %p , Adresse Recup = %p ",&SignalHandlerSegmentViolation,OffsetRetour);
		ValTemp = ((unsigned char *)lpAddress)[0];
		((unsigned char *)lpAddress)[0] = ValTemp;
	}

	TRACE_DEFAULT("Flag_Read = %d ",Flag_Read);
	TRACE_DEFAULT("Flag_Write = %d ",Flag_Write);

	sigaction(SIGSEGV,&old_act, &act);

	if ((Flag_Read == 1) && (Flag_Write == 1))
	{
		prot = OS_PAGE_READWRITE;
	}
	if ((Flag_Read == 1) && (Flag_Write == 0))
	{
		prot = OS_PAGE_READONLY;
	}
	if ((Flag_Read == 0) && (Flag_Write == 0))
	{
		prot = OS_PAGE_NOACCESS;
	}
	return prot;
}
#endif
#endif
int OS_MemoryProtect(LPVOID lpAddress, size_t dwSize, DWORD prot,DWORD *old_prot)
{
	//TRACE_DEFAULT("OS_MemoryProtect(lpAddress=%p,dwSize=%d,prot=0x%X,*old_prot)",lpAddress,dwSize,prot);
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		return VirtualProtect(lpAddress, dwSize, prot , old_prot);
	#else
		int ret;
		#if READ_ACCESS_PAGE == 1
			Flag_Read = 0;
			Flag_Write= 0;
		#endif
		uintptr_t AlignedAdress = ((uintptr_t)lpAddress & (~(PAGE_SIZE-1)));
		size_t AlignedSize = ((uintptr_t)lpAddress - (uintptr_t)AlignedAdress) + dwSize;

		// Ne marche pas si l'on est déjà dans une exception SIGSEGV
		#if READ_ACCESS_PAGE == 1
			*old_prot = getPageAccess(AlignedAdress);
		#else
		*old_prot =  OS_PAGE_NOACCESS ;
		#endif
		msync((LPVOID)AlignedAdress, AlignedSize, MS_SYNC|MS_INVALIDATE);
		ret = mprotect((LPVOID)AlignedAdress,AlignedSize,prot);

		TRACE_DEFAULT("OS_MemoryProtect(lpAddress=%p,dwSize=%d,prot=0x%X,old_prot=0x%X) = %d ",lpAddress,AlignedSize,prot,*old_prot,ret);
		return ret==0?1:0;
	#endif
}

LPVOID OS_MemoryAllocation(LPVOID lpAddress, size_t dwSize, DWORD flAllocationType, DWORD flProtect)
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__) )
		return VirtualAlloc(lpAddress,dwSize, flAllocationType, flProtect );
	#else
	    void * map;
	    DWORD Protect = flProtect;
	    LPVOID Address = lpAddress;
	    if ( ((flAllocationType&OS_MEM_RESET_UNDO)!=0) || ((flAllocationType&OS_MEM_RESET)!=0) )
	    {
	    	Protect = PROT_NONE;
	    }
	    if (((flAllocationType&OS_MEM_RESERVE)!=0))
	    {
	    	map = mmap((void*)0, dwSize, PROT_NONE, MAP_PRIVATE|MAP_ANON, -1, 0);
	        msync(map, dwSize, MS_SYNC|MS_INVALIDATE);
	    	Address = map;
	    }
	    if ((flAllocationType&OS_MEM_COMMIT)!=0)
	    {
	    	map = mmap(Address, dwSize, Protect, MAP_FIXED|MAP_SHARED|MAP_ANON, -1, 0);
	        msync(Address, dwSize, MS_SYNC|MS_INVALIDATE);
	    }
		mprotect(lpAddress,dwSize,Protect);
	    msync(map, dwSize, MS_SYNC|MS_INVALIDATE);
	    TRACE_DEFAULT("OS_MemoryAllocation(lpAddress=%p, dwSize=%d, flAllocationType=0x%X, flProtect=0x%X) = %p ",lpAddress,dwSize,flAllocationType,flProtect,map);
	    return map;
	#endif
}

int OS_MemoryRelease(LPVOID lpAddress, size_t dwSize)
{
	#if (defined(WIN32) || defined (WIN64) || defined (__MINGW32__) || defined (__MINGW64__))
		if (lpAddress != NULL)
		{
			return VirtualFree(lpAddress, 0, MEM_RELEASE);
		}
		else
		{
			return - 1;
		}
	#else
		msync(lpAddress, dwSize, MS_SYNC);
		return munmap(lpAddress,dwSize);
	#endif
}

