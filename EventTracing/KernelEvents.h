#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <evntrace.h>

enum class KernelEventTypes : uint32_t {
	None = 0,

	Process = EVENT_TRACE_FLAG_PROCESS,
	Thread = EVENT_TRACE_FLAG_THREAD,
	ImageLoad = EVENT_TRACE_FLAG_IMAGE_LOAD,
	Registry = EVENT_TRACE_FLAG_REGISTRY,
	DiskIO = EVENT_TRACE_FLAG_DISK_IO,
	DiskFileIO = DiskIO | EVENT_TRACE_FLAG_DISK_FILE_IO,
	PageFaults = EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS,
	HardPageFaults = EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS,
	Network = EVENT_TRACE_FLAG_NETWORK_TCPIP,
	DebugPrint = EVENT_TRACE_FLAG_DBGPRINT,

	ProcessCounters = EVENT_TRACE_FLAG_PROCESS_COUNTERS,
	ContextSwitche = EVENT_TRACE_FLAG_CSWITCH,
	DPC = EVENT_TRACE_FLAG_DPC,
	Interrupt = EVENT_TRACE_FLAG_INTERRUPT,
	SystemCall = EVENT_TRACE_FLAG_SYSTEMCALL,
	DiskIoInit = EVENT_TRACE_FLAG_DISK_IO_INIT,
	ALPC = EVENT_TRACE_FLAG_ALPC,
	SplitIO = EVENT_TRACE_FLAG_SPLIT_IO,
	Driver = EVENT_TRACE_FLAG_DRIVER,
	Profile = EVENT_TRACE_FLAG_PROFILE,
	FileIO = EVENT_TRACE_FLAG_FILE_IO,
	FileIOInit = EVENT_TRACE_FLAG_FILE_IO_INIT,

	Dispatcher = EVENT_TRACE_FLAG_DISPATCHER,
	VirtualAlloc = EVENT_TRACE_FLAG_VIRTUAL_ALLOC,

	VAMap = EVENT_TRACE_FLAG_VAMAP,
	NoSysConfig = EVENT_TRACE_FLAG_NO_SYSCONFIG,

	Job = EVENT_TRACE_FLAG_JOB,
	Debug = EVENT_TRACE_FLAG_DEBUG_EVENTS,
};
DEFINE_ENUM_FLAG_OPERATORS(KernelEventTypes);

DEFINE_GUID( /* 3d6fa8d0-fe05-11d0-9dda-00c04fd7ba7c */
	ProcessGuid,
	0x3d6fa8d0,
	0xfe05,
	0x11d0,
	0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c
);

DEFINE_GUID( /* 45d8cccd-539f-4b72-a8b7-5c683142609a */
	ALPCGuid,
	0x45d8cccd,
	0x539f,
	0x4b72,
	0xa8, 0xb7, 0x5c, 0x68, 0x31, 0x42, 0x60, 0x9a
);

DEFINE_GUID( /* 3d6fa8d4-fe05-11d0-9dda-00c04fd7ba7c */
	DiskIoGuid,
	0x3d6fa8d4,
	0xfe05,
	0x11d0,
	0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c
);

DEFINE_GUID( /* 90cbdc39-4a3e-11d1-84f4-0000f80464e3 */
	FileIoGuid,
	0x90cbdc39,
	0x4a3e,
	0x11d1,
	0x84, 0xf4, 0x00, 0x00, 0xf8, 0x04, 0x64, 0xe3
);

DEFINE_GUID( /* 2cb15d1d-5fc1-11d2-abe1-00a0c911f518 */
	ImageLoadGuid,
	0x2cb15d1d,
	0x5fc1,
	0x11d2,
	0xab, 0xe1, 0x00, 0xa0, 0xc9, 0x11, 0xf5, 0x18
);

DEFINE_GUID( /* 3d6fa8d3-fe05-11d0-9dda-00c04fd7ba7c */
	PageFaultGuid,
	0x3d6fa8d3,
	0xfe05,
	0x11d0,
	0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c
);

DEFINE_GUID( /* ce1dbfb4-137e-4da6-87b0-3f59aa102cbc */
	PerfInfoGuid,
	0xce1dbfb4,
	0x137e,
	0x4da6,
	0x87, 0xb0, 0x3f, 0x59, 0xaa, 0x10, 0x2c, 0xbc
);

DEFINE_GUID( /* AE53722E-C863-11d2-8659-00C04FA321A1 */
	RegistryGuid,
	0xae53722e,
	0xc863,
	0x11d2,
	0x86, 0x59, 0x0, 0xc0, 0x4f, 0xa3, 0x21, 0xa1
);

DEFINE_GUID( /* 9a280ac0-c8e0-11d1-84e2-00c04fb998a2 */
	TcpIpGuid,
	0x9a280ac0,
	0xc8e0,
	0x11d1,
	0x84, 0xe2, 0x00, 0xc0, 0x4f, 0xb9, 0x98, 0xa2
);

DEFINE_GUID( /* 3d6fa8d1-fe05-11d0-9dda-00c04fd7ba7c */
	ThreadGuid,
	0x3d6fa8d1,
	0xfe05,
	0x11d0,
	0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c
);

DEFINE_GUID( /* bf3a50c5-a9c9-4988-a005-2df0b7c80f80 */
	UdpIpGuid,
	0xbf3a50c5,
	0xa9c9,
	0x4988,
	0xa0, 0x05, 0x2d, 0xf0, 0xb7, 0xc8, 0x0f, 0x80
);



struct KernelEvent {
	std::wstring Name;
	uint32_t Opcode;
};

struct KernelEventCategory {
	std::wstring Name;
	KernelEventTypes EnableFlag;
	const GUID* Guid;
	std::vector<KernelEvent> Events;

	static const std::vector<KernelEventCategory>& GetAllCategories(KernelEventTypes type);
};

