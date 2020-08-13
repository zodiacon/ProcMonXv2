#pragma once

#include "KernelEvents.h"
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <wil\resource.h>

class EventData;

using EventCallback = std::function<void(std::shared_ptr<EventData>)>;

class TraceManager final {
public:
	~TraceManager();

	bool AddKernelEventTypes(KernelEventTypes types);
	bool RemoveKernelEventTypes(KernelEventTypes types);
	bool SetKernelEventTypes(KernelEventTypes types);

	bool Start(EventCallback callback);
	bool Stop();
	bool IsRunning() const;

	std::wstring GetProcessImageById(DWORD pid) const;
	static std::wstring GetDosNameFromNtName(PCWSTR name);

private:
	std::wstring GetkernelEventName(EVENT_RECORD* rec) const;
	void AddProcessName(DWORD pid, std::wstring name);
	bool RemoveProcessName(DWORD pid);
	void EnumProcesses();
	bool ParseProcessStartStop(EventData* data);
	void OnEventRecord(PEVENT_RECORD rec);
	DWORD Run();

private:
	struct ProcessInfo {
		DWORD Id;
		DWORD ParentId;
		std::wstring ImageName;
	};

	TRACEHANDLE _handle{ 0 };
	TRACEHANDLE _hTrace{ 0 };
	EVENT_TRACE_PROPERTIES* _properties;
	std::unique_ptr<BYTE[]> _propertiesBuffer;
	EVENT_TRACE_LOGFILE _traceLog = { 0 };
	wil::unique_handle _hProcessThread;
	EventCallback _callback;
	KernelEventTypes _kernelEventTypes{ KernelEventTypes::None };
	mutable std::shared_mutex _processesLock;
	std::unordered_map<DWORD, std::wstring> _processes;
	mutable std::unordered_map<DWORD, std::wstring> _kernelEventNames;
};

