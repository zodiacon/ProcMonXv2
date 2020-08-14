#include "pch.h"
#include "TraceManager.h"
#include <TlHelp32.h>
#include "EventData.h"
#include <assert.h>
#include <VersionHelpers.h>

#pragma comment(lib, "tdh")

TraceManager::~TraceManager() {
	Stop();
}

bool TraceManager::AddKernelEventTypes(KernelEventTypes types) {
	if (IsRunning())
		return false;

	_kernelEventTypes |= types;
	return true;
}

bool TraceManager::RemoveKernelEventTypes(KernelEventTypes types) {
	if (IsRunning())
		return false;

	_kernelEventTypes &= ~types;
	return true;
}

bool TraceManager::SetKernelEventTypes(KernelEventTypes types) {
	if (IsRunning())
		return false;

	_kernelEventTypes = types;
	return true;
}

bool TraceManager::Start(EventCallback cb) {
	if (_handle || _hTrace)
		return true;

	auto sessionName = KERNEL_LOGGER_NAME;

	_callback = cb;

	// {6990501B-4484-4EF0-8793-84159B8D4728}
	static const GUID dummyGuid =
	{ 0x6990501b, 0x4484, 0x4ef0, { 0x87, 0x93, 0x84, 0x15, 0x9b, 0x8d, 0x47, 0x28 } };

	auto size = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAME);
	_propertiesBuffer = std::make_unique<BYTE[]>(size);
	bool isWin8Plus = ::IsWindows8OrGreater();
	for (;;) {
		::memset(_propertiesBuffer.get(), 0, size);

		_properties = reinterpret_cast<EVENT_TRACE_PROPERTIES*>(_propertiesBuffer.get());
		_properties->EnableFlags = (ULONG)(_kernelEventTypes | KernelEventTypes::Process);
		_properties->Wnode.BufferSize = (ULONG)size;
		_properties->Wnode.Guid = isWin8Plus ? dummyGuid : SystemTraceControlGuid;
		_properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
		_properties->Wnode.ClientContext = 1;
		_properties->FlushTimer = 1;
		_properties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_NO_PER_PROCESSOR_BUFFERING | EVENT_TRACE_SYSTEM_LOGGER_MODE;
		_properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

		auto error = ::StartTrace(&_handle, sessionName, _properties);
		if (error == ERROR_ALREADY_EXISTS) {
			error = ::ControlTrace(_hTrace, KERNEL_LOGGER_NAME, _properties, EVENT_TRACE_CONTROL_STOP);
			if (error != ERROR_SUCCESS)
				return false;
			continue;
		}
		break;
	}

	//if (error != ERROR_SUCCESS)
	//	return false;

	CLASSIC_EVENT_ID events[2] = { 0 };
	events[0].EventGuid = ProcessGuid;
	events[0].Type = 0;
	events[1].EventGuid = ImageLoadGuid;
	events[1].Type = 0xff;

	::TraceSetInformation(_handle, TraceStackTracingInfo, events, sizeof(events));

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
	if (_hTrace) {
		::CloseTrace(_hTrace);
		_hTrace = 0;
	}
	if (_handle) {
		::ControlTrace(_hTrace, KERNEL_LOGGER_NAME, _properties, EVENT_TRACE_CONTROL_STOP);
		_handle = 0;
	}
	::WaitForSingleObject(_hProcessThread.get(), 3000);
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

bool TraceManager::ParseProcessStartStop(EventData* data) {
	if (data->GetEventName().substr(0, 8) != L"Process/")
		return false;

	auto& header = data->GetHeader();

	switch (header.EventDescriptor.Opcode) {
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
					data->SetProcessName(pname);
				}
			}
			break;
		}

		case 11:		// process terminated
			//RemoveProcessName(header.ProcessId);
			break;

	}

	return true;
}

void TraceManager::OnEventRecord(PEVENT_RECORD rec) {
	auto pid = rec->EventHeader.ProcessId;
	auto eventName = GetkernelEventName(rec);
	auto data = std::make_shared<EventData>(rec, GetProcessImageById(pid), eventName);

	if (rec->EventHeader.ProviderId == StackWalkGuid)
		DebugBreak();

	bool processEvent = ParseProcessStartStop(data.get());
	if (rec->ExtendedData)
		DebugBreak();

	if (_callback && (!processEvent || ((_kernelEventTypes & KernelEventTypes::Process) == KernelEventTypes::Process)))
		_callback(data);
}

DWORD TraceManager::Run() {
	EnumProcesses();
	FILETIME now;
	::GetSystemTimeAsFileTime(&now);
	auto error = ::ProcessTrace(&_hTrace, 1, &now, nullptr);
	return error;
}

std::wstring TraceManager::GetkernelEventName(EVENT_RECORD* rec) const {
	//auto& desc = rec->EventHeader.EventDescriptor;
	//auto key = desc.Task | (desc.Opcode << 16) | (desc.Version << 24);
	//if (auto it = _kernelEventNames.find(key); it != _kernelEventNames.end())
	//	return it->second;

	BYTE buffer[1 << 10];
	ULONG size = sizeof(buffer);
	auto info = reinterpret_cast<PTRACE_EVENT_INFO>(buffer);
	if (::TdhGetEventInformation(rec, 0, nullptr, info, &size) == STATUS_SUCCESS) {
		auto name = std::wstring((PCWSTR)((PBYTE)info + info->TaskNameOffset)) + L"/" + std::wstring((PCWSTR)((PBYTE)info + info->OpcodeNameOffset));
		//_kernelEventNames.insert({ key, name });
		return name;
	}
	return L"";
}

void TraceManager::AddProcessName(DWORD pid, std::wstring name) {
	std::lock_guard locker(_processesLock);
	_processes.insert({ pid, std::move(name) });
}

bool TraceManager::RemoveProcessName(DWORD pid) {
	std::lock_guard locker(_processesLock);
	return _processes.erase(pid);
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
