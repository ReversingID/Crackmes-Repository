#include <stdint.h>
#include <errno.h>
#include <stdlib.h>

#include "AntiDebug.h"
#include "CTrace.h"
#include "ObfuscatedString.h"
#include "sha256.h"


#if (defined(WIN32) || defined(WIN64))
#include <vector>
#else
#include <sys/stat.h>
#endif

static volatile uint8_t DIGEST_REF[SHA_256_SIZE+1] = "\x00|TOTO-TATA-TITI-TUTU-TETE-TYTY|";

#if (defined(WIN32) || defined(WIN64))


#else
	extern char __executable_start;
	extern char __etext;
#endif

#define DEBRAILLAGE_ANTI_DEBUG

#ifdef DEBRAILLAGE_ANTI_DEBUG
	#define DEBRAILLAGE_ANTI_DEBUG_1
	#define DEBRAILLAGE_ANTI_DEBUG_2
	#define DEBRAILLAGE_ANTI_DEBUG_3
	#define DEBRAILLAGE_ANTI_DEBUG_4
#endif

#if (defined(WIN32) || defined(WIN64))
	__declspec(allocate(".Secure"))     __declspec(align(PAGE_SIZE))  void * __start_Secure;
	__declspec(allocate(".Secure$ZZZ")) __declspec(align(PAGE_SIZE))  void * __stop_Secure;
	static size_t __size_Secure = ((uintptr_t)(&__stop_Secure) - ((uintptr_t)&__start_Secure));
#else
	#if !defined (__MINGW32__) && !defined (__MINGW64__)
		#include <sys/ptrace.h>
		#include <sys/types.h>
		#include <sys/wait.h>
		#include <sys/stat.h>
		#include <sys/capability.h>
		#include <string.h>
		#include <fcntl.h>
		#include <unistd.h>
	#else
		#include <windows.h>
	#endif

	extern void  * __start_Secure  ;
	extern void  * __stop_Secure   ;
	void * __stop_aligned __attribute__((aligned(PAGE_SIZE))) __attribute__((section("Secure")));

	static size_t __size_Secure = (((uintptr_t)(&__stop_Secure))&(~(PAGE_SIZE-1))) - ((uintptr_t)&__start_Secure);

	#if !defined (__MINGW32__) && !defined (__MINGW64__)
	int checkGrsec()
	{
	    struct stat  file_stat;
	    int ret = 0;
	    TRACE_DEFAULT(_STR2WSTR(""));
	    if (stat("/proc/sys/kernel/grsecurity",&file_stat) == 0)
	    {
	    	ret= -1;
	    }
	    TRACE_DEFAULT(_STR2WSTR("ret = %d"),ret);
		return ret;
	}

	int checkPcap()
		{
			cap_flag_value_t value;
			cap_t current;
			int ret = 0;
			TRACE_DEFAULT(_STR2WSTR(""));
			/*
			 *  If we're running under linux, we first need to check if we have
			 *  permission to ptrace. We do that using the capabilities
			 *  functions.
			 */
			current = cap_get_proc();
			if (!current)
			{
				TRACE_DEFAULT(_STR2WSTR("Failed getting process capabilities\n"));
			    ret= -1;
			}
			else
			{
				if (cap_get_flag(current, CAP_SYS_PTRACE, CAP_PERMITTED, &value) < 0)
				{
					TRACE_DEFAULT(_STR2WSTR("Failed getting permitted ptrace capability state\n"));
					ret = -1;
				}
				else
				{
					if ((value == CAP_SET) && (cap_get_flag(current, CAP_SYS_PTRACE, CAP_EFFECTIVE, &value) < 0))
					{
						TRACE_DEFAULT(_STR2WSTR("Failed getting effective ptrace capability state\n"));
						ret = -1;
					}
				}
				cap_free(current);
			}
			TRACE_DEFAULT(_STR2WSTR("ret = %d"),ret);
			return ret;
		}

		unsigned int IsDebuggerPresent(void)
		{
			char buf[1024];
			int debugger_present = 0;
			int status_fd ;
			TRACE_DEFAULT(_STR2WSTR(""));

			status_fd = open("/proc/self/status", O_RDONLY);

			if (status_fd == -1)
			{
				TRACE_DEFAULT(_STR2WSTR("open failed : /proc/self/status"));
			}
			else
			{
				ssize_t num_read = read(status_fd, buf, sizeof(buf));

				if (num_read > 0)
				{
					static const char TracerPid[] = "TracerPid:";
					char *tracer_pid;

					buf[num_read] = 0;
					tracer_pid    = strstr(buf, TracerPid);
					tracer_pid   += sizeof(TracerPid) - 1;
					strstr(tracer_pid, "\n")[0] ='\0';

					TRACE_DEFAULT(_STR2WSTR("get tracerPid : %d"),atoi(tracer_pid));
					debugger_present = atoi(tracer_pid);
				}
				else
				{
					TRACE_DEFAULT(_STR2WSTR("read failed : /proc/self/status"));
				}
			}

			TRACE_DEFAULT(_STR2WSTR("\e[31mdebugger_present = %d\e[0m"),debugger_present);
			#ifdef DEBRAILLAGE_ANTI_DEBUG_3
				debugger_present = 0;
			#endif
			return debugger_present;
		}

		unsigned int  checkDebugger(void)
		{
			int debugger_present = 0;

			TRACE_DEFAULT(_STR2WSTR(""));
			if (checkPcap() == 0 && checkGrsec()==0)
			{
				int status;
				int pid = fork();

				if (pid == 0)
				{
					TRACE_DEFAULT("checkDebugger_fork_start");
					int ppid = getppid();
					debugger_present = 0;
					/* Child */
					if (ptrace(PTRACE_ATTACH, ppid, NULL, NULL) == -1)
					{
						TRACE_DEFAULT("PTRACE_ATTACH: %s ",strerror(errno));
						debugger_present = 1;
					}
					else
					{
						/* Wait for the parent to stop and continue it */
						waitpid(ppid, NULL, 0);
						if (ptrace(PTRACE_DETACH, ppid, NULL, NULL) == -1)
						{
							TRACE_DEFAULT("PTRACE_DETACH: %s ",strerror(errno));
						}
					}
					TRACE_DEFAULT("checkDebugger_fork_end");
					quick_exit (debugger_present);
				}
				else if (pid > 0)
				{
					waitpid(pid, &status, WUNTRACED|WCONTINUED);
					debugger_present = WEXITSTATUS(status);
					waitpid(pid, &status, WUNTRACED|WCONTINUED);
					if (ptrace(PTRACE_CONT, pid, NULL, NULL) == -1)
					{
						TRACE_DEFAULT("PTRACE_CONT: %s ",strerror(errno));
					}
					TRACE_DEFAULT("checkDebugger_fork_end_wait (status = %d)",status);
				}
				else if (pid < 0)
				{
					perror("fork");
				}
			}
			TRACE_DEFAULT(_STR2WSTR("\e[31mdebugger_present = %d\e[0m"),debugger_present);
			#ifdef DEBRAILLAGE_ANTI_DEBUG_3
				debugger_present = 0;
			#endif
			return debugger_present;
		}
	#endif
