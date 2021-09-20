# APC's
(Stolen from Practical Reverse Engineering):  
APCs are functions that execute in a particular thread context.  
They can be divided into two types: kernel-mode and user-mode.  
Kernel-mode APCs can be either normal or special; normal ones execute at PASSIVE_LEVEL,  
whereas special ones execute at APC_LEVEL (both execute in kernel mode).  
User APCs execute at PASSIVE_LEVEL in user mode when the thread is in an alertable state.  

## NtQueueApcThreadEx
```c
NTSTATUS NtQueueApcThreadEx(
	HANDLE ThreadHandle, 
	HANDLE MemoryReserveHandle, 
	PVOID ApcRoutine, 
	PVOID NormalContext, 
	PVOID SystemArgument1, 
	PVOID SystemArgument2
){
	_KTHREAD* CurrentThread = KeGetCurrentThread();
	_KTHREAD* TargetApcThread = NULL;
	PVOID MemoryReserver = NULL;
	KAPC* ApcObj = NULL;

	NTSTATUS status = ObReferenceObjectByHandle(
		ThreadHandle,
		THREAD_SET_CONTEXT,
		PsThreadType,
		CurrentThread->PreviousMode,
		&TargetApcThread,
		NULL
	);

	if (!NT_SUCCESS(status)) {
		// return the status from ObReferenceObjectByHandle
		goto cleanup;
	}

	if (!TargetApcThread->SystemThread) {
		status = STATUS_INVALID_HANDLE;
		goto cleanup;
	}

	_EWOW64PROCESS* Wow64Process = TargetApcThread->Process->Wow64Process;
	_EWOW64PROCESS* ApcStateWow64Process = TargetApcThread->ApcState.Process->Wow64Process;
	if (ApcStateWow64Process && 
		ApcStateWow64Process->Machine != IMAGE_FILE_MACHINE_I386 && 
		ApcStateWow64Process->Machine != IMAGE_FILE_MACHINE_ARMNT &&
		Wow64Process &&
		Wow64Process->Machine != IMAGE_FILE_MACHINE_AMD64 &&
		(__int64)ApcRoutine >> 2 > 0xffffffff
	) {
		status = STATUS_INVALID_HANDLE;
		goto cleanup;
	}

	PVOID KernelRoutine = NULL;
	PVOID RundownRoutine = NULL;

	if (MemoryReserveHandle && MemoryReserveHandle != (HANDLE) 1) {
		NTSTATUS status = ObReferenceObjectByHandle(
			MemoryReserveHandle,
			0x2, // non-documented access type
			PspMemoryReserveObjectTypes,
			CurrentThread->PreviousMode,
			&MemoryReserver,
			NULL
		);

		if(!NT_SUCCESS(status)){
			goto cleanup;
		}
		
		// this is my hypothesis for this check:
		// atomically, check if the MemoryReserver is occupied.
		// if it is: return an err.
		// else: atomically mark it as occupied and continue
		// 
		// (pseudocode)
		// struct MEMORY_RESERVER {
		//     unsigned int IsOccupied;
		//     char pad[4]; 
		//     char ReservedMemory[sizeof(RESERVED_TYPE)] 
		// } 
		//
		// unsigned int IsMemoryReserverOccupied = MemoryReserver->IsOccupied;
		// MemoryReserver->IsOccupied = MemoryReserver->IsOccupied == FALSE ? TRUE : FALSE;
		// if (!IsMemoryReserverOccupied) {...}
		if(!_InterlockedCompareExchange((unsigned int*)MemoryReserver, 1, 0)) {
			status = STATUS_INVALID_PARAMETER_2;
			goto cleanup;
		}

		ApcObj = (KAPC*) ((BYTE*)MemoryReserver + 8);
		RundownRoutine = PspUserApcReserveKernelRoutine;
		KernelRoutine = PspUserApcReserveKernelRoutine;
		// NTSTATUS __fastcall PspUserApcReserveKernelRoutine(void *ReservedObject) {
		//   MEMORY_RESERVER* MemoryReserver = (char *)ReservedObject - 8;
		//   MemoryReserver->IsOccupied = FALSE;
		//   return ObfDereferenceObject(MemoryReserver);
		// }
	}
	else {
		ApcObj = (KAPC*) ExAllocatePoolWithQuotaTag(sizeof(KAPC), NonPagedPoolNx, 'Psap');
		RundownRoutine = ExFreePool;
		KernelRoutine = ExFreePool;
	}


	KeInitializeApc(
		ApcObj, 
		TargetApcThread,
		OriginalApcEnvironment,
		(PKKERNEL_ROUTINE)ExFreePool,
		(PKRUNDOWN_ROUTINE)ExFreePool,
		ApcRoutine,
		UserMode,
		NormalContext
	);

	if(!KeInsertApcQueue(ApcObj, SystemArgument1, SystemArgument2, 0)){
		// wrapper around indirect calls
		_guard_dispatch_icall(RundownRoutine, ApcObj);

		status = STATUS_UNSUCCESSFUL;	
		goto cleanup;
	}

	status = STATUS_SUCCESS;

cleanup:
	if (TargetApcThread)
		ObfDereferenceObject(TargetApcThread);
	
	if (MemoryReserver)
		ObfDereferenceObject(MemoryReserver);

	return status;
}
```

