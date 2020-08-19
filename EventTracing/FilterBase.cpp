#include "pch.h"
#include "FilterBase.h"

FilterBase::FilterBase(PCWSTR name) : _name(name) {
}

void FilterBase::SetName(PCWSTR name) {
	_name = name;
}

const std::wstring& FilterBase::GetName() const {
	return _name;
}
