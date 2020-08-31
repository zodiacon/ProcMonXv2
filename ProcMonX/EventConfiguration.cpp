#include "pch.h"
#include "EventConfiguration.h"
#include <wil\resource.h>
#include "IniFile.h"

void EventsConfiguration::AddCategory(EventConfigCategory cat) {
	_categories.push_back(std::move(cat));
}

void EventsConfiguration::Clear() {
	_categories.clear();
}

const std::vector<EventConfigCategory>& EventsConfiguration::GetCategories() const {
	return _categories;
}

const EventConfigCategory* const EventsConfiguration::GetCategory(PCWSTR name) const {
	auto it = std::find_if(_categories.begin(), _categories.end(), [&](auto& c) { return c.Name == name; });
	return it == _categories.end() ? nullptr : &(*it);
}

EventConfigCategory* EventsConfiguration::GetCategory(PCWSTR name) {
	auto it = std::find_if(_categories.begin(), _categories.end(), [&](auto& c) { return c.Name == name; });
	return it == _categories.end() ? nullptr : &(*it);
}

int EventsConfiguration::RemoveAdvanced(bool advanced) {
	auto it = std::remove_if(_categories.begin(), _categories.end(), [&](const auto& cat) { 
		return KernelEventCategory::GetCategory(cat.Name.c_str())->Advanced == advanced; 
		});
	_categories.erase(it, _categories.end());

	return 0;
}

bool EventsConfiguration::Save(PCWSTR path) {
	::DeleteFile(path);
	IniFile file(path);
	
	for (auto& cat : GetCategories()) {
		CString text;
		for (auto oc : cat.Opcodes)
			text.Format(L"%s%d,", (PCWSTR)text, oc);
		if (!file.WriteString(L"Events", cat.Name.c_str(), text))
			return false;
	}

	return true;
}

bool EventsConfiguration::Load(PCWSTR path, bool clean) {
	IniFile file(path);
	if (clean)
		_categories.clear();

	auto data = file.ReadSection(L"Events");
	for (auto& cat : data) {
		auto equal = cat.Find(L'=');
		ATLASSERT(equal >= 0);
		if (equal < 0)
			return false;
		EventConfigCategory category;
		category.Name = cat.Left(equal);
		auto events = cat.Mid(equal + 1);
		if (!events.IsEmpty()) {
			int start = 0;
			for(;;) {
				auto id = events.Tokenize(L",", start);
				if (id.IsEmpty())
					break;
				category.Opcodes.push_back(_wtoi(id));
			}
		}
		_categories.push_back(category);
	}
	return true;
}

bool EventConfigCategory::Contains(int opcode) const {
	return std::find(Opcodes.begin(), Opcodes.end(), opcode) != Opcodes.end();
}
