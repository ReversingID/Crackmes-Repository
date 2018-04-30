======================== Phase 0 ==========================================


crackme.zip is realy x86 exe file. build with static c/c++ CRT. no optimization.


in the main function:


2 checks for debugger:


  if (IsDebuggerPresent()) ExitProcess(0);


and


	if (((_PEB*)(ULONG_PTR)__readfsdword(FIELD_OFFSET(_TEB, ProcessEnvironmentBlock)))->BeingDebugged) ExitProcess(0);


here we simply can zero BeingDebugged in PEB


the time check via GetTickCount


dword[ebp-14]=GetTickCount();

dword[ebp-10]=GetTickCount(); <=100ms


most simply avoid this - patch GetTickCount for it always return 0 (xor eax,eax;ret - 3 bytes - 33 c0 c3)


read 10 bytes password (buf), and first do several checks:


buf[7]+buf[6]=cd

buf[8]+buf[5]=c9

buf[7]+buf[6]+buf[3]=13a

buf[9]+buf[4]+buf[8]+buf[5]=16f

buf[1]+buf[0]=c2

buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6]+buf[7]+buf[8]+buf[9]=39b


and finally compute it hash as:


#pragma intrinsic(_rotr)


 ULONG hash(PBYTE sz, ULONG n)

{

	ULONG x = 0;

	do 

	{

		x = _rotr(x, 9) ^ *sz++;

	} while (--n);

	return x;

}


 hash must be 1928f914


i am not strong for elegant crack hash here, so i simply decided use brute force:


 void brutforce()

{

	BYTE buf[10];

	buf[2] = 0x30;

	buf[3] = 0x6D;


 	buf[0] = ' ', buf[1] = 0xc2 - ' ';

	do 

	{

		buf[4] = ' ', buf[9] = 0xa6 - ' ';


 		do 

		{

			buf[5] = ' ', buf[8] = 0xc9 - ' ';


 			do 

			{

				buf[6] = ' ', buf[7] = 0xcd - ' ';


 				do 

				{

					if (hash(buf, 10) == 0x1928f914)

					{

						DbgPrint("%.10s\n", buf);

						return;

					}


 				} while (++buf[6], --buf[7] >= ' ');


 			} while (++buf[5], --buf[8] >= ' ');


 		} while (++buf[4], --buf[9] >= ' ');


 	} while (++buf[0], --buf[1] >= ' ');

}


 after several seconds i got password - "Pr0m3theUs"


with it i decrypt message:



"Congratulations! You guessed the right password, but the message you see is wrong.

Try to look for some unreferenced data, that can be decrypted the same way as this text."


i view that at offset 0x100 from it also encrypted data:


"https://join.eset.com/ae50b61499d27d7da010c718f265a9a1/crackme.zip"


i download file - now this is really zip archive with 3 files:


info.txt

EsetCrackme2015.dll

EsetCrackme2015.exe


so let look for `EsetCrackme2015.exe`


========================== Phase 1 =======================================


at first this exe not used any c/c++ CRT. at second very notable (as in first exe) the "A" (ansi) api is called. this is very bad choise for programming and make live for reverse enginer more easy :) say if we need hook call to `CreateFileW` or `LoadLibraryW` - we set bp on it, but got very many stops - because differeent system component calls this W api too. but if we set bp on CreateFileA or LoadLibraryA - we got only "interesting" calls. because system itself never call A version.


+++++++++++++++++++++++++++++++++++++++++++++

	CreateMutexA(0, TRUE, "EssetCrackme2015");

	// how about check returned handle ?!

	if (GetLastError() == ERROR_ALREADY_EXISTS)

	{

		MessageBoxA(0, "Application already launched ...", "Error", MB_ICONWARNING|MB_OK);

	}

	else

	{

		CHAR name[MAX_PATH];

		GetModuleFileNameA(0, name, RTL_NUMBER_OF(name));

		// of course check Return Value not need :)

		memcpy(name + strlen(name) - 3, "dll", 3);


 		if (!LoadLibraryA(name))

		{

			MessageBoxA(0, name, "Missing DLL file", MB_ICONWARNING|MB_OK);

		}

	}

	// ExitProcess who will be called ??? 

---------------------------------------------