#endif

unsigned int  AntiDebug::various_rdtsc(void)
{
	clock_t res;
	clock_t t = clock();
	unsigned int ret = 0;

	int i; //Time-consuming crap
	for (i = 2; i<7137001; i++)
		if (7137001 % i == 0)
			break;
	res = clock() - t ;

	if (this->RDTSC==0)
	{
		this->RDTSC = res + 1500 /*adjust this*/ ;
		ret = 0;
	}
	else if (res > (this->RDTSC ))
	{
		ret = 1;
	}

	TRACE_DEFAULT(_STR2WSTR("\e[31mdebugger_present = %d\e[0m"),ret);
#ifdef DEBRAILLAGE_ANTI_DEBUG_3
	ret = 0;
#endif
	return ret;
}

void AntiDebug::decode_data(ACCESS_TIME time, ACCESS_MODE mode, ULONG_PTR adresseVar, size_t memorySize)
{
	if ((mode == READ) /* || ((mode == WRITE) && (time=UNDO)) */)
	{
		if (memorySize != 0)
		{
			ns_EncodingString::decode(adresseVar, memorySize);
		}
	}
	return;
}

BOOLEAN AntiDebug::AccessControler(ULONG_PTR _ProtectedMemoryAdress,ACCESS_MODE mode,size_t memorySize)
{
	TRACE_DEFAULT(_STR2WSTR("_ProtectedMemoryAdress=%p, mode=%d, memorySize=%d"),_ProtectedMemoryAdress,mode,memorySize);

	if ((_ProtectedMemoryAdress > ((ULONG_PTR)&__start_Secure)) && (_ProtectedMemoryAdress<((ULONG_PTR)&__stop_Secure)))
	{
		if ((mode == READ) || (mode == WRITE))
		{
			return TRUE;
		}
	}
	return FALSE;
}

