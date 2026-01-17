#pragma once

#include <string_view>
#include <array>

namespace Carp
{

namespace detail
{
consteval std::array<char, 11> GetCompileTime()
{
    std::string_view date{ __DATE__ };
    std::array<char, 11> res{ "0000-00-00" };
    // 年
    std::copy(date.begin() + date.rfind(' ') + 1, date.end(), res.begin());
    // 月
    std::string_view months = "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec";
    auto month = months.find(date.substr(0, 3)) / 4 + 1;
    res[5] = static_cast<char>(month / 10) + '0';
    res[6] = static_cast<char>(month % 10) + '0';
    // 日
    auto day_pos = date.find_first_not_of(' ', 4);
    if (date[day_pos + 1] == ' ')
    {
        res[9] = date[day_pos];
    }
    else
    {
        res[8] = date[day_pos];
        res[9] = date[day_pos + 1];
    }
    return res;
}

constexpr auto _COMPILE_TIME_ARR = GetCompileTime();

constexpr std::string_view ENGINE_NAME = "Carp";
constexpr std::string_view COMPILE_TIME = std::string_view{_COMPILE_TIME_ARR.data(), _COMPILE_TIME_ARR.size()};
constexpr std::string_view AUTHOR_NAME = "FerociousMagikarp";

} // namespace detail

class OptionContainer;

class Engine
{
public:
	Engine() = default;
	~Engine() = default;
	Engine(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine& operator=(Engine&&) = delete;

	void InitOptions(OptionContainer& container) const;

	consteval static std::string_view GetEngineName() noexcept { return detail::ENGINE_NAME; }
	consteval static std::string_view GetAuthorName() noexcept { return detail::AUTHOR_NAME; }
};

} // namespace Carp
