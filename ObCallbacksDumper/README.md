# Object Manager Callbacks

The two main API's interfacing with the object manager callbacks are: `ObRegisterCallbacks` and `ObUnRegisterCallbacks`.

## ObRegisterCallbacks

(simplified)

```c
typedef struct {
    LIST_ENTRY CallbackListEntry;                                    // 0x0
    OB_OPERATION Operations;                                         // 0x10
    PINTERNAL_OB_REGISTRATION_STRUCT InternalRegistrationContextPtr; // 0x18
    POBJECT_TYPE ObjectType;                                         // 0x20
    POB_PRE_OPERATION_CALLBACK  PreOperation;                        // 0x28
    POB_POST_OPERATION_CALLBACK PostOperation;                       // 0x30
	EX_PUSH_LOCK Lock;                                               // 0x38
} INTERNAL_REGISTRATION_RECORD, *PINTERNAL_REGISTRATION_RECORD; // sizeof = 0x64


typedef struct {
    USHORT Version;                                                // 0x0
    USHORT RegisteredCallbacksCount;                               // 0x2
    PVOID RegistrationContext;                                     // 0x8
    UNICODE_STRING Altitude;                                       // 0x10
    INTERNAL_REGISTRATION_RECORD Callbacks[1];                     // 0x20
    CHAR AltitudeBuffer[1];
} INTERNAL_OB_REGISTRATION_STRUCT, *PINTERNAL_OB_REGISTRATION_STRUCT;

NTSTATUS ObRegisterCallbacks(POB_CALLBACK_REGISTRATION CallbackRegistration, PVOID *RegistrationHandle) {
    if (CallbackRegistration->Version != OB_FLT_REGISTRATION_VERSION ||
        CallbackRegistration.OperationRegistrationCount == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    uint32_t InnerContextSize = CallbackRegistration->Altitude.Length + 0x20 + CallbackRegistration->OperationRegistrationCount * sizeof(INTERNAL_REGISTRATION_RECORD);    
    PINTERNAL_OB_REGISTRATION_STRUCT InnerContext = ExAllocatePoolWithTag(
        PagedPool, 
        InnerContextSize,
        'ObFl'
    );
    if (!InnerContext) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    memmove(InnerContext, 0, InnerContextSize);
    
	NTSTATUS status = STATUS_SUCCESS;
    for (unsigned int i = 0; i < CallbackRegistration->OperationRegistrationCount; i++) {
        POB_OPERATION_REGISTRATION CurrentEntry = CallbackRegistration->OperationRegistration[i];
        if (CurrentEntry->Operation == 0 || !IS_VALID_OBJECT_TYPE(CurrentEntry->ObjectType)
           || (CurrentEntry->PreOperation == NULL && CurrentEntry->PostOperation == NULL)) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        
        if ((CurrentEntry->PreOperation && !MmVerifyCallbackFunctionCheckFlags(CurrentEntry->PreOperation, 0x20)) ||
            (CurrentEntry->PostOperation && !MmVerifyCallbackFunctionCheckFlags(CurrentEntry->PostOperation, 0x20))) {
            status = STATUS_ACCESS_DEFINED;
            break;
        }
        
        PINTERNAL_REGISTRATION_RECORD CallbackRecord = &InnerContext->Callbacks[i];
        InitializeListHead(&CallbackRecord->CallbackListEntry);
        ExInitializePushLock(&CallbackRecord->Lock);
        CallbackRecord->InternalRegistrationContextPtr = InnerContext;
        CallbackRecord->PreOperation = CurrentEntry->PreOperation;
        CallbackRecord->PostOperation = CurrentEntry->PostOperation;
        
        POBJECT_TYPE ObjectType = *CurrentEntry->ObjectType;
        CallbackRecord->ObjectType = ObjectType;
        
        // Inserts callback by altitude to the linked list ObjectType->CallbackList
        ObpInsertCallbackByAltitude(ObjectType, CallbackRecord);
        InnerContext->RegisteredCallbacksCount++;
    }
    
    if (status == STATUS_SUCCESS) {
        *RegistrationHandle = InnerContext;
    }
    return status;
}
```