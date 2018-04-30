#include "StdAfx.h"

_NT_BEGIN

HANDLE ghHeap;

#pragma warning(disable : 4100 4273)

NTSTATUS OkDeviceRequest (PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

	DbgPrint("%p>%s(%p, %p %x.%x)\n", _ReturnAddress(), __FUNCTION__, DeviceObject, Irp, IrpSp->MajorFunction, IrpSp->MinorFunction);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IofCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS InvalidDeviceRequest (PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

	DbgPrint("%p>%s(%p, %p %x.%x)\n", _ReturnAddress(), __FUNCTION__, DeviceObject, Irp, IrpSp->MajorFunction, IrpSp->MinorFunction);

	Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
	Irp->IoStatus.Information = 0;
	IofCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_INVALID_DEVICE_REQUEST;
}

struct IO_CLIENT_EXTENSION 
{
	IO_CLIENT_EXTENSION * NextExtension;
	void * ClientIdentificationAddress;
	UCHAR Data[];

	void* operator new(size_t, ULONG size)
	{
		return LocalAlloc(0, FIELD_OFFSET(IO_CLIENT_EXTENSION, Data[size]));
	}
};

struct DRIVER_EXTENSION_EX : DRIVER_EXTENSION 
{
	IO_CLIENT_EXTENSION * ClientDriverExtension;

	DRIVER_EXTENSION_EX(PDRIVER_OBJECT DrvObject)
	{
		DriverObject = DrvObject;
		AddDevice = 0;
		Count = 0;
		ServiceKeyName = {};
		ClientDriverExtension = 0;
	}

	void* operator new(size_t, PDRIVER_OBJECT DrvObject)
	{
		return DrvObject + 1;
	}
};

PDRIVER_OBJECT AllocateDriver(PDRIVER_DISPATCH MajorFunction = InvalidDeviceRequest)
{
	if (PDRIVER_OBJECT DriverObject = (PDRIVER_OBJECT)new UCHAR[sizeof(DRIVER_OBJECT)+sizeof(DRIVER_EXTENSION_EX)])
	{
		RtlZeroMemory(DriverObject, sizeof(DRIVER_OBJECT)+sizeof(DRIVER_EXTENSION_EX));

#ifdef _WIN64
		__stosq
#else
		__stosd
#endif
			((PULONG_PTR)DriverObject->MajorFunction, (ULONG_PTR)MajorFunction, RTL_NUMBER_OF(DriverObject->MajorFunction));

		DriverObject->Type = IO_TYPE_DRIVER;
		DriverObject->Size = sizeof(DRIVER_OBJECT);
		DriverObject->DeviceObject = 0;
		DriverObject->DriverInit = 0;
		DriverObject->DriverExtension = new(DriverObject) DRIVER_EXTENSION_EX(DriverObject);

		return DriverObject;
	}

	return 0;
}

void CloseEvent(PRKEVENT Event)
{
	NtClose(*(PHANDLE)&Event->Header.SignalState);
}

VOID
KeInitializeEvent (
				   _Out_ PRKEVENT Event,
				   _In_ EVENT_TYPE Type,
				   _In_ BOOLEAN State
				   )
{
	RtlZeroMemory(Event, sizeof(KEVENT));
	Event->Header.Type = (UCHAR)Type;
	Event->Header.Size = sizeof(KEVENT);
	ZwCreateEvent((PHANDLE)&Event->Header.SignalState, EVENT_ALL_ACCESS, 0, Type, State);
	DbgPrint("%p>%s(%p[%p], %x %x)\n", _ReturnAddress(), __FUNCTION__, Event, *(PHANDLE)&Event->Header.SignalState, Type, State);
}

LONG
KeSetEvent (
			_Inout_ PRKEVENT Event,
			_In_ KPRIORITY /*Increment*/,
			_In_ _Literal_ BOOLEAN /*Wait*/
			)
{
	LONG PreviousState;
	ZwSetEvent(*(PHANDLE)&Event->Header.SignalState, &PreviousState);
	DbgPrint("%p>%s(%p[%p])=%x\n", _ReturnAddress(), __FUNCTION__, Event, *(PHANDLE)&Event->Header.SignalState, PreviousState);
	return PreviousState;
}

NTSTATUS
KeWaitForSingleObject (
					   _In_ _Points_to_data_ PVOID Object,
					   _In_ _Strict_type_match_ KWAIT_REASON /*WaitReason*/,
					   _In_ __drv_strictType(KPROCESSOR_MODE/enum _MODE,__drv_typeConst) KPROCESSOR_MODE /*WaitMode*/,
					   _In_ BOOLEAN Alertable,
					   _In_opt_ PLARGE_INTEGER Timeout
					   )
{
	HANDLE hObject = *(PHANDLE)&reinterpret_cast<DISPATCHER_HEADER*>(Object)->SignalState;
	LARGE_INTEGER t = {};
	if (Timeout) t.QuadPart = -Timeout->QuadPart;
	DbgPrint("%p>%s(%p[%p], %x %I64u)...\n", _ReturnAddress(), __FUNCTION__, Object, hObject, Alertable, t.QuadPart);
	NTSTATUS status = ZwWaitForSingleObject(hObject, Alertable, Timeout);
	DbgPrint("%p>%s(%p[%p], %x %I64u)=%x\n", _ReturnAddress(), __FUNCTION__, Object, hObject, Alertable, t.QuadPart, status);
	return status;
}

PVOID
NTAPI
ExAllocatePoolWithTag (
					   _In_ __drv_strictTypeMatch(__drv_typeExpr) POOL_TYPE /*PoolType*/,
					   _In_ SIZE_T NumberOfBytes,
					   _In_ ULONG Tag
					   )
{
	PVOID pv = HeapAlloc(ghHeap, 0, NumberOfBytes);
	ULONG64 t = Tag;
	DbgPrint("%p>%s(%p, %s)=%p\n", _ReturnAddress(), __FUNCTION__, NumberOfBytes, &t, pv);
	return pv;
}

VOID
ExFreePoolWithTag (
				   _Pre_notnull_ __drv_freesMem(Mem) PVOID P,
				   _In_ ULONG Tag
				   )
{
	ULONG64 t = Tag;
	DbgPrint("%p>%s(%p, %s)\n", _ReturnAddress(), __FUNCTION__, P, &t);
	HeapFree(ghHeap, 0, P);
}

#undef ExFreePool
VOID
ExFreePool (
				   _Pre_notnull_ __drv_freesMem(Mem) PVOID P
				   )
{
	DbgPrint("%p>%s(%p)\n", _ReturnAddress(), __FUNCTION__, P);
	HeapFree(ghHeap, 0, P);
}


_Must_inspect_result_
NTSTATUS
PsCreateSystemThread(
					 _Out_ PHANDLE ThreadHandle,
					 _In_ ULONG /*DesiredAccess*/,
					 _In_opt_ POBJECT_ATTRIBUTES /*ObjectAttributes*/,
					 _In_opt_  HANDLE /*ProcessHandle*/,
					 _Out_opt_ PCLIENT_ID ClientId,
					 _In_ PKSTART_ROUTINE StartRoutine,
					 _In_opt_ _When_(return >= 0, __drv_aliasesMem) PVOID StartContext
					 )
{
	NTSTATUS status = RtlCreateUserThread(NtCurrentProcess(), 0, TRUE, 0, 0, 0, StartRoutine, StartContext, ThreadHandle, ClientId);
	DbgPrint("%p>%s(pfn=%p, ctx=%p)=%x, %p\n", _ReturnAddress(), __FUNCTION__, StartRoutine, StartContext, status, *ThreadHandle);
	return status;
}

PDEVICE_OBJECT
IoAttachDeviceToDeviceStack(
								_In_  PDEVICE_OBJECT SourceDevice,
								_In_  PDEVICE_OBJECT TargetDevice
								)
{
	DbgPrint("%p>%s(%p <- %p)\n", _ReturnAddress(), __FUNCTION__, TargetDevice, SourceDevice);

	if (SourceDevice->DeviceObjectExtension->AttachedTo)
	{
		__debugbreak();
	}

	while (PDEVICE_OBJECT AttachedDevice = TargetDevice->AttachedDevice)
	{
		TargetDevice = AttachedDevice;
	}
	
	InterlockedIncrement(&TargetDevice->ReferenceCount);

	SourceDevice->StackSize = TargetDevice->StackSize + 1;
	SourceDevice->DeviceObjectExtension->AttachedTo = TargetDevice;
	TargetDevice->AttachedDevice = SourceDevice;

	return TargetDevice;
}

NTSTATUS
FASTCALL
IofCallDriver(
			  _In_ PDEVICE_OBJECT DeviceObject,
			  _Inout_ __drv_aliasesMem PIRP Irp
			  )
{
	DbgPrint("%p>%s(%p, %p)\n", _ReturnAddress(), __FUNCTION__, DeviceObject, Irp);
	//
	// Ensure that this is really an I/O Request Packet.
	//

	if (Irp->Type != IO_TYPE_IRP)
	{
		__debugbreak();
	}

	//
	// Update the IRP stack to point to the next location.
	//

	if (--Irp->CurrentLocation <= 0) {
		__debugbreak();
	}

	PIO_STACK_LOCATION irpSp = --Irp->Tail.Overlay.CurrentStackLocation;

	//
	// Save a pointer to the device object for this request so that it can
	// be used later in completion.
	//

	irpSp->DeviceObject = DeviceObject;

	return DeviceObject->DriverObject->MajorFunction[irpSp->MajorFunction]( DeviceObject, Irp );
}

void IopDeleteIrp(PIRP Irp)
{
	DbgPrint("IopDeleteIrp(%p)\n", Irp);

	if (Irp->UserIosb)
	{
		*Irp->UserIosb = Irp->IoStatus;
	}

	ULONG Flags = Irp->Flags;

	if (Flags & IRP_BUFFERED_IO)
	{
		if ((Flags & IRP_INPUT_OPERATION) && Irp->IoStatus.Information && !NT_ERROR( Irp->IoStatus.Status ))
		{
			memcpy(Irp->UserBuffer, Irp->AssociatedIrp.SystemBuffer, Irp->IoStatus.Information);
		}

		if (Flags & IRP_DEALLOCATE_BUFFER)
		{
			delete [] Irp->AssociatedIrp.SystemBuffer;
		}
	}

	if (PMDL MdlAddress = Irp->MdlAddress)
	{
		do 
		{
			PVOID pv = MdlAddress;

			MdlAddress = MdlAddress->Next;

			delete pv;

		} while (MdlAddress);
	}

	if (Irp->PendingReturned && Irp->UserEvent)
	{
		KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, FALSE);
	}

	delete [] Irp;
}

