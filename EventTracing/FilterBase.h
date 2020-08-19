#pragma once

struct FilterContext {
};

enum class FilterResult {
	None,
	Include,
	Exclude,
};

class FilterBase abstract {
public:
	FilterBase(PCWSTR name = L"");

	virtual FilterResult Eval(FilterContext& context) = 0;

	void SetName(PCWSTR name);
	const std::wstring& GetName() const;

private:
	std::wstring _name;
};

