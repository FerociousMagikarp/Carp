#include "controller.h"
#include <variant>
#include <iostream>
#include <string_view>
#include <optional>
#include <vector>
#include "protocol/uci_command.h"
#include "protocol/ucci_command.h"
#include "protocol/option.h"
#include "core/engine.h"
#include "utils/osyncstream.h"

namespace Carp
{
enum class ProtocolType
{
	uci,
	ucci,
};

constexpr std::string_view QUIT_COMMAND = "quit";
constexpr std::string_view UCI_COMMAND = "uci";
constexpr std::string_view UCCI_COMMAND = "ucci";

// 去除前后的空格和换行
static std::string_view Trim(std::string_view value) noexcept
{
	const auto start = value.find_first_not_of(" \t\r\n");
	if (start == std::string_view::npos)
		return "";

	const auto end = value.find_last_not_of(" \t\r\n");
	return value.substr(start, end - start + 1);
}

// 按照空格和制表符来分割
static std::vector<std::string_view> Split(std::string_view value) noexcept
{
	std::vector<std::string_view> res;
	while (!value.empty())
	{
		auto pos = value.find_first_of(" \t");
		auto val = Trim(value.substr(0, pos));
		if (!val.empty())
			res.push_back(val);
		if (pos != std::string_view::npos)
			value = value.substr(pos + 1);
		else
			break;
	}
	return res;
}

static std::optional<ProtocolType> CheckProtocol(std::string_view cmd) noexcept
{
	if (cmd == UCI_COMMAND)
		return ProtocolType::uci;
	else if (cmd == UCCI_COMMAND)
		return ProtocolType::ucci;
	return std::nullopt;
}

using command_t = std::variant<std::monostate, UciCommand, UcciCommand>;

class Command
{
public:
	Command(ProtocolType type, Engine& engine, OptionContainer& cont) : m_type(type)
	{
		switch (type)
		{
		case ProtocolType::uci:
			m_command.emplace<UciCommand>(engine, cont);
			break;
		case ProtocolType::ucci:
			m_command.emplace<UcciCommand>(engine, cont);
			break;
		}
	}

	friend std::ostream& operator<< (std::ostream& os, const Command& command);
	bool IsSameType(ProtocolType type) const noexcept { return m_type == type; }
	std::string AnalyzeCommand(std::span<std::string_view> command);

private:
	command_t m_command;
	ProtocolType m_type;
};

std::string Command::AnalyzeCommand(std::span<std::string_view> command)
{
	return std::visit([command](auto&& val)->std::string {
		using type = std::decay_t<decltype(val)>;
		if constexpr (!std::is_same_v<type, std::monostate>)
			return val.AnalyzeCommand(command);
		else
			return "";
	}, m_command);
}

Controller::Controller() :
	m_engine(std::make_unique<Engine>()),
	m_option_container(std::make_unique<OptionContainer>())
{
	m_engine->InitOptions(*m_option_container);
}

Controller::~Controller() = default;

void Controller::Loop()
{
	std::string cmd_str;

	// 先把引擎名和作者打出来
	std::cout << Engine::GetEngineName() << " by " << Engine::GetAuthorName() << std::endl;
	while (std::getline(std::cin, cmd_str))
	{
		std::string_view cmd = Trim(cmd_str);
		// 先检查是不是退出命令
		if (cmd == QUIT_COMMAND)
			break;
		// 再检查是不是修改协议的命令
		auto protocol_type = CheckProtocol(cmd);
		// 如果有协议类型就走切换协议的逻辑，否则就走解析命令的逻辑
		if (protocol_type.has_value())
		{
			// 如果协议类型不同就切换新的协议
			if (m_command == nullptr || !m_command->IsSameType(*protocol_type))
				m_command = std::make_unique<Command>(*protocol_type, *m_engine, *m_option_container);
			OSyncStream{ std::cout } << *m_command << std::endl;
		}
		else
		{
			// 如果没指定协议就忽略
			if (m_command == nullptr)
				continue;
			auto split_cmd = Split(cmd);
			if (split_cmd.empty())
				continue;
			auto result = m_command->AnalyzeCommand(split_cmd);
			if (!result.empty())
				OSyncStream{ std::cout } << result << std::endl;
		}
	}
}

std::ostream& operator<<(std::ostream& os, const Command& command)
{
	std::visit([&os](auto&& val)->void {
		using type = std::decay_t<decltype(val)>;
		if constexpr (!std::is_same_v<type, std::monostate>)
			os << val;
	}, command.m_command);
	return os;
}

} // namespace Carp
