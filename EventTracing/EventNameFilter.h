#pragma once
#include "StringCompareFilterBase.h"

class EventNameFilter : public StringCompareFilterBase {
public:
	EventNameFilter(std::wstring name, CompareType type, FilterAction action);

	virtual FilterAction Eval(FilterContext& context) const override;

};

