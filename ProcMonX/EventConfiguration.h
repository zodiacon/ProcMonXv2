#pragma once

#include "KernelEvents.h"

struct EventConfigCategory {
	std::wstring Name;
	std::vector<int> Opcodes;

	bool Contains(int opcode) const;
};

class EventsConfiguration {
public:
	void AddCategory(EventConfigCategory cat);
	void Clear();
	const std::vector<EventConfigCategory>& GetCategories() const;
	const EventConfigCategory* const GetCategory(PCWSTR name) const;
	EventConfigCategory* GetCategory(PCWSTR name);

	bool Save(PCWSTR path);
	bool Load(PCWSTR path, bool clean = true);

private:
	std::vector<EventConfigCategory> _categories;
};

