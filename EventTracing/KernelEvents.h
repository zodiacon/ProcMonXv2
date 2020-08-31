#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <evntrace.h>

enum class KernelEventTypes : uint64_t {
	None = 0,

	Process =			EVENT_TRACE_FLAG_PROCESS,
	Thread =			EVENT_TRACE_FLAG_THREAD,
	ImageLoad =			EVENT_TRACE_FLAG_IMAGE_LOAD,
	Registry =			EVENT_TRACE_FLAG_REGISTRY,
	DiskIO =			EVENT_TRACE_FLAG_DISK_IO,
	DiskFileIO =		DiskIO | EVENT_TRACE_FLAG_DISK_FILE_IO,
	PageFaults =		EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS,
	HardPageFaults =	EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS,
	Network =			EVENT_TRACE_FLAG_NETWORK_TCPIP,
	DebugPrint =		EVENT_TRACE_FLAG_DBGPRINT,

	ProcessCounters =	EVENT_TRACE_FLAG_PROCESS_COUNTERS,
	ContextSwitch =		EVENT_TRACE_FLAG_CSWITCH,
	DPC =				EVENT_TRACE_FLAG_DPC,
	Interrupt =			EVENT_TRACE_FLAG_INTERRUPT,
	SystemCall =		EVENT_TRACE_FLAG_SYSTEMCALL,
	DiskIoInit =		EVENT_TRACE_FLAG_DISK_IO_INIT,
	ALPC =				EVENT_TRACE_FLAG_ALPC,
	SplitIO =			EVENT_TRACE_FLAG_SPLIT_IO,
	Driver =			EVENT_TRACE_FLAG_DRIVER,
	Profile =			EVENT_TRACE_FLAG_PROFILE,
	FileIO =			EVENT_TRACE_FLAG_FILE_IO | EVENT_TRACE_FLAG_FILE_IO_INIT,
	FileIOInit =		EVENT_TRACE_FLAG_FILE_IO_INIT,

	Dispatcher =		EVENT_TRACE_FLAG_DISPATCHER,
	VirtualAlloc =		EVENT_TRACE_FLAG_VIRTUAL_ALLOC,
	VAMap =				EVENT_TRACE_FLAG_VAMAP,
	VirtualMemory =		VirtualAlloc | VAMap,
	NoSysConfig =		EVENT_TRACE_FLAG_NO_SYSCONFIG,

	Job =				EVENT_TRACE_FLAG_JOB,
	Debug =				EVENT_TRACE_FLAG_DEBUG_EVENTS,

	// Mask[1]
	PerfMemory =			0x20000001 | (1LL << 32),
	PerfProfile =			0x20000002 | (1LL << 32),
	PerfContextSwitch =		0x20000004 | (1LL << 32),
	PerfDrivers =			0x20000010 | (1LL << 32),
	PerfPool =				0x20000040 | (1LL << 32),
	PerfSyncObjects =		0x20020000 | (1LL << 32),
	PerfVirtualAlloc =		0x20008000 | (1LL << 32),
	PerfSession =			0x20400000 | (1LL << 32),
	PerfMemInfo =			0x20080000 | (1LL << 32),

	// Mask[2]
	PerfHeap =			0x40000020 | (2LL << 32),
	PerfSysCalls =		0x40000040 | (2LL << 32),
	WorkerThread =		0x48000000 | (2LL << 32),
	ProcessFreeze =		0x40000002 | (2LL << 32),
	PerfEvents =		0x40000800 | (2LL << 32),
	PerfWSDetail =		0x40000008 | (2LL << 32),
	PerfTimer =			0x40020000 | (2LL << 32),
	PerfIPI =			0x40400000 | (2LL << 32),
	PerfClockIntr =		0x40040000 | (2LL << 32),
	Mask2All =			0x4fffffff | (2LL << 32),

	// Mask[4]
	PerfHandles =		0x80000040 | (4LL << 32),
	PerfObjects =		0x80000080 | (4LL << 32),
	PerfDebugger =		0x80000800 | (4LL << 32),
	PerfPower =			0x80008000 | (4LL << 32),
	PerfDllInfo =		0x80000008 | (4LL << 32),
	PerfFltIoInit =		0x80080000 | (4LL << 32),
	PerfFltIO =			0x80100000 | (4LL << 32),
	PerfFltFastIO =		0x80200000 | (4LL << 32),
	PerfFltFail =		0x80400000 | (4LL << 32),
	PerfFlt =			PerfFltIO | PerfFltIoInit | PerfFltFastIO | PerfFltFail,
	PerfHvProfile =		0x80800000 | (4LL << 32),
	Mask4All =			0x8fffffff | (2LL << 32),

	// Mask[6]
	PerfConfigSystem =	0xC0000001 | (6LL << 32),
	PerfConfigGraphics = 0xC0000002 | (6LL << 32),
};
DEFINE_ENUM_FLAG_OPERATORS(KernelEventTypes);

DEFINE_GUID( /* 54849625-5478-4994-a5ba-3e3b0328c30d */
	SecurityProviderGuid,
	0x54849625,
	0x5478,
	0x4994,
	0xa5, 0xba, 0x3e, 0x3b, 0x03, 0x28, 0xc3, 0x0d
);

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