VOID
FASTCALL
IofCompleteRequest(
				   _In_ PIRP Irp,
				   _In_ CCHAR /*PriorityBoost*/
				   )
{
	NTSTATUS status = Irp->IoStatus.Status;
	
	UCHAR StackCount = Irp->StackCount, CurrentLocation = Irp->CurrentLocation;
	
	DbgPrint("%p>%s(%p [%x, %p] %x/%x)\n", _ReturnAddress(), __FUNCTION__, Irp, 
		status, Irp->IoStatus.Information, CurrentLocation, StackCount);

	if (CurrentLocation > StackCount + 1 ||
		Irp->Type != IO_TYPE_IRP) {
			KeBugCheckEx( MULTIPLE_IRP_COMPLETE_REQUESTS, (ULONG_PTR) Irp, __LINE__, 0, 0 );
	}

	PIO_STACK_LOCATION irpSp = Irp->Tail.Overlay.CurrentStackLocation;

	while (CurrentLocation++ <= StackCount)
	{
		PIO_COMPLETION_ROUTINE CompletionRoutine = irpSp->CompletionRoutine;
		
		PVOID Context = irpSp->Context;

		UCHAR Control = irpSp++->Control;

		Irp->Tail.Overlay.CurrentStackLocation++, Irp->CurrentLocation++;

		Irp->PendingReturned = Control & SL_PENDING_RETURNED;

		if ( ( NT_SUCCESS( status ) && (Control & SL_INVOKE_ON_SUCCESS) ) ||
			(!NT_SUCCESS( status ) && (Control & SL_INVOKE_ON_ERROR)) ||
			(Irp->Cancel && (Control & SL_INVOKE_ON_CANCEL))
			)
		{
			PDEVICE_OBJECT DeviceObject = 0;

			if (CurrentLocation <= StackCount)
			{
				DeviceObject = irpSp->DeviceObject;
			}

			DbgPrint("CompletionRoutine(%p, %p, %p)...\n", DeviceObject, Irp, Context);

			status = CompletionRoutine(DeviceObject, Irp, Context);

			DbgPrint("CompletionRoutine(%p, %p, %p)=%x\n", DeviceObject, Irp, Context, status);

			if (status == STATUS_MORE_PROCESSING_REQUIRED)
			{
				return;
			}
		}
		else
		{
			if (Irp->PendingReturned && CurrentLocation <= StackCount) 
			{
				irpSp->Control |= SL_PENDING_RETURNED;
			}
		}
	}

	IopDeleteIrp(Irp);
}

