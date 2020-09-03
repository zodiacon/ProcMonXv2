#pragma once
#include "EventData.h"

struct FormatHelper {
	static std::wstring FormatProperty(const EventData* data, const EventProperty& prop);
	static CString FormatTime(LONGLONG ts);
	static CString VirtualAllocFlagsToString(DWORD flags, bool withNumeric = false);
	static CString MajorFunctionToString(UCHAR mf);
	static PCWSTR ObjectTypeToString(int type);
};

