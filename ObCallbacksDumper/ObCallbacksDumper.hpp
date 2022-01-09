#pragma once
#include <ntddk.h>


namespace driver_routines {
	NTSTATUS driver_create_close(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
	void driver_unload(_In_ PDRIVER_OBJECT DriverObject);
}