PDEVICE_OBJECT gpEsetDevice;

NTSTATUS
IoCreateDevice(
			   _In_  PDRIVER_OBJECT DriverObject,
			   _In_  ULONG DeviceExtensionSize,
			   _In_opt_ PUNICODE_STRING DeviceName,
			   _In_  DEVICE_TYPE DeviceType,
			   _In_  ULONG DeviceCharacteristics,
			   _In_  BOOLEAN /*Exclusive*/,
			   _Outptr_ PDEVICE_OBJECT * pDeviceObject
			   )
{
	DbgPrint("IoCreateDevice:%p %x %x %wZ\n", DriverObject, DeviceExtensionSize, DeviceType, DeviceName);

	ULONG Size = sizeof(DEVICE_OBJECT) + sizeof(DEVOBJ_EXTENSION) + DeviceExtensionSize;

	if (Size > MAXUSHORT || DeviceExtensionSize > Size)
	{
		return STATUS_INVALID_PARAMETER_2;
	}

	if (PDEVICE_OBJECT DeviceObject = (PDEVICE_OBJECT)new BYTE[Size])
	{
		RtlZeroMemory(DeviceObject, Size);
		
		PDEVOBJ_EXTENSION DeviceObjectExtension = (PDEVOBJ_EXTENSION)(DeviceObject + 1);

		DeviceObjectExtension->Type = IO_TYPE_DEVICE_OBJECT_EXTENSION;
		DeviceObjectExtension->Size = sizeof(DEVOBJ_EXTENSION);
		DeviceObjectExtension->DeviceObject = DeviceObject;

		DeviceObject->Type = IO_TYPE_DEVICE;
		DeviceObject->Size = (USHORT)Size;
		DeviceObject->ReferenceCount = 1;
		DeviceObject->DeviceType = DeviceType;
		DeviceObject->Characteristics = DeviceCharacteristics;
		DeviceObject->AttachedDevice = 0;
		DeviceObject->StackSize = 1;
		DeviceObject->Flags = DeviceName ? DO_DEVICE_HAS_NAME|DO_DEVICE_INITIALIZING : DO_DEVICE_INITIALIZING;

		DeviceObject->DeviceExtension = DeviceObjectExtension + 1;
		DeviceObject->DeviceObjectExtension = DeviceObjectExtension;

		DeviceObject->DriverObject = DriverObject;
		DeviceObject->NextDevice = DriverObject->DeviceObject;
		DriverObject->DeviceObject = DeviceObject;

		*pDeviceObject = DeviceObject;

		DbgPrint("DeviceObject=%p\n", DeviceObject);

		STATIC_UNICODE_STRING(EsetDevice, "\\Device\\45736574");

		if (DeviceName && RtlEqualUnicodeString(&EsetDevice, DeviceName, FALSE))
		{
			gpEsetDevice = DeviceObject;
		}

		return STATUS_SUCCESS;
	}

	return STATUS_INSUFFICIENT_RESOURCES;
}

void DereferenceDevice(PDEVICE_OBJECT DeviceObject)
{
	if (!InterlockedDecrement(&DeviceObject->ReferenceCount))
	{
		delete [] DeviceObject;
	}
}

VOID
IoDeleteDevice(
			   _In_ __drv_freesMem(Mem) PDEVICE_OBJECT DeviceObject
			   )
{
	DbgPrint("%p>%s(%p)\n", _ReturnAddress(), __FUNCTION__, DeviceObject);
	DereferenceDevice(DeviceObject);
}

NTSTATUS
IoDeleteSymbolicLink(
					 _In_ PUNICODE_STRING SymbolicLinkName
					 )
{
	DbgPrint("%p>%s(%wZ)\n", _ReturnAddress(), __FUNCTION__, SymbolicLinkName);
	return STATUS_SUCCESS;
}

VOID
IoDetachDevice(
			   _Inout_ PDEVICE_OBJECT TargetDevice
			   )
{
	if (TargetDevice->AttachedDevice->DeviceObjectExtension->AttachedTo != TargetDevice)
	{
		__debugbreak();
	}
	TargetDevice->AttachedDevice->DeviceObjectExtension->AttachedTo = 0;
	TargetDevice->AttachedDevice = 0;
	DereferenceDevice(TargetDevice);
}

PVOID
IoGetDriverObjectExtension(
						   _In_ PDRIVER_OBJECT DriverObject,
						   _In_ PVOID ClientIdentificationAddress
						   )
{
	DbgPrint("%p>%s(%p)\n", _ReturnAddress(), __FUNCTION__, DriverObject);

	DRIVER_EXTENSION_EX* DriverExtension = static_cast<DRIVER_EXTENSION_EX*>(DriverObject->DriverExtension);

	while (IO_CLIENT_EXTENSION * ClientDriverExtension = DriverExtension->ClientDriverExtension)
	{
		if (ClientDriverExtension->ClientIdentificationAddress == ClientIdentificationAddress)
		{
			DbgPrint("ClientDriverExtension=%p\n", ClientDriverExtension->Data);
			return ClientDriverExtension->Data;
		}
		ClientDriverExtension = ClientDriverExtension->NextExtension;
	}
	return 0;
}

