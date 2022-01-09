#pragma once
#include <ntddk.h>
#include <intrin.h>
#pragma warning( disable : 4201 )


namespace ob_callbacks {
	void dump_callbacks();
	bool register_dummy_callback();
	void unregister_dummy_callback();

    typedef struct _OBJECT_TYPE {
        struct _LIST_ENTRY TypeList;                                            //0x0
        struct _UNICODE_STRING Name;                                            //0x10
        VOID* DefaultObject;                                                    //0x20
        UCHAR Index;                                                            //0x28
        ULONG TotalNumberOfObjects;                                             //0x2c
        ULONG TotalNumberOfHandles;                                             //0x30
        ULONG HighWaterNumberOfObjects;                                         //0x34
        ULONG HighWaterNumberOfHandles;                                         //0x38
        unsigned char pad[0x78];                                                //0x40
        EX_PUSH_LOCK TypeLock;                                                  //0xb8
        ULONG Key;                                                              //0xc0
        struct _LIST_ENTRY CallbackList;                                        //0xc8
    } OBJECT_TYPE, *POBJECT_TYPE;

    typedef struct {
        LIST_ENTRY CallbackListEntry;                                    // 0x0
        OB_OPERATION Operations;                                         // 0x10
        PVOID InternalRegistrationContextPtr;                            // 0x18
        POBJECT_TYPE ObjectType;                                         // 0x20
        POB_PRE_OPERATION_CALLBACK  PreOperation;                        // 0x28
        POB_POST_OPERATION_CALLBACK PostOperation;                       // 0x30
        EX_PUSH_LOCK Lock;                                               // 0x38
    } INTERNAL_REGISTRATION_RECORD, * PINTERNAL_REGISTRATION_RECORD; // sizeof = 0x64

}
