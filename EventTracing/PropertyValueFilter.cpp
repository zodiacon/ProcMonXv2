#include "pch.h"
#include "PropertyValueFilter.h"
#include "EventData.h"

PropertyValueFilter::PropertyValueFilter(std::wstring name, CompareType type, FilterAction action) 
: StringCompareFilterBase(L"Property Value", name, type, action){
}

FilterAction PropertyValueFilter::Eval(FilterContext& context) const {
    auto action = FilterAction::None;
    for (auto& prop : context.Data->GetProperties()) {
        action = Compare(context.Data->FormatProperty(prop));
        if (action == GetAction())
            return action;
    }
    return action;
}