## NtQueueApcThread
```c
NTSTATUS NtQueueApcThread(
	HANDLE ThreadHandle, 
	PKNORMAL_ROUTINE ApcRoutine, 
	PVOID NormalContext,
	PVOID SystemArgument1, 
	PVOID SystemArgument2
){
	return NtQueueApcThreadEx(
		ThreadHandle,
		0, // MemoryReserveHandle = NULL
		ApcRoutine,
		NormalContext
		SystemArgument1,
		SystemArgument2,
	);
}
```
## KeInsertQueueApc
```c
BOOLEAN KeInsertQueueApc(
	PRKAPC Apc,
	PVOID SystemArgument1,
	PVOID SystemArgument2,
	KPRIORITY Increment
) {
	BOOLEAN Inserted;
	KIRQL OldIrql;
	KiAcquireThreadLockRaiseToDpc(Apc->Thread, &OldIrql);
	KPRCB* CurrentCPU = KeGetCurrentPcrb();

	if (Apc->Inserted || Apc->Thread->ApcQueueable) {
		Inserted = FALSE;
	}
	else {
		Apc->Inserted = TRUE;
		Apc->SystemArgument1 = SystemArgument1;
		Apc->SystemArgument2 = SystemArgument2;
	
		KiInsertQueueApc(Apc);
		KiSignalThreadForApc(CurrentCPU, Apc, OldIrql);		
	
		Inserted = TRUE;
	}

	KiReleaseThreadLockSafe(TargetThread);
	KiExitDispatcher(CurrentCPU, 0, 1, Increment, OldIrql);
	
	return Inserted;
}
```

## KiInsertQueueApc
```c
void KiInsertQueueApc(KAPC* Apc) {
	PKTHREAD ApcThread = Apc->Thread;
	APC_STATE* ApcState = !Apc->ApcStateIndex && ApcThread->ApcStateIndex ? ApcThread->ApcState : ApcThread->SavedApcState;
	InsertTailList(&ApcState->ApcListHead[Apc->ApcMode], &Apc->ApcListEntry);
}
```

## KiDeliverApc
```c
void KiDeliverApc(KPROCESSOR_MODE PreviouseMode, _LIST_ENTRY *a2, _KTRAP_FRAME *NewTrapFrame) {
	PKTHREAD CurrentThread = KeGetCurrentThread();
	
	PKTRAP_FRAME CurrentTrapFrame = CurrentThread->TrapFrame;
	PKPROCESS SavedApcStateProcess = CurrentThread->ApcState.Process;
	CurrentThread->TrapFrame = NewTrapFrame;
	CurrentThread->ApcState.KernelApcPending = 0;

	BOOLEAN IsSpecialApcEnabled = CurrentThread->SpecialApcDisable == 0;
	if (!IsSpecialApcEnabled)
		goto cleanup;

	PKAPC_STATE CurrentThreadApcState = &CurrentThread->ApcState;

	// check if apc queue is empty
	if (CurrentThreadApcState->ApcListHead[0].Flink == CurrentThreadApcState)
		goto cleanup;

	KIRQL OldIrql;
	KiAcquireThreadLockRaiseToDpc(CurrentThread, &OldIrql);

	// check if apc queue is empty (again)
    if (CurrentThreadApcState->ApcListHead[0].Flink == CurrentThreadApcState)
		goto cleanup;

	LIST_ENTRY* ApcListEntry = CurrentThreadApcState->ApcListHead[0].Flink;
	CurrentThread->ApcState.KernelApcPending = 0;
	PKAPC TargetApc = CONTAINING_RECORD(ApcListEntry, _KAPC, ApcListEntry);

	PVOID KernelRoutine = TargetApc->KernelRoutine;
	PVOID NormalRoutine = TargetApc->NormalRoutine;
	PVOID NormalContext = TargetApc->NormalContext;
	PVOID SystemArgument1 = TargetApc->SystemArgument1;
	PVOID SystemArgument2 = TargetApc->SystemArgument2;

	// NOTE: this is simplified. it'll be different for different types of apc's
	if (NormalRoutine) {
		if (CurrentThread->ApcState.InProgressFlags || CurrentThread->KernelApcDisable) {
			KiReleaseThreadLockSafe(CurrentThread);
			KeLowerIrql(APC_LEVEL);
			goto cleanup;
		}

		LIST_ENTRY *ApcFlink = TargetApc->Flink;
		LIST_ENTRY *ApcBlink = TargetApc->Blink;
		
		// check for curropted list
		if (ApcFlink->Blink != ApcListEntry || Blink->Flink != ApcListEntry)
			__fastfail(3);

		// unlink apc entry
		ApcBlink->Flink = ApcFlink;
		ApcFlink->Blink = ApcBlink;
		TargetApc->Inserted = FALSE;

		// also lowers irql
		KiReleaseThreadLockSafe(CurrentThread);

		KeRaiseIrql(APC_LEVEL, &OldIrql);
		KernelRoutine(
			TargetApc,
			&NormalRoutine, 
			&NormalContext, 
			&SystemArgument1, 
			&SystemArgument2
		);

		if (NormalRoutine) {
			KeLowerIrql(PASSIVE_LEVEL);
			NormalRoutine(NormalContext, SystemArgument1, SystemArgument2);
			KeRaiseIrql(APC_LEVEL, &OldIrql);
		}

		CurrentThread->ApcState.InProgressFlags = 0;

		if(PreviouseMode == UserMode) {
			KiInitializeUserApc(
			ExceptionFrame,
			TrapFrame,
			NormalRoutine,
			NormalContext,
			SystemArgument1,
			SystemArgument2,
			...
			...
		);
		}
	}
}
```
