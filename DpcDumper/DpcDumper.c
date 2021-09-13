#include <ntddk.h>
#include "DpcSignatures.h"

ULONG_PTR ProcessorDumpDpcQueueIpi(ULONG_PTR);

void DumpDpcData(KDPC_DATA* DpcData);

void TestDpcFunction(struct _KDPC* Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2);

void DriverUnload(_In_ PDRIVER_OBJECT DriverObject);

NTSTATUS DriverCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	KdPrint(("[+] Driver Entry\n"));

	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateClose;

	KIRQL OldIrql;
	KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
	
	// Queue a test DPC so we'll see something
	KDPC TestDpc;
	KeInitializeDpc(&TestDpc, &TestDpcFunction, NULL);
	KeInsertQueueDpc(&TestDpc, NULL, NULL);

	KeIpiGenericCall(ProcessorDumpDpcQueueIpi, 0);
	
	KeLowerIrql(OldIrql);

	return STATUS_SUCCESS;
}

ULONG_PTR ProcessorDumpDpcQueueIpi(ULONG_PTR Arg) {
	UNREFERENCED_PARAMETER(Arg);
	KPRCB* CurrentCpu = KeGetCurrentPrcb();
	
	unsigned __int8 ProcessorNumber = (unsigned __int8) CurrentCpu->Number;
	KdPrint(("[+] Dumping Dpc's for Processor: %x\n", ProcessorNumber));

	DumpDpcData(&CurrentCpu->DpcData[NORMAL_DPC_DATA_INDEX]);
	DumpDpcData(&CurrentCpu->DpcData[THREADED_DPC_DATA_INDEX]);
	
	return 0;
}

void DumpDpcData(KDPC_DATA* DpcData) {
	KeAcquireSpinLockAtDpcLevel(&DpcData->DpcLock);
	
	for (SINGLE_LIST_ENTRY* CurrentDpcEntry = DpcData->DpcList.ListHead.Next; CurrentDpcEntry; CurrentDpcEntry = CurrentDpcEntry->Next) {
		KDPC* CurrentDpc = (KDPC*)((unsigned __int8*)CurrentDpcEntry - offsetof(KDPC, DpcListEntry));
		KdPrint(("[+] Dpc routine at: %p\n", CurrentDpc->DeferredRoutine));
	}

	KeReleaseSpinLockFromDpcLevel(&DpcData->DpcLock);
}

void TestDpcFunction(
	struct _KDPC* Dpc,
	PVOID DeferredContext,
	PVOID SystemArgument1,
	PVOID SystemArgument2
) {
	UNREFERENCED_PARAMETER(Dpc);
	UNREFERENCED_PARAMETER(DeferredContext);
	UNREFERENCED_PARAMETER(SystemArgument1);
	UNREFERENCED_PARAMETER(SystemArgument2);
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