VOID
NTAPI
IoInitializeRemoveLockEx(
						 _Out_ PIO_REMOVE_LOCK Lock,
						 _In_ ULONG  AllocateTag, // Used only on checked kernels
						 _In_ ULONG  MaxLockedMinutes, // Used only on checked kernels
						 _In_ ULONG  HighWatermark, // Used only on checked kernels
						 _In_ ULONG  RemlockSize // are we checked or free
						 )
{
}

NTSTATUS
NTAPI
IoAcquireRemoveLockEx (
					   _Inout_ PIO_REMOVE_LOCK RemoveLock,
					   _In_opt_ PVOID          Tag, // Optional
					   _In_ PCSTR              File,
					   _In_ ULONG              Line,
					   _In_ ULONG              RemlockSize // are we checked or free
					   )
{
	return 0;
}

VOID
NTAPI
IoReleaseRemoveLockEx(
					  _Inout_ PIO_REMOVE_LOCK RemoveLock,
					  _In_opt_ PVOID          Tag, // Optional
					  _In_ ULONG              RemlockSize // are we checked or free
					  )
{
}

VOID
NTAPI
IoReleaseRemoveLockAndWaitEx(
							 _Inout_ PIO_REMOVE_LOCK RemoveLock,
							 _In_opt_ PVOID       Tag,
							 _In_ ULONG           RemlockSize // are we checked or free
							 )
{
}

NTSTATUS
PoCallDriver (
			  _In_ PDEVICE_OBJECT DeviceObject,
			  _Inout_ __drv_aliasesMem PIRP Irp
			  )
{
	DbgPrint("%p>%s(%p, %p)\n", _ReturnAddress(), __FUNCTION__, DeviceObject, Irp);
	return IofCallDriver(DeviceObject, Irp);
}

VOID
PoStartNextPowerIrp(
					_Inout_ PIRP Irp
					)
{
}

NTSTATUS
ObReferenceObjectByHandle(
						  _In_ HANDLE Handle,
						  _In_ ACCESS_MASK DesiredAccess,
						  _In_opt_ POBJECT_TYPE ObjectType,
						  _In_ KPROCESSOR_MODE AccessMode,
						  _Out_ PVOID *Object,
						  _Out_opt_ POBJECT_HANDLE_INFORMATION HandleInformation
						  )
{
	if (DISPATCHER_HEADER* pObj = new DISPATCHER_HEADER)
	{
		pObj->Lock = 1;

		NTSTATUS status = ZwDuplicateObject(NtCurrentProcess(), Handle, NtCurrentProcess(), (PHANDLE)&pObj->SignalState, 0, 0,
			DUPLICATE_SAME_ACCESS|DUPLICATE_SAME_ATTRIBUTES);

		if (0 > status)
		{
			__debugbreak();
			
			delete pObj;

			return status;
		}
		
		*Object = pObj;

		DbgPrint("%p>%s(%p)=%p[%p]\n", _ReturnAddress(), __FUNCTION__, Handle, pObj, *(PHANDLE)&pObj->SignalState);
	}
	return STATUS_INSUFFICIENT_RESOURCES;
}

LONG_PTR
FASTCALL
ObfDereferenceObject(
					 _In_ PVOID Object
					 )
{

	DISPATCHER_HEADER* pObj = reinterpret_cast<DISPATCHER_HEADER*>(Object);
	HANDLE hObject = *(PHANDLE)&pObj->SignalState;
	DbgPrint("%p>%s(%p)=[%p],%x\n", _ReturnAddress(), __FUNCTION__, Object, hObject, pObj->Lock);

	if (!InterlockedDecrement(&pObj->Lock))
	{
		DbgPrint("delete object at %p[%p]\n", pObj, hObject);
		NtClose(hObject);
		delete pObj;
	}
	return 0;
}

VOID
KeQuerySystemTime (
				   _Out_ PLARGE_INTEGER CurrentTime
				   )
{
	GetSystemTimeAsFileTime((PFILETIME)CurrentTime);
}

PVOID
ExAllocatePool (
				__drv_strictTypeMatch(__drv_typeExpr) _In_ POOL_TYPE /*PoolType*/,
				_In_ SIZE_T NumberOfBytes
				)
{
	PVOID pv = HeapAlloc(ghHeap, 0, NumberOfBytes);
	DbgPrint("%p>%s(%x)=%p\n", _ReturnAddress(), __FUNCTION__, NumberOfBytes, pv);
	return pv;
}

PVOID
MmMapLockedPagesSpecifyCache (
							  _Inout_ PMDL MemoryDescriptorList,
							  _In_ __drv_strictType(KPROCESSOR_MODE/enum _MODE,__drv_typeConst) 
							  KPROCESSOR_MODE AccessMode,
							  _In_ __drv_strictTypeMatch(__drv_typeCond) MEMORY_CACHING_TYPE CacheType,
							  _In_opt_ PVOID RequestedAddress,
							  _In_     ULONG BugCheckOnFailure,
							  _In_     ULONG Priority  // MM_PAGE_PRIORITY logically OR'd with MdlMapping*
							  )
{
	DbgPrint("%p>%s(%p %x %p)\n", _ReturnAddress(), __FUNCTION__, 
		MemoryDescriptorList, MemoryDescriptorList->MdlFlags, 
		MemoryDescriptorList->MappedSystemVa);

	if (MemoryDescriptorList->MdlFlags & MDL_SOURCE_IS_NONPAGED_POOL)
	{
		MemoryDescriptorList->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;

		return MemoryDescriptorList->MappedSystemVa;
	}

	__debugbreak();

	return 0;
}

NTSTATUS
PsTerminateSystemThread(
						_In_ NTSTATUS ExitStatus
						)
{
	DbgPrint("%p>%s(%x)\n", _ReturnAddress(), __FUNCTION__, ExitStatus);

	return RtlExitUserThread(ExitStatus);
}