void AntiDebug::readMemory(ULONG_PTR adresseVar,size_t memorySize)
{
	TRACE_DEFAULT(_STR2WSTR("adresseVar=%p, memorySize=%d"),adresseVar,memorySize);
	//	TRACE_DEFAULT(_STR2WSTR("Encoded message : %s"),adresseVar);
	TRACE_DEFAULT(_STR2WSTR(""));
	this->decode_data(DO, READ, adresseVar,memorySize);
	//TRACE_DEFAULT(_STR2WSTR"Decoded message : %s"),adresseVar);
}

void AntiDebug::writeMemory(ULONG_PTR adresseVar, size_t memorySize)
{
	TRACE_DEFAULT(_STR2WSTR("adresseVar=%p, memorySize=%d"), adresseVar, memorySize);
	//	TRACE_DEFAULT(_STR2WSTR("Decoded message : %s"),adresseVar);
	TRACE_DEFAULT(_STR2WSTR(""));
	this->decode_data(DO,WRITE,adresseVar, memorySize);
	//TRACE_DEFAULT(_STR2WSTR"Encoded message : %s"),adresseVar);
}

BOOLEAN AntiDebug::UndoAcces(ULONG_PTR _ProtectedMemoryAdress, ACCESS_MODE mode, size_t memorySize)
{
	TRACE_DEFAULT(_STR2WSTR("_ProtectedMemoryAdress=%p, mode=%d, memorySize=%d"),_ProtectedMemoryAdress,mode,memorySize);

	this->decode_data(UNDO,mode,_ProtectedMemoryAdress, memorySize);

	check();

	return TRUE;
}

AntiDebug::AntiDebug()
{
	TRACE_DEFAULT(_STR2WSTR(""));
}

unsigned int AntiDebug::check_memory()
{
	sha256_context sha_ctx;
	uint8 digest[SHA_256_SIZE];
	unsigned int ret = 0;
	unsigned char * __start_Text;
	size_t __size_Text;

#if (defined(WIN32) || defined(WIN64))
	HMODULE exe;
	const IMAGE_DOS_HEADER *pDosHdr      = nullptr;
	const IMAGE_SECTION_HEADER *pSection = nullptr;
	const IMAGE_NT_HEADERS*  pImageHdr   = nullptr;

	exe = GetModuleHandle(NULL);
	pDosHdr   = reinterpret_cast<const IMAGE_DOS_HEADER*>(exe);
	pImageHdr = reinterpret_cast<const IMAGE_NT_HEADERS*>(reinterpret_cast<const uint8_t*>(pDosHdr) + pDosHdr->e_lfanew);
	pSection  = reinterpret_cast<const IMAGE_SECTION_HEADER*>(pImageHdr + 1);
	if (strcmp((const char *)pSection->Name, ".text") != 0)
	{
		for (int i = 0; i < pImageHdr->FileHeader.NumberOfSections; ++i, ++pSection)
		{
			if (strcmp((const char *)pSection->Name, ".text") == 0)
			{
				break;
			}
		}
	}
	__start_Text = (unsigned char*)pImageHdr->OptionalHeader.ImageBase + pSection->VirtualAddress;
	__size_Text  = pImageHdr->OptionalHeader.SizeOfCode;
	/*
	fprintf(stderr, "__size_Text : %p\n", (void*)__start_Text);
	fprintf(stderr, "__size_Text : %p", (void *)__size_Text);
	for (unsigned int index=0; index < __size_Text; index++)
	{
		if ((index % 16) == 0)
		{
			fprintf(stderr, "\n");
		}
		fprintf(stderr,"0x%02X ", __start_Text[index]);
	}
	*/
#else
	unsigned char * __stop_Text;

	__start_Text = (unsigned char *)&__executable_start;
	__stop_Text  = (unsigned char *)&__etext;
	__size_Text = __stop_Text - __start_Text;
#endif

	sha256_starts(&sha_ctx);
	sha256_update(&sha_ctx, __start_Text, (unsigned long)__size_Text);
	sha256_finish(&sha_ctx, digest);

	TRACE_DEFAULT(_STR2WSTR("Status = %d"),DIGEST_REF[0]);

	if (DIGEST_REF[0] == 0x00)
	{
		printf("\\x0F");
		for (int i = 0; i < SHA_256_SIZE; i++)
		{
			printf("\\x%02X", digest[i]);
		}
		printf("\n");
		exit(0);
	}

	if (memcmp(digest, (const void*)&DIGEST_REF[1] , SHA_256_SIZE) != 0)
	{
		ret = 1;
	}

	TRACE_DEFAULT(_STR2WSTR("\e[31mmemory alterated = %d\e[0m"),ret);
#ifdef DEBRAILLAGE_ANTI_DEBUG_4
	ret = 0;
#endif
	return ret;
}

