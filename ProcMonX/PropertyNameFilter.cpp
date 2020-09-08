#include "pch.h"
#include "EventData.h"
#include "PropertyNameFilter.h"

PropertyNameFilter::PropertyNameFilter(std::wstring name, CompareType type, FilterAction action)
: StringCompareFilterBase(L"Property Value", name, type, action) {
}

FilterAction PropertyNameFilter::Eval(FilterContext& context) const {
    auto action = FilterAction::None;
    for (auto& prop : context.Data->GetProperties()) {
        action = Compare(prop.Name);
        if (action == GetAction())
            return action;
    }
    return action;
}
