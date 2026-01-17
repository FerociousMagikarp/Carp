#include "uci_command.h"
#include <functional>
#include <algorithm>
#include <numeric>
#include "option.h"
#include "core/engine.h"

namespace Carp
{
UciCommand::UciCommand(Engine& engine, OptionContainer& option_cont) :
	m_engine(engine),
	m_option_container(option_cont),
	m_commands{
		std::make_pair("setoption", &UciCommand::C_SetOption),
		std::make_pair("isready", &UciCommand::C_IsReady),
		std::make_pair("position", &UciCommand::C_Position),
		std::make_pair("go", &UciCommand::C_Go),
		std::make_pair("stop", &UciCommand::C_Stop),
		std::make_pair("ponderhit", &UciCommand::C_PonderHit),
	} {}

UciCommand::~UciCommand() = default;

std::string UciCommand::AnalyzeCommand(std::span<std::string_view> commands)
{
	auto iter = m_commands.find(commands.front());
	if (iter == m_commands.end())
		return "No such Command. Please check.";
	return std::invoke(iter->second, this, commands);
}

static Option& GetOption(OptionContainer& cont, std::span<std::string_view> name)
{
	// name是空的没机会走到这
	if (name.size() == 1)
		return cont[name.front()];
	// 如果名字有多个部分需要把名字用空格连接起来
	std::string final_name;
	std::size_t s_size = std::accumulate(name.begin(), name.end(), std::size_t{ 0 },
		[](std::size_t sum, std::string_view s) { return sum + s.size(); });
	final_name.reserve(s_size + name.size() - 1);
	final_name += name.front();
	for (std::size_t i = 1; i < name.size(); i++)
	{
		final_name += " ";
		final_name += name[i];
	}
	return cont[final_name];
}

std::string UciCommand::C_SetOption(std::span<std::string_view> commands)
{
	constexpr std::string_view NAME_STR = "name";
	constexpr std::string_view VALUE_STR = "value";
	if (commands.size() < 3 || commands[1] != NAME_STR)
		return "Use 'setoption name <id> [value <x>]' to set option.";
	// 从name往后，直到value之前，都有可能是名字的一部分
	std::span<std::string_view> name = commands.subspan(2);
	std::span<std::string_view> value{};
	auto value_iter = std::ranges::find(name, VALUE_STR);
	if (value_iter != name.end())
	{
		auto pos = std::distance(name.begin(), value_iter);
		value = name.subspan(pos + 1);
		name = name.subspan(0, pos);
	}

	auto& option = GetOption(m_option_container, name);
	if (!value.empty())
		option.Set(value.front()); // 只用第一个字符就可以，其他舍去
	else
		option.Set();
	return "";
}

std::string UciCommand::C_IsReady([[maybe_unused]] std::span<std::string_view> commands)
{
	return "readyok";
}

std::string UciCommand::C_Position(std::span<std::string_view> commands)
{
	return "";
}

std::string UciCommand::C_Go(std::span<std::string_view> commands)
{
	return "";
}

std::string UciCommand::C_Stop(std::span<std::string_view> commands)
{
	return "";
}

std::string UciCommand::C_PonderHit(std::span<std::string_view> commands)
{
	return "";
}

class OutputOptionUci : public OutputOption
{
public:
	explicit OutputOptionUci(const Option& option) : OutputOption(option) {}
	virtual ~OutputOptionUci() {}

	void Exec(std::ostream& os, const OptionCheck& option) const override
	{
		os << "option name " << option.GetName() << " type check default " << (option.m_default ? "true" : "false");
	}

	void Exec(std::ostream& os, const OptionSpin& option) const override
	{
		os << "option name " << option.GetName() << " type spin default " << option.m_default << " min " << option.m_min << " max " << option.m_max;
	}

	void Exec(std::ostream& os, const OptionCombo& option) const override
	{
		os << "option name " << option.GetName() << " type combo default " << option.m_default;
		for (const auto& item : option.m_items)
			os << " var " << item;
	}

	void Exec(std::ostream& os, const OptionButton& option) const override
	{
		os << "option name " << option.GetName() << " type button";
	}

	void Exec(std::ostream& os, const OptionString& option) const override
	{
		os << "option name " << option.GetName() << " type spin default " << option.m_default;
	}
};

std::ostream& operator<<(std::ostream& os, const UciCommand& uci)
{
	os << "id name " << Engine::GetEngineName() << '\n';
	os << "id author " << Engine::GetAuthorName() << '\n';
	uci.m_option_container.ForeachOption([&os](const Option& option)->void {
		os << OutputOptionUci{ option } << '\n';
	});
	os << "uciok";
	return os;
}

} // namespace Carp
