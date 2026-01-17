#pragma once

#include <string_view>
#include <array>
#include <algorithm>

namespace Carp
{

namespace detail
{
consteval auto GetCompileTime()
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

template <std::size_t E_SIZE, std::size_t T_SIZE, std::size_t N>
consteval auto ConcatEngineNameWithBuildTime(std::string_view engine_name, std::string_view build_time, const char(&link)[N])
{
    std::array<char, E_SIZE + T_SIZE + N> res{};
    std::copy(engine_name.begin(), engine_name.end(), res.begin());
    std::copy(std::begin(link), std::end(link), res.begin() + E_SIZE);
    std::copy(build_time.begin(), build_time.end(), res.begin() + E_SIZE + N);
    return res;
}

constexpr auto _BUILD_TIME_ARR = GetCompileTime();

constexpr std::string_view ENGINE_NAME = "Carp";
constexpr std::string_view BUILD_TIME = std::string_view{_BUILD_TIME_ARR.data(), _BUILD_TIME_ARR.size() - 1};
constexpr std::string_view AUTHOR_NAME = "FerociousMagikarp";

constexpr auto _ENGINE_NAME_WITH_BUILD_TIME = ConcatEngineNameWithBuildTime<ENGINE_NAME.size(), BUILD_TIME.size()>(ENGINE_NAME, BUILD_TIME, " built on ");
constexpr std::string_view ENGINE_NAME_WITH_BUILD_TIME = std::string_view{_ENGINE_NAME_WITH_BUILD_TIME.data(), _ENGINE_NAME_WITH_BUILD_TIME.size()};

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

	consteval static std::string_view GetEngineName() noexcept { return detail::ENGINE_NAME_WITH_BUILD_TIME; }
	consteval static std::string_view GetAuthorName() noexcept { return detail::AUTHOR_NAME; }
};

} // namespace Carp
