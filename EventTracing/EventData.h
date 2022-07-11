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
		static_assert(std::is_trivially_copyable<T>() && !std::is_pointer<T>());
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
	EventData(PEVENT_RECORD rec, std::wstring processName, const std::wstring& eventName, uint32_t index);

	void* operator new(size_t size);
	void operator delete(void* p);

	DWORD GetProcessId() const;
	DWORD GetThreadId() const;
	ULONGLONG GetTimeStamp() const;
	const GUID& GetProviderId() const;
	const EVENT_DESCRIPTOR& GetEventDescriptor() const;
	const std::wstring& GetProcessName() const;
	const std::wstring& GetEventName() const;
	uint32_t GetIndex() const;

	const std::vector<EventProperty>& GetProperties() const;
	const EventProperty* GetProperty(PCWSTR name) const;
	const EventData* GetStackEventData() const;
	std::wstring FormatProperty(const EventProperty& prop) const;
	uint64_t GetEventKey() const;

protected:
	void SetStackEventData(std::shared_ptr<EventData> data);
	void SetProcessName(std::wstring name);

private:
	inline static HANDLE s_hHeap = nullptr;
	inline static CRITICAL_SECTION s_HeapLock = {0};
	inline static volatile uint32_t s_Count = 0;

	ULONG _threadId, _processId;
	EVENT_DESCRIPTOR _eventDescriptor;
	ULONGLONG _timeStamp;
	ULONG _kernelTime, _userTime;
	GUID _providerId;
	std::wstring _processName;
	USHORT _headerFlags;
	const std::wstring& _eventName;
	mutable std::unique_ptr<BYTE[]> _buffer;
	PEVENT_RECORD _record;
	mutable std::vector<EventProperty> _properties;
	uint32_t _index;
	std::shared_ptr<EventData> _stackData;
};

