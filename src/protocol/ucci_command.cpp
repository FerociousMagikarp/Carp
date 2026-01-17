#include "ucci_command.h"
#include <functional>
#include <ranges>
#include "option.h"
#include "core/engine.h"

namespace Carp
{
static std::string TransToUCCIName(std::string_view sv)
{
	auto result = sv | std::views::filter([](char c) -> bool { return !std::isspace(c); })
					 | std::views::transform([](unsigned char c) -> char { return std::tolower(c); });
	return std::string{ result.begin(), result.end() };
}

UcciCommand::UcciCommand(Engine& engine, OptionContainer& option_cont) :
	m_engine(engine),
	m_option_container(option_cont),
	m_commands{
		std::make_pair("setoption", &UcciCommand::C_SetOption),
		std::make_pair("isready", &UcciCommand::C_IsReady),
		std::make_pair("position", &UcciCommand::C_Position),
		std::make_pair("banmoves", &UcciCommand::C_BanMoves),
		std::make_pair("go", &UcciCommand::C_Go),
		std::make_pair("stop", &UcciCommand::C_Stop),
		std::make_pair("ponderhit", &UcciCommand::C_PonderHit),
	}
{
	m_option_container.ForeachOption([this](const Option& option)->void {
		m_ucci_name_to_option_name[TransToUCCIName(option.GetName())] = option.GetName();
	});
}

UcciCommand::~UcciCommand() = default;

std::string UcciCommand::AnalyzeCommand(std::span<std::string_view> commands)
{
	auto iter = m_commands.find(commands.front());
	if (iter == m_commands.end())
		return "No such Command. Please check.";
	return std::invoke(iter->second, this, commands);
}

std::string UcciCommand::C_SetOption(std::span<std::string_view> commands)
{
	if (commands.size() < 2)
		return "Use 'setoption <name> [<value>]' to set option.";

	auto name_iter = m_ucci_name_to_option_name.find(std::string{ commands[1] });
	if (name_iter == m_ucci_name_to_option_name.end())
		return "Unknown option.";

	auto& option = m_option_container[name_iter->second];

	if (commands.size() >= 3)
		option.Set(commands[2]);
	else
		option.Set();
	return "";
}

std::string UcciCommand::C_IsReady([[maybe_unused]] std::span<std::string_view> commands)
{
	return "readyok";
}

std::string UcciCommand::C_Position(std::span<std::string_view> commands)
{
	return "";
}

std::string UcciCommand::C_BanMoves(std::span<std::string_view> commands)
{
	return "";
}

std::string UcciCommand::C_Go(std::span<std::string_view> commands)
{
	return "";
}

std::string UcciCommand::C_Stop(std::span<std::string_view> commands)
{
	return "";
}

std::string UcciCommand::C_PonderHit(std::span<std::string_view> commands)
{
	return "";
}

class OutputOptionUcci : public OutputOption
{
public:
	explicit OutputOptionUcci(const Option& option) : OutputOption(option) {}
	virtual ~OutputOptionUcci() {}

	void Exec(std::ostream& os, const OptionCheck& option) const override
	{
		os << "option " << TransToUCCIName(option.GetName()) << " type check default " << (option.m_default ? "true" : "false");
	}

	void Exec(std::ostream& os, const OptionSpin& option) const override
	{
		os << "option " << TransToUCCIName(option.GetName()) << " type spin min " << option.m_min << " max " << option.m_max << " default " << option.m_default;
	}

	void Exec(std::ostream& os, const OptionCombo& option) const override
	{
		os << "option " << TransToUCCIName(option.GetName()) << " type combo";
		for (const auto& item : option.m_items)
			os << " var " << item;
		os << " default " << option.m_default;
	}

	void Exec(std::ostream& os, const OptionButton& option) const override
	{
		os << "option " << TransToUCCIName(option.GetName()) << " type button";
	}

	void Exec(std::ostream& os, const OptionString& option) const override
	{
		os << "option " << TransToUCCIName(option.GetName()) << " type spin default " << option.m_default;
	}
};

std::ostream& operator<<(std::ostream& os, const UcciCommand& ucci)
{
	os << "id name " << Engine::GetEngineName() << '\n';
	os << "id author " << Engine::GetAuthorName() << '\n';
	ucci.m_option_container.ForeachOption([&os](const Option& option)->void {
		os << OutputOptionUcci{ option } << '\n';
	});
	os << "ucciok";
	return os;
}

} // namespace Carp
