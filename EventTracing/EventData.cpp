#include "pch.h"
#include "EventData.h"
#include <in6addr.h>

// EventProperty

EventProperty::EventProperty(EVENT_PROPERTY_INFO& info) : Info(info) {
}

EventProperty::~EventProperty() {
	if (_allocated)
		::free(Data);
}

void EventProperty::Allocate(ULONG size) {
	Data = (BYTE*)malloc(size);
	_allocated = true;
}

PCWSTR EventProperty::GetUnicodeString() const {
	return (PCWSTR)Data;
}

PCSTR EventProperty::GetAnsiString() const {
	return (PCSTR)Data;
}

// EventData

EventData::EventData(PEVENT_RECORD rec, std::wstring processName, std::wstring eventName) : _record(rec),
	_processName(std::move(processName)), _eventName(std::move(eventName)) {
	_header = rec->EventHeader;
	_cpu = rec->BufferContext.ProcessorNumber;

	// parse event specific data

	ULONG size = 0;
	auto error = ::TdhGetEventInformation(rec, 0, nullptr, _info, &size);
	if (error == ERROR_INSUFFICIENT_BUFFER) {
		_buffer = std::make_unique<BYTE[]>(size);
		_info = reinterpret_cast<PTRACE_EVENT_INFO>(_buffer.get());
		error = ::TdhGetEventInformation(rec, 0, nullptr, _info, &size);
	}
	::SetLastError(error);
}

DWORD EventData::GetProcessId() const {
	return _header.ProcessId;
}

const EVENT_HEADER& EventData::GetHeader() const {
	return _header;
}

const std::wstring& EventData::GetProcessName() const {
	return _processName;
}

const std::wstring& EventData::GetEventName() const {
	return _eventName;
}

DWORD EventData::GetThreadId() const {
	return _header.ThreadId;
}

DWORD EventData::GetCPU() const {
	return _cpu;
}

LONGLONG EventData::GetTimeStamp() const {
	return _header.TimeStamp.QuadPart;
}

const std::vector<EventProperty>& EventData::GetProperties() const {
	if (!_properties.empty() || _info == nullptr)
		return _properties;

	_properties.reserve(_info->TopLevelPropertyCount);
	auto userDataLength = _record->UserDataLength;
	BYTE* data = (BYTE*)_record->UserData;

	for (ULONG i = 0; i < _info->TopLevelPropertyCount; i++) {
		auto& prop = _info->EventPropertyInfoArray[i];
		EventProperty property(prop);
		property.Name.assign((WCHAR*)((BYTE*)_info + prop.NameOffset));
		ULONG len = prop.length;
		if (len == 0) {
			PROPERTY_DATA_DESCRIPTOR desc;
			desc.PropertyName = (ULONGLONG)property.Name.c_str();
			desc.ArrayIndex = ULONG_MAX;
			if (ERROR_SUCCESS == ::TdhGetPropertySize(_record, 0, nullptr, 1, &desc, &len)) {
				//				property.Allocate(len);
				//				::TdhGetProperty(_record, 0, nullptr, 1, &desc, len, property.Data);				
			}
		}
		property.Length = len;
		property.Data = data;
		data += len;
		userDataLength -= (USHORT)len;

		_properties.push_back(std::move(property));
	}

	return _properties;
}

const EventProperty* EventData::GetProperty(PCWSTR name) const {
	for (auto& prop : GetProperties())
		if (prop.Name == name)
			return &prop;
	return nullptr;
}

void EventData::SetDetails(std::wstring details) {
	_details = std::move(details);
}

const std::wstring& EventData::GetDetails() const {
	return _details;
}

std::wstring EventData::FormatProperty(const EventProperty& property) const {
	ULONG size;
	EVENT_MAP_INFO* eventMap = nullptr;
	auto& prop = property.Info;
	WCHAR buffer[1024];
	if (prop.nonStructType.MapNameOffset) {
		auto mapName = (PWSTR)((PBYTE)_info + prop.nonStructType.MapNameOffset);
		eventMap = reinterpret_cast<EVENT_MAP_INFO*>(buffer);
		size = sizeof(buffer);
		::TdhGetEventMapInformation(_record, mapName, eventMap, &size);
	}
	size = sizeof(buffer);

	// length special case for IPV6
	auto len = prop.length;
	if (prop.nonStructType.InType == TDH_INTYPE_BINARY && prop.nonStructType.OutType == TDH_OUTTYPE_IPV6)
		len = sizeof(IN6_ADDR);

	USHORT consumed;
	auto status = ::TdhFormatProperty(_info, eventMap, (_header.Flags & EVENT_HEADER_FLAG_32_BIT_HEADER) ? 4 : 8,
		prop.nonStructType.InType, prop.nonStructType.OutType, len, (USHORT)property.Length, property.Data,
		&size, buffer, &consumed);
	if (status == STATUS_SUCCESS)
		return std::wstring(buffer);

	return L"";
}

void EventData::SetProcessName(std::wstring name) {
	_processName = std::move(name);
}

