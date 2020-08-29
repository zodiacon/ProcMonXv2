#pragma once
#include "FilterBase.h"

class StringCompareFilterBase abstract : public FilterBase {
public:
	using FilterBase::FilterBase;

protected:
	StringCompareFilterBase(std::wstring name, std::wstring text, CompareType type, FilterAction action);
	FilterAction Compare(const std::wstring& str) const;

	virtual bool InitFromParams(const std::wstring& params) override;
	virtual std::wstring GetParams() override;

private:
	std::wstring _text;
};

