#include <ntddk.h>
#include <intrin.h>
#include "undocumented.h"

void DriverUnload(_In_ PDRIVER_OBJECT DriverObject);
NTSTATUS DriverCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

void DumpNotifyRoutinesArr(CallbackBlock** NotifyRoutines) {
	for (unsigned __int64 i = 0; i < NOTIFY_ROUTINE_ARR_SIZE; i++) {
		if (!NotifyRoutines[i]) {
			continue;
		}

		CallbackBlock* CurrentCallback = (CallbackBlock*)(((unsigned __int64)NotifyRoutines[i]) & 0xfffffffffffffff0);

		ExAcquireRundownProtection(&CurrentCallback->RundownProtection);
		KdPrint(("[+] Callback at: %p\n", CurrentCallback->CallbackRoutine));
		ExReleaseRundownProtection(&CurrentCallback->RundownProtection);
	}
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	KdPrint(("[+] Driver Entry\n"));

	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateClose;

	unsigned __int8* NtoskrnlBase = (unsigned __int8*) GetNtoskrnlBase();
	KdPrint(("[+] Ntoskrnl base at: %p\n", NtoskrnlBase));
	
	CallbackBlock** PsCreateThreadNotifyRoutinesArray = (CallbackBlock**) (NtoskrnlBase + THREAD_NOTIFY_ROUTINE_ARR_OFFSET);
	KdPrint(("[+] ***************DUMPING CREATE THREAD NOTIFY ROUTINES***************\n"));
	DumpNotifyRoutinesArr(PsCreateThreadNotifyRoutinesArray);

	CallbackBlock** PsCreateProcessNotifyRoutinesArray = (CallbackBlock**) (NtoskrnlBase + PROCESS_NOTIFY_ROUTINE_ARR_OFFSET);
	KdPrint(("[+] ***************DUMPING CREATE PROCESS NOTIFY ROUTINES***************\n"));
	DumpNotifyRoutinesArr(PsCreateProcessNotifyRoutinesArray);

	CallbackBlock** PsImageLoadNotifyRoutinesArray = (CallbackBlock**) (NtoskrnlBase + IMAGE_LOAD_NOTIFY_ROUTINE_ARR_OFFSET);
	KdPrint(("[+] ***************DUMPING IMAGE LOAD NOTIFY ROUTINES***************\n"));
	DumpNotifyRoutinesArr(PsImageLoadNotifyRoutinesArray);

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
