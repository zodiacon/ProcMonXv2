#include "pch.h"
#include "ProcessIdFilter.h"
#include "EventData.h"

ProcessIdFilter::ProcessIdFilter(DWORD pid, FilterAction action) : _pid(pid) {
	SetAction(action);
}

void ProcessIdFilter::SetProcessId(DWORD pid) {
	_pid = pid;
}

FilterAction ProcessIdFilter::Eval(FilterContext& context) {
	return _pid == context.Data->GetProcessId() ? GetAction() : GetDefaultAction();
}
