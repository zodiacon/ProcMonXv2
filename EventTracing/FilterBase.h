#pragma once

class EventData;

enum class CompareType {
	Equals,
	NotEqual,
	Contains,
	NotContains,
	GreaterThan,
	LessThan,
};

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
	FilterBase(std::wstring name, CompareType compare, FilterAction action = FilterAction::None);
	~FilterBase();

	virtual FilterAction Eval(FilterContext& context) const = 0;

	const std::wstring& GetName() const;
	void Enable(bool enable);
	bool IsEnabled() const;

	FilterAction GetAction() const;
	void SetAction(FilterAction action);

	CompareType GetCompareType() const;
	void SetCompareType(CompareType compare);

	static FilterAction GetDefaultAction();
	static void SetDefaultAction(FilterAction action);

	virtual bool InitFromParams(const std::wstring& params) = 0;
	virtual std::wstring GetParams() = 0;

private:
	FilterAction _action;
	CompareType _compare;
	inline static FilterAction _defaultAction = FilterAction::None;
	std::wstring _name;
	bool _enabled{ true };
};

