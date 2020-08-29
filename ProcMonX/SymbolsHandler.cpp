#include "pch.h"
#include "SymbolsHandler.h"

#pragma comment(lib, "dbghelp")
#pragma comment(lib, "ntdll")

enum SYSTEM_INFORMATION_CLASS {
	SystemModuleInformation = 11
};

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

extern "C" NTSTATUS NTAPI NtQuerySystemInformation(
	_In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
	_Out_writes_bytes_opt_(SystemInformationLength) PVOID SystemInformation,
	_In_ ULONG SystemInformationLength,
	_Out_opt_ PULONG ReturnLength
);

SymbolInfo::SymbolInfo() {
	auto size = sizeof(SYMBOL_INFO) + MAX_SYM_NAME;
	m_Symbol = static_cast<SYMBOL_INFO*>(malloc(size));
	::memset(m_Symbol, 0, size);
	m_Symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	m_Symbol->MaxNameLen = MAX_SYM_NAME;
}

SymbolInfo::~SymbolInfo() {
	::free(m_Symbol);
}


SymbolsHandler::SymbolsHandler(HANDLE hProcess, PCSTR searchPath, DWORD symOptions) {
	m_hProcess = hProcess;
	::SymSetOptions(symOptions);
	ATLVERIFY(::SymInitialize(m_hProcess, searchPath, TRUE));
}

std::unique_ptr<SymbolsHandler> SymbolsHandler::CreateForProcess(DWORD pid, PCSTR searchPath) {
	auto hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, pid);
	if (!hProcess)
		return nullptr;
	return std::make_unique<SymbolsHandler>(hProcess, searchPath);
}


SymbolsHandler::~SymbolsHandler() {
	::SymCleanup(m_hProcess);
	if (m_hProcess != ::GetCurrentProcess())
		::CloseHandle(m_hProcess);
}

HANDLE SymbolsHandler::GetHandle() const {
	return m_hProcess;
}

ULONG64 SymbolsHandler::LoadSymbolsForModule(PCSTR moduleName, DWORD64 baseAddress) {
	auto address = ::SymLoadModuleEx(m_hProcess, nullptr, moduleName, nullptr, baseAddress, 0, nullptr, 0);
	return address;
}

std::unique_ptr<SymbolInfo> SymbolsHandler::GetSymbolFromName(PCSTR name) {
	auto symbol = std::make_unique<SymbolInfo>();
	if (::SymFromName(m_hProcess, name, symbol->GetSymbolInfo()))
		return symbol;
	return nullptr;
}

std::unique_ptr<SymbolInfo> SymbolsHandler::GetSymbolFromAddress(DWORD64 address, PDWORD64 offset) {
	auto symbol = std::make_unique<SymbolInfo>();
	if (::SymFromAddr(m_hProcess, address, offset, symbol->GetSymbolInfo())) {
		symbol->ModuleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
		::SymGetModuleInfo64(m_hProcess, address, &symbol->ModuleInfo);
		return symbol;
	}
	return nullptr;
}

IMAGEHLP_MODULE64 SymbolsHandler::GetModuleInfo(DWORD64 address) const {
	IMAGEHLP_MODULE64 info = { sizeof(info) };
	::SymGetModuleInfo64(m_hProcess, address, &info);
	return info;
}

bool SymbolsHandler::LoadDefaultModules() {
	// load kernel modules
	PVOID base[1024];
	DWORD needed;
	CHAR path[MAX_PATH];
	::EnumDeviceDrivers(base, sizeof(base), &needed);
	auto count = min(sizeof(base), needed) / sizeof(PVOID);
	for (int i = 0; i < count; i++) {
		if (::GetDeviceDriverFileNameA(base[i], path, _countof(path))) {
			CStringA fullpath(path);
			fullpath.Replace("\\SystemRoot\\", "%SystemRoot%\\");
			if (fullpath.Mid(1, 2) == "??")
				fullpath = fullpath.Mid(4);
			auto address = LoadSymbolsForModule(fullpath, (DWORD64)base[i]);
			ATLASSERT(address);
		}
	}

	// load common user modules
	//HMODULE hModule[1024];
	//::EnumProcessModules(::GetCurrentProcess(), hModule, sizeof(hModule), &needed);
	//count = min(sizeof(base), needed) / sizeof(PVOID);
	//for (int i = 0; i < count; i++) {
	//	if (::GetModuleFileNameA(hModule[i], path, _countof(path))) {
	//		ATLVERIFY(LoadSymbolsForModule(path, (DWORD64)hModule[i]));
	//	}
	//}

	return true;
}

DWORD64 SymbolsHandler::LoadKernelModule(DWORD64 address) {
	auto size = 1 << 20;
	auto buffer = std::make_unique<BYTE[]>(size);
	if (0 != ::NtQuerySystemInformation(SystemModuleInformation, buffer.get(), size, nullptr))
		return 0;

	auto p = (RTL_PROCESS_MODULES*)buffer.get();
	auto count = p->NumberOfModules;
	for (ULONG i = 0; i < count; i++) {
		auto& module = p->Modules[i];
		if ((DWORD64)module.ImageBase <= address && (DWORD64)module.ImageBase + module.ImageSize > address) {
			// found the module
			ATLTRACE(L"Kernel module for address 0x%p found: %s\n", address, CString(module.FullPathName));
			CStringA fullpath(module.FullPathName);
			fullpath.Replace("\\SystemRoot\\", "%SystemRoot%\\");
			if (fullpath.Mid(1, 2) == "??")
				fullpath = fullpath.Mid(4);
			return LoadSymbolsForModule(fullpath, (DWORD64)module.ImageBase);
		}
	}
	ATLTRACE(L"Kernel module for address 0x%p not found\n", address);

	return 0;
}

BOOL SymbolsHandler::Callback(ULONG code, ULONG64 data) {
	MSG msg;
	while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		::DispatchMessage(&msg);

	return 0;
}
