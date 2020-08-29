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
			auto pname2 = _text;
			::_wcslwr_s(pname2.data(), pname2.size());
			auto substr = ::wcsstr(pname2.c_str(), _text.c_str()) != nullptr;
			if (compare == CompareType::Contains && substr)
				return GetAction();
			if (compare == CompareType::NotContains && !substr)
				return GetAction();
		}
		break;

	}
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

