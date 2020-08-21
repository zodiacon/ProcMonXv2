#include "pch.h"
#include "FilterBase.h"

FilterBase::FilterBase(PCWSTR name, FilterAction action) : _name(name), _action(action) {
}

FilterBase::~FilterBase() = default;

void FilterBase::SetName(PCWSTR name) {
	_name = name;
}

const std::wstring& FilterBase::GetName() const {
	return _name;
}

void FilterBase::Enable(bool enable) {
	_enabled = enable;
}

bool FilterBase::IsEnabled() const {
	return _enabled;
}

void FilterBase::SetAction(FilterAction action) {
	_action = action;
}

FilterAction FilterBase::GetAction() const {
	return _action;
}

FilterAction FilterBase::GetDefaultAction() {
	return _defaultAction;
}

void FilterBase::SetDefaultAction(FilterAction action) {
	_defaultAction = action;
}
