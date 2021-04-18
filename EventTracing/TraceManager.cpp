#include "pch.h"
#include "TraceManager.h"
#include <TlHelp32.h>
#include "EventData.h"
#include <assert.h>
#include <VersionHelpers.h>
#include <algorithm>
#include "FilterBase.h"

#pragma comment(lib, "tdh")

static bool EnablePrivilege(PCWSTR privilege) {
	HANDLE hToken;
	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
		return false;

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!::LookupPrivilegeValue(nullptr, privilege, &tp.Privileges[0].Luid))
		return false;

	BOOL success = ::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr);
	::CloseHandle(hToken);

	return success && ::GetLastError() == ERROR_SUCCESS;
}

TraceManager::TraceManager() {
	EnablePrivilege(SE_DEBUG_NAME);
	EnablePrivilege(SE_SYSTEM_PROFILE_NAME);
	_filters.reserve(8);
}

TraceManager::~TraceManager() {
	Stop();
}

bool TraceManager::AddKernelEventTypes(std::initializer_list<KernelEventTypes> types) {
	_kernelEventTypes.insert(types);
	return true;
}

bool TraceManager::SetKernelEventTypes(std::initializer_list<KernelEventTypes> types) {
	_kernelEventTypes = types;
	return true;
}

bool TraceManager::SetKernelEventStacks(std::initializer_list<std::wstring> categories) {
	_kernelEventStacks = categories;
	return true;
}

bool TraceManager::Start(EventCallback cb) {
	if (_handle || _hTrace)
		return true;

	auto sessionName = KERNEL_LOGGER_NAME;

	_callback = cb;
	_filteredEvents = 0;

	// {6990501B-4484-4EF0-8793-84159B8D4728}
	static const GUID dummyGuid =
	{ 0x6990501b, 0x4484, 0x4ef0, { 0x87, 0x93, 0x84, 0x15, 0x9b, 0x8d, 0x47, 0x28 } };

	auto size = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAME);
	_propertiesBuffer = std::make_unique<BYTE[]>(size);
	bool isWin8Plus = ::IsWindows8OrGreater();
	ULONG error;

	for (;;) {
		::memset(_propertiesBuffer.get(), 0, size);

		_properties = reinterpret_cast<EVENT_TRACE_PROPERTIES*>(_propertiesBuffer.get());
		_properties->EnableFlags = (ULONG)KernelEventTypes::Process;
		_properties->Wnode.BufferSize = (ULONG)size;
		_properties->Wnode.Guid = isWin8Plus ? dummyGuid : SystemTraceControlGuid;
		_properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
		_properties->Wnode.ClientContext = 1;
		_properties->FlushTimer = 1;
		_properties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_USE_LOCAL_SEQUENCE | EVENT_TRACE_SYSTEM_LOGGER_MODE;
		_properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

		error = ::StartTrace(&_handle, sessionName, _properties);
		if (error == ERROR_ALREADY_EXISTS) {
			error = ::ControlTrace(_hTrace, KERNEL_LOGGER_NAME, _properties, EVENT_TRACE_CONTROL_STOP);
			if (error != ERROR_SUCCESS)
				return false;
			continue;
		}
		break;
	}
	if (error != ERROR_SUCCESS)
		return false;

	error = UpdateEventConfig();
	if (error != ERROR_SUCCESS) {
		Stop();
		return false;
	}

	//WCHAR srpc[] = L"{6AD52B32-D609-4BE9-AE07-CE8DAE937E39}";
	//GUID rpc;
	//if (SUCCEEDED(::CLSIDFromString(srpc, &rpc))) {
	//	ENABLE_TRACE_PARAMETERS params = { ENABLE_TRACE_PARAMETERS_VERSION_2 };
	//	error = ::EnableTraceEx2(_handle, &rpc, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_VERBOSE, 0, 0, INFINITE, &params);
	//}

	_traceLog.Context = this;
	_traceLog.LoggerName = (PWSTR)KERNEL_LOGGER_NAME;
	_traceLog.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD | PROCESS_TRACE_MODE_REAL_TIME;
	_traceLog.EventRecordCallback = [](PEVENT_RECORD record) {
		((TraceManager*)record->UserContext)->OnEventRecord(record);
	};
	_hTrace = ::OpenTrace(&_traceLog);
	if (!_hTrace)
		return false;

	// create a dedicated thread to process the trace
	_hProcessThread.reset(::CreateThread(nullptr, 0, [](auto param) {
		return ((TraceManager*)param)->Run();
		}, this, 0, nullptr));
	::SetThreadPriority(_hProcessThread.get(), THREAD_PRIORITY_HIGHEST);

	return true;
}

bool TraceManager::Stop() {
	if (_handle) {
		::ControlTrace(_hTrace, KERNEL_LOGGER_NAME, _properties, EVENT_TRACE_CONTROL_STOP);
		_handle = 0;
	}
	if (_hTrace) {
		::CloseTrace(_hTrace);
		_hTrace = 0;
	}
	if (WAIT_TIMEOUT == ::WaitForSingleObject(_hProcessThread.get(), 3000))
		::TerminateThread(_hProcessThread.get(), 1);
	_hProcessThread.reset();

	return true;
}

