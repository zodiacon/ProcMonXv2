#include "pch.h"
#include "FilterBase.h"

FilterBase::FilterBase(std::wstring name, CompareType compare, FilterAction action) : _name(std::move(name)), _compare(compare), _action(action) {
}

FilterBase::~FilterBase() = default;

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

CompareType FilterBase::GetCompareType() const {
	return _compare;
}

void FilterBase::SetCompareType(CompareType compare) {
	_compare = compare;
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
