#pragma once

#include "KernelEvents.h"
#include <atomic>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <functional>
#include <set>
#include <memory>
#include <wil\resource.h>

class EventData;
class FilterBase;

using EventCallback = std::function<void(std::shared_ptr<EventData>)>;

class TraceManager final {
public:
	TraceManager();
	~TraceManager();
	TraceManager(const TraceManager&) = delete;
	TraceManager& operator=(const TraceManager&) = delete;

	bool AddKernelEventTypes(std::initializer_list<KernelEventTypes> types);
	//bool RemoveKernelEventTypes(KernelEventTypes types);
	bool SetKernelEventTypes(std::initializer_list<KernelEventTypes> types);
	bool SetKernelEventStacks(std::initializer_list<std::wstring> categories);
	bool SetBackupFile(PCWSTR path);
	void Pause(bool pause);
	bool Start(EventCallback callback);
	bool Stop();
	bool IsRunning() const;
	bool IsPaused() const;

	void ResetIndex(uint32_t index = 0);
	int UpdateEventConfig();

	std::wstring GetProcessImageById(DWORD pid) const;
	static std::wstring GetDosNameFromNtName(PCWSTR name);

	void AddFilter(std::shared_ptr<FilterBase> filter);
	bool RemoveFilterAt(int index);
	void RemoveAllFilters();
	int GetFilterCount() const;
	uint32_t GetFilteredEventsCount() const;

	bool SwapFilters(int i1, int i2);
	std::shared_ptr<FilterBase> GetFilter(int index) const;

private:
	const std::wstring& GetkernelEventName(EVENT_RECORD* rec) const;
	void AddProcessName(DWORD pid, std::wstring name);
	bool RemoveProcessName(DWORD pid);
	void EnumProcesses();
	bool ParseProcessStartStop(EventData* data);
	void OnEventRecord(PEVENT_RECORD rec);
	DWORD Run();
	void HandleNoProcessId(EventData* data);

private:
	struct ProcessInfo {
		DWORD Id;
		std::wstring ImageName;
		LONGLONG CreateTime;
	};

	TRACEHANDLE _handle{ 0 };
	TRACEHANDLE _hTrace{ 0 };
	EVENT_TRACE_PROPERTIES* _properties;
	std::unique_ptr<BYTE[]> _propertiesBuffer;
	EVENT_TRACE_LOGFILE _traceLog = { 0 };
	wil::unique_handle _hProcessThread;
	EventCallback _callback;
	std::set<KernelEventTypes> _kernelEventTypes;
	std::set<std::wstring> _kernelEventStacks;
	mutable std::shared_mutex _processesLock;
	std::unordered_map<DWORD, std::wstring> _processes;
	mutable std::unordered_map<ULONGLONG, std::wstring> _kernelEventNames;
	std::vector<DWORD> _cleanupPids;
	std::shared_ptr<EventData> _lastEvent;
	std::shared_ptr<EventData> _lastExcluded;
	uint32_t _index{ 0 };
	std::vector<std::shared_ptr<FilterBase>> _filters;
	std::atomic<uint32_t> _filteredEvents{ 0 };
	wil::unique_handle _hMemMap;
	bool _isTraceProcesses{ true };
	bool _dumpUnnamedEvents{ true };
	std::atomic<bool> _isPaused{ false };
};

