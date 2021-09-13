#pragma once
#include <ntddk.h>
#include <intrin.h>

/*
* NOTE: This is correct for windows Redstone 5 (October Update) build 10.0.17763.107.
* It'll be a miracle if it works on any other version.
*/

// Indices for the KPRCB.DpcData array
#define THREADED_DPC_DATA_INDEX 0
#define NORMAL_DPC_DATA_INDEX 1

// nameless struct/union
#pragma warning(disable : 4201)

//0x10 bytes (sizeof)
typedef struct _KDPC_LIST
{
    struct _SINGLE_LIST_ENTRY ListHead;                                     //0x0
    struct _SINGLE_LIST_ENTRY* LastEntry;                                   //0x8
} KDPC_LIST;

//0x28 bytes (sizeof)
typedef struct _KDPC_DATA
{
    struct _KDPC_LIST DpcList;                                              //0x0
    ULONGLONG DpcLock;                                                      //0x10
    volatile LONG DpcQueueDepth;                                            //0x18
    ULONG DpcCount;                                                         //0x1c
    struct _KDPC* volatile ActiveDpc;                                       //0x20
} KDPC_DATA;

//0x7ec0 bytes (sizeof)
typedef struct _KPRCB
{
    unsigned long MxCsr;                                                            //0x0
    unsigned __int8 LegacyNumber;                                                     //0x4
    unsigned __int8 ReservedMustBeZero;                                               //0x5
    unsigned __int8 InterruptRequest;                                                 //0x6
    unsigned __int8 IdleHalt;                                                         //0x7
    struct _KTHREAD* CurrentThread;                                         //0x8
    struct _KTHREAD* NextThread;                                            //0x10
    struct _KTHREAD* IdleThread;                                            //0x18
    unsigned __int8 NestingLevel;                                                     //0x20
    unsigned __int8 ClockOwner;                                                       //0x21
    union
    {
        unsigned __int8 PendingTickFlags;                                             //0x22
        struct
        {
            unsigned __int8 PendingTick : 1;                                            //0x22
            unsigned __int8 PendingBackupTick : 1;                                      //0x22
        };
    };
    unsigned __int8 IdleState;                                                        //0x23
    unsigned long Number;                                                           //0x24
    unsigned __int64 RspBase;                                                      //0x28
    unsigned __int64 PrcbLock;                                                     //0x30
    char* PriorityState;                                                    //0x38
    char CpuType;                                                           //0x40
    char CpuID;                                                             //0x41
    union
    {
        unsigned __int8 CpuStep;                                                     //0x42
        struct
        {
            unsigned __int8 CpuStepping;                                              //0x42
            unsigned __int8 CpuModel;                                                 //0x43
        };
    };
    unsigned long MHz;                                                              //0x44
    unsigned __int64 HalReserved[8];                                               //0x48
    unsigned __int8 MinorVersion;                                                    //0x88
    unsigned __int8 MajorVersion;                                                    //0x8a
    unsigned __int8 BuildType;                                                        //0x8c
    unsigned __int8 CpuVendor;                                                        //0x8d
    unsigned __int8 CoresPerPhysicalProcessor;                                        //0x8e
    unsigned __int8 LogicalProcessorsPerCore;                                         //0x8f
    unsigned __int64 PrcbPad04[6];                                                 //0x90
    struct _KNODE* ParentNode;                                              //0xc0
    unsigned __int64 GroupSetMember;                                               //0xc8
    unsigned __int8 Group;                                                            //0xd0
    unsigned __int8 GroupIndex;                                                       //0xd1
    unsigned __int8 PrcbPad05[2];                                                     //0xd2
    unsigned long InitialApicId;                                                    //0xd4
    unsigned long ScbOffset;                                                        //0xd8
    unsigned long ApicMask;                                                         //0xdc
    void* AcpiReserved;                                                     //0xe0
    unsigned long CFlushSize;                                                       //0xe8
    unsigned char pad[4];                                                   //0xec
    union
    {
        struct
        {
            unsigned __int64 TrappedSecurityDomain;                                //0xf0
            union
            {
                unsigned __int8 BpbState;                                             //0xf8
                struct
                {
                    unsigned __int8 BpbCpuIdle : 1;                                     //0xf8
                    unsigned __int8 BpbFlushRsbOnTrap : 1;                              //0xf8
                    unsigned __int8 BpbIbpbOnReturn : 1;                                //0xf8
                    unsigned __int8 BpbIbpbOnTrap : 1;                                  //0xf8
                    unsigned __int8 BpbIbpbOnRetpolineExit : 1;                         //0xf8
                    unsigned __int8 BpbStateReserved : 3;                               //0xf8
                };
            };
            union
            {
                unsigned __int8 BpbFeatures;                                          //0xf9
                struct
                {
                    unsigned __int8 BpbClearOnIdle : 1;                                 //0xf9
                    unsigned __int8 BpbEnabled : 1;                                     //0xf9
                    unsigned __int8 BpbSmep : 1;                                        //0xf9
                    unsigned __int8 BpbFeaturesReserved : 5;                            //0xf9
                };
            };
            unsigned __int8 BpbCurrentSpecCtrl;                                       //0xfa
            unsigned __int8 BpbKernelSpecCtrl;                                        //0xfb
            unsigned __int8 BpbNmiSpecCtrl;                                           //0xfc
            unsigned __int8 BpbUserSpecCtrl;                                          //0xfd
            volatile SHORT PairRegister;                                    //0xfe
        };
        unsigned __int64 PrcbPad11[2];                                             //0xf0
    };
    unsigned char ProcessorState[0x5c0];                                //0x100
    struct _XSAVE_AREA_HEADER* ExtendedSupervisorState;                     //0x6c0
    unsigned long ProcessorSignature;                                               //0x6c8
    unsigned long ProcessorFlags;                                                   //0x6cc
    union
    {
        struct
        {
            unsigned __int8 BpbRetpolineExitSpecCtrl;                                 //0x6d0
            unsigned __int8 BpbTrappedRetpolineExitSpecCtrl;                          //0x6d1
            union
            {
                unsigned __int8 BpbTrappedBpbState;                                   //0x6d2
                struct
                {
                    unsigned __int8 BpbTrappedCpuIdle : 1;                              //0x6d2
                    unsigned __int8 BpbTrappedFlushRsbOnTrap : 1;                       //0x6d2
                    unsigned __int8 BpbTrappedIbpbOnReturn : 1;                         //0x6d2
                    unsigned __int8 BpbTrappedIbpbOnTrap : 1;                           //0x6d2
                    unsigned __int8 BpbTrappedIbpbOnRetpolineExit : 1;                  //0x6d2
                    unsigned __int8 BpbtrappedBpbStateReserved : 3;                     //0x6d2
                };
            };
            union
            {
                unsigned __int8 BpbRetpolineState;                                    //0x6d3
                struct
                {
                    unsigned __int8 BpbRunningNonRetpolineCode : 1;                     //0x6d3
                    unsigned __int8 BpbIndirectCallsSafe : 1;                           //0x6d3
                    unsigned __int8 BpbRetpolineEnabled : 1;                            //0x6d3
                    unsigned __int8 BpbRetpolineStateReserved : 5;                      //0x6d3
                };
            };
            unsigned long PrcbPad12b;                                               //0x6d4
        };
        unsigned __int64 PrcbPad12a;                                               //0x6d0
    };
    unsigned __int64 PrcbPad12[3];                                                 //0x6d8
    struct _KSPIN_LOCK_QUEUE LockQueue[17];                                 //0x6f0
    unsigned char PPLookasideList[0x100];                          //0x800
    struct _GENERAL_LOOKASIDE_POOL PPNxPagedLookasideList[32];              //0x900
    struct _GENERAL_LOOKASIDE_POOL PPNPagedLookasideList[32];               //0x1500
    struct _GENERAL_LOOKASIDE_POOL PPPagedLookasideList[32];                //0x2100
    unsigned __int64 PrcbPad20;                                                    //0x2d00
    struct _SINGLE_LIST_ENTRY DeferredReadyListHead;                        //0x2d08
    volatile long MmPageFaultCount;                                         //0x2d10
    volatile long MmCopyOnWriteCount;                                       //0x2d14
    volatile long MmTransitionCount;                                        //0x2d18
    volatile long MmDemandZeroCount;                                        //0x2d1c
    volatile long MmPageReadCount;                                          //0x2d20
    volatile long MmPageReadIoCount;                                        //0x2d24
    volatile long MmDirtyPagesWriteCount;                                   //0x2d28
    volatile long MmDirtyWriteIoCount;                                      //0x2d2c
    volatile long MmMappedPagesWriteCount;                                  //0x2d30
    volatile long MmMappedWriteIoCount;                                     //0x2d34
    unsigned long KeSystemCalls;                                                    //0x2d38
    unsigned long KeContextSwitches;                                                //0x2d3c
    unsigned long PrcbPad40;                                                        //0x2d40
    unsigned long CcFastReadNoWait;                                                 //0x2d44
    unsigned long CcFastReadWait;                                                   //0x2d48
    unsigned long CcFastReadNotPossible;                                            //0x2d4c
    unsigned long CcCopyReadNoWait;                                                 //0x2d50
    unsigned long CcCopyReadWait;                                                   //0x2d54
    unsigned long CcCopyReadNoWaitMiss;                                             //0x2d58
    volatile long IoReadOperationCount;                                     //0x2d5c
    volatile long IoWriteOperationCount;                                    //0x2d60
    volatile long IoOtherOperationCount;                                    //0x2d64
    union _LARGE_INTEGER IoReadTransferCount;                               //0x2d68
    union _LARGE_INTEGER IoWriteTransferCount;                              //0x2d70
    union _LARGE_INTEGER IoOtherTransferCount;                              //0x2d78
    volatile long PacketBarrier;                                            //0x2d80
    volatile long TargetCount;                                              //0x2d84
    volatile unsigned long IpiFrozen;                                               //0x2d88
    unsigned long PrcbPad30;                                                        //0x2d8c
    void* IsrDpcStats;                                                      //0x2d90
    unsigned long DeviceInterrupts;                                                 //0x2d98
    long LookasideIrpFloat;                                                 //0x2d9c
    unsigned long InterruptLastCount;                                               //0x2da0
    unsigned long InterruptRate;                                                    //0x2da4
    unsigned __int64 LastNonHrTimerExpiration;                                     //0x2da8
    struct _KPRCB* PairPrcb;                                                //0x2db0
    unsigned __int64 PrcbPad35[1];                                                 //0x2db8
    union _SLIST_HEADER InterruptObjectPool;                                //0x2dc0
    unsigned __int64 PrcbPad41[6];                                                 //0x2dd0
    KDPC_DATA DpcData[2];                                           //0x2e00
    void* DpcStack;                                                         //0x2e50
    long MaximumDpcQueueDepth;                                              //0x2e58
    unsigned long DpcRequestRate;                                                   //0x2e5c
    unsigned long MinimumDpcRate;                                                   //0x2e60
    unsigned long DpcLastCount;                                                     //0x2e64
    unsigned __int8 ThreadDpcEnable;                                                  //0x2e68
    volatile unsigned __int8 QuantumEnd;                                              //0x2e69
    volatile unsigned __int8 DpcRoutineActive;                                        //0x2e6a
    volatile unsigned __int8 IdleSchedule;                                            //0x2e6b
    union
    {
        volatile long DpcRequestSummary;                                    //0x2e6c
        SHORT DpcRequestSlot[2];                                            //0x2e6c
        struct
        {
            SHORT NormalDpcState;                                           //0x2e6c
            SHORT ThreadDpcState;                                           //0x2e6e
        };
        struct
        {
            unsigned long DpcNormalProcessingActive : 1;                              //0x2e6c
            unsigned long DpcNormalProcessingRequested : 1;                           //0x2e6c
            unsigned long DpcNormalThreadSignal : 1;                                  //0x2e6c
            unsigned long DpcNormalTimerExpiration : 1;                               //0x2e6c
            unsigned long DpcNormalDpcPresent : 1;                                    //0x2e6c
            unsigned long DpcNormalLocalInterrupt : 1;                                //0x2e6c
            unsigned long DpcNormalSpare : 10;                                        //0x2e6c
            unsigned long DpcThreadActive : 1;                                        //0x2e6c
            unsigned long DpcThreadRequested : 1;                                     //0x2e6c
            unsigned long DpcThreadSpare : 14;                                        //0x2e6c
        };
    };
    unsigned long LastTimerHand;                                                    //0x2e70
    unsigned long LastTick;                                                         //0x2e74
    unsigned long ClockInterrupts;                                                  //0x2e78
    unsigned long ReadyScanTick;                                                    //0x2e7c
    void* InterruptObject[256];                                             //0x2e80
    unsigned char TimerTable[0x2200];                                        //0x3680
    struct _KGATE DpcGate;                                                  //0x5880
    void* PrcbPad52;                                                        //0x5898
    struct _KDPC CallDpc;                                                   //0x58a0
    long ClockKeepAlive;                                                    //0x58e0
    unsigned __int8 PrcbPad60[2];                                                     //0x58e4
    unsigned __int8 NmiActive;                                                       //0x58e6
    long DpcWatchdogPeriod;                                                 //0x58e8
    long DpcWatchdogCount;                                                  //0x58ec
    volatile long KeSpinLockOrdering;                                       //0x58f0
    unsigned long DpcWatchdogProfileCumulativeDpcThreshold;                         //0x58f4
    void* CachedPtes;                                                       //0x58f8
    struct _LIST_ENTRY WaitListHead;                                        //0x5900
    unsigned __int64 WaitLock;                                                     //0x5910
    unsigned long ReadySummary;                                                     //0x5918
    long AffinitizedSelectionMask;                                          //0x591c
    unsigned long QueueIndex;                                                       //0x5920
    unsigned long PrcbPad75[3];                                                     //0x5924
    struct _KDPC TimerExpirationDpc;                                        //0x5930
    unsigned char ScbQueue[0x10];                                           //0x5970
    struct _LIST_ENTRY DispatcherReadyListHead[32];                         //0x5980
    unsigned long InterruptCount;                                                   //0x5b80
    unsigned long KernelTime;                                                       //0x5b84
    unsigned long UserTime;                                                         //0x5b88
    unsigned long DpcTime;                                                          //0x5b8c
    unsigned long InterruptTime;                                                    //0x5b90
    unsigned long AdjustDpcThreshold;                                               //0x5b94
    unsigned __int8 DebuggerSavedIRQL;                                                //0x5b98
    unsigned __int8 GroupSchedulingOverQuota;                                         //0x5b99
    volatile unsigned __int8 DeepSleep;                                               //0x5b9a
    unsigned __int8 PrcbPad80;                                                        //0x5b9b
    unsigned long DpcTimeCount;                                                     //0x5b9c
    unsigned long DpcTimeLimit;                                                     //0x5ba0
    unsigned long PeriodicCount;                                                    //0x5ba4
    unsigned long PeriodicBias;                                                     //0x5ba8
    unsigned long AvailableTime;                                                    //0x5bac
    unsigned long KeExceptionDispatchCount;                                         //0x5bb0
    unsigned long ReadyThreadCount;                                                 //0x5bb4
    unsigned __int64 ReadyQueueExpectedRunTime;                                    //0x5bb8
    unsigned __int64 StartCycles;                                                  //0x5bc0
    unsigned __int64 TaggedCyclesStart;                                            //0x5bc8
    unsigned __int64 TaggedCycles[2];                                              //0x5bd0
    unsigned __int64 GenerationTarget;                                             //0x5be0
    unsigned __int64 AffinitizedCycles;                                            //0x5be8
    unsigned __int64 ImportantCycles;                                              //0x5bf0
    unsigned __int64 UnimportantCycles;                                            //0x5bf8
    unsigned long DpcWatchdogProfileSingleDpcThreshold;                             //0x5c00
    volatile long MmSpinLockOrdering;                                       //0x5c04
    void* volatile CachedStack;                                             //0x5c08
    unsigned long PageColor;                                                        //0x5c10
    unsigned long NodeColor;                                                        //0x5c14
    unsigned long NodeShiftedColor;                                                 //0x5c18
    unsigned long SecondaryColorMask;                                               //0x5c1c
    unsigned __int8 PrcbPad81[7];                                                     //0x5c20
    unsigned __int8 TbFlushListActive;                                                //0x5c27
    unsigned __int64 PrcbPad82[2];                                                 //0x5c28
    unsigned __int64 CycleTime;                                                    //0x5c38
    unsigned __int64 Cycles[4][2];                                                 //0x5c40
    unsigned long CcFastMdlReadNoWait;                                              //0x5c80
    unsigned long CcFastMdlReadWait;                                                //0x5c84
    unsigned long CcFastMdlReadNotPossible;                                         //0x5c88
    unsigned long CcMapDataNoWait;                                                  //0x5c8c
    unsigned long CcMapDataWait;                                                    //0x5c90
    unsigned long CcPinMappedDataCount;                                             //0x5c94
    unsigned long CcPinReadNoWait;                                                  //0x5c98
    unsigned long CcPinReadWait;                                                    //0x5c9c
    unsigned long CcMdlReadNoWait;                                                  //0x5ca0
    unsigned long CcMdlReadWait;                                                    //0x5ca4
    unsigned long CcLazyWriteHotSpots;                                              //0x5ca8
    unsigned long CcLazyWriteIos;                                                   //0x5cac
    unsigned long CcLazyWritePages;                                                 //0x5cb0
    unsigned long CcDataFlushes;                                                    //0x5cb4
    unsigned long CcDataPages;                                                      //0x5cb8
    unsigned long CcLostDelayedWrites;                                              //0x5cbc
    unsigned long CcFastReadResourceMiss;                                           //0x5cc0
    unsigned long CcCopyReadWaitMiss;                                               //0x5cc4
    unsigned long CcFastMdlReadResourceMiss;                                        //0x5cc8
    unsigned long CcMapDataNoWaitMiss;                                              //0x5ccc
    unsigned long CcMapDataWaitMiss;                                                //0x5cd0
    unsigned long CcPinReadNoWaitMiss;                                              //0x5cd4
    unsigned long CcPinReadWaitMiss;                                                //0x5cd8
    unsigned long CcMdlReadNoWaitMiss;                                              //0x5cdc
    unsigned long CcMdlReadWaitMiss;                                                //0x5ce0
    unsigned long CcReadAheadIos;                                                   //0x5ce4
    volatile long MmCacheTransitionCount;                                   //0x5ce8
    volatile long MmCacheReadCount;                                         //0x5cec
    volatile long MmCacheIoCount;                                           //0x5cf0
    unsigned long PrcbPad91;                                                        //0x5cf4
    void* MmInternal;                                                       //0x5cf8
    unsigned char PowerState[0x200];                               //0x5d00
    void* HyperPte;                                                         //0x5f00
    struct _LIST_ENTRY ScbList;                                             //0x5f08
    struct _KDPC ForceIdleDpc;                                              //0x5f18
    struct _KDPC DpcWatchdogDpc;                                            //0x5f58
    struct _KTIMER DpcWatchdogTimer;                                        //0x5f98
    struct _CACHE_DESCRIPTOR Cache[5];                                      //0x5fd8
    unsigned long CacheCount;                                                       //0x6014
    volatile unsigned long CachedCommit;                                            //0x6018
    volatile unsigned long CachedResidentAvailable;                                 //0x601c
    void* WheaInfo;                                                         //0x6020
    void* EtwSupport;                                                       //0x6028
    void* ExSaPageArray;                                                    //0x6030
    unsigned long KeAlignmentFixupCount;                                            //0x6038
    unsigned long PrcbPad95;                                                        //0x603c
    union _SLIST_HEADER HypercallPageList;                                  //0x6040
    unsigned __int64* StatisticsPage;                                              //0x6050
    unsigned __int64 PrcbPad85[5];                                                 //0x6058
    void* HypercallCachedPages;                                             //0x6080
    void* VirtualApicAssist;                                                //0x6088
    unsigned char PackageProcessorSet[0xa8];                               //0x6090
    unsigned __int64 PrcbPad86;                                                    //0x6138
    unsigned __int64 SharedReadyQueueMask;                                         //0x6140
    struct _KSHARED_READY_QUEUE* SharedReadyQueue;                          //0x6148
    unsigned long SharedQueueScanOwner;                                             //0x6150
    unsigned long ScanSiblingIndex;                                                 //0x6154
    unsigned __int64 CoreProcessorSet;                                             //0x6158
    unsigned __int64 ScanSiblingMask;                                              //0x6160
    unsigned __int64 LLCMask;                                                      //0x6168
    unsigned __int64 CacheProcessorMask[5];                                        //0x6170
    struct _PROCESSOR_PROFILE_CONTROL_AREA* ProcessorProfileControlArea;    //0x6198
    void* ProfileEventIndexAddress;                                         //0x61a0
    void** DpcWatchdogProfile;                                              //0x61a8
    void** DpcWatchdogProfileCurrentEmptyCapture;                           //0x61b0
    void* SchedulerAssist;                                                  //0x61b8
    unsigned char SynchCounters[0xb8];                                   //0x61c0
    unsigned __int64 PrcbPad94;                                                    //0x6278
    unsigned char FsCounters[0x10];                            //0x6280
    unsigned __int8 VendorString[13];                                                 //0x6290
    unsigned __int8 PrcbPad100[3];                                                    //0x629d
    unsigned __int64 FeatureBits;                                                  //0x62a0
    union _LARGE_INTEGER UpdateSignature;                                   //0x62a8
    unsigned __int64 PteBitCache;                                                  //0x62b0
    unsigned long PteBitOffset;                                                     //0x62b8
    unsigned long PrcbPad105;                                                       //0x62bc
    struct _CONTEXT* Context;                                               //0x62c0
    unsigned long ContextFlagsInit;                                                 //0x62c8
    unsigned long PrcbPad115;                                                       //0x62cc
    struct _XSAVE_AREA* ExtendedState;                                      //0x62d0
    void* IsrStack;                                                         //0x62d8
    unsigned char EntropyTimingState[0x150];                       //0x62e0
    unsigned __int64 PrcbPad110;                                                   //0x6430
    struct
    {
        unsigned long UpdateCycle;                                                  //0x6438
        union
        {
            SHORT PairLocal;                                                //0x643c
            struct
            {
                unsigned __int8 PairLocalLow;                                         //0x643c
                unsigned __int8 PairLocalForceStibp : 1;                                //0x643d
                unsigned __int8 Reserved : 4;                                           //0x643d
                unsigned __int8 Frozen : 1;                                             //0x643d
                unsigned __int8 ForceUntrusted : 1;                                     //0x643d
                unsigned __int8 SynchIpi : 1;                                           //0x643d
            };
        };
        union
        {
            SHORT PairRemote;                                               //0x643e
            struct
            {
                unsigned __int8 PairRemoteLow;                                        //0x643e
                unsigned __int8 Reserved2;                                            //0x643f
            };
        };
        unsigned __int8 Trace[24];                                                    //0x6440
        unsigned __int64 LocalDomain;                                              //0x6458
        unsigned __int64 RemoteDomain;                                             //0x6460
        struct _KTHREAD* Thread;                                            //0x6468
    } StibpPairingTrace;                                                    //0x6438
    struct _SINGLE_LIST_ENTRY AbSelfIoBoostsList;                           //0x6470
    struct _SINGLE_LIST_ENTRY AbPropagateBoostsList;                        //0x6478
    struct _KDPC AbDpc;                                                     //0x6480
    unsigned char IoIrpStackProfilerCurrent[0x54];               //0x64c0
    unsigned char IoIrpStackProfilerPrevious[0x54];              //0x6514
    unsigned char SecureFault[0x10];                          //0x6568
    unsigned __int64 PrcbPad120;                                                   //0x6578
    unsigned char LocalSharedReadyQueue[0x270];                      //0x6580
    unsigned __int64 PrcbPad125[2];                                                //0x67f0
    unsigned long TimerExpirationTraceCount;                                        //0x6800
    unsigned long PrcbPad127;                                                       //0x6804
    unsigned char TimerExpirationTrace[0x100];               //0x6808
    unsigned __int64 PrcbPad128[7];                                                //0x6908
    struct _REQUEST_MAILBOX* Mailbox;                                       //0x6940
    unsigned __int64 PrcbPad130[7];                                                //0x6948
    unsigned char McheckContext[0x50 * 2];                         //0x6980
    unsigned __int64 PrcbPad134[4];                                                //0x6a20
    struct _KLOCK_QUEUE_HANDLE SelfmapLockHandle[4];                        //0x6a40
    unsigned __int64 PrcbPad134a[4];                                               //0x6aa0
    unsigned __int8 PrcbPad138[960];                                                  //0x6ac0
    unsigned __int64 KernelDirectoryTableBase;                                     //0x6e80
    unsigned __int64 RspBaseShadow;                                                //0x6e88
    unsigned __int64 UserRspShadow;                                                //0x6e90
    unsigned long ShadowFlags;                                                      //0x6e98
    unsigned long DbgMceNestingLevel;                                               //0x6e9c
    unsigned long DbgMceFlags;                                                      //0x6ea0
    unsigned long PrcbPad139;                                                       //0x6ea4
    unsigned __int64 PrcbPad140[507];                                              //0x6ea8
    unsigned char RequestMailbox[0x40];                              //0x7e80
} KPRCB;

__forceinline KPRCB* KeGetCurrentPrcb() {
    return (KPRCB*)__readgsqword(FIELD_OFFSET(KPCR, CurrentPrcb));
}