so most interesting happens in EsetCrackme2015.dll which is encrypted. ok - try set BPs on most interesting calls, like create file, thread, process, key, VirtualAlloc[Ex] and look what will be (if function begin with mov edi,edi - set bp on next instruction)


1.) before any api call - called func_401C03 function in exe. it return address(eax) which will be called. so this let us easy track all api calls.

so set bp at VA_401C52 (ret 4). frequently func_401C03 called not direct but from small shell func_401c79 (when api called from dll primary). bp - VA_401C9B (ret 4).





in any case, now we easily can track all calls;


2.) after first VirtualAlloc call we found very interesting function in EsetCrackme2015.dll at 0ac9 rva:


/* decode routine */

void rva_0AC9(PVOID pvData, ULONG cbData);


it take pointer to some data and it size and decode it. so set bp on begin and end of this routine and we will be look decrypted data


here used some NOT ALIGNED(!!) structure


#pragma pack(push,1)

struct S1

{

	USHORT NotInteresting;

	ULONG Size;

	UCHAR Data[/*Size*/];

};

#pragma pack(pop)


first, what decoded - some codeblock, size 200c.





second what decoded - some EXE file with size 1e000. this already interesting, let save it on disk







eset.exe

SHA256: 	d4da1e83f207d268b97f921c2916bc44658c4734772079d479ad329369a71876 


ok, look what next


CreateProcessA("<GetSystemDirectory()>\svchost.exe", suspended);

VirtualAllocEx(hProcess, 0x400000, 18000)


note - this is frequently fail on win10 because address range already in use.






if, sometime, this code not fail - next was injecting this eset.exe to svchost, call SetThreadContext, for redirect to eset.exe entry point and resume thread.

if code fail, we got still running EsetCrackme2015.exe and svchost.exe zombie in suspended state "live" in system.

ok, let kill unhappy svchost and direct exec eset.exe in debugger without trick with stupid implementation


before begin debug eset.exe - i look for EsetCrackme2015.exe and view notable file \Device\NamedPipe\EsetCrackmePipe.


ok, clear that EsetCrackme2015.exe will be do some IPC with eset.exe via this EsetCrackmePipe, also name is unicue enough - let search it in google and..i take 3 results:

https://quequero.org/2016/01/eset-crackme-challenge-2015-walkthrough/

http://mshetta.blogspot.com/2016/11/join-eset-crackme-2015-solution.html

https://github.com/fvrmatteo/CV/tree/master/ESET2015


of course i read this, of course i can use this ready results. but let go forward anyway


=========================== Phase 2 ======================================

eset.exe:


here we have DialogBoxParamW (W !! i can not believe, really not A ?!) call in entry point and all.

so all is happens in DialogFunc at 1830 rva.

only WM_INITDIALOG and WM_COMMAND handled.

ok, what is on WM_INITDIALOG ?

create some thread (rva_2340) (let suspend it until)

then called function, which can be more effective implemented in single api call 

RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE, TRUE, FALSE, &b);

for what you need SE_DEBUG_PRIVILEGE ?! result you not check. if fail - so fail. but ok.

than already interesting proc (rva_1680) called


here used next procedure at rva_1f90. it comunicate with `EsetCrackme2015.exe` via `EsetCrackmePipe`


DWORD SendPipeCommand([in] BYTE Type, [in] USHORT ID, [out] void** ppvData, ULONG TryCount);


it send to pipe 1 byte `Type` and then 2 bytes `ID` and then read from pipe:

1. one byte return `Type`

2. 2 bytes ID

3. 4 bytes - size of next DATA

4. allocate from heap this `size`

5. read to allocated buffer `size` bytes

6. store buffer in *ppvData

7. return `size`


the TryCount - how many try connect to pipe if attempt fail. in exe TryCount set to ffffffff (so infinite, from init dialog) and to 5 (when button is pressed)


let describe call to SendPipeCommand as Request[Type, ID]=size (and let ignore TryCount)


so return to rva_1f90 proc:


Request[1, bb01]=7b


checked that 7b data size exactly returned


data tranformed by xor and copied to global variable v_12c90. here we have 3 40 bytes null terminated strings

3*(40+1)=123=7b






Request[1, bb02]=11


again xor and we have string `RFV1aV4fQ1FydFxk` which copied to global vaiable v_13008.