NTSTATUS
IoAllocateDriverObjectExtension(
								_In_  PDRIVER_OBJECT DriverObject,
								_In_  PVOID ClientIdentificationAddress,
								_In_  ULONG DriverObjectExtensionSize,
								PVOID *DriverObjectExtension
								)
{
	DbgPrint("%p>%s(%p %x)\n", _ReturnAddress(), __FUNCTION__, DriverObject, DriverObjectExtensionSize);

	if (IO_CLIENT_EXTENSION * ClientDriverExtension = new(DriverObjectExtensionSize) IO_CLIENT_EXTENSION)
	{
		DRIVER_EXTENSION_EX* DriverExtension = static_cast<DRIVER_EXTENSION_EX*>(DriverObject->DriverExtension);

		ClientDriverExtension->ClientIdentificationAddress = ClientIdentificationAddress;
		ClientDriverExtension->NextExtension = DriverExtension->ClientDriverExtension;
		DriverExtension->ClientDriverExtension = ClientDriverExtension;

		*DriverObjectExtension = ClientDriverExtension->Data;

		DbgPrint("ClientDriverExtension=%p\n", ClientDriverExtension->Data);
		return 0;
	}

	return STATUS_INSUFFICIENT_RESOURCES;
}

DECLSPEC_CACHEALIGN volatile KSYSTEM_TIME KeTickCount = {};

DECLSPEC_NORETURN
VOID
NTAPI
KeBugCheckEx(
			 _In_ ULONG BugCheckCode,
			 _In_ ULONG_PTR BugCheckParameter1,
			 _In_ ULONG_PTR BugCheckParameter2,
			 _In_ ULONG_PTR BugCheckParameter3,
			 _In_ ULONG_PTR BugCheckParameter4
			 )
{
	DbgPrint("%p>KeBugCheckEx(%x, %p, %p %p %p)\n", _ReturnAddress(), BugCheckCode,
		BugCheckParameter1, BugCheckParameter2, BugCheckParameter3, BugCheckParameter4);

	__debugbreak();
}

void FreeFileObject(PFILE_OBJECT FileObject)
{
	RtlFreeUnicodeString(&FileObject->FileName);
	delete FileObject;
}

NTSTATUS AllocateFileObject(PFILE_OBJECT* pFileObject, PDEVICE_OBJECT DeviceObject, PCUNICODE_STRING FileName)
{
	if (PFILE_OBJECT FileObject = new FILE_OBJECT)
	{
		RtlZeroMemory(FileObject, sizeof(FILE_OBJECT));

		NTSTATUS status = RtlDuplicateUnicodeString(0, FileName, &FileObject->FileName);

		if (0 <= status)
		{
			FileObject->Type = IO_TYPE_FILE;

			FileObject->Flags = FO_HANDLE_CREATED|FO_SYNCHRONOUS_IO;

			FileObject->DeviceObject = DeviceObject;

			*pFileObject = FileObject;

			return STATUS_SUCCESS;
		}

		delete FileObject;

		return status;
	}

	return STATUS_INSUFFICIENT_RESOURCES;
}

PIRP AllocateIrp(PDEVICE_OBJECT DeviceObject)
{
	CHAR StackSize = DeviceObject->StackSize;

	if (!StackSize)
	{
		__debugbreak();
		return 0;
	}

	if (PIRP Irp = (PIRP)new UCHAR[sizeof(IRP) + StackSize * sizeof(IO_STACK_LOCATION)])
	{
		RtlZeroMemory(Irp, sizeof(IRP) + StackSize * sizeof(IO_STACK_LOCATION));

		Irp->Type = IO_TYPE_IRP;
		Irp->StackCount = StackSize;
		Irp->CurrentLocation = StackSize + 1;
		Irp->Tail.Overlay.CurrentStackLocation = StackSize + (PIO_STACK_LOCATION)(Irp + 1);

		Irp->Tail.Overlay.Thread = (PETHREAD)GetCurrentThreadId();
		return Irp;
	}

	return 0;
}

BOOL AllocateMdl(PVOID Buffer, ULONG Length, PIRP Irp)
{
	if (PMDL Mdl = new MDL)
	{
		Mdl->Next = 0;
		Mdl->Size = sizeof(Mdl);
		Mdl->MdlFlags = MDL_SOURCE_IS_NONPAGED_POOL;
		Mdl->MappedSystemVa = Buffer;
		Mdl->ByteOffset = BYTE_OFFSET(Buffer);
		Mdl->StartVa = (PVOID)PAGE_ALIGN(Buffer);
		Mdl->ByteCount = Length;
		Mdl->Process = 0;
		Irp->MdlAddress = Mdl;

		return TRUE;
	}

	return FALSE;
}

NTSTATUS SynchronousServiceTail(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	KEVENT Event;

	KeInitializeEvent(&Event, NotificationEvent, FALSE);

	Irp->UserEvent = &Event;

	NTSTATUS status = IofCallDriver(DeviceObject, Irp);

	if (status == STATUS_PENDING)
	{
		KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, 0);
	}

	CloseEvent(&Event);

	return status;
}

NTSTATUS StartDevice(PDEVICE_OBJECT DeviceObject, PIO_COMPLETION_ROUTINE CompletionRoutine, PVOID Context)
{
	PIRP Irp = AllocateIrp(DeviceObject);

	if (!Irp)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	PIO_STACK_LOCATION IrpSp = IoGetNextIrpStackLocation(Irp);

	IrpSp->MajorFunction = IRP_MJ_PNP;
	IrpSp->MinorFunction = IRP_MN_START_DEVICE;

	if (CompletionRoutine)
	{
		IrpSp->Control = SL_INVOKE_ON_CANCEL|SL_INVOKE_ON_ERROR|SL_INVOKE_ON_SUCCESS;
		IrpSp->Context = Context;
		IrpSp->CompletionRoutine = CompletionRoutine;
	}

	return SynchronousServiceTail(DeviceObject, Irp);
}

