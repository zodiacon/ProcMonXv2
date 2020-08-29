#pragma once

#include "FilterBase.h"

struct FilterDescription {
	std::wstring Name;
	FilterAction Action{ FilterAction::None };
	FilterAction DefaultAction{ FilterAction::None };
	CompareType Compare{ CompareType::Equals };
	std::wstring Parameters;
	bool Enabled{ true };
};

class FilterConfiguration {
public:
	bool AddFilter(FilterDescription desc);
	int GetFilterCount() const;
	bool SwapFilters(int index1, int index2);
	bool RemoveFilter(int index);
	FilterDescription* GetFilter(int index);
	const FilterDescription* GetFilter(int index) const;

	void Clear();

	bool Save(PCWSTR path);
	bool Load(PCWSTR path);

private:
	std::vector<FilterDescription> _filters;
};

