#include "pch.h"
#include "ProcessNameFilter.h"
#include "EventData.h"

ProcessNameFilter::ProcessNameFilter(std::wstring name, CompareType type, FilterAction action) : 
	StringCompareFilterBase(L"Process Name", name, type, action) {
}

FilterAction ProcessNameFilter::Eval(FilterContext& context) const {
	return Compare(context.Data->GetProcessName());
}

