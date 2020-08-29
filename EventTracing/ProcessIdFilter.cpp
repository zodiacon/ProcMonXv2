#include "pch.h"
#include "ProcessIdFilter.h"
#include "EventData.h"

ProcessIdFilter::ProcessIdFilter(DWORD pid, CompareType compare, FilterAction action) : FilterBase(L"ProcessId", compare, action), _pid(pid) {
}

void ProcessIdFilter::SetProcessId(DWORD pid) {
	_pid = pid;
}

FilterAction ProcessIdFilter::Eval(FilterContext& context) const {
	return _pid == context.Data->GetProcessId() ? GetAction() : GetDefaultAction();
}

bool ProcessIdFilter::InitFromParams(const std::wstring& params) {
	if (params.empty())
		return false;

	_pid = _wtoi(params.c_str());
	return true;
}

std::wstring ProcessIdFilter::GetParams() {
	return std::to_wstring(_pid);
}