DebuggerStatus AntiDebug::start()
{

	TRACE_DEFAULT(_STR2WSTR(""));
	this->RDTSC = 0;

	srand((unsigned int)time(NULL));
	various_rdtsc();
	debuggerStatut = DEBUGGER_NO_INIT;
	if (!check(2))
	{
		startMemoryProtection(
				[](ULONG_PTR _ProtectedMemoryAdress, ACCESS_MODE mode, size_t memorySize) -> BOOLEAN { return AntiDebug::getInstance().AccessControler(_ProtectedMemoryAdress, mode, memorySize); },
				[](ULONG_PTR _ProtectedMemoryAdress, ACCESS_MODE mode, size_t memorySize) -> BOOLEAN { return AntiDebug::getInstance().UndoAcces(_ProtectedMemoryAdress, mode, memorySize); },
				[](ULONG_PTR adresseVar, size_t memorySize) -> void { return AntiDebug::getInstance().readMemory(adresseVar, memorySize);  },
				[](ULONG_PTR adresseVar, size_t memorySize) -> void { return AntiDebug::getInstance().writeMemory(adresseVar, memorySize); }
		);
		debuggerStatut = NO_DEBUGGER_INIT;
	}
	check();
	return debuggerStatut;
}

AntiDebug::~AntiDebug()
{
	TRACE_DEFAULT(_STR2WSTR(""));
	if (debuggerStatut != DEBUGGER_NO_INIT)
	{
		stopMemoryProtection();
	}
}

unsigned int AntiDebug::check(unsigned int nbcheck)
{
	unsigned int ret = 0;
	unsigned int random = rand() % nbcheck;

	if (random == 0)
	{
		ret = IsDebuggerPresent();
	}
	else if (random == 1)
	{
		ret = checkDebugger();
	}
	else if(random == 2)
	{
		ret = various_rdtsc();
	}
	else if(random == 3)
	{
		ret = check_memory();
	}
	if (ret != 0 )
	{
		debugger();
	}
	else
	{
		noDebugger();
	}
	TRACE_DEFAULT(_STR2WSTR("debuggerStatut = %02X presenceDebugger = %d "), debuggerStatut,ret);
	return ret;
}


void AntiDebug::debugger()
{
	DWORD old_mode;
	TRACE_DEFAULT(_STR2WSTR(""));

	if ((debuggerStatut == NO_DEBUGGER_INIT) || (debuggerStatut == NO_DEBUGGER_INIT_VERROU))
	{
		debuggerStatut = DEBUGGER_INIT;
		if (OS_MemoryProtect((LPVOID)&__start_Secure,__size_Secure, OS_PAGE_READWRITE , &old_mode) == FALSE)
		{
			TRACE_DEFAULT("OS_MemoryProtect ERROR");
		}
		stopMemoryProtection();
	}
}

void AntiDebug::noDebugger()
{
	DWORD old_mode;
	TRACE_DEFAULT(_STR2WSTR(""));

	if (debuggerStatut == NO_DEBUGGER_INIT)
	{
		if (check_memory() == 0)
		{
			if (OS_MemoryProtect((LPVOID)&__start_Secure, __size_Secure, OS_PAGE_NOACCESS, &old_mode) == FALSE)
			{
				TRACE_DEFAULT("OS_MemoryProtect ERROR");
			}
			else
			{
				debuggerStatut = NO_DEBUGGER_INIT_VERROU;
			}
		}
		else
		{
			debugger();
		}
	}
}

/**************************************************************************/
/* Run AntiDebug before Main                                              */
/**************************************************************************/
static DebuggerStatus test = AntiDebug::getInstance().start();
