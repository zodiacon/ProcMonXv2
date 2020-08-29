#pragma once
#include "FilterBase.h"

class ProcessIdFilter : public FilterBase {
public:
	ProcessIdFilter(DWORD pid, CompareType compare, FilterAction action);
	
	void SetProcessId(DWORD pid);
	
	// Inherited via FilterBase
	virtual FilterAction Eval(FilterContext& context) const override;
	virtual bool InitFromParams(const std::wstring& params) override;
	virtual std::wstring GetParams() override;

private:
	DWORD _pid;
};

