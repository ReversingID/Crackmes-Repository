
#include <OS.h>
#include <signal.h>
#include <MemoryProtection_internal.h>
#include <CTrace.h>

#define BUFFER_ASM_SIZE 32

#if (defined(WIN64) || defined (LIN64))
	#define printPtr _STR2WSTR("0x%016llX")
#elif (defined(WIN32) || defined (LIN32))
	#define printPtr _STR2WSTR("0x%08X")
#else
	#error Un des flags : WIN64, LIN64, WIN32 , LIN32 doit etre defini
#endif

#if (defined(WIN32) || defined(WIN64))
	#pragma region SECTION_TLS
	#pragma section(".tls")
	#pragma section(".tls$")
	#pragma section(".tls$ZZZ")
	#pragma endregion
	__declspec(allocate(".tls$")) STATIC_TLS ULONG_PTR _True_EIP;
	__declspec(allocate(".tls$")) STATIC_TLS size_t _ProtectedMemoryAdressSize;
	__declspec(allocate(".tls$")) STATIC_TLS DWORD _OldProtect;
	__declspec(allocate(".tls$")) STATIC_TLS ULONG_PTR _ProtectedMemoryAdress;
	__declspec(allocate(".tls$")) STATIC_TLS ULONG_PTR _MagicCode;
	__declspec(allocate(".tls$")) STATIC_TLS ACCESS_MODE _mode;
	__declspec(allocate(".tls$")) STATIC_TLS uintPtrSize __InstrAdress__;
	__declspec(allocate(".tls$")) STATIC_TLS bool StructuredSignalHandlerSegmentViolation_Flag = false;
#else
	STATIC_TLS ULONG_PTR _True_EIP 					__attribute__((section("TLS")));
	STATIC_TLS size_t _ProtectedMemoryAdressSize 	__attribute__((section("TLS")));
	STATIC_TLS DWORD _OldProtect 					__attribute__((section("TLS")));
	STATIC_TLS ULONG_PTR _ProtectedMemoryAdress 	__attribute__((section("TLS")));
	STATIC_TLS ULONG_PTR _MagicCode 				__attribute__((section("TLS")));
	STATIC_TLS ACCESS_MODE _mode 					__attribute__((section("TLS")));
	STATIC_TLS uintPtrSize __InstrAdress__			__attribute__((section("TLS")));
	STATIC_TLS bool StructuredSignalHandlerSegmentViolation_Flag __attribute__((section("TLS"))) = false  ;
#endif


#if (defined(LIN32) || defined (LIN64) || defined(__MINGW32__) || defined(__MINGW64__) )
	#ifndef PCONTEXT
		typedef mcontext_t* PCONTEXT;
	#endif
#endif

#if (defined(WIN64) || defined (WIN32) && (!defined(__MINGW32__) && !defined(__MINGW64__)) )
STATIC_TLS __declspec(align(16)) __declspec(allocate(".tls$")) unsigned char Save_FPU[512] ;
#elif (defined(LIN32) || defined (LIN64) || defined(__MINGW32__) || defined(__MINGW64__) )
STATIC_TLS __attribute__((aligned(16))) __attribute__((section("TLS"))) unsigned char Save_FPU[512] ;
#else
	#error Un des flags : WIN64, LIN64, WIN32 , LIN32 doit etre defini
#endif

BOOLEAN __AccessControler___default(ULONG_PTR _ProtectedMemoryAdress, ACCESS_MODE mode, size_t memorySize)
{
	return FALSE;
}
BOOLEAN __UndoAcces___default(ULONG_PTR _ProtectedMemoryAdress,ACCESS_MODE mode,size_t memorySize)
{
	return FALSE;
}


ULONG_PTR _CALL_C_DECL_ getTrueEIP(void)
{
	return _True_EIP;
}

// fxsave @
// fxrtore @
ULONG_PTR _CALL_C_DECL_ getBuffer_FPU(void)
{
	return (ULONG_PTR) Save_FPU;
}