bool TraceManager::IsRunning() const {
	return _hProcessThread != nullptr;
}

std::wstring TraceManager::GetProcessImageById(DWORD pid) const {
	std::shared_lock locker(_processesLock);
	if (auto it = _processes.find(pid); it != _processes.end())
		return it->second;
	return L"";
}

void TraceManager::EnumProcesses() {
	wil::unique_handle hSnapshot(::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
	if (!hSnapshot)
		return;

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);

	if (!::Process32First(hSnapshot.get(), &pe))
		return;

	_processes.clear();
	_processes.reserve(512);

	while (::Process32Next(hSnapshot.get(), &pe)) {
		_processes.insert({ pe.th32ProcessID, pe.szExeFile });
	}
}

void TraceManager::RemoveAllFilters() {
	_filters.clear();
}

int TraceManager::GetFilterCount() const {
	return (int)_filters.size();
}

bool TraceManager::SwapFilters(int i1, int i2) {
	if (i1 < 0 || i2 < 0 || i1 >= _filters.size() || i2 >= _filters.size() || i1 == i2)
		return false;

	std::swap(_filters[i1], _filters[i2]);
	return true;
}

std::shared_ptr<FilterBase> TraceManager::GetFilter(int index) const {
	if (index < 0 || index >= _filters.size())
		return nullptr;
	return _filters[index];
}

bool TraceManager::ParseProcessStartStop(EventData* data) {
	if (data->GetEventName().substr(0, 8) != L"Process/")
		return false;

	switch (data->GetEventDescriptor().Opcode) {
		case 1:		// process created
		{
			auto prop = data->GetProperty(L"ImageFileName");
			if (prop) {
				auto name = prop->GetAnsiString();
				if (name) {
					std::wstring pname;
					pname.assign(name, name + strlen(name));
					AddProcessName(data->GetProperty(L"ProcessId")->GetValue<DWORD>(), pname);
					assert(!pname.empty());
					//data->SetProcessName(pname);
				}
			}
			break;
		}

		case 2:		// process end
			RemoveProcessName(data->GetProcessId());
			break;

	}

	return true;
}

void TraceManager::ResetIndex(uint32_t index) {
	_index = index;
}

int TraceManager::UpdateEventConfig() {
	typedef struct _PERFINFO_GROUPMASK {
		ULONG Masks[8];
	} PERFINFO_GROUPMASK;

	PERFINFO_GROUPMASK gm{};
	gm.Masks[0] = EVENT_TRACE_FLAG_PROCESS;
	for (auto type : _kernelEventTypes) {
		gm.Masks[((uint64_t)type) >> 32] |= (ULONG)type;
	}
	auto error = ::TraceSetInformation(_handle, TraceSystemTraceEnableFlagsInfo, &gm, sizeof(gm));
	if (error != ERROR_SUCCESS)
		return error;

	std::vector<CLASSIC_EVENT_ID> stacks;
	stacks.reserve(32);
	for (auto& name : _kernelEventStacks) {
		auto cat = KernelEventCategory::GetCategory(name.c_str());
		assert(cat);
		for (auto& evt : cat->Events) {
			CLASSIC_EVENT_ID id{};
			id.EventGuid = *cat->Guid;
			id.Type = evt.Opcode;
			stacks.push_back(id);
		}
	}

	error = ::TraceSetInformation(_handle, TraceStackTracingInfo, stacks.data(), (ULONG)stacks.size() * sizeof(CLASSIC_EVENT_ID));
	return error;
}

void TraceManager::OnEventRecord(PEVENT_RECORD rec) {
	if (_handle == 0 || _isPaused)
		return;

	auto pid = rec->EventHeader.ProcessId;
	auto& eventName = GetkernelEventName(rec);
	if (eventName.empty() && _dumpUnnamedEvents)
		return;

	// use the separate heap
	std::shared_ptr<EventData> data(new EventData(rec, GetProcessImageById(pid), eventName, ++_index));

	// force copying properties
	data->GetProperties();

	if (rec->EventHeader.ProviderId == StackWalkGuid) {
		_index--;
		if (_lastEvent) {
			if (_lastEvent->GetThreadId() != data->GetProperty(L"StackThread")->GetValue<DWORD>())
				return;

			_lastEvent->SetStackEventData(data);
			_lastEvent.reset();
		}
		_lastExcluded = nullptr;
		return;
	}

	bool processEvent = ParseProcessStartStop(data.get());

	if (!processEvent && data->GetProcessId() == 0 || data->GetProcessId() == (DWORD)-1) {
		HandleNoProcessId(data.get());
	}

	if (!_filters.empty()) {
		// evaluate filters
		FilterContext context = { data.get() };
		auto result = FilterAction::None;
		for (auto& filter : _filters) {
			if (!filter->IsEnabled())
				continue;
			auto action = filter->Eval(context);
			if (action == FilterAction::Exclude) {
				result = action;
				break;
			}

			if (action == FilterAction::Include) {
				result = FilterAction::Include;
				break;
			}
		}
		if (result == FilterAction::Exclude) {
			_filteredEvents++;
			_lastExcluded = data;
			return;
		}
	}

	_lastEvent = data;

	if (_callback && (!processEvent || _isTraceProcesses))
		_callback(data);
}

