#pragma once
#include "FilterBase.h"

enum class CompareType {
	Equals,
	NotEqual,
	Contains,
	NotContains
};

class ProcessNameFilter : public FilterBase {
public:
	using FilterBase::FilterBase;

	ProcessNameFilter(std::wstring name, CompareType type);

	// Inherited via FilterBase
	virtual FilterAction Eval(FilterContext& context) override;

private:
	std::wstring _name;
	CompareType _type;
};