void _CALL_C_DECL_ restauration(void)
{
	if ((__Write__ != NULL) && (_mode == WRITE))
	{
		__Write__(_ProtectedMemoryAdress, _ProtectedMemoryAdressSize);
	}
	if (__UndoAcces__ != NULL)
	{
		__UndoAcces__(_ProtectedMemoryAdress, _mode, _ProtectedMemoryAdressSize);
	}
	// Restauration des droits
	OS_MemoryProtect((LPVOID)_ProtectedMemoryAdress,(size_t)_ProtectedMemoryAdressSize,_OldProtect, &_OldProtect);
	// libreration de la fonction construite
	OS_MemoryRelease((LPVOID)_MagicCode,0);
	// On efface les variables globale
	_True_EIP = 0 ;
	_OldProtect = 0 ;
	_ProtectedMemoryAdress = 0 ;
	_ProtectedMemoryAdressSize =0 ;
	_MagicCode = 0 ;
	_mode = READ ;
	StructuredSignalHandlerSegmentViolation_Flag = false;
}


int __input_hook_x__(ud_t* u)
{
  unsigned int c;

  c = ((char*)__InstrAdress__)[0];
  __InstrAdress__ = __InstrAdress__ + 1 ;

  return (int) (c & 0xFF);
}	



static void getInstrSizeAndMemorySize(uintPtrSize _InstrAdress,size_t *InstrLength,size_t *MemorySize, ud_t * ud_obj)
{
	size_t len;
	const struct ud_operand* operand;
	unsigned int i;

	ud_init(ud_obj);
	ud_set_mode(ud_obj,asm_used);


	#if (defined(WIN32) ||  defined(WIN64))
		ud_set_syntax(ud_obj, UD_SYN_INTEL);
	#else
		ud_set_syntax(ud_obj, UD_SYN_ATT);
	#endif

	__InstrAdress__ = _InstrAdress;
	ud_set_input_hook(ud_obj, __input_hook_x__);

	len = ud_disassemble(ud_obj);
	__InstrAdress__ = 0;



	*MemorySize = sizeof(uintPtrSize);
	*InstrLength = len;

	i = 0;

	if (ud_insn_mnemonic(ud_obj) == UD_Ijmp)
	{
		TRACE_DEFAULT(_STR2WSTR("Instruction non geree"));
		TRACE_DEFAULT(_STR2WSTR("L'instruction effectuant l'acces a l'adresse [") printPtr _STR2WSTR("] ==>  %-24s"), _InstrAdress, ud_insn_asm(ud_obj));
		return;
	}

	operand = ud_insn_opr(ud_obj,i);
	while ( operand != NULL)
	{
		if (operand->type == UD_OP_MEM )
		{
			*MemorySize = operand->size/8;
			break;
		}
		i++;
		operand = ud_insn_opr(ud_obj,i);
	}

	TRACE_DEFAULT(_STR2WSTR("L'instruction effectuant l'acces a l'adresse [") printPtr _STR2WSTR("] ==>  %-24s"), _InstrAdress ,ud_insn_asm(ud_obj));
	TRACE_DEFAULT(_STR2WSTR("L'instruction fait %d octet%s, elle essaie d'acceder a %d octet%s"),
			len        ,(len>1)?_STR2WSTR("s"):_STR2WSTR(""),
			*MemorySize,(*MemorySize>1)?_STR2WSTR("s"):_STR2WSTR("")
			);

}

