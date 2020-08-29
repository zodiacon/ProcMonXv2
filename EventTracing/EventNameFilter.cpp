#include "pch.h"
#include "EventNameFilter.h"
#include "EventData.h"

EventNameFilter::EventNameFilter(std::wstring name, CompareType type, FilterAction action) :
    StringCompareFilterBase(L"Event Name", name, type, action) {
}

FilterAction EventNameFilter::Eval(FilterContext& context) const {
    return Compare(context.Data->GetEventName());
}
