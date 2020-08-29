#include "pch.h"
#include "StringCompareFilterBase.h"

StringCompareFilterBase::StringCompareFilterBase(std::wstring name, std::wstring text, CompareType type, FilterAction action) :
	FilterBase(std::move(name), type, action), _text(std::move(text)) {
}

FilterAction StringCompareFilterBase::Compare(const std::wstring& str) const {
	auto compare = GetCompareType();
	switch (compare) {
		case CompareType::Equals:
		case CompareType::NotEqual:
		{
			auto equal = ::_wcsicmp(str.c_str(), _text.c_str()) == 0;
			if (compare == CompareType::Equals && equal)
				return GetAction();
			if (compare == CompareType::NotEqual && !equal)
				return GetAction();
		}
		break;

		case CompareType::Contains:
		case CompareType::NotContains:
		{
			auto text2(_text);
			::_wcslwr_s(text2.data(), text2.size() + 1);
			auto str2(str);
			::_wcslwr_s(str2.data(), str2.size() + 1);

			auto substr = str2.find(text2) != std::wstring::npos;
			if (compare == CompareType::Contains && substr)
				return GetAction();
			if (compare == CompareType::NotContains && !substr)
				return GetAction();
		}
		break;

		default:
			return FilterAction::None;
	}
	if (GetAction() == FilterAction::Include)
		return FilterAction::Exclude;

	return GetDefaultAction();
}

bool StringCompareFilterBase::InitFromParams(const std::wstring& params) {
	if (params.empty())
		return false;

	_text = params;

	return true;
}

std::wstring StringCompareFilterBase::GetParams() {
	return _text;
}