generally this is all what is done on WM_INITDIALOG. dialog become visible - 3 edits, 3 buttons.


let resume rva_2340 thread and look what is here. some base64 decoding and called SetWindowLong for first edit with GWLP_WNDPROC to rva_2360.

thread exit.


rva_2360 do special processing on WM_GETTEXT message, all another skip to default procedure


so need enter some data to edit and press button.


when button pressed executed next code at rva_1760:

GetDlgItem

GetWindowTextA

look like SHA-1(text) (how minimum 67452301 EFCDAB89 98BADCFE 10325476 C3D2E1F0 used and 20 byte hash)

and hash compared with stored at v_12c90 + 29*i via lstrcmpA


so compared user text hash with pre calculated hash. if hash ok (or we change eax to 0 after lstrcmpA)

Request[2, bb00+i] will be send from DialogProc (where i in [1,2,3] - password number)


but when first password entered, because edit is subclassed, when we call GetWindowTextA - rva_2360 WM_GETTEXT handler called.


entered text some transformed (look like base64 but not exactly) and compared via lstrcmpA with v_13008 string (loaded as [1,bb02])-RFV1aV4fQ1FydFxk)

if compare ok (set eax to 0) was send signal Request[2,bb01] to pipe


here i use ready result - "What it do is it base64 encode the password then decrement each odd indexed character, finally it compares it with RFV1aV4fQ1FydFxk which it got from the file 0xBB02."


so `RFV1aV4fQ1FydFxk` -> `RGV2aW4gQ2FzdGxl` -> base64 decode -> `Devin Castle`


this is valid first password.


when we enter it - Request[2, bb01] will be send


on this request /* decode routine */

void rva_0AC9(PVOID pvData, ULONG cbData);


again called in EsetCrackme2015.exe with 8e8e size - this is zip file. and drv.zip is saved on disk. (really 2 time)











============================== Phase 4 ====================================


so what do with driver ? let be original, not listen instructions and do some not usual things. we make user mode dll from this driver.

we need set IMAGE_FILE_DLL in IMAGE_FILE_HEADER.Characteristics. after this we can simply load it with LoadLibraryW.

but how will be with import from ntoskrnl.exe ?

we make own exe which implement this import yourself.

at first ~half api really implemented in ntdll.dll. so we simply forward this import to ntdll.

rest need implement yourself, but emulation not so hard and big. as result we will be not need debug driver but user mode dll and have trace log from kernel api which we implement.

first need create skeleton of src file - parse ntifs.h, ntddk.h and wdm.h - create list of functions (with signature) which is `NTKERNELAPI` marked. then walk by driver import. if import name found in ntdll - forward this function (in def file) to ntdll, like `ZwCreateFile=ntdll.ZwCreateFile`, for `NTKERNELAPI` write empty body. and finally need be implement functions bodies. parser work ok, only `KeTickCount` was not `NTKERNELAPI` and not imported from ntdll.


ok. assume we somehow implement ~30 api. but how with driver entry point which different from dll entry point ? for this will be use `LdrRegisterDllNotification`. when get notify about driver loaded (detect it by TimeDateStamp 54e368c5) - found it LDR_DATA_TABLE_ENTRY, save EntryPoint (wich is actually PDRIVER_INITIALIZE type) and replace to own DllMain. from which we called original EntryPoint and then AddDevice and finally send IRP_MN_START_DEVICE to it


ok. go... run own ntoskrnl.exe which call LoadLibraryW(L"crackme_drv.sys"):


*********************************************************************************

** driver initialization


Eset DriverObject=0148A8A8

53179034>NT::IoAllocateDriverObjectExtension(0148A8A8 c)

ClientDriverExtension=01489540

53179077>NT::ExAllocatePoolWithTag(0000006C, Ram1)=038D05B8

DriverEntry=0

IoCreateDevice:01485C70 0 2a (null)

DeviceObject=01486828

PhysicalDeviceObject=01486828

5317116C>NT::IoGetDriverObjectExtension(0148A8A8)

ClientDriverExtension=01489540

IoCreateDevice:0148A8A8 98 7 \Device\45736574

DeviceObject=014882C8


RtlQueryRegistryValues called here for check some data under `Parameters`: "DiskSize", "RootDirEntries", "SectorsPerCluster", "DriveLetter"