NTSTATUS SendIoctl(PFILE_OBJECT FileObject, PIO_COMPLETION_ROUTINE CompletionRoutine, PVOID Context,
				   PIO_STATUS_BLOCK IoStatusBlock,
				   ULONG IoControlCode,
				   PVOID InputBuffer,
				   ULONG InputBufferLength,
				   PVOID OutputBuffer,
				   ULONG OutputBufferLength)
{
	PDEVICE_OBJECT DeviceObject = FileObject->DeviceObject;

	PIRP Irp = AllocateIrp(DeviceObject);

	if (!Irp)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	Irp->UserBuffer = OutputBuffer;
	Irp->UserIosb = IoStatusBlock;

	PIO_STACK_LOCATION IrpSp = IoGetNextIrpStackLocation(Irp);

	IrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
	IrpSp->FileObject = FileObject;

	IrpSp->Parameters.DeviceIoControl.IoControlCode = IoControlCode;
	IrpSp->Parameters.DeviceIoControl.InputBufferLength = InputBufferLength;
	IrpSp->Parameters.DeviceIoControl.OutputBufferLength = OutputBufferLength;
	IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = InputBuffer;

	switch (IoControlCode & 3)
	{
	case METHOD_BUFFERED:
		if (ULONG BufferLength = max(InputBufferLength, OutputBufferLength))
		{
			if (Irp->AssociatedIrp.SystemBuffer = new UCHAR[BufferLength])
			{
				Irp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;

				if (InputBufferLength)
				{
					memcpy(Irp->AssociatedIrp.SystemBuffer, InputBuffer, InputBufferLength);
				}

				if (OutputBufferLength)
				{
					Irp->Flags |= IRP_INPUT_OPERATION;
				}
			}
			else
			{
				IopDeleteIrp(Irp);
				return STATUS_INSUFFICIENT_RESOURCES;
			}
		}
		break;

	case METHOD_IN_DIRECT:
	case METHOD_OUT_DIRECT:

		if (InputBufferLength)
		{
			Irp->AssociatedIrp.SystemBuffer = InputBuffer;
		}

		if (OutputBufferLength)
		{
			if (!AllocateMdl(OutputBuffer, OutputBufferLength, Irp))
			{
				IopDeleteIrp(Irp);
				return STATUS_INSUFFICIENT_RESOURCES;
			}
		}
		break;
	}

	if (CompletionRoutine)
	{
		IrpSp->Control = SL_INVOKE_ON_CANCEL|SL_INVOKE_ON_ERROR|SL_INVOKE_ON_SUCCESS;
		IrpSp->Context = Context;
		IrpSp->CompletionRoutine = CompletionRoutine;
	}

	return SynchronousServiceTail(DeviceObject, Irp);
}

NTSTATUS DoCreate(PFILE_OBJECT FileObject, 
				PIO_COMPLETION_ROUTINE CompletionRoutine, 
				PVOID Context,
				PIO_STATUS_BLOCK IoStatusBlock,
				PIO_SECURITY_CONTEXT SecurityContext, 
				USHORT ShareAccess,
				ULONG Disposition,
				ULONG Options)
{
	PDEVICE_OBJECT DeviceObject = FileObject->DeviceObject;

	PIRP Irp = AllocateIrp(DeviceObject);

	if (!Irp)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	Irp->UserIosb = IoStatusBlock;

	PIO_STACK_LOCATION IrpSp = IoGetNextIrpStackLocation(Irp);

	IrpSp->MajorFunction = IRP_MJ_CREATE;
	IrpSp->FileObject = FileObject;

	IrpSp->Parameters.Create.SecurityContext = SecurityContext;
	IrpSp->Parameters.Create.Options = Options|(Disposition << 24);
	IrpSp->Parameters.Create.ShareAccess = ShareAccess;

	if (CompletionRoutine)
	{
		IrpSp->Control = SL_INVOKE_ON_CANCEL|SL_INVOKE_ON_ERROR|SL_INVOKE_ON_SUCCESS;
		IrpSp->Context = Context;
		IrpSp->CompletionRoutine = CompletionRoutine;
	}

	return SynchronousServiceTail(DeviceObject, Irp);
}

NTSTATUS CloseCleanup(PFILE_OBJECT FileObject, PIO_COMPLETION_ROUTINE CompletionRoutine, PVOID Context, UCHAR MajorFunction)
{
	PDEVICE_OBJECT DeviceObject = FileObject->DeviceObject;

	PIRP Irp = AllocateIrp(DeviceObject);

	if (!Irp)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	PIO_STACK_LOCATION IrpSp = IoGetNextIrpStackLocation(Irp);

	IrpSp->MajorFunction = MajorFunction;
	IrpSp->MinorFunction = IRP_MN_NORMAL;
	IrpSp->FileObject = FileObject;

	if (CompletionRoutine)
	{
		IrpSp->Control = SL_INVOKE_ON_CANCEL|SL_INVOKE_ON_ERROR|SL_INVOKE_ON_SUCCESS;
		IrpSp->Context = Context;
		IrpSp->CompletionRoutine = CompletionRoutine;
	}

	return SynchronousServiceTail(DeviceObject, Irp);
}

