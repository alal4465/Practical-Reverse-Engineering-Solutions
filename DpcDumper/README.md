# DPC's

## Background
Deferred procedure calls (DPCs) are routines executed at DISPATCH_LEVEL in arbitrary thread context on a particular processor(by default, the processor that queued the DPC).

They are called in three places:
* `KiIdleLoop`: While “idling,” it checks the PRCB to determine if DPCs are waiting and if so to call *KiRetireDpcList* to process all DPCs.
* `KiExecuteDpc`:  *KiStartDpcThread* creates a thread (*KiExecuteDpc*) for each processor, which processes the DPC queue whenever it runs.
* `IRQL` drops: when the irql of a processor drops to DISPATCH_LEVEL or, it processes all of it's DPC's.

## Some minimized pseudocode of stuff I reversed:

* **KeInsertDpc**
```c
BOOLEAN KeInsertQueueDpc(PRKDPC Dpc, PVOID SystemArgument1, PVOID SystemArgument2) {
	return KiInsertQueueDpc(Dpc, SystemArgument1, SystemArgument2, 0, 0);
}
```

* **KiInsertQueueDpc**
```c
BOOLEAN KiInsertQueueDpc(Dpc, SystemArgument1, SystemArgument2, 0, 0){
	uint16_t current_irql = KeGetCurrentIrql();
	uint16_t old_irql = current_irql;
	KeRaiseIrql(0xF);

	PKPRCB TargetCPU;
	uint8_t is_dpc_processor_num_not_equal_to_current = FALSE;
	if(KDPC->Number >= 500) {
		 TargetCPU = KiProcessorBlock[0x500 - KDPC->Number];
		 if (!dpc_target_proc){
		 	KeBugCheckEx();
		 }
	}
	else {
		TargetCPU = KeGetCurretPrcb();
		if(KDPC->Number != KPCRB->Number){
			is_dpc_processor_num_not_equal_to_current = TRUE;
		} 
	}


	uint8_t dpc_data_offset = (TargetCPU->type == ThreadedDpcObjet && TargetCPU.ThreadDpcEnable) ? 1 : 0;
	DpcData dpc_data = KPRCB.DpcData[dpc_data_offset];

	if (KDPC->Importance == 2) { // High Importance
		if(!DpcData.DpcList.ListHead.Next) {
			DpcData->DpcList.LastEntry =  &TargetCPU->DpcListEntry;
		}

		TargetCPU->DpcListEntry.Next = &DpcData->DpcList.ListHead.Next;
		DpcData->DpcList.ListHead.Next = &TargetCPU->DpcListEntry;
	}
	else {
		TargetCPU->DpcListEntry.Next = NULL;
		DpcData->DpcList.LastEntry->Next = TargetCPU->DpcListEntry;
		DpcData->DPcList.LastEntry = TargetCPU->DpcListEntry;
	}
}
```

* **KiRetireDpcList**
```c
// a whole bunch of flags and cycles stuff....
// ...
NTSTATUS KiRetireDpcList(_KPRCB* KPRCB) {
	KiExecuteAllDpcs(KPRCB, CurrentThread, &zero_inintialized, 0i64);
	if ( (KPRCB->DpcRequestSlot[0] & 4) != 0 )
	{
	 	_enable(); // sti
	 	KeSignalGate(&KPRCB->DpcGate, 0);
	 	_disable(); // cli
	}	
}

```

* **KiExecuteDpc**
```c
BOOLEAN KiTryToEndDpcProcessing(uint32_t* ThreadDpcState, DPC_DATA* DpcData) {
	// using a lock cmpxhg
	if (*ThreadDpcState == 1) { // maybe executing?
		*ThreadDpcState = 0;
		ActiveDpc = NULL;
		return TRUE;
	}

	return FALSE;
}

// Created as a system thread to execute dpc's
void KiExecuteDpc(_KPRCB *KPRCB) {
	PKTHREAD CurrentThread = KeGetCurrentThread();
	KeSetPriorityThread(CurrentThread, 0x1f);

	// sets the current thread to run on the dpc's processor(maybe)
	KiSetSystemAffinityThreadToProcessor(KPRCB->Number);

	while(TRUE) {
		uint64_t OutValue;
		KiExecuteAllDpcs(KPRCB, CurrentThread, &OutValue, 1);
		KiTryToEndDpcProcessing(&KPRCB->ThreadDpcState, &KPRCB.DpcData[1]); // 1 is the threaded dpc index
	}
}
```

* **KiExecuteAllDpcs**
```c
__int64 KiExecuteAllDpcs(
	_KPRCB *KPRCB,
	_KTHREAD *ExecThread, 
	void *OutValue, 
	_QWORD DpcDataArrOffIsThreaded
){
	// equivalent to: _KDPC_DATA* DpcData = &KPRCB->DpcData[DpcDataArrOffIsThreaded]
	__int64 PrcbDpcDataOffset = 0x5c0 + DpcDataArrOffIsThreaded * 5;
	_KDPC_DATA* DpcData = (_KDPC_DATA*) ((BYTE*)KPRCB + 8 * PrcbDpcDataOffset);
	
	while(DpcData.DpcQueueDepth != 0) {
		KxWaitForSpinLockAndAcquire(&DpcData->Lock);

		SingleListEntry* CurrentDpcListEntry = DpcData->DpcList.ListHead.Next;
		SingleListEntry* NextDpcListEntry = CurrentDpcListEntry->Next;
		DpcData->DpcList.ListHead.Next = NextDpcListEntry->Next;

		if(NextDpcListEntry == NULL){
			DpcData->DpcList.LastEntry = &(DpcList->Dpclist.ListHead);
		}

		_KDPC* KDPC = (*KDPC) ((*BYTE)CurrentDpcListEntry - 8);
		PKDEFERRED_ROUTINE DeferredRoutine = KDPC->DeferredRoutine;
		void* DeferredContext = KDPC->DeferredContext;
		void* SystemArgument1 = KDPC->SystemArgument1;
		void* SystemArgument2 = KDPC->SystemArgument2;
		void* DpcEntryDpcData = KDPC->DpcData;

		// the processor's DpcData NOT the KDPC's DpcData
		DpcData.DpcQueueDepth--;

		KxReleaseSpinLock(&DpcData->Lock);

		// a wrapper for dispatching indirect calls while making some checks.
		// rax = FuncPtr
		// rcx = arg1
		// rdx = arg2
		// ...
		__guard_dispatch_icall(_KDPC* Dpc, DeferredContext, SystemArgument1, SystemArgument2);
	}
}

```
