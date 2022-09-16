#include "pch.h"
#include "FilterConfiguration.h"
#include "IniFile.h"

bool FilterConfiguration::AddFilter(FilterDescription desc) {
    _filters.push_back(std::move(desc));
    return true;
}

int FilterConfiguration::GetFilterCount() const {
    return (int)_filters.size();
}

bool FilterConfiguration::SwapFilters(int i1, int i2) {
    if (i1 < 0 || i2 < 0 || i1 >= _filters.size() || i2 >= _filters.size() || i1 == i2)
        return false;

    std::swap(_filters[i1], _filters[i2]);
    return true;
}

bool FilterConfiguration::RemoveFilter(int index) {
    if (index < 0 || index >= _filters.size())
        return false;
    _filters.erase(_filters.begin() + index);
    return true;
}

FilterDescription* FilterConfiguration::GetFilter(int index) {
    if (index < 0 || index >= _filters.size())
        return nullptr;

    return _filters.data() + index;
}

const FilterDescription* FilterConfiguration::GetFilter(int index) const {
    if (index < 0 || index >= _filters.size())
        return nullptr;

    return _filters.data() + index;
}

void FilterConfiguration::Clear() {
    _filters.clear();
}

bool FilterConfiguration::Save(PCWSTR path) {
    IniFile file(path);
    CString text;

    int i = 1;
    for (auto& filter : _filters) {
        text.Format(L"Filter%d", i);
        file.WriteString(text, L"Type", filter.Name.c_str());
        file.WriteInt(text, L"Action", (int)filter.Action);
        file.WriteInt(text, L"DefaultAction", (int)filter.Action);
        file.WriteBool(text, L"Enabled", filter.Enabled);
        file.WriteString(text, L"Parameters", filter.Parameters.c_str());
        i++;
    }

    return true;
}

bool FilterConfiguration::Load(PCWSTR path) {
    IniFile file(path);
    CString text;
    for (int i = 1;; i++) {
        text.Format(L"Filter%d", i);
        auto name = file.ReadString(text, L"Type");
        if (name.IsEmpty() || name == L"")
            break;
        FilterDescription desc;
        desc.Name = name;
        desc.Action = (FilterAction)file.ReadInt(text, L"Action");
        desc.DefaultAction = (FilterAction)file.ReadInt(text, L"DefaultAction");
        desc.Enabled = file.ReadBool(text, L"Enabled", true);
        desc.Parameters = file.ReadString(text, L"Parameters");
        AddFilter(desc);
    }
    return true;
}
