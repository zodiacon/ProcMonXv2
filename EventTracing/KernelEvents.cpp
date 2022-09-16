#include "pch.h"
#include "KernelEvents.h"
#include <winternl.h>

const std::vector<KernelEventCategory> KernelEvents {
	{ L"Process", KernelEventTypes::Process, &ProcessGuid,
		{
			{ L"Start", 1 },
			{ L"End", 2 },
			{ L"Terminate", 11 },
			{ L"DC Start", 3 },
			{ L"DC Stop", 4 },
			{ L"Defunct", 39 },
			{ L"Perf Counter", 32 },
			{ L"Perf Counter Rundown", 33 },
		}
	},

	{ L"ALPC", KernelEventTypes::ALPC, &ALPCGuid,
		{
			{ L"Send Message", 0x21 },
			{ L"Receive Message", 0x22 },
			{ L"Wait For Reply", 0x23 },
			{ L"Wait For New Message", 0x24 },
			{ L"Unwait", 0x25 },
			{ L"Connect Request", 0x26 },
			{ L"Connect Success", 0x27 },
			{ L"Connect Fail", 0x28 },
			{ L"Close Port", 0x29 },
		}, true
	},

	{ L"Thread", KernelEventTypes::Thread, &ThreadGuid,
		{
			{ L"Start", 1 },
			{ L"End", 2 },
			{ L"Set Name", 72 },
		}
	},

	{ L"Image", KernelEventTypes::ImageLoad, &ImageLoadGuid,
		{
			{ L"Load", 10 },
			{ L"Unload", 2 },
			//{ L"DC Start", 3 },
			//{ L"DC Stop", 4 },
			{ L"Relocation", 0x20 },
			{ L"Kernel Base", 0x21 },
			{ L"Hypercall Page", 0x22 },
		}
	},

	{ L"File", KernelEventTypes::FileIO, &FileIoGuid,
		{
			{ L"Name", 0 },
			{ L"File Create", 32 },
			{ L"File Delete", 35 },
			{ L"File Rundown", 36 },
			{ L"Create", 64 },
			{ L"Dir Enum", 72 },
			{ L"Dir Notify", 77 },
			{ L"Set Info", 69 },
			{ L"Delete", 70 },
			{ L"Rename", 71 },
			{ L"Query Info", 74 },
			{ L"FS Control", 75 },
			{ L"Operation End", 76 },
			{ L"Read", 67 },
			{ L"Write", 68 },
			{ L"Cleanup", 65 },
			{ L"Close", 66 },
			{ L"Flush", 73 },
		}
	},

	{ L"Registry", KernelEventTypes::Registry, &RegistryGuid,
		{
			{ L"Create Key", EVENT_TRACE_TYPE_REGCREATE },
			{ L"Open Key", EVENT_TRACE_TYPE_REGOPEN },
			{ L"Delete Key", EVENT_TRACE_TYPE_REGDELETE },
			{ L"Query Key", EVENT_TRACE_TYPE_REGQUERY },
			{ L"Set Value", EVENT_TRACE_TYPE_REGSETVALUE },
			{ L"Delete Value", EVENT_TRACE_TYPE_REGDELETEVALUE },
			{ L"Query Value", EVENT_TRACE_TYPE_REGQUERYVALUE },
			{ L"Enum Key", EVENT_TRACE_TYPE_REGENUMERATEKEY },
			{ L"Enum Value", EVENT_TRACE_TYPE_REGENUMERATEVALUEKEY },
			{ L"Query Multiple Values", EVENT_TRACE_TYPE_REGQUERYMULTIPLEVALUE },
			{ L"Set Key Information", EVENT_TRACE_TYPE_REGSETINFORMATION },
			{ L"Flush Key", EVENT_TRACE_TYPE_REGFLUSH },
			{ L"KCB Create", EVENT_TRACE_TYPE_REGKCBCREATE },
			{ L"KCB Delete", EVENT_TRACE_TYPE_REGKCBDELETE },
			{ L"KCB Rundown Begin", EVENT_TRACE_TYPE_REGKCBRUNDOWNBEGIN },
			{ L"KCB Rundown End", EVENT_TRACE_TYPE_REGKCBRUNDOWNEND },
			{ L"Virtualize Key", EVENT_TRACE_TYPE_REGVIRTUALIZE },
			{ L"Close Key", EVENT_TRACE_TYPE_REGCLOSE },
			{ L"Set Security Descriptor", EVENT_TRACE_TYPE_REGSETSECURITY },
			{ L"Query Security Descriptor", EVENT_TRACE_TYPE_REGQUERYSECURITY },
			{ L"Commit Tx", EVENT_TRACE_TYPE_REGCOMMIT },
			{ L"Prepare Tx", EVENT_TRACE_TYPE_REGPREPARE },
			{ L"Rollback Tx", EVENT_TRACE_TYPE_REGROLLBACK },
			{ L"Load Key", EVENT_TRACE_TYPE_REGMOUNTHIVE },
		}
	},

	{ L"TCP", KernelEventTypes::Network, &TcpIpGuid,
		{
			{ L"TCP Send IPv4", EVENT_TRACE_TYPE_SEND },
			{ L"TCP Receive IPv4", EVENT_TRACE_TYPE_RECEIVE },
			{ L"TCP Connect IPv4", EVENT_TRACE_TYPE_CONNECT },
			{ L"TCP Disconnect IPv4", EVENT_TRACE_TYPE_DISCONNECT },
			{ L"TCP Retransmit IPv4", EVENT_TRACE_TYPE_RETRANSMIT },
			{ L"TCP Accept IPv4", EVENT_TRACE_TYPE_ACCEPT },
			{ L"TCP Reconnect IPv4", EVENT_TRACE_TYPE_RECONNECT },
			{ L"TCP Fail", EVENT_TRACE_TYPE_CONNFAIL },
			{ L"TCP Copy IPv4", 18 },
			{ L"TCP Send IPv6", 26 },
			{ L"TCP Receive IPv6", 27 },
			{ L"TCP Disconnect IPv6", 29 },
			{ L"TCP Retransmit IPv6", 30 },
			{ L"TCP Reconnect IPv6", 32 },
			{ L"TCP Copy IPv6", 34 },
			{ L"TCP Connect IPv6", 28 },
			{ L"TCP Accept IPv6", 31 },
			{ L"Copy ARP", EVENT_TRACE_TYPE_COPY_ARP },
			{ L"Full Ack", EVENT_TRACE_TYPE_ACKFULL },
			{ L"Partial Ack", EVENT_TRACE_TYPE_ACKPART },
			{ L"Duplicate Ack", EVENT_TRACE_TYPE_ACKDUP },
		}
	},

	{ L"UDP", KernelEventTypes::Network, &UdpIpGuid,
		{
			{ L"UDP Send IPv4", 10 },
			{ L"UDP Receive IPv4", 11 },
			{ L"UDP Send IPv6", 26 },
			{ L"UDP Receive IPv6", 27 },
			{ L"UDP Send IPv4", 10 },
			{ L"UDP Receive IPv4", 11 },
			{ L"UDP Send IPv6", 26 },
			{ L"UDP Receive IPv6", 27 },
		}
	},

	{ L"Handles", KernelEventTypes::PerfHandles, &ObjectGuid,
		{
			{ L"Create Handle", 32 },
			{ L"Close Handle", 33 },
			{ L"Duplicate Handle", 34 },
		}, true
	},

	{ L"Objects", KernelEventTypes::PerfObjects, &ObjectGuid,
		{
			{ L"Create Object", 48 },
			{ L"Delete Object", 49 },
			{ L"Reference Object", 50 },
			{ L"Dereference Object", 51 },
		}, true
	},

	{ L"Kernel Pool", KernelEventTypes::PerfPool, &PoolGuid,
		{
			{ L"Pool Alloc", 0x20 },
			{ L"Pool Session Alloc", 0x21 },
			{ L"Pool Free", 0x22 },
			{ L"Pool (Session) Free", 0x23 },
			{ L"Add Pool Page", 0x24 },
			{ L"Add Session Pool Page", 0x25 },
			{ L"Big Pool Page", 0x26 },
			{ L"Big Session Pool Page", 0x27 },
		}, true
	},

	{ L"Disk I/O", KernelEventTypes::DiskIO, &DiskIoGuid,
		{
			{ L"Read", 10 },
			{ L"Write", 11 },
			{ L"Read Init", 12 },
			{ L"Write Init", 13 },
			{ L"Flush Init", 15 },
			{ L"Flush Buffers", 14 },
		}
	},

	{ L"Virtual Memory", KernelEventTypes::VirtualMemory, &PageFaultGuid,
		{
			{ L"Virtual Alloc", 98 },
			{ L"Virtual Free", 99 },
		}
	},

	{ L"Page Fault", KernelEventTypes::PageFaults, &PageFaultGuid,
		{
			{ L"Hard Fault", 0x20 },
			{ L"Transition Fault", 10 },
			{ L"Demand Zero Fault", 11 },
			{ L"Copy on Write", 12 },
			{ L"Guard Page Fault", 13 },
			{ L"Hard Page Fault", 14 },
			{ L"Access Violation", 15 },
			{ L"Image Load Backed", 105 },
		}, true
	},

	{ L"Debug", KernelEventTypes::DebugPrint, &GUID_NULL,
		{
			{ L"Debug Print", 0x20 },
		}, true
	},

	{ L"Heap", KernelEventTypes::PerfHeap, &HeapGuid,
		{
			{ L"Heap Create", 0x20 },
			{ L"Heap Alloc", 0x21 },
			{ L"Heap ReAlloc", 0x22 },
			{ L"Heap Destroy", 0x22 },
			{ L"Heap Free", 0x24 },
			{ L"Heap Extend", 0x25 },
			{ L"Heap Snapshot", 0x26 },
			{ L"Heap Create Snapshot", 0x27 },
			{ L"Heap Destroy Snapshot", 0x28 },
			{ L"Heap Extend Snapshot", 0x29 },
			{ L"Heap Contract", 0x2a },
			{ L"Heap Lock", 0x2b },
			{ L"Heap Unlock", 0x2c },
			{ L"Heap Validate", 0x2d },
			{ L"Heap Walk", 0x2e },
		}, true
	},

	{ L"Job", KernelEventTypes::Job, &JobGuid,
		{
			{ L"Create", 0x20 },
			{ L"Terminate", 0x21 },
			{ L"Open", 0x22 },
			{ L"Assign Process", 0x23 },
			{ L"Remove Process", 0x24 },
			{ L"Set", 0x25 },
			{ L"Query", 0x26 },
		}, true
	},

	{ L"Driver", KernelEventTypes::Driver, &DiskIoGuid,
		{
			{ L"Init", 0x20 },
			{ L"Init Complete", 0x21 },
			{ L"Major Function Call", 0x22 },
			{ L"Major Function Return", 0x23 },
			{ L"Completion Routine Call", 0x24 },
			{ L"Completion Routine Return", 0x25 },
			{ L"Device Call", 0x26 },
			{ L"Device Return", 0x27 },
			{ L"Start I/O Call", 0x28 },
			{ L"Start I/O Return", 0x29 },
		}, true
	},

	{ L"Kernel Thread Pool", KernelEventTypes::WorkerThread, &ThreadGuid,
		{
			{ L"Create", 1 },
			{ L"Delete", 2 },
		}, true
	},

	{ L"File System Filter", KernelEventTypes::PerfFlt, &FileIoGuid,
		{
			{ L"Pre Operation Init", 0x60 },
			{ L"Post Operation Init", 0x61 },
			{ L"Pre Operation Completion", 0x62 },
			{ L"Post Operation Completion", 0x63 },
			{ L"Pre Operation Failure", 0x64 },
			{ L"Post Operation Failure", 0x65 },
		}, true
	},

	//{ L"Experimental", KernelEventTypes::PerfHeap, &HeapGuid,
	//	{
	//		{ L"Create", 1 },
	//		{ L"Delete", 2 },
	//	}, true
	//},

};

const std::vector<KernelEventCategory>& KernelEventCategory::GetAllCategories() {
	return KernelEvents;
}

const KernelEventCategory* KernelEventCategory::GetCategory(PCWSTR name) {
	for (auto& cat : GetAllCategories())
		if (cat.Name == name)
			return &cat;
	return nullptr;
}
