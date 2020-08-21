#pragma once
#include "FilterBase.h"

class ProcessIdFilter : public FilterBase {
public:
	using FilterBase::FilterBase;

	ProcessIdFilter(DWORD pid, FilterAction action);
	
	void SetProcessId(DWORD pid);
	
	// Inherited via FilterBase
	virtual FilterAction Eval(FilterContext& context) override;

private:
	DWORD _pid;
};

