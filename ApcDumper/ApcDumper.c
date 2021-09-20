#include <ntddk.h>
#include <intrin.h>
#include "ApcSignatures.h"

void DriverUnload(_In_ PDRIVER_OBJECT DriverObject);
NTSTATUS DriverCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

inline void ResolveKernelRoutines() {
	UNICODE_STRING InitializeApcRoutineName = RTL_CONSTANT_STRING(L"KeInitializeApc");
	KeInitializeApc = (KeInitializeApcFunc) MmGetSystemRoutineAddress(&InitializeApcRoutineName);

	UNICODE_STRING InsertApcRoutineName = RTL_CONSTANT_STRING(L"KeInsertQueueApc");
	KeInsertQueueApc = (KeInsertQueueApcFunc) MmGetSystemRoutineAddress(&InsertApcRoutineName);

	if (!KeInitializeApc || !KeInsertQueueApc) {
		DbgBreakPoint();
	}

	KdPrint(("[+] KeInsertQueueApc at: %p\n", KeInsertQueueApc));
	KdPrint(("[+] KeInitializeApc at: %p\n", KeInitializeApc));
}

void TestApc(
	PVOID SystemArgument1,
	PVOID SystemArgument2,
	PVOID SystemArgument3,
	PCONTEXT ContextRecord
) {
	KdPrint(("[+] TestApc called\n"));
	UNREFERENCED_PARAMETER(SystemArgument1);
	UNREFERENCED_PARAMETER(SystemArgument2);
	UNREFERENCED_PARAMETER(SystemArgument3);
	UNREFERENCED_PARAMETER(ContextRecord);
}

inline void DumpCurrentThreadApcs() {
	KThread* CurrentThread = (KThread*) KeGetCurrentThread();
	PKAPC_STATE ApcState = &CurrentThread->ApcState;

	KeAcquireSpinLockAtDpcLevel(&CurrentThread->ThreadLock);

	// dump kernelmode apc's in the current thread
	for (LIST_ENTRY* CurrentApcListEntry = ApcState->ApcListHead[KernelMode].Flink; CurrentApcListEntry != &ApcState->ApcListHead[KernelMode] ; CurrentApcListEntry = CurrentApcListEntry->Flink) {
		KAPC* CurrentApc = (KAPC*)((unsigned __int8*)CurrentApcListEntry - offsetof(KAPC, ApcListEntry));
		
		// KAPC->Reserved[2] == ApcRoutine
		KdPrint(("[+] KernelMode Apc: %p\n", CurrentApc->Reserved[2]));
	}

	// dump usermode apc's in the current thread
	for (LIST_ENTRY* CurrentApcListEntry = ApcState->ApcListHead[UserMode].Flink; CurrentApcListEntry != &ApcState->ApcListHead[UserMode]; CurrentApcListEntry = CurrentApcListEntry->Flink) {
		KAPC* CurrentApc = (KAPC*)((unsigned __int8*)CurrentApcListEntry - offsetof(KAPC, ApcListEntry));
		
		// KAPC->Reserved[2] == ApcRoutine
		KdPrint(("[+] UserMode Apc: %p\n", CurrentApc->Reserved[2]));
	}

	KeReleaseSpinLockFromDpcLevel(&CurrentThread->ThreadLock);
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	KdPrint(("[+] Driver Entry\n"));

	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateClose;
	
	ResolveKernelRoutines();
	PKAPC Apc = ExAllocatePool(NonPagedPool, sizeof(KAPC));

	KIRQL OldIrql;
	KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

	// queue a test apc so we'll see something
	KeInitializeApc(Apc, KeGetCurrentThread(), OriginalApcEnvironment, (void*)ExFreePool, (void*)ExFreePool, (void*)TestApc, KernelMode, NULL);
	KeInsertQueueApc(Apc, NULL, NULL, 0);

	DumpCurrentThreadApcs();

	KeLowerIrql(OldIrql);

	return STATUS_SUCCESS;
}

NTSTATUS DriverCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	KdPrint(("[+] Driver Create\\Close Called\n"));

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

void DriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	KdPrint(("[+] Driver Unload\n"));
}