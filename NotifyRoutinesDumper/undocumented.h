#pragma once
#include <ntddk.h>

// this is very build dependent
// I'm too lazy to make it generic for this exercise
#define THREAD_NOTIFY_ROUTINE_ARR_OFFSET 0x4da7f0
#define PROCESS_NOTIFY_ROUTINE_ARR_OFFSET 0x4dabf0
#define IMAGE_LOAD_NOTIFY_ROUTINE_ARR_OFFSET 0x4da9f0

#define NOTIFY_ROUTINE_ARR_SIZE 0x40

typedef struct {
	EX_RUNDOWN_REF RundownProtection;
	void* CallbackRoutine;
	__int64 Flags;
} CallbackBlock;

void* GetNtoskrnlBase();
