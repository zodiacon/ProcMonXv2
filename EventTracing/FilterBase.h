#pragma once

class EventData;

struct FilterContext {
	EventData* Data;
};

enum class FilterAction {
	None,
	Include,
	Exclude,
};

class FilterBase abstract {
public:
	FilterBase(PCWSTR name = L"", FilterAction action = FilterAction::None);
	~FilterBase();

	virtual FilterAction Eval(FilterContext& context) = 0;

	void SetName(PCWSTR name);
	const std::wstring& GetName() const;
	void Enable(bool enable);
	bool IsEnabled() const;
	void SetAction(FilterAction action);
	FilterAction GetAction() const;
	static FilterAction GetDefaultAction();
	static void SetDefaultAction(FilterAction action);

private:
	FilterAction _action;
	inline static FilterAction _defaultAction = FilterAction::Exclude;
	std::wstring _name;
	bool _enabled{ true };
};

