#pragma once

#include "FilterBase.h"

struct FilterFactory {
	static std::vector<CString> GetFilterNames();
	static std::shared_ptr<FilterBase> CreateFilter(PCWSTR name, CompareType compare, PCWSTR params, FilterAction action);
};

