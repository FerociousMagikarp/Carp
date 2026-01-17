#include "option.h"
#include <algorithm>
#include <charconv>

namespace Carp
{

std::size_t CaseInsensitiveHash::operator()(std::string_view s) const noexcept
{
	// 就用简单的 FNV1a 哈希就可以
	constexpr std::size_t OFFSET = std::conditional_t<
		sizeof(std::size_t) == 8,
		std::integral_constant<std::size_t, 14695981039346656037ULL>,
		std::integral_constant<std::size_t, 2166136261U>
	>::value;
	constexpr std::size_t PRIME = std::conditional_t<
		sizeof(std::size_t) == 8,
		std::integral_constant<std::size_t, 1099511628211ULL>,
		std::integral_constant<std::size_t, 16777619U>
	>::value;

	std::size_t val = OFFSET;
	for (unsigned char c : s)
	{
		val ^= static_cast<std::size_t>(std::tolower(c));
		val *= PRIME;
	}

	return std::size_t();
}

bool CaseInsensitiveEqual::operator()(std::string_view left, std::string_view right) const noexcept
{
	if (left.size() != right.size())
		return false;
	return std::ranges::equal(left, right, [](unsigned char c_left, unsigned char c_right) -> bool {
			return std::tolower(c_left) == std::tolower(c_right);
		});
	return false;
}

void Option::OnChanged() const noexcept
{
	if (m_on_change)
		m_on_change(*this);
}

void OptionCheck::Output(std::ostream& os, const OutputOption& output) const
{
	output.Exec(os, *this);
}

void OptionCheck::Set(bool val)
{
	m_value = val;
	OnChanged();
}

void OptionCheck::Set(std::string_view val)
{
	constexpr std::string_view TRUE_STR = "true";
	constexpr std::string_view FALSE_STR = "false";
	if (CaseInsensitiveEqual{}(val, TRUE_STR))
		Set(true);
	else if (CaseInsensitiveEqual{}(val, FALSE_STR))
		Set(false);
}

void OptionSpin::Output(std::ostream& os, const OutputOption& output) const
{
	output.Exec(os, *this);
}

void OptionSpin::Set(int val)
{
	if (val < m_min || val > m_max)
		return;
	m_value = val;
	OnChanged();
}

void OptionSpin::Set(std::string_view val)
{
	int value;
	auto [ptr, ec] = std::from_chars(val.data(), val.data() + val.size(), value);
	if (ec == std::errc{})
		Set(value);
}

void OptionCombo::Output(std::ostream& os, const OutputOption& output) const
{
	output.Exec(os, *this);
}

void OptionCombo::Set(std::string_view val)
{
	auto iter = m_item_set.find(val);
	if (iter == m_item_set.cend())
		return;

	m_value = val;
	OnChanged();
}

void OptionButton::Output(std::ostream& os, const OutputOption& output) const
{
	output.Exec(os, *this);
}

void OptionButton::Set()
{
	OnChanged();
}

void OptionButton::Set([[maybe_unused]] std::string_view)
{
	OnChanged();
}

void OptionString::Output(std::ostream& os, const OutputOption& output) const
{
	output.Exec(os, *this);
}

void OptionString::Set(std::string_view val)
{
	m_value = val;
	OnChanged();
}

class OptionEmpty final : public Option
{
public:
	OptionEmpty() : Option("") {}
	virtual ~OptionEmpty() {}

	void Output(std::ostream& os, const OutputOption&) const { }
};

static OptionEmpty OPTION_EMPTY;

const Option& OptionContainer::operator[](std::string_view name) const
{
	auto iter = m_option_map.find(name);
	if (iter == m_option_map.end())
		return OPTION_EMPTY;
	return *iter->second;
}

Option& OptionContainer::operator[](std::string_view name)
{
	auto iter = m_option_map.find(name);
	if (iter == m_option_map.end())
		return OPTION_EMPTY;
	return *iter->second;
}

void OptionContainer::AddOption(std::unique_ptr<Option> option)
{
	m_option_map.insert(std::make_pair(option->GetName(), option.get()));
	m_options.push_back(std::move(option));
}

} // namespace Carp
