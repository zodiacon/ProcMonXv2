#include "pch.h"
#include "FormatHelper.h"
#include "TraceManager.h"
#include <functional>

std::wstring FormatHelper::FormatProperty(const EventData* data, const EventProperty& prop) {
	static const auto statusFunction = [](auto, auto& p) {
		CString result;
		ATLASSERT(p.GetLength() == sizeof(DWORD));
		result.Format(L"0x%08X", p.GetValue<DWORD>());
		return (PCWSTR)result;
	};

	static const auto int64ToHex = [](auto, auto& p) -> std::wstring {
		ATLASSERT(p.GetLength() == sizeof(LONGLONG));
		CString text;
		text.Format(L"0x%llX", p.GetValue<LONGLONG>());
		return (PCWSTR)text;
	};

	static const std::unordered_map<std::wstring, std::function<std::wstring(const EventData*, const EventProperty&)>> functions{
		{ L"Status", statusFunction },
		{ L"NtStatus", statusFunction },
		{ L"InitialTime", int64ToHex },
		{ L"TimeDateStamp", statusFunction },
		{ L"PageFault/VirtualAlloc;Flags", [](auto, auto& p) -> std::wstring {
			ATLASSERT(p.GetLength() == sizeof(DWORD));
			return (PCWSTR)VirtualAllocFlagsToString(p.GetValue<DWORD>(), true);
			} },
		{ L"MajorFunction", [](auto, auto& p) -> std::wstring {
			ATLASSERT(p.GetLength() == sizeof(DWORD));
			return (PCWSTR)MajorFunctionToString((UCHAR)p.GetValue<DWORD>());
			} },
		{ L"FileName", [](auto data, auto& p) -> std::wstring {
			auto value = data->FormatProperty(p);
			if (value[0] == L'\\')
				value = TraceManager::GetDosNameFromNtName(value.c_str());
			return value;
			} },
	};

	auto it = functions.find(data->GetEventName() + L";" + prop.Name);
	if (it == functions.end())
		it = functions.find(prop.Name);
	if (it == functions.end())
		return L"";

	return (it->second)(data, prop);
}

CString FormatHelper::FormatTime(LONGLONG ts) {
	CString text;
	text.Format(L".%06u", (ts / 10) % 1000000);
	return CTime(*(FILETIME*)&ts).Format(L"%x %X") + text;
}

CString FormatHelper::VirtualAllocFlagsToString(DWORD flags, bool withNumeric) {
	CString text;
	if (flags & MEM_COMMIT)
		text += L"MEM_COMMIT | ";
	if (flags & MEM_RESERVE)
		text += L"MEM_RESERVE | ";

	if (!text.IsEmpty())
		text = text.Left(text.GetLength() - 3);

	if (withNumeric)
		text.Format(L"%s (0x%X)", text, flags);

	return text;
}

CString FormatHelper::MajorFunctionToString(UCHAR mf) {
	static PCWSTR major[] = {
		L"CREATE",
		L"CREATE_NAMED_PIPE",
		L"CLOSE",
		L"READ",
		L"WRITE",
		L"QUERY_INFORMATION",
		L"SET_INFORMATION",
		L"QUERY_EA",
		L"SET_EA",
		L"FLUSH_BUFFERS",
		L"QUERY_VOLUME_INFORMATION",
		L"SET_VOLUME_INFORMATION",
		L"DIRECTORY_CONTROL",
		L"FILE_SYSTEM_CONTROL",
		L"DEVICE_CONTROL",
		L"INTERNAL_DEVICE_CONTROL",
		L"SHUTDOWN",
		L"LOCK_CONTROL",
		L"CLEANUP",
		L"CREATE_MAILSLOT",
		L"QUERY_SECURITY",
		L"SET_SECURITY",
		L"POWER",
		L"SYSTEM_CONTROL",
		L"DEVICE_CHANGE",
		L"QUERY_QUOTA",
		L"SET_QUOTA",
		L"PNP"
	};

	static PCWSTR major_flt[] = {
		L"ACQUIRE_FOR_SECTION_SYNCHRONIZATION",
		L"RELEASE_FOR_SECTION_SYNCHRONIZATION",
		L"ACQUIRE_FOR_MOD_WRITE",
		L"RELEASE_FOR_MOD_WRITE",
		L"ACQUIRE_FOR_CC_FLUSH",
		L"RELEASE_FOR_CC_FLUSH",
		L"QUERY_OPEN",
	};

	static PCWSTR major_flt2[] = {
		L"FAST_IO_CHECK_IF_POSSIBLE",
		L"NETWORK_QUERY_OPEN",
		L"MDL_READ",       
		L"MDL_READ_COMPLETE",
		L"PREPARE_MDL_WRITE",     
		L"MDL_WRITE_COMPLETE",
		L"VOLUME_MOUNT",   
		L"VOLUME_DISMOUNT"
	};	  

	CString text;
	if (mf < _countof(major))
		text.Format(L"IRP_MJ_%s (%d)", major[mf], (int)mf);
	else if(mf >= 255 - 7)
		text.Format(L"IRP_MJ_%s (%d)", major_flt[255 - mf - 1], (int)mf);
	else if (mf >= 255 - 20)
		text.Format(L"IRP_MJ_%s (%d)", major_flt2[255 - mf - 7], (int)mf);
	else
		text.Format(L"%d", (int)mf);
	return text;
}