i set "DiskSize" to 0x100000 (1MB) under registry, it and allocated on next call


53171255>NT::ExAllocatePoolWithTag(00100000, Ram2)=02DA5020


FAT structures is intialized here, at begin of "Ram2" memory allocated.

finally calculated size of FAT structures at begin of "disk", offset from which user data is started

and stored at DWORD v_04F4;






5317129D>NT::IoAttachDeviceToDeviceStack(01486828 <- 014882C8)

531712D0>NT::KeInitializeEvent(01488430[000000D4], 0 0)

53171317>NT::PsCreateSystemThread(pfn=53171F20, ctx=014882C8)=0, 000001B4 // handle not closed !!

53171336>NT::ObReferenceObjectByHandle(000001B4)=01485F00[000001BC]

5317137D>NT::PsCreateSystemThread(pfn=531722F0, ctx=014882C8)=0, 000001C0 // handle not closed !!

5317139C>NT::ObReferenceObjectByHandle(000001C0)=01485D48[000001C4]

AddDevice=0

Eset DeviceObject=014882C8

00096A33>NT::KeInitializeEvent(010FF8C0[000001B8], 0 0)

00096A47>NT::IofCallDriver(014882C8, 014873E8)

531715E2>NT::KeInitializeEvent(010FF870[000001C8], 0 0)

53171612>NT::IofCallDriver(01486828, 014873E8)

000976B2>NT::OkDeviceRequest(01486828, 014873E8 1b.0)

00097BBC>NT::IofCompleteRequest(014873E8 [0, 00000000] 1/2)

CompletionRoutine(014882C8, 014873E8, 010FF870)...

53171951>NT::KeSetEvent(010FF870[000001C8])=0

CompletionRoutine(014882C8, 014873E8, 010FF870)=c0000016

5317165E>NT::IofCompleteRequest(014873E8 [0, 00000000] 2/2)

IopDeleteIrp(014873E8)

StartDevice=0


*********************************************************************************

** thread#1 (rva_1F20)


53171F67>NT::KeWaitForSingleObject(01488430[000000D4], 0 10000000)...

53171F67>NT::KeWaitForSingleObject(01488430[000000D4], 0 10000000)=102


Request[1,aa02]=9 ("ESETConst" after decrypt)


 53172152>NT::ExAllocatePool(9)=038D0640

531731EA>NT::ExAllocatePool(408)=038D0668

53173219>NT::ExFreePoolWithTag(038D0668, )


v_74F8 - UNICODE_STRING "ESETConst" // critical error here in driver !


so here thread send Request[1,aa02] to user mode and get back 9 bytes. after decrypt - "ESETConst" (NOT NULL TERMINATED !!)

and you use on this string RtlInitAnsiString which wait Pointer to a null-terminated string.. so you make fatal error here.





as result i need make special fix for this


 531721E5>NT::ExFreePoolWithTag(038D0640, )

53172269>NT::PsCreateSystemThread(pfn=53172670, ctx=014882C8)=0, 000001CC

53172286>NT::PsCreateSystemThread(pfn=531719E0, ctx=014882C8)=0, 000001D0

531722A5>NT::ObReferenceObjectByHandle(000001CC)=0148A4F0[000001D4]

531722C3>NT::ObReferenceObjectByHandle(000001D0)=01489170[000001D8]

531722DF>NT::PsTerminateSystemThread(0)


*********************************************************************************

** thread#2 (rva_22F0)


5317232F>NT::ExAllocatePool(21ac)=038D0640

531731EA>NT::ExAllocatePool(408)=038D2808

53173219>NT::ExFreePoolWithTag(038D2808, )


some base independed code (21ac size) is decrypted here






53172376>NT::ExAllocatePool(2b5)=038D2808

531731EA>NT::ExAllocatePool(408)=038D2AD8

53173219>NT::ExFreePoolWithTag(038D2AD8, )


some data (2b5 size) is decrypted here


531723D3>NT::KeWaitForSingleObject(01488430[000000D4], 0 10000000)...

531723D3>NT::KeWaitForSingleObject(01488430[000000D4], 0 10000000)=102


request[1,aa06]=48


5317258D>NT::ExAllocatePool(48)=038D2AD8

