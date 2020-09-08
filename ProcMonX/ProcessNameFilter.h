#pragma once
#include "StringCompareFilterBase.h"

class ProcessNameFilter final : public StringCompareFilterBase {
public:
	using StringCompareFilterBase::StringCompareFilterBase;

	ProcessNameFilter(std::wstring name, CompareType type, FilterAction action);

	virtual FilterAction Eval(FilterContext& context) const override;

};