NTSTATUS DoReadWriteSync(PFILE_OBJECT FileObject, 
						 PIO_COMPLETION_ROUTINE CompletionRoutine, 
						 PVOID Context,
						 UCHAR MajorFunction,
						 PIO_STATUS_BLOCK IoStatusBlock,
						 PVOID Buffer, 
						 ULONG Length, 
						 PLARGE_INTEGER ByteOffset)
{
	PDEVICE_OBJECT DeviceObject = FileObject->DeviceObject;

	PIRP Irp = AllocateIrp(DeviceObject);

	if (!Irp)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	Irp->UserIosb = IoStatusBlock;
	Irp->UserBuffer = Buffer;

	ULONG Flags = DeviceObject->Flags;

	if (Flags & DO_BUFFERED_IO)
	{
		Irp->AssociatedIrp.SystemBuffer = Buffer;
	}
	else if (Flags & DO_DIRECT_IO)
	{
		if (!AllocateMdl(Buffer, Length, Irp))
		{
			IopDeleteIrp(Irp);
			return STATUS_INSUFFICIENT_RESOURCES;
		}
	}

	PIO_STACK_LOCATION IrpSp = IoGetNextIrpStackLocation(Irp);

	IrpSp->MajorFunction = MajorFunction;
	IrpSp->MinorFunction = IRP_MN_NORMAL;
	IrpSp->FileObject = FileObject;

	IrpSp->Parameters.Write.ByteOffset = ByteOffset ? *ByteOffset : FileObject->CurrentByteOffset;
	IrpSp->Parameters.Write.Length = Length;

	if (CompletionRoutine)
	{
		IrpSp->Control = SL_INVOKE_ON_CANCEL|SL_INVOKE_ON_ERROR|SL_INVOKE_ON_SUCCESS;
		IrpSp->Context = Context;
		IrpSp->CompletionRoutine = CompletionRoutine;
	}

	return SynchronousServiceTail(DeviceObject, Irp);
}

enum MapType {
	MapRead, MapExecute, MapReadWrite
};

NTSTATUS MapFileData(POBJECT_ATTRIBUTES poa, MapType Type, PVOID *BaseAddress, PSIZE_T ViewSize)
{
	HANDLE hFile, hSection;
	IO_STATUS_BLOCK iosb;

	static ACCESS_MASK FileDesiredAccess[] = { 
		FILE_GENERIC_READ, FILE_GENERIC_READ|FILE_GENERIC_EXECUTE, FILE_GENERIC_READ|FILE_GENERIC_WRITE 
	};

	static ACCESS_MASK SectionDesiredAccess[] = { 
		SECTION_MAP_READ, SECTION_MAP_READ|SECTION_MAP_EXECUTE, SECTION_MAP_READ|SECTION_MAP_WRITE 
	};

	static ULONG PageProtection[] = {
		PAGE_READONLY, PAGE_EXECUTE_WRITECOPY, PAGE_READWRITE
	};

	NTSTATUS status = NtOpenFile(&hFile, FileDesiredAccess[Type], 
		poa, &iosb, FILE_SHARE_VALID_FLAGS, FILE_SYNCHRONOUS_IO_NONALERT);

	if (0 <= status)
	{
		status = NtCreateSection(&hSection, 
			SectionDesiredAccess[Type], 0, 0, PageProtection[Type], SEC_COMMIT, hFile);

		NtClose(hFile);

		if (0 <= status)
		{
			*BaseAddress = 0;
			*ViewSize = 0;
			status = ZwMapViewOfSection(hSection, NtCurrentProcess(), BaseAddress, 0, 0, 0, ViewSize, ViewUnmap, 0, PageProtection[Type]);
			NtClose(hSection);
		}
	}

	return status;
}

NTSTATUS UnmapFile(PVOID BaseAddress)
{
	return ZwUnmapViewOfSection(NtCurrentProcess(), BaseAddress);
}

void Go(POBJECT_ATTRIBUTES poa)
{
	NTSTATUS status;

	STATIC_UNICODE_STRING(Name, "\\Device\\45736574\\PunchCard.bmp");

	PFILE_OBJECT FileObject;

	if (0 <= (status = AllocateFileObject(&FileObject, gpEsetDevice, &Name)))
	{
		IO_STATUS_BLOCK iosb;
		IO_SECURITY_CONTEXT sctx = { 0, 0, FILE_GENERIC_WRITE|FILE_GENERIC_READ };

		if (0 <= (status = DoCreate(FileObject, 0, 0, &iosb, &sctx, FILE_SHARE_VALID_FLAGS, 
			FILE_OVERWRITE_IF, FILE_NON_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_NONALERT)))
		{
			DISK_GEOMETRY dg;
			PARTITION_INFORMATION pi;

			if (0 <= (status = SendIoctl(FileObject, 0, 0, &iosb, IOCTL_DISK_GET_DRIVE_GEOMETRY, 0, 0, &dg, sizeof(dg)))
				&&
				0 <= (status = SendIoctl(FileObject, 0, 0, &iosb, IOCTL_DISK_GET_PARTITION_INFO, 0, 0, &pi, sizeof(pi))))
			{
				if ((dg.BytesPerSector & (dg.BytesPerSector - 1)) || dg.BytesPerSector > PAGE_SIZE)
				{
					__debugbreak();
				}

				PVOID buf;
				SIZE_T Length;

				if (0 <= (status = MapFileData(poa, MapReadWrite, &buf, &Length)))
				{
					Length &= ~(dg.BytesPerSector - 1);

					if (Length < (ULONGLONG)pi.PartitionLength.QuadPart)
					{
						pi.PartitionLength.QuadPart -= Length;

						if (0 <= (status = DoReadWriteSync(FileObject, 0, 0, IRP_MJ_WRITE, 0, buf, (ULONG)Length, 
							(PLARGE_INTEGER)&pi.PartitionLength)) &&
							0 <= (status = DoReadWriteSync(FileObject, 0, 0, IRP_MJ_READ, 0, buf, (ULONG)Length, 
							(PLARGE_INTEGER)&pi.PartitionLength)))
						{
						}
					}

					UnmapFile(buf);
				}
			}

			CloseCleanup(FileObject, 0, 0, IRP_MJ_CLEANUP);
			CloseCleanup(FileObject, 0, 0, IRP_MJ_CLOSE);
		}

		FreeFileObject(FileObject);
	}
}

PDRIVER_INITIALIZE gDriverEntry;

