#pragma once
#include "StringCompareFilterBase.h"

class PropertyValueFilter final : public StringCompareFilterBase {
public:
	PropertyValueFilter(std::wstring name, CompareType type, FilterAction action);

	virtual FilterAction Eval(FilterContext& context) const override;
};

