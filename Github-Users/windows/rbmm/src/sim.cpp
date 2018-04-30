
VOID
KeInitializeEvent (
    _Out_ PRKEVENT Event,
    _In_ EVENT_TYPE Type,
    _In_ BOOLEAN State
    )
{
  return 0;
}

LONG
KeSetEvent (
    _Inout_ PRKEVENT Event,
    _In_ KPRIORITY Increment,
    _In_ _Literal_ BOOLEAN Wait
    )
{
  return 0;
}

NTSTATUS
KeWaitForSingleObject (
    _In_ _Points_to_data_ PVOID Object,
    _In_ _Strict_type_match_ KWAIT_REASON WaitReason,
    _In_ __drv_strictType(KPROCESSOR_MODE/enum _MODE,__drv_typeConst) KPROCESSOR_MODE WaitMode,
    _In_ BOOLEAN Alertable,
    _In_opt_ PLARGE_INTEGER Timeout
    )
{
  return 0;
}

PVOID
NTAPI
ExAllocatePoolWithTag (
    _In_ __drv_strictTypeMatch(__drv_typeExpr) POOL_TYPE PoolType,
    _In_ SIZE_T NumberOfBytes,
    _In_ ULONG Tag
    )
{
  return 0;
}

VOID
ExFreePoolWithTag (
    _Pre_notnull_ __drv_freesMem(Mem) PVOID P,
    _In_ ULONG Tag
    )
{
  return 0;
}

_Must_inspect_result_
NTSTATUS
PsCreateSystemThread(
    _Out_ PHANDLE ThreadHandle,
    _In_ ULONG DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_opt_  HANDLE ProcessHandle,
    _Out_opt_ PCLIENT_ID ClientId,
    _In_ PKSTART_ROUTINE StartRoutine,
    _In_opt_ _When_(return >= 0, __drv_aliasesMem) PVOID StartContext
    )
{
  return 0;
}

PDEVICE_OBJECT
IoAttachDeviceToDeviceStack(
    _In_ _When_(return!=0, __drv_aliasesMem)
    PDEVICE_OBJECT SourceDevice,
    _In_ PDEVICE_OBJECT TargetDevice
    )
{
  return 0;
}

NTSTATUS
FASTCALL
IofCallDriver(
    _In_ PDEVICE_OBJECT DeviceObject,
    _Inout_ __drv_aliasesMem PIRP Irp
    )
{
  return 0;
}

VOID
FASTCALL
IofCompleteRequest(
    _In_ PIRP Irp,
    _In_ CCHAR PriorityBoost
    )
{
  return 0;
}

NTSTATUS
IoCreateDevice(
    _In_  PDRIVER_OBJECT DriverObject,
    _In_  ULONG DeviceExtensionSize,
    _In_opt_ PUNICODE_STRING DeviceName,
    _In_  DEVICE_TYPE DeviceType,
    _In_  ULONG DeviceCharacteristics,
    _In_  BOOLEAN Exclusive,
    _Outptr_result_nullonfailure_
    _At_(*DeviceObject,
        __drv_allocatesMem(Mem)
        _When_((((_In_function_class_(DRIVER_INITIALIZE))
               ||(_In_function_class_(DRIVER_DISPATCH)))),
             __drv_aliasesMem))
    PDEVICE

VOID
IoDeleteDevice(
    _In_ __drv_freesMem(Mem) PDEVICE_OBJECT DeviceObject
    )
{
  return 0;
}

NTSTATUS
IoDeleteSymbolicLink(
    _In_ PUNICODE_STRING SymbolicLinkName
    )
{
  return 0;
}

VOID
IoDetachDevice(
    _Inout_ PDEVICE_OBJECT TargetDevice
    )
{
  return 0;
}

PVOID
IoGetDriverObjectExtension(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PVOID ClientIdentificationAddress
    )
{
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
  return 0;
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
  return 0;
}

VOID
NTAPI
IoReleaseRemoveLockAndWaitEx(
    _Inout_ PIO_REMOVE_LOCK RemoveLock,
    _In_opt_ PVOID       Tag,
    _In_ ULONG           RemlockSize // are we checked or free
    )
{
  return 0;
}

NTSTATUS
PoCallDriver (
    _In_ PDEVICE_OBJECT DeviceObject,
    _Inout_ __drv_aliasesMem PIRP Irp
    )
{
  return 0;
}

VOID
PoStartNextPowerIrp(
    _Inout_ PIRP Irp
    )
{
  return 0;
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
  return 0;
}

LONG_PTR
FASTCALL
ObfDereferenceObject(
    _In_ PVOID Object
    )
{
  return 0;
}

VOID
KeQuerySystemTime (
    _Out_ PLARGE_INTEGER CurrentTime
    )
{
  return 0;
}

PVOID
ExAllocatePool (
    __drv_strictTypeMatch(__drv_typeExpr) _In_ POOL_TYPE PoolType,
    _In_ SIZE_T NumberOfBytes
    )
{
  return 0;
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
  return 0;
}

NTSTATUS
PsTerminateSystemThread(
    _In_ NTSTATUS ExitStatus
    )
{
  return 0;
}

NTSTATUS
IoAllocateDriverObjectExtension(
    _In_  PDRIVER_OBJECT DriverObject,
    _In_  PVOID ClientIdentificationAddress,
    _In_  ULONG DriverObjectExtensionSize,
    // When successful, this always allocates already-aliased memory.
    _Post_ _At_(*DriverObjectExtension, _When_(return==0,
    __drv_aliasesMem __drv_allocatesMem(Mem) _Post_notnull_))
    _When_(return == 0, _Outptr_result_bytebuffer_(DriverObjectExtensionSize))
    PVOID *DriverObjectExtension
    )
{
  return 0;
}
KeTickCount

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
  return 0;
}