DWORD TraceManager::Run() {
	EnumProcesses();
	FILETIME now;
	::GetSystemTimeAsFileTime(&now);
	auto error = ::ProcessTrace(&_hTrace, 1, &now, nullptr);
	return error;
}

uint32_t TraceManager::GetFilteredEventsCount() const {
	return _filteredEvents;
}

void TraceManager::HandleNoProcessId(EventData* data) {
	DWORD tid = 0;
	if (data->GetThreadId() == 0 || data->GetThreadId() == (DWORD)-1) {
		auto prop = data->GetProperty(L"ThreadId");
		if (!prop)
			prop = data->GetProperty(L"TThreadId");
		if (prop) {
			tid = prop->GetValue<DWORD>();
			data->_threadId = tid;
		}
	}
	if (data->GetProcessId() == 0 || data->GetProcessId() == (DWORD)-1) {
		auto prop = data->GetProperty(L"PID");
		if (prop == nullptr)
			prop = data->GetProperty(L"ProcessId");
		if (prop) {
			auto pid = prop->GetValue<DWORD>();
			data->_processId = pid;
			data->SetProcessName(GetProcessImageById(pid));
		}
		else if (tid) {
			wil::unique_handle hThread(::OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, tid));
			if (hThread) {
				auto pid = ::GetProcessIdOfThread(hThread.get());
				if (pid) {
					data->_processId = pid;
					data->SetProcessName(GetProcessImageById(pid));
				}
			}
		}
	}
}

const std::wstring& TraceManager::GetkernelEventName(EVENT_RECORD* rec) const {
	static const std::wstring empty;
	auto& desc = rec->EventHeader.EventDescriptor;
	auto key = rec->EventHeader.ProviderId.Data1 ^ desc.Opcode;
	if (auto it = _kernelEventNames.find(key); it != _kernelEventNames.end())
		return it->second;

	static BYTE buffer[1 << 11];
	ULONG size = sizeof(buffer);
	auto info = reinterpret_cast<PTRACE_EVENT_INFO>(buffer);
	if (::TdhGetEventInformation(rec, 0, nullptr, info, &size) == STATUS_SUCCESS) {
		const auto name = std::wstring((PCWSTR)((PBYTE)info + info->TaskNameOffset)) + L"/" + std::wstring((PCWSTR)((PBYTE)info + info->OpcodeNameOffset));
		_kernelEventNames.insert({ key, name });
		return _kernelEventNames[key];
	}
	return empty;
}

void TraceManager::AddProcessName(DWORD pid, std::wstring name) {
	std::lock_guard locker(_processesLock);
	_processes.insert({ pid, std::move(name) });
}

void TraceManager::AddFilter(std::shared_ptr<FilterBase> filter) {
	_filters.push_back(std::move(filter));
}

bool TraceManager::RemoveProcessName(DWORD pid) {
	std::lock_guard locker(_processesLock);
	return _processes.erase(pid);
}

bool TraceManager::IsPaused() const {
	return _isPaused;
}

bool TraceManager::RemoveFilterAt(int index) {
	if (index < 0 || index >= _filters.size())
		return false;
	_filters.erase(_filters.begin() + index);
	return true;
}

std::wstring TraceManager::GetDosNameFromNtName(PCWSTR name) {
	static std::vector<std::pair<std::wstring, std::wstring>> deviceNames;
	static bool first = true;
	if (first) {
		auto drives = ::GetLogicalDrives();
		int drive = 0;
		while (drives) {
			if (drives & 1) {
				// drive exists
				WCHAR driveName[] = L"X:";
				driveName[0] = (WCHAR)(drive + 'A');
				WCHAR path[MAX_PATH];
				if (::QueryDosDevice(driveName, path, MAX_PATH)) {
					deviceNames.push_back({ path, driveName });
				}
			}
			drive++;
			drives >>= 1;
		}
		first = false;
	}

	for (auto& [ntName, dosName] : deviceNames) {
		if (::_wcsnicmp(name, ntName.c_str(), ntName.size()) == 0)
			return dosName + (name + ntName.size());
	}
	return L"";
}

bool TraceManager::SetBackupFile(PCWSTR path) {
	if (path) {
		wil::unique_hfile hFile(::CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, TRUNCATE_EXISTING, 0, nullptr));
		if (!hFile)
			return false;
		_hMemMap.reset(::CreateFileMapping(hFile.get(), nullptr, PAGE_READWRITE | MEM_RESERVE, 32, 0, nullptr));
		if (!_hMemMap)
			return false;
	}
	return true;
}

void TraceManager::Pause(bool pause) {
	_isPaused = pause;
}
