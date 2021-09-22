#include "undocumented.h"
#include <intrin.h>
#include <aux_klib.h>
#pragma comment(lib, "aux_klib.lib")

void* GetNtoskrnlBase() {
	AuxKlibInitialize();
	ULONG BufferSize = 0;
	NTSTATUS status = AuxKlibQueryModuleInformation(&BufferSize, sizeof(AUX_MODULE_EXTENDED_INFO), NULL);
	if (!NT_SUCCESS(status)) {
		return NULL;
	}

	AUX_MODULE_EXTENDED_INFO* QueryModuleInfo = (AUX_MODULE_EXTENDED_INFO*)ExAllocatePool(PagedPool, BufferSize);
	if (!QueryModuleInfo) {
		return NULL;
	}

	for (AUX_MODULE_EXTENDED_INFO* CurrentModuleInfo = QueryModuleInfo; CurrentModuleInfo != (AUX_MODULE_EXTENDED_INFO*)(((unsigned char*)QueryModuleInfo) + BufferSize); CurrentModuleInfo++) {
		if (strstr((const char*)CurrentModuleInfo->FullPathName, "ntoskrnl.exe")){
			return CurrentModuleInfo->BasicInfo.ImageBase;
		}
	}

	DbgBreakPoint();
	return NULL;
}