static size_t CopyInstruction(void* _MagicCode, void const* Curent_EIP_RIP, size_t InstSize, ud_t * ud_obj, ACCESS_MODE mode, PCONTEXT ContextRecord)
{

#if ( defined(LIN32) ||  (defined(WIN32) && !defined(WIN64)) || defined(__MINGW32__) )
	size_t convertInst = InstSize;
	if (_MagicCode != NULL)
	{
		memcpy((void*)_MagicCode, (void*)Curent_EIP_RIP, InstSize);
	}
	return convertInst;
#endif

#if ( defined (LIN64) || defined(WIN64) || defined(__MINGW64__) ) 
	size_t  convertInst = 0;
	uintptr_t RIP;
	#if ( defined(WIN64))
		RIP = (uintptr_t)ContextRecord->Rip;
	#else
		
		RIP = (uintptr_t)ContextRecord->gregs[REG_RIP];
	#endif

	if (ud_insn_mnemonic(ud_obj) == UD_Imov)
	{
		/*
		------------------------------------------------------------
		mov [ reg ] , reg
		------------------------------------------------------------
		*/
		if ((ud_insn_opr(ud_obj, 0)->type == UD_OP_REG))
		{
			if ((ud_insn_opr(ud_obj, 1)->type == UD_OP_REG))
			{
				convertInst = InstSize;
				if (_MagicCode != NULL)
				{
					memcpy((void*)_MagicCode, (void*)Curent_EIP_RIP, InstSize);
				}
				return convertInst;
			}
		}

		/*
		------------------------------------------------------------
		mov [ rip + offset ] , rax
		------------------------------------------------------------
		*/
		if ((ud_insn_opr(ud_obj, 0)->type == UD_OP_MEM) && (ud_insn_opr(ud_obj, 0)->base == UD_R_RIP))
		{
			if ((ud_insn_opr(ud_obj, 1)->type == UD_OP_REG) && (ud_insn_opr(ud_obj, 1)->base == UD_R_RAX))
			{
				uintptr_t addrDest = (RIP + InstSize + (uintptr_t)((ud_insn_opr(ud_obj, 0))->lval).sdword);
				/*
				------------------------------------------------------------
				https://defuse.ca/online-x86-assembler.htm#disassembly
				push rbx
				movabs rbx, (rip + offset)
				mov [rbx], rax
				pop rbx
				------------------------------------------------------------
				*/
				convertInst = 15;
				char Instr[15] = { (char)0x53, (char)0x48, (char)0xbb,
					(char)((addrDest & 0x00000000000000FF) >> 0),
					(char)((addrDest & 0x000000000000FF00) >> 8),
					(char)((addrDest & 0x0000000000FF0000) >> 16),
					(char)((addrDest & 0x00000000FF000000) >> 24),
					(char)((addrDest & 0x000000FF00000000) >> 32),
					(char)((addrDest & 0x0000FF0000000000) >> 40),
					(char)((addrDest & 0x00FF000000000000) >> 48),
					(char)((addrDest & 0xFF00000000000000) >> 56),
					(char)0x48, (char)0x89, (char)0x03, (char)0x5b };
				if (_MagicCode != NULL)
				{
					memcpy((void*)_MagicCode, (void*)Instr, 15);
				}
				return convertInst;
			}
		}
		/*
		------------------------------------------------------------
		mov [ rip + offset ] , eax
		------------------------------------------------------------
		*/
		if ((ud_insn_opr(ud_obj, 0)->type == UD_OP_MEM) && (ud_insn_opr(ud_obj, 0)->base == UD_R_RIP))
		{
			if ((ud_insn_opr(ud_obj, 1)->type == UD_OP_REG) && (ud_insn_opr(ud_obj, 1)->base == UD_R_EAX))
			{
				uintptr_t addrDest = (RIP + InstSize + (uintptr_t)((ud_insn_opr(ud_obj, 0))->lval).sdword);
				/*
				------------------------------------------------------------
				https://defuse.ca/online-x86-assembler.htm#disassembly
				push rbx
				movabs rbx, (rip + offset)
				mov [rbx], eax
				pop rbx
				------------------------------------------------------------
				*/
				convertInst = 14;
				char Instr[14] = { (char)0x53, (char)0x48, (char)0xbb,
					(char)((addrDest & 0x00000000000000FF) >> 0),
					(char)((addrDest & 0x000000000000FF00) >> 8),
					(char)((addrDest & 0x0000000000FF0000) >> 16),
					(char)((addrDest & 0x00000000FF000000) >> 24),
					(char)((addrDest & 0x000000FF00000000) >> 32),
					(char)((addrDest & 0x0000FF0000000000) >> 40),
					(char)((addrDest & 0x00FF000000000000) >> 48),
					(char)((addrDest & 0xFF00000000000000) >> 56),
					(char)0x89, (char)0x03, (char)0x5b };
				if (_MagicCode != NULL)
				{
					memcpy((void*)_MagicCode, (void*)Instr, 14);
				}
				return convertInst;
			}
		}
		/*
		------------------------------------------------------------
		mov [ rip + offset ] , ax
		------------------------------------------------------------
		*/
		if ((ud_insn_opr(ud_obj, 0)->type == UD_OP_MEM) && (ud_insn_opr(ud_obj, 0)->base == UD_R_RIP))
		{
			if ((ud_insn_opr(ud_obj, 1)->type == UD_OP_REG) && (ud_insn_opr(ud_obj, 1)->base == UD_R_AX))
			{
				uintptr_t addrDest = (RIP + InstSize + (uintptr_t)((ud_insn_opr(ud_obj, 0))->lval).sdword);
				/*
				------------------------------------------------------------
				https://defuse.ca/online-x86-assembler.htm#disassembly
				push rbx
				movabs rbx, (rip + offset)
				mov [rbx], ax
				pop rbx
				------------------------------------------------------------
				*/
				convertInst = 15;
				char Instr[15] = { (char)0x53, (char)0x48, (char)0xbb,
					(char)((addrDest & 0x00000000000000FF) >> 0),
					(char)((addrDest & 0x000000000000FF00) >> 8),
					(char)((addrDest & 0x0000000000FF0000) >> 16),
					(char)((addrDest & 0x00000000FF000000) >> 24),
					(char)((addrDest & 0x000000FF00000000) >> 32),
					(char)((addrDest & 0x0000FF0000000000) >> 40),
					(char)((addrDest & 0x00FF000000000000) >> 48),
					(char)((addrDest & 0xFF00000000000000) >> 56),
					(char)0x66, (char)0x89, (char)0x03, (char)0x5b };
				if (_MagicCode != NULL)
				{
					memcpy((void*)_MagicCode, (void*)Instr, 15);
				}
				return convertInst;
			}
		}
		/*
		------------------------------------------------------------
		mov [ rip + offset ] , al
		------------------------------------------------------------
		*/
		if ((ud_insn_opr(ud_obj, 0)->type == UD_OP_MEM) && (ud_insn_opr(ud_obj, 0)->base == UD_R_RIP))
		{
			if ((ud_insn_opr(ud_obj, 1)->type == UD_OP_REG) && (ud_insn_opr(ud_obj, 1)->base == UD_R_AL))
			{
				uintptr_t addrDest = (RIP + InstSize + (uintptr_t)((ud_insn_opr(ud_obj, 0))->lval).sdword);
				/*
				------------------------------------------------------------
				https://defuse.ca/online-x86-assembler.htm#disassembly
				push rbx
				movabs rbx, (rip + offset)
				mov [rbx], al
				pop rbx
				------------------------------------------------------------
				*/
				convertInst = 14;
				char Instr[14] = { (char)0x53, (char)0x48, (char)0xbb,
					(char)((addrDest & 0x00000000000000FF) >> 0),
					(char)((addrDest & 0x000000000000FF00) >> 8),
					(char)((addrDest & 0x0000000000FF0000) >> 16),
					(char)((addrDest & 0x00000000FF000000) >> 24),
					(char)((addrDest & 0x000000FF00000000) >> 32),
					(char)((addrDest & 0x0000FF0000000000) >> 40),
					(char)((addrDest & 0x00FF000000000000) >> 48),
					(char)((addrDest & 0xFF00000000000000) >> 56),
					(char)0x88, (char)0x03, (char)0x5b };
				if (_MagicCode != NULL)
				{
					memcpy((void*)_MagicCode, (void*)Instr, 14);
				}
				return convertInst;
			}
		}
		/*
		------------------------------------------------------------
		mov [ rip + offset ] , Immediate_8bits
		------------------------------------------------------------
		*/
		if ((ud_insn_opr(ud_obj, 0)->type == UD_OP_MEM) && (ud_insn_opr(ud_obj, 0)->base == UD_R_RIP))
		{
			if ((ud_insn_opr(ud_obj, 1)->type == UD_OP_IMM)  && (ud_insn_opr(ud_obj, 1)->size == 8))
			{
				uintptr_t addrDest = (RIP + InstSize + (uintptr_t)((ud_insn_opr(ud_obj, 0))->lval).sdword);
				/*
				------------------------------------------------------------
				https://defuse.ca/online-x86-assembler.htm#disassembly
				push rax
				push rbx
				movabs rbx, (rip + offset)
				mov al, Immediate_8bits
				mov [rbx], al
				pop rbx
				pop rax
				------------------------------------------------------------
				*/
				convertInst = 18;
				char Instr[18] = { (char)0x50, (char)0x53, (char)0x48, (char)0xbb,
					(char)((addrDest & 0x00000000000000FF) >> 0),
					(char)((addrDest & 0x000000000000FF00) >> 8),
					(char)((addrDest & 0x0000000000FF0000) >> 16),
					(char)((addrDest & 0x00000000FF000000) >> 24),
					(char)((addrDest & 0x000000FF00000000) >> 32),
					(char)((addrDest & 0x0000FF0000000000) >> 40),
					(char)((addrDest & 0x00FF000000000000) >> 48),
					(char)((addrDest & 0xFF00000000000000) >> 56),
					(char)0xb0, (char) ((ud_insn_opr(ud_obj, 1))->lval).sbyte,
					(char)0x88, (char)0x03, (char)0x5b, (char)0x58 };
				if (_MagicCode != NULL)
				{
					memcpy((void*)_MagicCode, (void*)Instr, 18);
				}
				return convertInst;
			}
		}
		/* 
		end
		*/
	}

	if (convertInst == 0)
	{
		convertInst = InstSize;
		if (_MagicCode != NULL)
		{
			memcpy((void*)_MagicCode, (void*)Curent_EIP_RIP, InstSize);
		}
		return convertInst;
	}

	return convertInst;
#endif

}