BOOLEAN CALLBACK DllMain( HMODULE hDllHandle, DWORD dwReason, LPVOID /*lpreserved*/ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hDllHandle);

		BOOLEAN fOk = FALSE;

		if (PDRIVER_OBJECT DriverObject = AllocateDriver())
		{			
			DbgPrint("Eset DriverObject=%p\n", DriverObject);

			STATIC_UNICODE_STRING(TestKey, "\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet001\\Services\\Eset");

			NTSTATUS status = gDriverEntry(DriverObject, const_cast<PUNICODE_STRING>(&TestKey));

			DbgPrint("DriverEntry=%x\n", status);

			if (0 <= status && DriverObject->DriverExtension->AddDevice)
			{
				if (PDRIVER_OBJECT LowDrv = AllocateDriver(OkDeviceRequest))
				{
					PDEVICE_OBJECT PhysicalDeviceObject, EsetDeviceObject;

					if (0 <= IoCreateDevice(LowDrv, 0, 0, FILE_DEVICE_BUS_EXTENDER, 0, 0, &PhysicalDeviceObject))
					{
						DbgPrint("PhysicalDeviceObject=%p\n", PhysicalDeviceObject);

						PhysicalDeviceObject->StackSize = 1;

						status = DriverObject->DriverExtension->AddDevice(DriverObject, PhysicalDeviceObject);
						
						DbgPrint("AddDevice=%x\n", status);

						if (0 <= status)
						{
							EsetDeviceObject = PhysicalDeviceObject;

							while (EsetDeviceObject = EsetDeviceObject->AttachedDevice)
							{
								if (EsetDeviceObject->DriverObject == DriverObject)
								{
									DbgPrint("Eset DeviceObject=%p\n", EsetDeviceObject);

									status = StartDevice(EsetDeviceObject, 0, 0);

									DbgPrint("StartDevice=%x\n", status);

									if (EsetDeviceObject == gpEsetDevice)
									{
										fOk = TRUE;
									}
								}
							}
						}

						DereferenceDevice(PhysicalDeviceObject);
					}
				}
			}

			return fOk;
		}
	}

	return TRUE;
}

VOID CALLBACK DllNotify(
	_In_     LDR_DLL_NOTIFICATION_REASON NotificationReason,
	_In_     PCLDR_DLL_NOTIFICATION_DATA NotificationData,
	_In_opt_ PVOID                       Context
	)
{
	if (NotificationReason == LDR_DLL_NOTIFICATION_REASON_LOADED)
	{
		PVOID DllBase = NotificationData->DllBase;

		if (PIMAGE_NT_HEADERS pinth = RtlImageNtHeader(DllBase))
		{
			if (pinth->FileHeader.TimeDateStamp == 0x54e368c5)
			{
				_LDR_DATA_TABLE_ENTRY* ldte;
				if (0 <= LdrFindEntryForAddress(DllBase, ldte))
				{
					gDriverEntry = (PDRIVER_INITIALIZE)ldte->EntryPoint;
					ldte->EntryPoint = DllMain;
				}
			}
		}
	}
}

extern "C"
{
	PVOID __imp_LdrRegisterDllNotification;
}

#pragma comment(linker, "/alternatename:__imp__LdrRegisterDllNotification@16=___imp_LdrRegisterDllNotification")


struct TRANSFORM_CONTEXT 
{
	PBYTE pbData;
	SIZE_T cbData;
	PVOID pvKey;
	SIZE_T cbKey;
};

void TestDecrypt()
{
	STATIC_OBJECT_ATTRIBUTES(oa_code, "\\global??\\d:\\21ac.cod");
	STATIC_OBJECT_ATTRIBUTES(oa_object, "\\global??\\d:\\2b5.dat");
	STATIC_OBJECT_ATTRIBUTES(oa_data, "\\global??\\d:\\PunchCard.bmp");

	void (__stdcall * pvCode)(const void* Object, PVOID hmod, ULONG , TRANSFORM_CONTEXT* ctx);

	PVOID Object;

	PVOID pvData;
	SIZE_T cbData;

	if (0 <= MapFileData(&oa_code, MapExecute, (void**)&pvCode, &cbData))
	{
		if (0 <= MapFileData(&oa_object, MapRead, &Object, &cbData))
		{
			if (0 <= MapFileData(&oa_data, MapReadWrite, &pvData, &cbData))
			{
				ghHeap = GetProcessHeap();

				const ULONG BlockSize = 0x200;

				if (SIZE_T BlockCount = (cbData + BlockSize - 1) / BlockSize)
				{
					TRANSFORM_CONTEXT ctx = { (PBYTE)pvData, BlockSize };

					do 
					{
						pvCode(Object, &__ImageBase, 4, &ctx);
					} while (ctx.pbData += BlockSize, --BlockCount);
				}

				UnmapFile(pvData);
			}

			UnmapFile(Object);
		}

		UnmapFile(pvCode);
	}
}

VOID LoadDrv()
{
	TestDecrypt();

	if (__imp_LdrRegisterDllNotification = GetProcAddress(GetModuleHandle(L"ntdll"), "LdrRegisterDllNotification"))
	{
		PVOID Cookie;
		if (0 <= LdrRegisterDllNotification(0, DllNotify, 0, &Cookie))
		{
			if (ghHeap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0x800000, 0))
			{
				if (LoadLibraryW(L"d:\\crackme_drv.sys"))//
				{
					static LARGE_INTEGER li = { 0, MINLONG };
					ZwDelayExecution(TRUE, &li);
					__debugbreak();

					STATIC_OBJECT_ATTRIBUTES(oa, "\\global??\\d:\\PunchCard.bmp");
					Go(&oa);

					ZwDelayExecution(TRUE, &li);
					__debugbreak();
				}
			}
		}
	}
}

// for fix bug with not null-terminated "ESETConst" string 
VOID NTAPI RtlInitAnsiString( PANSI_STRING DestinationString, PCSZ SourceString )
{
	DestinationString->Buffer = (PSZ)SourceString;

	if (
		SourceString[0] == 'E' &&
		SourceString[1] == 'S' &&
		SourceString[2] == 'E' &&
		SourceString[3] == 'T' &&
		SourceString[4] == 'C' &&
		SourceString[5] == 'o' &&
		SourceString[6] == 'n' &&
		SourceString[7] == 's' &&
		SourceString[8] == 't' 
		)
	{
		DestinationString->Length = 9;
		DestinationString->MaximumLength = 9;
	}
	else
	{
		DestinationString->MaximumLength = DestinationString->Length = (USHORT)strlen(SourceString);
	}
}

_NT_END

