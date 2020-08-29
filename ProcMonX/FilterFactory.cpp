#include "pch.h"
#include "FilterFactory.h"
#include "EventNameFilter.h"
#include "ProcessIdFilter.h"
#include "ProcessNameFilter.h"
#include "PropertyValueFilter.h"

static PCWSTR names[] = {
	L"Process Name", L"Process Id", L"Event Name", L"Property Value",
};

std::vector<CString> FilterFactory::GetFilterNames() {
	return std::vector<CString>(std::begin(names), std::end(names));
}

std::shared_ptr<FilterBase> FilterFactory::CreateFilter(PCWSTR name, CompareType compare, PCWSTR params, FilterAction action) {
	std::shared_ptr<FilterBase> filter;

	for (int i = 0; i < _countof(names); i++) {
		if (::wcscmp(name, names[i]) == 0) {
			switch (i) {
				case 0:
					filter = std::make_shared<ProcessNameFilter>(params, compare, action);
					break;

				case 1:
					filter = std::make_shared<ProcessIdFilter>(_wtoi(params), compare, action);
					break;

				case 2:
					filter = std::make_shared<EventNameFilter>(params, compare, action);
					break;

				case 3:
					filter = std::make_shared<PropertyValueFilter>(params, compare, action);
					break;
			}
		}
	}
	return filter;
}