531731EA>NT::ExAllocatePool(408)=038D2B38

53173219>NT::ExFreePoolWithTag(038D2B38, )


48 bytes copied at offset 26d from 2b5 data block (so exactly to the end of block)






53172646>NT::ExFreePoolWithTag(038D2AD8, )


BOOLEAN v_04f0 = TRUE;


5317265C>NT::PsTerminateSystemThread(0)


*********************************************************************************

** thread#3 (rva_2670)


 53172700>NT::KeWaitForSingleObject(01488430[000000D4], 0 10000000)...

53172700>NT::KeWaitForSingleObject(01488430[000000D4], 0 10000000)=102


request[1, aa10]=ignored


--------------------------------------------------


at this point in user mode decrypted and saved to current folder:

PunchCardReader.exe and PuncherMachine.exe

decrypted name \\?\GLOBALROOT\Device\45736574

decrypted some data and try save it as \\?\GLOBALROOT\Device\45736574\PunchCard.bmp

but i path and store it to current folder also as PunchCard.bmp


note - in user mode i hold bp now at decrypt routine rva_0AC9 and CreateFileA only

--------------------------------------------------


531727C9>NT::PsTerminateSystemThread(0)


*********************************************************************************

** thread#4 (rva_19E0)


53171A57>NT::IoGetDriverObjectExtension(0148A8A8)

ClientDriverExtension=01489540


 @@loop:

53171A89>NT::KeWaitForSingleObject(01488430[000000D4], 0 20000000)...

53171A89>NT::KeWaitForSingleObject(01488430[000000D4], 0 20000000)=102


RtlCheckRegistryKey(0, HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\eset\EsetCrackme)


ZwQueryValueKey( "ESETConst", KeyValuePartialInformation )


53171C0F>NT::ExAllocatePool(10)=038D2AD8


wait for REG_SZ data type only


53171CB3>NT::ExAllocatePool(4)=038D2B00


 53171D24>NT::ExFreePoolWithTag(038D2B00, )


convert value string to ansi


strcmp(v_0500, str)

memcpy(v_0500, str)

BOOLEAN v_04f1 = TRUE;


53171DBD>NT::ExFreePoolWithTag(038D2AD8, )


goto loop; //suspend thread here again



*********************************************************************************

** my code


i create file object "\\Device\\45736574\\PunchCard.bmp" (really driver ignore file names - too primitive)

send IOCTL_DISK_GET_DRIVE_GEOMETRY and IOCTL_DISK_GET_PARTITION_INFO ioctls - what calc user data begin offset on FAT (v_04F4).

but simply write data to the end of disk. it simply copied (memmove) to Ram2 memory at ByteOffset which i set

and finally send Read request. device check that i read user space data (offset >= v_04F4) (interesting that on write - no this check - so we can overwrite FAT structures data)

so if we read from user partition space:

if both v_04f0 (some code and data unpacked) and v_04f1 ("ESETConst" is not empty string) is true called some func_30E0

otherwise RtlRandom called for data before copy


00096A33>NT::KeInitializeEvent(010FFBF4[000001D0], 0 0)

00096A47>NT::IofCallDriver(014882C8, 01486148)

5317804D>NT::IofCompleteRequest(01486148 [0, 00000000] 2/2)

IopDeleteIrp(01486148)

00096A33>NT::KeInitializeEvent(010FFBE0[000001D0], 0 0)

00096A47>NT::IofCallDriver(014882C8, 01486148)

531729CA>NT::IofCompleteRequest(01486148 [0, 00000018] 2/2)

IopDeleteIrp(01486148)

00096A33>NT::KeInitializeEvent(010FFBE0[000001D0], 0 0)

00096A47>NT::IofCallDriver(014882C8, 01486148)

531729CA>NT::IofCompleteRequest(01486148 [0, 00000020] 2/2)

IopDeleteIrp(01486148)

00096A33>NT::KeInitializeEvent(010FFBE8[000001CC], 0 0)

00096A47>NT::IofCallDriver(014882C8, 01486148)

53172D5C>NT::IofCompleteRequest(01486148 [0, 00014000] 2/2)

IopDeleteIrp(01486148)

00096A33>NT::KeInitializeEvent(010FFBE8[000001D0], 0 0)