DEFINE_GUID( /* DEF2FE46-7BD6-4b80-bd94-F57FE20D0CE3 */
	StackWalkGuid,
	0xdef2fe46,
	0x7bd6,
	0x4b80,
	0xbd, 0x94, 0xf5, 0x7f, 0xe2, 0xd, 0xc, 0xe3
);

DEFINE_GUID( /* 89497f50-effe-4440-8cf2-ce6b1cdcaca7 */
	ObjectGuid,
	0x89497f50,
	0xeffe,
	0x4440,
	0x8c, 0xf2, 0xce, 0x6b, 0x1c, 0xdc, 0xac, 0xa7
);

DEFINE_GUID( /* E43445E0-0903-48c3-B878-FF0FCCEBDD04 */
	PowerGuid,
	0xe43445e0,
	0x903,
	0x48c3,
	0xb8, 0x78, 0xff, 0xf, 0xcc, 0xeb, 0xdd, 0x4
);

DEFINE_GUID( /* F8F10121-B617-4A56-868B-9dF1B27FE32C */
	MmcssGuid,
	0xf8f10121,
	0xb617,
	0x4a56,
	0x86, 0x8b, 0x9d, 0xf1, 0xb2, 0x7f, 0xe3, 0x2c
);

DEFINE_GUID( /* b2d14872-7c5b-463d-8419-ee9bf7d23e04 */
	DpcGuid,
	0xb2d14872,
	0x7c5b,
	0x463d,
	0x84, 0x19, 0xee, 0x9b, 0xf7, 0xd2, 0x3e, 0x04
);

DEFINE_GUID( /* d837ca92-12b9-44a5-ad6a-3a65b3578aa8 */
	SplitIoGuid,
	0xd837ca92,
	0x12b9,
	0x44a5,
	0xad, 0x6a, 0x3a, 0x65, 0xb3, 0x57, 0x8a, 0xa8
);

DEFINE_GUID( /* c861d0e2-a2c1-4d36-9f9c-970bab943a12 */
	ThreadPoolGuid,
	0xc861d0e2,
	0xa2c1,
	0x4d36,
	0x9f, 0x9c, 0x97, 0x0b, 0xab, 0x94, 0x3a, 0x12
);

DEFINE_GUID( /* 0268a8b6-74fd-4302-9dd0-6e8f1795c0cf */
	PoolGuid,
	0x0268a8b6,
	0x74fd,
	0x4302,
	0x9d, 0xd0, 0x6e, 0x8f, 0x17, 0x95, 0xc0, 0xcf
);

DEFINE_GUID( /* 222962ab-6180-4b88-a825-346b75f2a24a */
	HeapGuid,
	0x222962ab,
	0x6180,
	0x4b88,
	0xa8, 0x25, 0x34, 0x6b, 0x75, 0xf2, 0xa2, 0x4a
);

DEFINE_GUID( /* d781ca11-61c0-4387-b83d-af52d3d2dd6a */
	HeapRangeGuid,
	0xd781ca11,
	0x61c0,
	0x4387,
	0xb8, 0x3d, 0xaf, 0x52, 0xd3, 0xd2, 0xdd, 0x6a
);

DEFINE_GUID( /* 05867806-c246-43ef-a147-e17d2bdb1496 */
	HeapSummaryGuid,
	0x05867806,
	0xc246,
	0x43ef,
	0xa1, 0x47, 0xe1, 0x7d, 0x2b, 0xdb, 0x14, 0x96
);

DEFINE_GUID( /* 3AC66736-CC59-4cff-8115-8DF50E39816B */
	CritSecGuid,
	0x3ac66736,
	0xcc59,
	0x4cff,
	0x81, 0x15, 0x8d, 0xf5, 0xe, 0x39, 0x81, 0x6b
);

DEFINE_GUID( /* 13976D09-A327-438c-950B-7F03192815C7  */
	DbgPrintGuid,
	0x13976d09,
	0xa327,
	0x438c,
	0x95, 0xb, 0x7f, 0x3, 0x19, 0x28, 0x15, 0xc7
);

DEFINE_GUID( /* D56CA431-61BF-4904-A621-00E0381E4DDE */
	DriverVerifierGuid,
	0xd56ca431,
	0x61bf,
	0x4904,
	0xa6, 0x21, 0x0, 0xe0, 0x38, 0x1e, 0x4d, 0xde
);


DEFINE_GUID( /* E21D2142-DF90-4d93-BBD9-30E63D5A4AD6 */
	NtdllTraceGuid,
	0xe21d2142,
	0xdf90,
	0x4d93,
	0xbb, 0xd9, 0x30, 0xe6, 0x3d, 0x5a, 0x4a, 0xd6
);

DEFINE_GUID( /* d3de60b2-a663-45d5-9826-a0a5949d2cb0 */
	LoadMUIDllGuid,
	0xd3de60b2,
	0xa663,
	0x45d5,
	0x98, 0x26, 0xa0, 0xa5, 0x94, 0x9d, 0x2c, 0xb0
);

DEFINE_GUID( /* 3282fc76-feed-498e-8aa7-e70f459d430e */
	JobGuid,
	0x3282fc76,
	0xfeed,
	0x498e,
	0x8a, 0xa7, 0xe7, 0x0f, 0x45, 0x9d, 0x43, 0x0e
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
	bool Advanced{ false };

	static const std::vector<KernelEventCategory>& GetAllCategories();
	static const KernelEventCategory* GetCategory(PCWSTR name);
};

