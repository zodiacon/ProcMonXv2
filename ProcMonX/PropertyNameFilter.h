#pragma once
#include "StringCompareFilterBase.h"

class PropertyNameFilter : public StringCompareFilterBase {
public:
	PropertyNameFilter(std::wstring name, CompareType type, FilterAction action);

	virtual FilterAction Eval(FilterContext& context) const override;

};