00096A47>NT::IofCallDriver(014882C8, 01486148)


func_30E0 somehow transform our data (which we first write to Ram2). so need reverse it



struct TRANSFORM_CONTEXT

{

	PBYTE pbData;

	SIZE_T cbData;

	PVOID pvKey;

	SIZE_T cbKey;

};


 struct SOME_OBJECT

{

	/*000*/WORD m_Version;//3713

	/*002*/ULONG m_Size;//2b5

	/*006*/ULONG m_offset1;

	/*00a*/ULONG m_offset2;

	/*00e*/BOOL m_bKernelMode;

};


 struct CODE

{

	/*000*/void (*m_Fns[0xff])();// !! yes, not 0x100

	/*3fc*/BOOLEAN m_bQuit;


 	/*405*/BOOL m_bKernelMode;

	/*409*/HMODULE m_hmod;// must export ExAllocatePool and ExFreePool

	/*40d*/ULONG m_u;

	/*411*/TRANSFORM_CONTEXT * m_ctx;

	/*416*/LONG_PTR m_Result;

	/*456*/PBYTE m_stream;

	/*45a*/PVOID m_pO2;

	/*45e*/ULONG m_ptr;


 	CODE();

	~CODE();


 	// fn_0282C880 hash

	HMODULE GetModuleByHash(ULONG hash);//282CA00

	PVOID GetProcAddressByHash(HMODULE hmod, ULONG hash);


 	LONG_PTR Run(const SOME_OBJECT* pObject, HMODULE hmod, ULONG u_4, TRANSFORM_CONTEXT* ctx)

	{

		m_u = u_4;

		m_ctx = ctx;

		m_bKernelMode = pObject->m_bKernelMode;


 		enum {

			kernel32_hash = 0x19bdac28

		};


 		if (!hmod && !m_bKernelMode)

		{

			m_hmod = GetModuleByHash(kernel32_hash);//kernel32.dll

		}

		else

		{

			m_hmod = hmod;

		}


 		SOME_OBJECT* pNewObject = (SOME_OBJECT*)new UCHAR[pObject->m_Size];


 		memcpy(pNewObject, pObject, pObject->m_Size);


 		//...


 		m_bQuit = FALSE;


 		do

		{

			m_Fns[m_stream[m_ptr++]]();// what will be if 0xff in stream ?!

		} while (!m_bQuit);


 		return m_Result;

	}


 	static LONG_PTR __stdcall _Run(const SOME_OBJECT* pObject, HMODULE hmod, ULONG u_4, TRANSFORM_CONTEXT* ctx)

	{

		CODE This;

		return This.Run(pObject, hmod, u_4, ctx);

	}

};


 char g_ESETConst[64];


 typedef LONG_PTR (__cdecl * CODE_RUN)(const SOME_OBJECT* pObject, HMODULE hmod, ULONG u_4, TRANSFORM_CONTEXT* ctx);


 void func_3080(PBYTE pbData, SIZE_T cbData, SOME_OBJECT* pObject, HMODULE hmod, CODE_RUN Code)

{

	TRANSFORM_CONTEXT ctx = { pbData, cbData, g_ESETConst, strlen(g_ESETConst) };


 	// called as __cdecl !!

	Code(pObject, hmod, 4, &ctx);// Code -> CODE_RUN::_Run

}


 struct ESET_DEV_EXT

{

	CODE_RUN Code;			//21ac size

	SOME_OBJECT* pObject;	//2b5 size

};


 void Transform_30E0(PDEVICE_OBJECT DeviceObject, PBYTE pbData, SIZE_T cbData)

{

	ESET_DEV_EXT* DeviceExtension = (ESET_DEV_EXT*)DeviceObject->DeviceExtension;


 	const ULONG BlockSize = 0x200;


 	if (SIZE_T BlockCount = (cbData + BlockSize - 1) / BlockSize)

	{

		PVOID hmod = PcToFileHeader(ZwClose);


 		CODE_RUN Code = DeviceExtension->Code;

		SOME_OBJECT* pObject = DeviceExtension->pObject;


 		do

		{

			func_3080(pbData, BlockSize, pObject, (HMODULE)hmod, Code);


 		} while (pbData += BlockSize, --BlockCount);

	}

}


