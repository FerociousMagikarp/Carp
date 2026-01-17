#pragma once

#include <memory>
#include <iostream>
#include <functional>
#include <concepts>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace Carp
{

struct CaseInsensitiveHash
{
	std::size_t operator()(std::string_view s) const noexcept;
};

struct CaseInsensitiveEqual
{
	bool operator()(std::string_view left, std::string_view right) const noexcept;
};

class OutputOption;
class Option;

template <typename F>
concept OnOptionChangedFunc = requires(F f, const Option & option) { { f(option) } -> std::same_as<void>; };

class Option
{
protected:
	const std::string m_name;
	std::function<void(const Option&)> m_on_change;

public:
	explicit Option(std::string name) noexcept : m_name(std::move(name)), m_on_change(nullptr) {}
	template <OnOptionChangedFunc F>
	Option(std::string name, F&& f) noexcept : m_name(std::move(name)), m_on_change(std::forward<F>(f)) {}
	virtual ~Option() {}

	virtual void Output(std::ostream&, const OutputOption&) const = 0;

	virtual void Set() {}
	// 通过字符串转成对应需要的类型
	virtual void Set(std::string_view) {}

	std::string_view GetName() const noexcept { return m_name; }
	void OnChanged() const noexcept;
};

class OptionCheck final : public Option
{
private:
	bool m_value;
public:
	const bool m_default;

	OptionCheck(std::string name, bool default_value) noexcept :
		Option(std::move(name)), m_value(default_value), m_default(default_value) {}
	template <OnOptionChangedFunc F>
	OptionCheck(std::string name, bool default_value, F&& f) noexcept :
		Option(std::move(name), std::forward<F>(f)), m_value(default_value), m_default(default_value) {}
	virtual ~OptionCheck() {}

	void Output(std::ostream&, const OutputOption&) const override;
	void Set(bool val);
	void Set(std::string_view val) override;
	bool Get() const noexcept { return m_value; }
};

class OptionSpin final : public Option
{
private:
	int m_value;
public:
	const int m_min;
	const int m_max;
	const int m_default;

	OptionSpin(std::string name, int default_value, int min_value, int max_value) noexcept :
		Option(std::move(name)),
		m_value(default_value),
		m_min(min_value),
		m_max(max_value),
		m_default(default_value) {}
	template <OnOptionChangedFunc F>
	OptionSpin(std::string name, int default_value, int min_value, int max_value, F&& f) noexcept :
		Option(std::move(name), std::forward<F>(f)),
		m_value(default_value),
		m_min(min_value),
		m_max(max_value),
		m_default(default_value) {}
	virtual ~OptionSpin() {}

	void Output(std::ostream&, const OutputOption&) const override;
	void Set(int val);
	void Set(std::string_view val) override;
	int Get() const noexcept { return m_value; }
};

class OptionCombo final : public Option
{
private:
	std::string m_value;
public:
	const std::string m_default;
	const std::vector<std::string> m_items;
	const std::unordered_set<std::string_view> m_item_set;

	OptionCombo(std::string name, std::string_view default_value, std::vector<std::string> items) noexcept :
		Option(std::move(name)),
		m_value(default_value),
		m_default(default_value),
		m_items(std::move(items)),
		m_item_set(m_items.begin(), m_items.end()) {}
	template <OnOptionChangedFunc F>
	OptionCombo(std::string name, std::string_view default_value, std::vector<std::string> items, F&& f) noexcept :
		Option(std::move(name), std::forward<F>(f)),
		m_value(default_value),
		m_default(default_value),
		m_items(std::move(items)),
		m_item_set(m_items.begin(), m_items.end()) {}
	virtual ~OptionCombo() {}

	void Output(std::ostream&, const OutputOption&) const override;
	void Set(std::string_view val) override;
	std::string_view Get() const noexcept { return m_value; }
};

class OptionButton final : public Option
{
public:
	template <OnOptionChangedFunc F>
	OptionButton(std::string name, F&& f) noexcept :
		Option(std::move(name), std::forward<F>(f)) {}
	virtual ~OptionButton() {}

	void Output(std::ostream&, const OutputOption&) const override;
	void Set() override;
	void Set(std::string_view) override;
};

class OptionString final : public Option
{
private:
	std::string m_value;
public:
	const std::string m_default;

	OptionString(std::string name, std::string_view default_value) noexcept :
		Option(std::move(name)),
		m_value(default_value),
		m_default(default_value) {}
	template <OnOptionChangedFunc F>
	OptionString(std::string name, std::string_view default_value, F&& f) noexcept :
		Option(std::move(name), std::forward<F>(f)),
		m_value(default_value),
		m_default(default_value) {}
	virtual ~OptionString() {}

	void Output(std::ostream&, const OutputOption&) const override;
	void Set(std::string_view val) override;
	std::string_view Get() const noexcept { return m_value; }
};

class OutputOption
{
private:
	const Option& m_option;
public:
	explicit OutputOption(const Option& option) : m_option(option) {}
	virtual ~OutputOption() {}
	// 子类实现不同的输出函数
	virtual void Exec(std::ostream&, const OptionCheck&) const = 0;
	virtual void Exec(std::ostream&, const OptionSpin&) const = 0;
	virtual void Exec(std::ostream&, const OptionCombo&) const = 0;
	virtual void Exec(std::ostream&, const OptionButton&) const = 0;
	virtual void Exec(std::ostream&, const OptionString&) const = 0;

	friend std::ostream& operator<<(std::ostream& os, const OutputOption& output)
	{
		output.m_option.Output(os, output);
		return os;
	}
};

class OptionContainer
{
private:
	std::vector<std::unique_ptr<Option>> m_options;
	std::unordered_map<std::string_view, Option*, CaseInsensitiveHash, CaseInsensitiveEqual> m_option_map;

public:
	OptionContainer() = default;
	~OptionContainer() = default;
	OptionContainer(const OptionContainer&) = delete;
	OptionContainer(OptionContainer&&) = delete;
	OptionContainer& operator=(const OptionContainer&) = delete;
	OptionContainer& operator=(OptionContainer&&) = delete;

	const Option& operator[] (std::string_view name) const;
	Option& operator[] (std::string_view name);

	template <typename T, typename... Args>
		requires (std::derived_from<T, Option> && std::constructible_from<T, Args...>)
	void AddOption(Args&&... args)
	{
		AddOption(std::make_unique<T>(std::forward<Args>(args)...));
	}

	template <typename F>
		requires requires(F f, const Option& op) { { f(op) } -> std::same_as<void>; }
	void ForeachOption(F&& f)
	{
		for (const auto& option : m_options)
			f(*option);
	}

private:
	void AddOption(std::unique_ptr<Option> option);

	friend class Engine;
};

} // namespace Carp
