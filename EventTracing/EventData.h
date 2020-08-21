#pragma once

#include <tdh.h>
#include <evntcons.h>
#include <assert.h>

struct EventProperty {
	friend class EventData;

	EventProperty(EVENT_PROPERTY_INFO& info);

	std::wstring Name;
	EVENT_PROPERTY_INFO& Info;
	ULONG GetLength() const {
		return (ULONG)Data.size();
	}

	template<typename T>
	T GetValue() const {
		static_assert(std::is_pod<T>() && !std::is_pointer<T>());
		assert(sizeof(T) == Data.size());
		return *(T*)Data.data();
	}

	BYTE* GetData() {
		return Data.data();
	}

	const BYTE* GetData() const {
		return Data.data();
	}

	PCWSTR GetUnicodeString() const;
	PCSTR GetAnsiString() const;

private:
	std::vector<BYTE> Data;
	void* Allocate(ULONG size);
};

class EventData {
	friend class TraceManager;
public:
	EventData(PEVENT_RECORD rec, std::wstring processName, std::wstring eventName);

	DWORD GetProcessId() const;
	const EVENT_HEADER& GetHeader() const;
	const std::wstring& GetProcessName() const;
	const std::wstring& GetEventName() const;
	DWORD GetThreadId() const;
	DWORD GetCPU() const;
	LONGLONG GetTimeStamp() const;
	void SetDetails(std::wstring details);
	const std::wstring& GetDetails() const;

	const std::vector<EventProperty>& GetProperties() const;
	const EventProperty* GetProperty(PCWSTR name) const;
	const EventData* GetStackEventData() const;
	std::wstring FormatProperty(const EventProperty& prop) const;

protected:
	void SetStackEventData(std::shared_ptr<EventData> data);
	void SetProcessName(std::wstring name);

private:
	EVENT_HEADER _header;
	std::wstring _processName;
	std::wstring _eventName;
	std::unique_ptr<BYTE[]> _buffer;
	PTRACE_EVENT_INFO _info{ nullptr };
	PEVENT_RECORD _record;
	mutable std::vector<EventProperty> _properties;
	std::wstring _details;
	std::shared_ptr<EventData> _stackData;
	DWORD _cpu;
};