first of all it search for module base which export ZwClose api. than take from DeviceExtension saved pointer to code (21ac) and data (2b5) blocks.

this which thread#2 (rva_22F0) prepare. calculated how many blocks of 0x200 bytes containing our data. and called sequential called func_3080 on every 200 bytes block

func_3080 simply calculate length of string readed from "ESETConst" and call code (21ac).

but here you make mistake again - you call "code" as __cdecl function, while i view that it is really __stdcall

also why you many time call strlen in loop ? too hard do this once before loop and not use shim func_3080 ?


the function which is called from "code" - is __stdcall static member function of some object (~0x470 byte size)


	static LONG_PTR __stdcall _Run(const SOME_OBJECT* pObject, HMODULE hmod, ULONG u_4, TRANSFORM_CONTEXT* ctx)

	{

		CODE This;

		return This.Run(pObject, hmod, u_4, ctx);

	}


at begin of CODE object is table of 0xff function pointers. i sure that this is else one your error (at all too many errors and not the best code) - must be 0x100 functions.


you call it in loop


		m_bQuit = FALSE;


 		do

		{

			m_Fns[m_stream[m_ptr++]]();

		} while (!m_bQuit);


what will be if 0xff in stream ?!


the code design (by idea only) run in both kernel and user mode. mode is get from SOME_OBJECT.m_bKernelMode (2b5 object) and copied to CODE.m_bKernelMode.

but in general it's amazing that it seems like you do not know what data alignment is. when you see how the data is located in your structures it really infuriates.


if m_bKernelMode - CODE try found and call ExAllocatePool and ExFree pool from hmod (module which export ZwClose). otherwise first you search for kernel32.dll by hash and try call VirtualAlloc and VirtualFree from it, but at some place anyway try found ExFreePoll in kernel32.. so code running as user-mode crashed. else one your error

during transformation process you allocate several memory blocks and free it at the end, but not all:


00EB0B36>NT::ExAllocatePool(2b5)=015B28D0

00EB0B36>NT::ExAllocatePool(400000)=02F1C020

00EB0B36>NT::ExAllocatePool(10)=015A9190 // !! memory not free !!

00EB0B36>NT::ExAllocatePool(137)=015A6C50

00EB0B36>NT::ExAllocatePool(400000)=0332D020

00EB0B36>NT::ExAllocatePool(100)=015A8578

00EB0BB6>NT::ExFreePool(015A8578)

00EB0BB6>NT::ExFreePool(015A6C50)

00EB0BB6>NT::ExFreePool(0332D020)

00EB0BB6>NT::ExFreePool(015B28D0)

00EB0BB6>NT::ExFreePool(02F1C020)


you forget free 0x10 bytes memory block. count of errors is amazing


function do some memory transformation. 2 sequential transformations return data to original state. so `fn(fn(X))==X`


of course bmp not decoded correct - the my arbitrary data, which i enter to "ESETConst" key is used. but need not arbitrary but exactly string. from where get this string ? don't know how many time i thinking about this but Mohamed Shetta blog for hint - simply pass 0,0 as pvKey, cbKey in TRANSFORM_CONTEXT. obviously choice but hard found it. with this bmp is decryped ok.







but key2 i anyway until not found. i decide set bp on memory allocations and especially free - for view content of allocated memory blocks - we can wait some decrypted data here (especially free). also at this stage i not need use your driver more - i save on disk both 21ac and 2b5 memory blocks. and know how need use it. also i need export ExAllocatePool and ExFreePool from self exe and pass own &__ImageBase as hmod. this work great and make debugging more easy.


first of all i spy context of 10 byte block which you not free and found that it hold TRANSFORM_CONTEXT structure. only pvKey, cbKey already not 0 but point to "Barbakan Krakowski" string. this is second password (for edit 2) in eset.exe












========================================================================

about #3 stage - both PuncherMachine.exe and PunchCardReader.exe is CLR images. i have zero knowledge on CLR and all my debuging techique almost always fail for CLR. i can trace only windows api calls, but what is executed in managed code - complete black box for me.


i only note that read 2bo data from - request[1,ff00]=2b0. but how this data is used - i not catch. i try set bp on memory access. but this any way inside managed code (this access) - i not understand how it used - too many transormation, movs etc.

