#include <ntddk.h>
#include <intrin.h>
#include "ObCallbacksDumper.hpp"
#include "ob_callbacks.hpp"


extern "C" 
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);
	KdPrint(("[+] Driver Entry\n"));

	DriverObject->DriverUnload = driver_routines::driver_unload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = driver_routines::driver_create_close;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = driver_routines::driver_create_close;

	if (!ob_callbacks::register_dummy_callback()) {
		__debugbreak();
		return STATUS_FAILED_DRIVER_ENTRY;
	}
	ob_callbacks::dump_callbacks();

	return STATUS_SUCCESS;
}

NTSTATUS driver_routines::driver_create_close(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	KdPrint(("[+] Driver Create\\Close Called\n"));

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

void driver_routines::driver_unload(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	KdPrint(("[+] Driver Unload\n"));

	ob_callbacks::unregister_dummy_callback();
}