#if ((defined(WIN32) ||  defined(WIN64)) || defined(__MINGW32__) || defined(__MINGW64__) )
	LONG WINAPI StructuredSignalHandlerSegmentViolation(PEXCEPTION_POINTERS pExceptionPtrs)
{
	size_t InstSize ;
	uintPtrSize EIP;
	ud_t  ud_obj;

#if (defined(WIN64) ||  defined(LIN64))
	EIP = pExceptionPtrs->ContextRecord->Rip;
#else
	#if (defined(WIN32) ||  defined(LIN32))
	EIP = pExceptionPtrs->ContextRecord->Eip;
	#endif
#endif
	
    TRACE_DEFAULT(_STR2WSTR("ACCESS A LA ZONE MEMOIRE PROTEGEE ") printPtr _STR2WSTR(" EN MODE %s"),
		pExceptionPtrs->ExceptionRecord->ExceptionInformation[1],
		((pExceptionPtrs->ExceptionRecord->ExceptionInformation[0]==READ)?_STR2WSTR("LECTURE"):_STR2WSTR("ECRITURE")));
	
	if ((StructuredSignalHandlerSegmentViolation_Flag == false) && (EIP != 0))
	{
		StructuredSignalHandlerSegmentViolation_Flag = true ; 

		if (pExceptionPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
		{
			_mode = (pExceptionPtrs->ExceptionRecord->ExceptionInformation[0]==0)?READ:WRITE;
			_ProtectedMemoryAdress = pExceptionPtrs->ExceptionRecord->ExceptionInformation[1];
			getInstrSizeAndMemorySize(EIP,&InstSize,&_ProtectedMemoryAdressSize, &ud_obj);

			if ((_mode == READ) || (_mode == WRITE))
			{
				if ((__AccessControler__ != NULL) && (__AccessControler__(_ProtectedMemoryAdress,_mode,_ProtectedMemoryAdressSize) == TRUE))
				{
					size_t code_size = ((unsigned __int64)&EndMagicFunction)- ((unsigned __int64)&StartMagicFunction);
					size_t new_code_size = CopyInstruction(NULL,(void*)EIP, InstSize, &ud_obj, _mode, pExceptionPtrs->ContextRecord);
					_MagicCode  = (ULONG_PTR)OS_MemoryAllocation(NULL,code_size+ new_code_size,OS_MEM_RESERVE | OS_MEM_COMMIT,OS_PAGE_EXECUTE_READWRITE);
					memset((void*)_MagicCode, 0x90, new_code_size);
					CopyInstruction((void*)_MagicCode,(void*)EIP,InstSize, &ud_obj, _mode, pExceptionPtrs->ContextRecord);
					CopyMemory((void*)(((char*)_MagicCode)+ new_code_size),(void*)(&StartMagicFunction),code_size) ;

					#if (defined(WIN64) ||  defined(LIN64))
						pExceptionPtrs->ContextRecord->Rip = _MagicCode ;
					#else
						#if (defined(WIN32) ||  defined(LIN32))
							pExceptionPtrs->ContextRecord->Eip = _MagicCode;
						#endif
					#endif

					_True_EIP = EIP + InstSize;
					pExceptionPtrs->ExceptionRecord->ExceptionAddress = (PVOID) _MagicCode;
					OS_MemoryProtect((LPVOID)_ProtectedMemoryAdress, _ProtectedMemoryAdressSize, OS_PAGE_READWRITE , &_OldProtect);
					if ((__Read__ != NULL)&& (_mode == READ))
					{
						__Read__(_ProtectedMemoryAdress, _ProtectedMemoryAdressSize);
					}
					return EXCEPTION_CONTINUE_EXECUTION ;
				}
			}
		}
	}
	InstSize = 0;
	_ProtectedMemoryAdressSize =  0 ;
	StructuredSignalHandlerSegmentViolation_Flag = false;
	return EXCEPTION_EXECUTE_HANDLER ;
}
#else
	void StructuredSignalHandlerSegmentViolation (int sig, siginfo_t *siginfo, void  *contextPtr)
	{
		size_t InstSize ;
		uintPtrSize EIP;
		ud_t  ud_obj;
		ucontext_t*  pExceptionPtrs = (ucontext_t *) contextPtr;

		#ifdef LIN64
			EIP = pExceptionPtrs->uc_mcontext.gregs[REG_RIP] ;
		#else
			#ifdef LIN32
				EIP = pExceptionPtrs->uc_mcontext.gregs[REG_EIP] ;
			#endif
		#endif

		TRACE_DEFAULT("ACCESS A LA ZONE MEMOIRE PROTEGEE "printPtr" EN MODE %s",
			siginfo->si_addr,
			(pExceptionPtrs->uc_mcontext.gregs[REG_ERR] & 0x2)?"ECRITURE":"LECTURE");

		if (StructuredSignalHandlerSegmentViolation_Flag == false)
		{
			StructuredSignalHandlerSegmentViolation_Flag = true ;
			if (sig == SIGSEGV)
			{
				if (pExceptionPtrs->uc_mcontext.gregs[REG_ERR] & 0x2)
				{
					_mode  = WRITE;
				}
				else
				{
					_mode  = READ;
				}
				_ProtectedMemoryAdress = (ULONG_PTR)siginfo->si_addr;
				getInstrSizeAndMemorySize(EIP,&InstSize,&_ProtectedMemoryAdressSize, &ud_obj);
				if ((_mode == READ) || (_mode == WRITE))
				{
					if ((__AccessControler__ != NULL) && (__AccessControler__(_ProtectedMemoryAdress,_mode,_ProtectedMemoryAdressSize) == TRUE))
					{
						size_t code_size = (((size_t)&EndMagicFunction)-((size_t)&StartMagicFunction));
						size_t new_code_size = CopyInstruction(NULL, (void*)EIP, InstSize, &ud_obj, _mode, &(pExceptionPtrs->uc_mcontext));
						_MagicCode  = (ULONG_PTR)OS_MemoryAllocation(NULL,code_size+ new_code_size,OS_MEM_RESERVE | OS_MEM_COMMIT,OS_PAGE_EXECUTE_READWRITE);
						CopyInstruction((void*)_MagicCode, (void*)EIP, new_code_size, &ud_obj,  _mode,  &(pExceptionPtrs->uc_mcontext));
						memcpy((void*)(((char*)_MagicCode)+ new_code_size),(void*)(&StartMagicFunction),code_size) ;
						#ifdef LIN64
							pExceptionPtrs->uc_mcontext.gregs[REG_RIP] = (greg_t)_MagicCode;
						#else
							#ifdef LIN32
							pExceptionPtrs->uc_mcontext.gregs[REG_EIP] = (greg_t)_MagicCode;
							#endif
						#endif

						_True_EIP = (ULONG_PTR) (EIP + InstSize);
						OS_MemoryProtect((LPVOID)_ProtectedMemoryAdress, _ProtectedMemoryAdressSize, OS_PAGE_READWRITE , &_OldProtect);
						if ((__Read__ != NULL)&& (_mode == READ))
						{
							__Read__(_ProtectedMemoryAdress, _ProtectedMemoryAdressSize);
						}
						#ifdef LIN64
						pExceptionPtrs->uc_mcontext.gregs[REG_RIP] = (long int)_MagicCode;
						#else
							#ifdef LIN32
							pExceptionPtrs->uc_mcontext.gregs[REG_EIP] = (long int)_MagicCode;
							#endif
						#endif
						StructuredSignalHandlerSegmentViolation_Flag = false;
						return ;
					}
				}
			}
		}
		InstSize = 0;
		_ProtectedMemoryAdressSize =  0 ;
		StructuredSignalHandlerSegmentViolation_Flag = false;
		//return ;
		fprintf(stderr,"Segmentation fault !!!\n");
		fflush(stderr);
		fflush(stdout);
		exit(-1);
	}
#endif




