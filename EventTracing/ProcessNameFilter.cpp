#include "pch.h"
#include "ProcessNameFilter.h"
#include "EventData.h"

FilterAction ProcessNameFilter::Eval(FilterContext& context) {
	auto& pname = context.Data->GetProcessName();

	switch (_type) {
		case CompareType::Equals:
		case CompareType::NotEqual:
		{
			auto equal = ::_wcsicmp(pname.c_str(), _name.c_str()) == 0;
			if (_type == CompareType::Equals && equal)
				return GetAction();
			if (_type == CompareType::NotEqual && !equal)
				return GetAction();
		}
		break;

		case CompareType::Contains:
		case CompareType::NotContains:
		{
			auto pname2 = pname;
			::_wcslwr_s(pname2.data(), pname2.size());
			auto substr = ::wcsstr(pname2.c_str(), _name.c_str()) != nullptr;
			if (_type == CompareType::Contains && substr)
				return GetAction();
			if (_type == CompareType::NotContains && !substr)
				return GetAction();
		}
		break;

	}
	return FilterAction::None;
}
