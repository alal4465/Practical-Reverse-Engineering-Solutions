# Notify Routines - Relevant Function Decompilations

## PsSetCreateThreadNotifyRoutine
```c
NTSTATUS PsSetCreateThreadNotifyRoutine(PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine) {
	return PspSetCreateThreadNotifyRoutine(NotifyRoutine, 0i64);
}
```

## PspSetCreateThreadNotifyRoutine
```c
struct CallbackObj {
	_EX_RUNDOWN_REF RundownProtection;
	void *CallbackRoutine;
	__int64 Flags;
};

CallbackObj* ExAllocateCallBack(void *CallbackRoutine, __int64 Flags) {
	CallbackObj* Callback = (CallbackObj *)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(CallbackObj), 'brbC');
	if (Callback) {
    	// 0 initializes the RundownProtection field
    	ExInitializePushLock(&Callback->RundownProtection.Count);

    	Callback->CallbackRoutine = CallbackRoutine;
    	Callback->Flags = Flags;
  }

  return Callback;
}

NTSTATUS PspSetCreateThreadNotifyRoutine(PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine, __int64 Flags) {
	CallbackRoutine* Callback = ExAllocateCallback(NotifyRoutine, Flags);
	if (!Callback)
    	return STATUS_INSUFFICIENT_RESOURCES;

	// find an empty place in the array of thread notifications (size = 0x40)
	// and store the callback object ptr in it
    unsigned __int64 FailedCallbackInsertionCount = 0;
    while (!ExCompareExchangeCallBack(
    	(signed __int64 *)&PspCreateThreadNotifyRoutine.Ptr + FailedCallbackInsertionCount,
		Callback,
		0
	)) {
    	FailedCallbackInsertionCount++;
    	if (FailedCallbackInsertionCount >= 0x40) {
    		// no free space in the callback array
    		ExFreePoolWithTag(Callback, 0);
    		return STATUS_INSUFFICIENT_RESOURCES;
    	}
  	}

  	// check if callback flags 0th bit is set(likely indicates a nonsystem notify routine)
  	if ( (flags_ & 1) != 0 ) {
  		// increase count of nonsystem thread notify routines 
		_InterlockedIncrement(&PspCreateThreadNotifyRoutineNonSystemCount);
    
		// if the 3rd bit of PspNotifyEnableMask is off, turn it on
		// this bit likely points out whether normal nonsystem notify routines are enalbed or not
		if ( (PspNotifyEnableMask & 0x10) == 0 )
			_interlockedbittestandset(&PspNotifyEnableMask, 4u);
  }

	// notify routine is of normal type
	else {
  
  		// increase count of normal thread notify routines 
    	_InterlockedIncrement(&PspCreateThreadNotifyRoutineCount);
    
    	// if the 3rd bit of PspNotifyEnableMask is off, turn it on
    	// this bit likely points out whether normal thread notify routines are enalbed or not
    	if ( (PspNotifyEnableMask & 8) == 0 )
      		_interlockedbittestandset(&PspNotifyEnableMask, 3u);
  }
}
```
